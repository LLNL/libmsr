/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

// Necessary for pread & pwrite.
#define _XOPEN_SOURCE 500

#include <errno.h>
#include <fcntl.h>
#include <hwloc.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "msr_core.h"
#include "memhdlr.h"
#include "msr_counters.h"
#include "cpuid.h"
#include "libmsr_error.h"
#include "libmsr_debug.h"

static struct topo g_topo_list;

/// @brief Retrieve unique index of a logical processor.
///
/// For a dual socket system, maps cores on socket 1 to a continuous index
/// based on socket 0. For example, if socket 0 has cores 0-12, then core 0 on
/// socket 1 would be mapped to index 13, core 1 on socket would be mapped to
/// index 14, and so on.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] thread Unique thread identifier.
///
/// @return Number of logical processors, else -1.
static uint64_t devidx(int socket, int core, int thread)
{
    uint64_t num_sockets, num_cores_per_socket, num_threads_per_core;

    core_config(&num_cores_per_socket, &num_threads_per_core, &num_sockets, NULL);
    if (!g_topo_list.discontinuous_mapping)
    {
        // continuous mapping
        return (thread * num_sockets * num_cores_per_socket) + (socket * num_cores_per_socket) + core;
    }
    else
    {
        // discontinuous mapping
        return (thread * num_sockets * num_cores_per_socket) + (core * num_sockets) + socket;
    }
    return -1;
}

/// @brief Retrieve file descriptor per logical processor.
///
/// @param [in] dev_idx Unique logical processor identifier.
///
/// @return Unique file descriptor, else NULL.
static int *core_fd(const int dev_idx)
{
    static int init = 0;
    static int *file_descriptors = NULL;
    static uint64_t devices = 0;

    if (!init)
    {
        init = 1;
        uint64_t numDevs = num_devs();;
        devices = numDevs;
        file_descriptors = (int *) libmsr_malloc(devices * sizeof(int));
    }
    if (dev_idx < devices)
    {
        return &(file_descriptors[dev_idx]);
    }
    libmsr_error_handler("core_fd(): Array reference out of bounds", LIBMSR_ERROR_ARRAY_BOUNDS, getenv("HOSTNAME"), __FILE__, __LINE__);
    return NULL;
}

