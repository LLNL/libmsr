/* msr_core.c
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
 * Edited by Scott Walker, walker91@llnl.gov
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 */

// Necessary for pread & pwrite.
#define _XOPEN_SOURCE 500

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>	
#include <sys/stat.h> 	
#include <linux/ioctl.h>
#include <fcntl.h>	
#include <stdint.h>	
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include "msr_core.h"
#include "msr_counters.h"
#include "cpuid.h"

#define LIBMSR_DEBUG_TAG "LIBMSR"
#define LIBMSR_DEBUG     1
#define FILENAME_SIZE 1024
//#define USE_MSR_SAFE_BETA 1

//static int core_fd[NUM_DEVS];

#define MSR_MAX_BATCH_OPS 50
#define X86_IOC_MSR_BATCH _IOWR('c', 0xA2, struct msr_bundle_desc)

struct msr_op
{
    union msrdata
    {
        __u32 data32[2]; // for hi/lo access
        __u64 data64; // for full 64 bit access
    } d;
    __u64 mask; // used by kernel
    __u32 msr; // msr address
    __u32 isread; // non-zero if operation is a read
    int error; // has zero if operation was a success
};

struct msr_cpu_ops
{
    __u32 cpu; // the CPU where the operations are performed
    int nOps; // number of operations for this CPU
    struct msr_op ops[MSR_MAX_BATCH_OPS];
};

struct msr_bundle_desc
{
    int numMsrBundles; // number of jobs in batch
    struct msr_cpu_ops * bundle;
};

// TODO: add this array to the memory handler
static int * core_fd(const int dev_idx)
{
    static int init = 1;
    static int * file_descriptors = NULL;
    static uint64_t devices = 0;
    if (init)
    {
        init = 0;
        uint64_t sockets = 0, coresPerSocket = 0, threadsPerCore = 0;
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        devices = NUM_DEVS_NEW;
        file_descriptors = (int *) malloc(devices * sizeof(int));
        if (file_descriptors == NULL)
        {
            fprintf(stderr, "%s %s::%d ERROR: could not allocate memory\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__);
            exit(-1);
        }
    }
    if (dev_idx < devices)
    {
        return &(file_descriptors[dev_idx]); 
    }
    fprintf(stderr, "%s %s::%d ERROR: reference out of array bounds\n", getenv("HOSTNAME"), __FILE__, __LINE__);
    return NULL;
}

static uint64_t * batch_ops(struct msr_op * op, uint64_t cpu)
{
    static struct msr_bundle_desc b;
    static int * fd = NULL;
    if (fd == NULL)
    {
        fd = core_fd(0);
    }
    if (op == NULL)
    {
        // time to perform batch operation
        if (ioctl(fd, X86_IOC_MSR_BATCH, &b) < 0)
        {
            fprintf(stderr, "%s %s::%d ERROR: ioctl failure\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            exit(-1);
        }
        int i, j;
        for (i = 0; i < b.numMsrBundles; i++)
        {
            for (j = 0; j < b.bundle[i].nOps; j++)
            {
                if (b.bundle[i].ops[j].error)
                {
                    fprintf(stderr, "CPU %d, %s %x Err %d\n", b.bundle[i].cpu, b.bundle[i].ops[j].isread ? "RDMSR" : "WRMSR",
                            b.bundle[i].ops[j].msr, b.bundle[i].ops[j].error);
                }
            }
        }
    }
    if (b.bundle == NULL)
    {
        b.numMsrBundles = 0;
        b.bundle = (struct msr_cpu_ops *) malloc(sizeof(struct msr_cpu_ops)); 
        b.bundle[b.numMsrBundles].cpu = cpu;
        b.bundle[b.numMsrBundles].nOps = 0;
        return NULL;
    }
    int i;
    // walk out to the bundle of the designated cpu, or to the end of the array
    for (i = 0; i < b.numMsrBundles && b.bundle[i].cpu != cpu; i++);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "BATCH: stopped at bundle %d which as cpu %u, function has cpu %lu\n",
            i, b.bundle[i].cpu, cpu);
#endif
    if (b.bundle[i].cpu == cpu)
    {
        b.bundle[i].ops[b.bundle[i].nOps - 1] = *op;
        b.bundle[i].nOps++;
        assert(b.bundle[i].nOps <= 50);
        return (uint64_t *) &(b.bundle[i].ops[b.bundle[i].nOps - 1].d.data64);
    }
    b.numMsrBundles++;
//    if (b.bundle[b.numMsrBundles].nOps >  50)
//    {
        // TODO:
//    }
    struct msr_cpu_ops * temp = b.bundle;
    temp = (struct msr_cpu_ops *) realloc(b.bundle, b.numMsrBundles * sizeof(struct msr_cpu_ops));
    if (temp == NULL)
    {
        fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"),
                __FILE__, __LINE__);
        exit(-1);
    }
    b.numMsrBundles++;
    b.bundle[b.numMsrBundles - 1].nOps = 1;
    b.bundle[b.numMsrBundles - 1].ops[b.bundle[b.numMsrBundles - 1].nOps] = *op;
    return (uint64_t *) &(b.bundle[b.numMsrBundles].ops[b.bundle[i].nOps - 1].d.data64);
}

// TODO: add all dynamic arrays to this memory handler
// I will have to either have each array in the program assigned to a single address, or return their location
// because on realloc the array could move
int memory_handler(void ** address, int dealloc)
{
    static void ** arrays = NULL;
    static unsigned last = 0;
    static unsigned size = 3;
    if (dealloc)
    {
        int i;
        for (i = 0; i < last; i++)
        {
            if (arrays[i])
            {
                free(arrays[i]);
            }
        }
        if (arrays)
        {
            free(arrays);
        }
    }
    else
    {
        if (arrays == NULL)
        {
            arrays = (void **) malloc(size * sizeof(void *));
            if (arrays == NULL)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not allocate memory\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                exit(-1);
            }
        }
        if (last >= size)
        {
            void ** temp = arrays;
            size *= 2;
            temp = (void **) realloc(arrays, size * sizeof(void *));
            if (temp == NULL)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not allocate memory\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
            }
            arrays = temp;
        }
        if (*address)
        {
            arrays[last] = *address;
            last++;
        }
    }
    return 0;
}

int core_config(uint64_t * coresPerSocket, uint64_t * threadsPerCore, uint64_t * sysSockets, int * HTenabled)
{
    static int init = 1;
    static uint64_t cores = 0;
    static uint64_t threads = 0;
    static uint64_t sockets = 0;
    static int hyperthreading = 0;
    if (init)
    {
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: detecting core configuration\n");
#endif
        init = 0;
        cpuid_detect_core_conf(&cores, &threads, &sockets, &hyperthreading);
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: core config complete. cores per socket is %lu, threads per core is %lu, sockets is %lu, htenabled is %d\n", cores, threads, sockets, hyperthreading);
#endif
    }
    if (coresPerSocket)
    {
        *coresPerSocket = cores;
    }
    if (sysSockets)
    {
        *sysSockets = sockets;
    }
    if (HTenabled)
    {
        *HTenabled = hyperthreading;
    }
    if (threadsPerCore)
    {
        // use number of threads actually available.
        *threadsPerCore = 1 + hyperthreading;
    }
    return 0;
}

int sockets_assert(const unsigned * socket, const int location, const char * file)
{
    static uint64_t sockets = 0;

    if (sockets < 1)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    if (*socket > sockets)
    {
        fprintf(stderr, "%s %s::%d ERROR: requested non-existent socket %d\n", 
                getenv("HOSTNAME"), file, location, *socket);
        exit(-1);
    }
    return 0;
}