/// @brief Allocate space for batch arrays.
///
/// @param [out] batchsel Storage for batch operations.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @param [out] opssize Size of specific set of batch operations.
///
/// @return 0 if successful, else NULL pointer as a result of libmsr_calloc(),
/// libmsr_malloc(), or libmsr_realloc().
static int batch_storage(struct msr_batch_array **batchsel, const int batchnum, unsigned **opssize)
{
    static struct msr_batch_array *batch = NULL;
    static unsigned arrsize = 1;
    static unsigned *size = NULL;
    int i;

    if (batch == NULL)
    {
#ifdef BATCH_DEBUG
        fprintf(stderr, "BATCH: initializing batch ops\n");
#endif
        arrsize = (batchnum + 1 > arrsize ? batchnum + 1 : arrsize);
        size = (unsigned *) libmsr_calloc(arrsize, sizeof(unsigned));
        batch = (struct msr_batch_array *) libmsr_calloc(arrsize, sizeof(struct msr_batch_array));
        for (i = 0; i < arrsize; i++)
        {
            size[i] = 0;
            batch[i].ops = NULL;
            batch[i].numops = 0;
        }
    }
    if (batchnum + 1 > arrsize)
    {
#ifdef BATCH_DEBUG
        fprintf(stderr, "BATCH: reallocating array of batches for batch %d\n", batchnum);
#endif
        unsigned oldsize = arrsize;
        arrsize = batchnum + 1;
        batch = (struct msr_batch_array *) libmsr_realloc(batch, arrsize * sizeof(struct msr_batch_array));
        size = (unsigned *) libmsr_realloc(size, arrsize * sizeof(unsigned));
        for (; oldsize < arrsize; oldsize++)
        {
            batch[oldsize].ops = NULL;
            batch[oldsize].numops = 0;
            size[oldsize] = 0;
        }
    }
    if (batchsel == NULL)
    {
        libmsr_error_handler("batch_storage(): Loading uninitialized batch", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    *batchsel = &batch[batchnum];
    if (opssize != NULL)
    {
        *opssize = &size[batchnum];
    }
    return 0;
}

/// @brief Default to pread/pwrite if msr_batch driver does not exist.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @param [in] type libmsr_batch_op_type_e type of batch operation.
///
/// @return 0 if successful, else -1 if batch_storage() fails.
static int compatibility_batch(int batchnum, int type)
{
    struct msr_batch_array *batch = NULL;
    int i;

    fprintf(stderr, "Warning: <libmsr> No /dev/cpu/msr_batch, using compatibility batch: compatibility_batch(): %s: %s:%s::%d\n", strerror(errno), getenv("HOSTNAME"), __FILE__, __LINE__);
    if (batch_storage(&batch, batchnum, NULL))
    {
        return -1;
    }
    for (i = 0; i < batch->numops; i++)
    {
        if (type == BATCH_READ)
        {
            read_msr_by_idx(batch->ops[i].cpu, batch->ops[i].msr, (uint64_t *) &batch->ops[i].msrdata);
        }
        else
        {
            write_msr_by_idx(batch->ops[i].cpu, batch->ops[i].msr, (uint64_t)batch->ops[i].msrdata);
        }
    }
    return 0;
}

/// @brief Execute read/write batch operation on a specific set of batch
/// registers.
///
/// @param [in] batchnum libmsr_data_type_e data type of batch operation.
///
/// @param [in] type libmsr_batch_op_type_e type of batch operation.
///
/// @return 0 if successful, else -1 if batch_storage() fails or if batch
/// allocation is for 0 or less operations.
static int do_batch_op(int batchnum, int type)
{
    static int batchfd = 0;
    struct msr_batch_array *batch = NULL;
    int res = 0;
    int i, j;

    if (batchfd == 0)
    {
        if ((batchfd = open(MSR_BATCH_DIR, O_RDWR)) < 0)
        {
            perror(MSR_BATCH_DIR);
            batchfd = -1;
        }
    }
#ifdef USE_NO_BATCH
    return compatibility_batch(batchnum, type);
#endif
    if (batchfd < 0)
    {
        return compatibility_batch(batchnum, type);
    }
    if (batch_storage(&batch, batchnum, NULL))
    {
        return -1;
    }
#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH %d: %s MSRs, numops %u\n", batchnum, (type == BATCH_READ ? "reading" : "writing"), batch->numops);
#endif
    if (batch->numops <= 0)
    {
        libmsr_error_handler("do_batch_op(): Using empty batch", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    /* If current flag is the opposite type, switch the flags. */
    if ((type == BATCH_WRITE && batch->ops[0].isrdmsr) || (type == BATCH_READ && !batch->ops[0].isrdmsr))
    {
        __u8 readflag = (__u8) (type == BATCH_READ ? 1 : 0);
        for (j = 0; j < batch->numops; j++)
        {
            batch->ops[j].isrdmsr = readflag;
        }
    }
    res = ioctl(batchfd, X86_IOC_MSR_BATCH, batch);
    if (res < 0)
    {
        libmsr_error_handler("do_batch_op(): IOctl failed, does /dev/cpu/msr_batch exist?", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        for (i = 0; i < batch->numops; i++)
        {
            if (batch->ops[i].err)
            {
                fprintf(stderr, "Operation failed on CPU %d, MSR 0x%x, ERR (%s)\n", batch->ops[i].cpu, batch->ops[i].msr, strerror(batch->ops[i].err));
            }
        }
    }
#ifdef BATCH_DEBUG
    int k;
    for (k = 0; k < batch->numops; k++)
    {
        fprintf(stderr, "BATCH %d: msr 0x%x cpu %u data 0x%lx (at %p)\n", batchnum, batch->ops[k].msr, batch->ops[k].cpu, (uint64_t)batch->ops[k].msrdata, &batch->ops[k].msrdata);
    }
#endif
    return res;
}

/// @brief Retrieve mapping of CPU hardware threads in a single socket.
///
/// @return 1 upon success.
int find_cpu_top(void)
{
    uint64_t cores_per_socket, nsockets, threads_per_core;
    int nthreads;
    int i;
    hwloc_topology_t topology;
    hwloc_obj_t pu_obj, core_obj, socket_obj;
    unsigned int core_depth, socket_depth;

    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);

    core_depth = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    socket_depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);

    core_config(&cores_per_socket, &threads_per_core, &nsockets, NULL);

    nthreads = cores_per_socket * threads_per_core * nsockets;
    g_topo_list.thread_map = (struct hwthread*) malloc(nthreads * sizeof(struct hwthread));

    for (i = 0; i < nthreads; i++)
    {
        pu_obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, i);
        core_obj = hwloc_get_ancestor_obj_by_depth(topology, core_depth, pu_obj);
        socket_obj = hwloc_get_ancestor_obj_by_depth(topology, socket_depth, pu_obj);

#ifdef TOPOLOGY_DEBUG
        printf("i=%2d pu_obj->os_index=%2d core_id=%2d socket_id=%2d\n", i, pu_obj->os_index, core_obj->logical_index, socket_obj->logical_index);
#endif
        g_topo_list.thread_map[pu_obj->os_index].apic_id= i;
        g_topo_list.thread_map[pu_obj->os_index].thread_id = pu_obj->os_index;
        g_topo_list.thread_map[pu_obj->os_index].core_id = core_obj->logical_index;
        g_topo_list.thread_map[pu_obj->os_index].socket_id = socket_obj->logical_index;
        g_topo_list.thread_map[pu_obj->os_index].sibling = pu_obj->sibling_rank;

        // check for continuous or discontinuous CPU ordering scheme
        if (pu_obj->os_index == 1)
        {
            if (pu_obj->os_index != g_topo_list.thread_map[pu_obj->os_index].core_id)
            {
                g_topo_list.discontinuous_mapping = 1;
            }
        }
    }

#ifdef TOPOLOGY_DEBUG
    printf("discontinuous mapping = %d\n", g_topo_list.discontinuous_mapping);
#endif

    return 1;
}

uint64_t num_cores(void)
{
    static uint64_t coresPerSocket = 0;
    static uint64_t sockets = 0;
    static int init = 0;

    if (!init)
    {
        core_config(&coresPerSocket, NULL, &sockets, NULL);
        init = 1;
    }
    return coresPerSocket * sockets;
}

uint64_t num_sockets(void)
{
    static uint64_t sockets = 0;
    static int init = 0;

    if (!init)
    {
        core_config(NULL, NULL, &sockets, NULL);
        init = 1;
    }
    return sockets;
}

uint64_t num_devs(void)
{
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    static int init = 0;

    if (!init)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        init = 1;
    }
    return coresPerSocket * threadsPerCore * sockets;
}

uint64_t cores_per_socket(void)
{
    static uint64_t coresPerSocket = 0;
    static int init = 0;

    if (!init)
    {
        core_config(&coresPerSocket, NULL, NULL, NULL);
        init = 1;
    }
    return coresPerSocket;
}

int allocate_batch(int batchnum, size_t bsize)
{
    unsigned *size = NULL;
    struct msr_batch_array *batch = NULL;
    int i;

#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: allocating batch %d\n", batchnum);
#endif
    if (batch_storage(&batch, batchnum, &size))
    {
        return -1;
    }
#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: batch %d is at %p\n", batchnum, batch);
#endif
    *size = bsize;
    if (batch->ops != NULL)
    {
        libmsr_error_handler("allocate_batch(): Conflicting batch pointers", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
#ifdef MEMHDLR_DEBUG
    fprintf(stderr, "MEMHDLR: size of batch %d is %d\n", batchnum, *size);
#endif
    batch->ops = (struct msr_batch_op *) libmsr_calloc(*size, sizeof(struct msr_batch_op));
    for (i = batch->numops; i < *size; i++)
    {
        batch->ops[i].err = 0;
    }
    return 0;
}

int free_batch(int batchnum)
{
    struct msr_batch_array *batch = NULL;
    static unsigned *size = NULL;

    if (batch == NULL)
    {
        if (batch_storage(&batch, batchnum, &size))
        {
            return -1;
        }
    }
    size[batchnum] = 0;
    libmsr_free(batch[batchnum].ops);
    return 0;
}

int create_batch_op(off_t msr, uint64_t cpu, uint64_t **dest, const int batchnum)
{
    struct msr_batch_array *batch = NULL;
    unsigned *size = NULL;

#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: creating new batch operation\n");
#endif
    if (batch_storage(&batch, batchnum, &size))
    {
        return -1;
    }

#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: batch %d is at %p\n", batchnum, batch);
#endif
    if (batch->numops > *size)
    {
        libmsr_error_handler("create_batch_op(): Batch is full, you likely used the wrong size", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    batch->numops++;
    batch->ops[batch->numops-1].msr = msr;
    batch->ops[batch->numops-1].cpu = (__u16) cpu;
    batch->ops[batch->numops-1].isrdmsr = (__u8) 1;
    batch->ops[batch->numops-1].err = 0;
    *dest = (uint64_t *) &batch->ops[batch->numops - 1].msrdata;
#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: destination of msr %lx on core %lx (at %p) is %p\n", msr, cpu, dest, &batch->ops[batch->numops - 1].msrdata);
    fprintf(stderr, "\tbatch numops is %d\n", batch->numops);
#endif
    return 0;
}

void core_config(uint64_t *coresPerSocket, uint64_t *threadsPerCore, uint64_t *sysSockets, int *HTenabled)
{
    static int init = 0;
    hwloc_topology_t topology;
    static int ncores = 0;
    static int nthreads = 0;
    static int nsockets = 0;
    static int hyperthreading = 0;

    if (!init)
    {
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: detecting core configuration\n");
#endif
        hwloc_topology_init(&topology);
        hwloc_topology_load(topology);

        nsockets = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_SOCKET);
        ncores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
        nthreads = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);

        if ((nthreads/ncores) > 1)
        {
            hyperthreading = 1;
        }
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: core config complete. cores per socket is %lu, threads per core is %lu, sockets is %lu, htenabled is %d\n", cores, threads, sockets, hyperthreading);
#endif
        init = 1;
    }
    if (coresPerSocket != NULL)
    {
        *coresPerSocket = ncores/nsockets;
    }
    if (sysSockets != NULL)
    {
        *sysSockets = nsockets;
    }
    if (HTenabled != NULL)
    {
        *HTenabled = hyperthreading;
    }
    if (threadsPerCore != NULL)
    {
        *threadsPerCore = nthreads/ncores;
    }
}

int sockets_assert(const unsigned *socket, const int location, const char *file)
{
    static uint64_t sockets = 0;

    if (!sockets)
    {
        sockets = num_sockets();
    }
    if (*socket > sockets)
    {
        libmsr_error_handler("sockets_assert(): Requested invalid socket", LIBMSR_ERROR_PLATFORM_ENV, getenv("HOSTNAME"), __FILE__, location);
        return -1;
    }
    return 0;
}

int threads_assert(const unsigned *thread, const int location, const char *file)
{
    static uint64_t threadsPerCore = 0;

    if (threadsPerCore < 1)
    {
        core_config(NULL, &threadsPerCore, NULL, NULL);
    }
    if (*thread > threadsPerCore)
    {
        libmsr_error_handler("threads_assert(): Requested invalid thread", LIBMSR_ERROR_PLATFORM_ENV, getenv("HOSTNAME"), __FILE__, location);
        return -1;
    }
    return 0;
}

int cores_assert(const unsigned *core, const int location, const char *file)
{
    static uint64_t coresPerSocket = 0;
    if (coresPerSocket < 1)
    {
        core_config(&coresPerSocket, NULL, NULL, NULL);
    }
    if (*core > coresPerSocket)
    {
        libmsr_error_handler("cores_assert(): Requested invalid core", LIBMSR_ERROR_PLATFORM_ENV, getenv("HOSTNAME"), __FILE__, location);
        return -1;
    }
    return 0;
}

int stat_module(char *filename, int *kerneltype, int *dev_idx)
{
    struct stat statbuf;

    if (*kerneltype == 3)
    {
        if (stat(filename, &statbuf))
        {
            fprintf(stderr, "Warning: <libmsr> Could not stat %s: stat_module(): %s: %s:%s::%d\n", filename, strerror(errno), getenv("HOSTNAME"), __FILE__, __LINE__);
            *kerneltype = 1;
            return -1;
        }
        if (!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR))
        {
            fprintf(stderr, "Warning: <libmsr> Incorrect permissions on msr_whitelist: stat_module(): %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            *kerneltype = 1;
            return -1;
        }
        *kerneltype = 0;
        return 0;
    }
    if (stat(filename, &statbuf))
    {
        if (*kerneltype)
        {
            libmsr_error_handler("stat_module(): Could not stat file", LIBMSR_ERROR_MSR_MODULE, getenv("HOSTNAME"), __FILE__, __LINE__);
            /* Could not find any msr module so exit. */
            return -1;
        }
        /* Could not find msr_safe module so try the msr module. */
        libmsr_error_handler("stat_module(): Could not stat file", LIBMSR_ERROR_MSR_MODULE, getenv("HOSTNAME"), __FILE__, __LINE__);
        *kerneltype = 1;
        /* Restart loading file descriptors for each device. */
        *dev_idx = -1;
        return 0;
    }
    if (!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR))
    {
        libmsr_error_handler("stat_module(): Read/write permissions denied for file", LIBMSR_ERROR_MSR_MODULE, getenv("HOSTNAME"), __FILE__, __LINE__);
        *kerneltype = 1;
        *dev_idx = -1;
        if (kerneltype != NULL)
        {
            libmsr_error_handler("stat_module(): Could not find any valid MSR module with correct permissions", LIBMSR_ERROR_MSR_MODULE, getenv("HOSTNAME"), __FILE__, __LINE__);
            /* Could not find any msr module with RW permissions, so exit. */
            return -1;
        }
    }
    return 0;
}

int init_msr(void)
{
    int dev_idx;
    int ret;
    int *fileDescriptor = NULL;
    char filename[FILENAME_SIZE];
    static int init = 0;
    int kerneltype = 3; // 0 is msr_safe, 1 is msr

    ret = find_cpu_top();
    if (ret < 0)
    {
        return ret;
    }
    uint64_t numDevs = num_devs();

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s Initializing %lu device(s).\n", getenv("HOSTNAME"), (numDevs));
#endif
    if (init)
    {
        return 0;
    }
    snprintf(filename, FILENAME_SIZE, "/dev/cpu/msr_whitelist");
    stat_module(filename, &kerneltype, 0);
    /* Open the file descriptor for each device's msr interface. */
    for (dev_idx = 0; dev_idx < numDevs; dev_idx++)
    {
        /* Use the msr_safe module, or default to the msr module. */
        if (kerneltype)
        {
            snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr", dev_idx);
        }
        else
        {
            snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr_safe", dev_idx);
        }
        if (stat_module(filename, &kerneltype, &dev_idx) < 0)
        {
            return -1;
        }
        if (dev_idx < 0)
        {
            continue;
        }
        /* Open the msr module, else return the appropriate error message. */
        fileDescriptor = core_fd(dev_idx);
        *fileDescriptor = open(filename, O_RDWR);
        if (*fileDescriptor == -1)
        {
            libmsr_error_handler("init_msr(): Could not open file", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
            if (kerneltype)
            {
                libmsr_error_handler("init_msr(): Could not open any valid MSR module", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
                /* Could not open any msr module, so exit. */
                return -1;
            }
            kerneltype = 1;
            dev_idx = -1;
        }
    }
    init = 1;
    return 0;
}

int finalize_msr(void)
{
    int dev_idx;
    int rc;
    int *fileDescriptor = NULL;
    uint64_t numDevs = num_devs();

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: finalize_msr\n");
#endif

    /* Close the file descriptors. */
    for (dev_idx = 0; dev_idx < numDevs; dev_idx++)
    {
        fileDescriptor = core_fd(dev_idx);
        if (fileDescriptor != NULL)
        {
            rc = close(*fileDescriptor);
            if (rc != 0)
            {
                libmsr_error_handler("finalize_msr(): Could not close file", LIBMSR_ERROR_MSR_CLOSE, getenv("HOSTNAME"), __FILE__, __LINE__);
                return -1;
            }
            else
            {
                *fileDescriptor = 0;
            }
        }
    }
    memhdlr_finalize();
    return 0;
}

int write_msr_by_coord(unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t val)
{
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;

    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (write_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx) val=%lu devidx=%lu\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr, val, devidx(socket, core, thread));
    return write_msr_by_idx_and_verify(devidx(socket, core, thread), msr, val);
#endif
    return write_msr_by_idx(devidx(socket, core, thread), msr, val);
}

int read_msr_by_coord(unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t *val)
{
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr);
#endif
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    if (val == NULL)
    {
        libmsr_error_handler("read_msr_by_coord(): Received NULL pointer", LIBMSR_ERROR_MSR_READ, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
    return read_msr_by_idx(devidx(socket, core, thread), msr, val);
}

int read_msr_by_coord_batch(unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t **val, int batchnum)
{
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;

#ifdef BATCH_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_msr_by_coord_batch) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr);
#endif
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef BATCH_DEBUG
    fprintf(stderr, "DEBUG: passed operation on msr 0x%lx (socket %u, core %u, thread %u) to BATCH OPS with destination %p\n", msr, socket, core, thread, val);
#endif
    create_batch_op(msr, devidx(socket, core, thread), val, batchnum);
    return 0;
}

int read_batch(const int batchnum)
{
    return do_batch_op(batchnum, BATCH_READ);
}

int write_batch(const int batchnum)
{
    return do_batch_op(batchnum, BATCH_WRITE);
}

int load_socket_batch(off_t msr, uint64_t **val, int batchnum)
{
    int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;

    if (val == NULL)
    {
        libmsr_error_handler("load_socket_batch(): Given uninitialized array", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_all_sockets) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
    fprintf(stderr, "sockets %lu, cores %lu, threads %lu\n", sockets, coresPerSocket, threadsPerCore);
#endif
    for (val_idx = 0; val_idx < sockets; val_idx++)
    {
        for (dev_idx = 0; dev_idx < NUM_DEVS; dev_idx++)
        {
            if (g_topo_list.thread_map[dev_idx].socket_id == val_idx)
            {
#ifdef TOPOLOGY_DEBUG
                printf("Creating socket batch on CPU %d at index %d\n", dev_idx, val_idx);
#endif
                create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
                break;
            }
        }
    }
    return 0;
}

int load_core_batch(off_t msr, uint64_t **val, int batchnum)
{
    int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    static uint64_t coretotal = 0;

    if (val == NULL)
    {
        libmsr_error_handler("load_core_batch(): Given uninitialized array", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        coretotal = sockets * coresPerSocket;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
#endif
    for (val_idx = 0; val_idx < coretotal; val_idx++)
    {
        for (dev_idx = 0; dev_idx < NUM_DEVS; dev_idx++)
        {
            if (g_topo_list.thread_map[dev_idx].core_id == val_idx)
            {
#ifdef TOPOLOGY_DEBUG
                printf("Creating core batch on CPU %d at index %d\n", dev_idx, val_idx);
#endif
                create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
                break;
            }
        }
    }
    return 0;
}

int load_thread_batch(off_t msr, uint64_t **val, int batchnum)
{
    int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;

    if (val == NULL)
    {
        libmsr_error_handler("load_thread_batch(): Given uninitialized array", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
#endif
    for (dev_idx = 0, val_idx = 0; dev_idx < NUM_DEVS; dev_idx++, val_idx++)
    {
#ifdef TOPOLOGY_DEBUG
        printf("Creating thread batch on CPU %d at index %d\n", dev_idx, val_idx);
#endif
        create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
    }
    return 0;
}

int read_msr_by_idx(int dev_idx, off_t msr, uint64_t *val)
{
    int rc;
    int *fileDescriptor = NULL;

    fileDescriptor = core_fd(dev_idx);
    if (fileDescriptor == NULL)
    {
        return -1;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (read_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
#endif
    rc = pread(*fileDescriptor, (void*)val, (size_t)sizeof(uint64_t), msr);
    if (rc != sizeof(uint64_t))
    {
        libmsr_error_handler("read_msr_by_idx(): Pread failed", LIBMSR_ERROR_MSR_READ, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int write_msr_by_idx(int dev_idx, off_t msr, uint64_t val)
{
    int rc;
    int *fileDescriptor = NULL;

    fileDescriptor = core_fd(dev_idx);
    if (fileDescriptor == NULL)
    {
        return -1;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
#endif
    rc = pwrite(*fileDescriptor, &val, (size_t)sizeof(uint64_t), msr);
    if (rc != sizeof(uint64_t))
    {
        libmsr_error_handler("write_msr_by_idx(): Pwrite failed", LIBMSR_ERROR_MSR_WRITE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int write_msr_by_idx_and_verify(int dev_idx, off_t msr, uint64_t val)
{
    int rc;
    uint64_t test = 0;
    int *fileDescriptor = NULL;

    fileDescriptor = core_fd(dev_idx);
    if (fileDescriptor == NULL)
    {
        return -1;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"), LIBMSR_DEBUG_TAG, __FILE__, __LINE__, msr, msr);
#endif
    rc = pwrite(*fileDescriptor, &val, (size_t)sizeof(uint64_t), msr);
    if (rc != sizeof(uint64_t))
    {
        libmsr_error_handler("write_msr_by_idx_and_verify(): Pwrite failed", LIBMSR_ERROR_MSR_WRITE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (!pread(*fileDescriptor, (void *) &test, (size_t) sizeof(uint64_t), msr))
    {
        libmsr_error_handler("write_msr_by_idx_and_verify(): Verification of write failed", LIBMSR_ERROR_MSR_WRITE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (val != test)
    {
        libmsr_error_handler("write_msr_by_idx_and_verify(): Write unsuccessful", LIBMSR_ERROR_MSR_WRITE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}