int threads_assert(const unsigned * thread, const int location, const char * file)
{
    static uint64_t threadsPerCore = 0;

    if (threadsPerCore < 1)
    {
        core_config(NULL, &threadsPerCore, NULL, NULL);
    }
    if (*thread > threadsPerCore)
    {
        fprintf(stderr, "%s %s::%d ERROR: requested non-existent thread %d\n", 
                getenv("HOSTNAME"), file, location, *thread);
        exit(-1);
    }
    return 0;
}

int cores_assert(const unsigned * core, const int location, const char * file)
{
    static uint64_t coresPerSocket = 0;

    if (coresPerSocket < 1)
    {
        core_config(&coresPerSocket, NULL, NULL, NULL);
    }
    if (*core > coresPerSocket)
    {
        fprintf(stderr, "%s %s::%d ERROR: requested non-existent core %d\n", 
                getenv("HOSTNAME"), file, location, *core);
        exit(-1);
    }
    return 0;
}

int core_storage(int recover, recover_data * recoverValue)
{
    static unsigned allocated = 0;
    static recover_data  * recovery = NULL;
    static unsigned size = 0;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    int i;

    if (recovery == NULL)
    {
        allocated = 2;
        recovery = (recover_data *) malloc(allocated * sizeof(recover_data));
        if (recovery == NULL)
        {
            fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"),
                    __FILE__, __LINE__);
            return -1;
        }
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
    if (recoverValue)
    {
        if (size > allocated)
        {
            fprintf(stderr, "%s %s::%d ERROR: allocation error\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
        }
        else
        {
            if (size == allocated)
            {
                recover_data * temp = recovery;
                allocated *= 2;
                temp = (recover_data *) realloc(recovery, allocated * sizeof(recover_data));
                if (temp == NULL)
                {
                    fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"),
                            __FILE__, __LINE__);
                    free(recovery);
                    return -1;
                }
                else
                {
                    fprintf(stderr, "ALLOC: realloc from %p to %p, space %u, size %u\n", recovery, temp, allocated, size);
                    recovery = temp;
                }
            }
            if (recoverValue)
            {
                for (i = 0; i < size; i++)
                {
                    if (recovery[i].msr == recoverValue->msr && recovery[i].socket == recoverValue->socket &&
                        recovery[i].core == recoverValue->core && recovery[i].thread == recoverValue->thread)
                    {
                        return 0;
                    }
                }
                recovery[size] = *recoverValue;
                size++;
            }
        }
    }
    if (recover)
    {
        for( i = 0; i < size; i++)
        {
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "DEBUG: item %d, recovering msr %lx, on socket %d, core %d, thread %d, with data %lx\n",
                    i, recovery[i].msr, recovery[i].socket, recovery[i].core, recovery[i].thread, recovery[i].bits);
#endif
            write_msr_by_idx(recovery[i].socket * coresPerSocket + recovery[i].core * threadsPerCore +
                             recovery[i].thread, recovery[i].msr, recovery[i].bits);
        }
        // TODO: fix this
        fprintf(stderr, "DEBUG: core storage is at %p, first value is %lx\n", recovery, recovery[0].bits);
        if (recovery != NULL)
        {
            free(recovery);
        }
    }
    return 0;
}

// Initialize the MSR module file descriptors
int init_msr()
{
	int dev_idx;
    int * fileDescriptor = NULL;
	char filename[1025];
	struct stat statbuf;
	static int initialized = 0;
    int kerneltype = 0; // 0 is msr_safe, 1 is msr
    uint64_t coresPerSocket = 0;
    uint64_t threadsPerCore = 0;
    uint64_t sockets = 0;

    core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL); 
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s Initializing %lu device(s).\n", getenv("HOSTNAME"), (sockets * coresPerSocket * threadsPerCore));
#endif
	if( initialized ){
		return 0;
	}
    // open the file descriptor for each device's msr interface
	for (dev_idx=0; dev_idx < (sockets * coresPerSocket * threadsPerCore); dev_idx++)
    {
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "found module %d\n", dev_idx);
#endif
        // use the msr_safe module, or default to the msr module
        if (kerneltype)
        {
            snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr", dev_idx);    
        }
        else
        {
            snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr_safe", dev_idx);
#ifdef USE_MSR_SAFE_BETA
            snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr_safe_beta", dev_idx);
#endif
        }
        // check if the module is there, if not return the appropriate error message.
        if (stat(filename, &statbuf))
        {
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
                fprintf(stderr, "%s %s::%d FATAL ERROR: could not stat any valid module. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not find any msr module so exit
                exit(1);
            }
            // could not find msr_safe so try the msr module
            fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s.\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
            kerneltype = 1;
            // restart loading file descriptors for each device
            dev_idx = -1;
            continue;
		}
        // check to see if the module has the correct permissions
		if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
        {
			fprintf(stderr, "%s %s::%d  ERROR: Read/write permissions denied for %s.\n", 
				    getenv("HOSTNAME"),__FILE__, __LINE__, filename);
            kerneltype = 1;
            dev_idx = -1;
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d FATAL ERROR: could not find any valid module with correct permissions. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not find any msr module with RW permissions, so exit
                exit(2);
            }
            continue;
		}
        // try to open the msr module, if you cant then return the appropriate error message
        fileDescriptor = core_fd(dev_idx);
        *fileDescriptor = open(filename, O_RDWR);
		//core_fd[dev_idx] = open( filename, O_RDWR );
		if(*fileDescriptor == -1)
        {
            fprintf(stderr, "%s %s::%d  ERROR: could not open %s: %s.\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, filename, strerror(errno));
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d FATAL ERROR: could not open any valid module. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not open any msr module, so exit
                exit(3);
            }
            kerneltype = 1;
            dev_idx = -1;
		}
	}
	initialized = 1;
	return 0;
}

int finalize_msr(const int restore)
{
	int dev_idx;
    int rc;
    int * fileDescriptor = NULL;
    uint64_t coresPerSocket = 0;
    uint64_t threadsPerCore = 0;
    uint64_t sockets = 0;

    core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
    
    if (restore)
    {
        core_storage(restore, NULL);
    }
    //close the file descriptors
	for (dev_idx=0; dev_idx < (sockets * coresPerSocket * threadsPerCore); dev_idx++)
    {
        fileDescriptor = core_fd(dev_idx);
		if(fileDescriptor)
        {
			//rc = close(core_fd[dev_idx]);
            rc = close(*fileDescriptor);
			if( rc != 0 )
            {
                fprintf(stderr, "%s %s::%d ERROR: could not close file /dev/cpu/%d/msr or msr_safe: %s\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__, dev_idx, strerror(errno));
                return -1;
			}
            else
            {
				//core_fd[dev_idx] = 0;
                *fileDescriptor = 0;
			}
		}
	}
    return 0;
}

int
write_msr_by_coord( unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t  val )
{
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    recover_data oldvalue;
    oldvalue.socket = socket;
    oldvalue.core = core;
    oldvalue.thread = thread;
    oldvalue.msr = msr;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
    read_msr_by_idx(socket * coresPerSocket + core * threadsPerCore + thread, msr, &oldvalue.bits);
    core_storage(0, &oldvalue);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (write_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx) val=%lu\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr, val);
    return write_msr_by_idx_and_verify( socket * coresPerSocket + core * threadsPerCore + thread, msr, val );
#endif
    return write_msr_by_idx( socket * coresPerSocket + core * threadsPerCore + thread, msr, val );
}

int
read_msr_by_coord(  unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t *val ){
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr);
#endif
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
	return read_msr_by_idx( socket * coresPerSocket + core * threadsPerCore + thread, msr, val );
}

int
read_msr_by_coord_batch(  unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t *val ){
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr);
#endif
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    struct msr_op op;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
    op.msr = msr;
    *val = *batch_ops(&op, (socket * coresPerSocket + core * threadsPerCore + thread));
	//return read_msr_by_idx( socket * coresPerSocket + core * threadsPerCore + thread, msr, val );
    return 0;
}

int read_batch()
{
    batch_ops(NULL, 0);
    return 0;
}

int
write_all_sockets(   off_t msr, uint64_t  val )
{
	int dev_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0 || sockets == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_sockets) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx< (sockets * coresPerSocket * threadsPerCore); dev_idx += coresPerSocket * threadsPerCore )
    {
		if(write_msr_by_idx( dev_idx, msr, val ))
        {
            return -1;
        }
	}
    return 0;
}

int
write_all_cores(     off_t msr, uint64_t  val )
{
	int dev_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx += threadsPerCore )
    {
		if(write_msr_by_idx( dev_idx, msr, val ))
        {
            return -1;
        }
	}
    return 0;
}

int
write_all_threads(   off_t msr, uint64_t  val )
{
	int dev_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx++)
    {
		if(write_msr_by_idx( dev_idx, msr, val ))
        {
            return -1;
        }
	}
    return 0;
}

int
write_all_sockets_v( off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_sockets_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); 
        dev_idx += coresPerSocket * threadsPerCore, val_idx++ )
    {
		if(write_msr_by_idx( dev_idx, msr, val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}

int
write_all_cores_v(   off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_cores_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx += threadsPerCore, val_idx++ )
    {
		if (write_msr_by_idx( dev_idx, msr, val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}

int
write_all_threads_v( off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_threads_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx++, val_idx++ )
    {
		if(write_msr_by_idx( dev_idx, msr, val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}


int
read_all_sockets(    off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_sockets) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
    fprintf(stderr, "sockets %lu, cores %lu, threads %lu\n", sockets, coresPerSocket, threadsPerCore);
#endif
	for(dev_idx=0, val_idx=0; dev_idx< NUM_DEVS_NEW;// (sockets * coresPerSocket * threadsPerCore); 
        dev_idx += coresPerSocket * threadsPerCore, val_idx++ )
    {
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "reading device %d\n", dev_idx);
#endif
		if (read_msr_by_idx( dev_idx, msr, &val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}

int
read_all_cores(      off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx += threadsPerCore, val_idx++ )
    {
		if (read_msr_by_idx( dev_idx, msr, &val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}

int 
read_all_threads(    off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(sockets * coresPerSocket * threadsPerCore); dev_idx++, val_idx++ )
    {
		if(read_msr_by_idx( dev_idx, msr, &val[val_idx] ))
        {
            return -1;
        }
	}
    return 0;
}

int
read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val )
{
	int rc;
    int * fileDescriptor = NULL;
    fileDescriptor = core_fd(dev_idx);
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pread( *fileDescriptor, (void*)val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s:: %d ERROR: pread failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, *fileDescriptor, msr, msr, strerror(errno));
        return -1;
	}
    return 0;
}

int
write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
    int * fileDescriptor = NULL;
    fileDescriptor = core_fd(dev_idx);
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pwrite( *fileDescriptor, &val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, *fileDescriptor, msr, msr, strerror(errno));
        return -1;
	}
    return 0;
}

// This function does a read right after writing the msr to see if the write put in the correct data
int
write_msr_by_idx_and_verify( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
    uint64_t test = 0;
    int * fileDescriptor = NULL;
    fileDescriptor = core_fd(dev_idx);
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pwrite( *fileDescriptor, &val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, *fileDescriptor, msr, msr, strerror(errno));
    }
    if(!pread(*fileDescriptor, (void *) &test, (size_t) sizeof(uint64_t), msr))
    {
        fprintf(stderr, "%s %s::%d ERROR: could not verify write for core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, *fileDescriptor, msr, msr, strerror(errno));
        return -1;
    }
    if (val != test)
    {
        fprintf(stderr, "%s %s::%d ERROR: write core_fd[%d]=%d, msr=%ld (0x%lx) unsuccessful. %lx expected, had %lx. Diff %lx\n", 
        getenv("HOSTNAME"), __FILE__, __LINE__, dev_idx, *fileDescriptor, msr, msr, val, test, val ^ test);
        return -1;
    }

    return 0;
}
