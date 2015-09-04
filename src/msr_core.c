/* msr_core.c
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *
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
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

// Necessary for pread & pwrite.
#define _XOPEN_SOURCE 500

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>	
#include <sys/stat.h> 	
#include <sys/ioctl.h>
#include <fcntl.h>	
#include <stdint.h>	
#include <errno.h>
#include <string.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include "msr_core.h"
#include "memhdlr.h"
#include "msr_counters.h"
#include "cpuid.h"

#define LIBMSR_DEBUG_TAG "LIBMSR"
#define FILENAME_SIZE 1024

uint64_t num_cores()
{
    static uint64_t coresPerSocket = 0, sockets = 0;
    static int init = 1;
    if (init)
    {
        core_config(&coresPerSocket, NULL, &sockets, NULL);
        init = 0;
    }
    return coresPerSocket * sockets;
}

uint64_t num_sockets()
{
    static uint64_t sockets = 0;
    static int init = 1;
    if (init)
    {
        core_config(NULL, NULL, &sockets, NULL);
        init = 0;
    }
    return sockets;
}

uint64_t num_devs()
{
    static uint64_t coresPerSocket = 0, threadsPerCore = 0, sockets = 0;
    static int init = 1;
    if (init)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        init = 0;
    }
    return coresPerSocket * threadsPerCore * sockets;
}

uint64_t cores_per_socket()
{
    static uint64_t coresPerSocket = 0;
    static int init = 1;
    if (init)
    {
        core_config(&coresPerSocket, NULL, NULL, NULL);
        init = 0;
    }
    return coresPerSocket;
}

static int * core_fd(const int dev_idx)
{
    static int init = 1;
    static int * file_descriptors = NULL;
    static uint64_t devices = 0;
    if (init)
    {
        init = 0;
        uint64_t numDevs = num_devs();;
        devices = numDevs;;
        file_descriptors = (int *) libmsr_malloc(devices * sizeof(int));
    }
    if (dev_idx < devices)
    {
        return &(file_descriptors[dev_idx]); 
    }
    fprintf(stderr, "%s %s::%d ERROR: reference out of array bounds\n", getenv("HOSTNAME"), __FILE__, __LINE__);
    return NULL;
}

//#define BATCH_DEBUG 1

static int batch_storage(struct msr_batch_array ** batchsel, const int batchnum, unsigned ** opssize)
{
    static struct msr_batch_array * batch = NULL;
    static unsigned arrsize = 1;
    static unsigned * size = NULL;
    if (batch == NULL)
    {
#ifdef BATCH_DEBUG
        fprintf(stderr, "BATCH: initializing batch ops\n");
#endif
        arrsize = (batchnum + 1 > arrsize ? batchnum + 1 : arrsize);
        size = (unsigned *) libmsr_calloc(arrsize, sizeof(unsigned));
        batch = (struct msr_batch_array *) libmsr_calloc(arrsize, sizeof(struct msr_batch_array));
        int i;
        for (i = 0; i < arrsize; i++)
        {
            size[i] = 8;
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
            size[oldsize] = 8;
        }
    }
    *batchsel = &batch[batchnum];
    if (opssize)
    {
        *opssize = &size[batchnum];
    }
    return 0;
}

int allocate_batch(int batchnum, size_t bsize)
{
    unsigned * size = NULL;
    struct msr_batch_array * batch = NULL;
    if (batch_storage(&batch, batchnum, &size))
    {
        return -1;
    }
    *size = bsize;
    if (batch->ops != NULL)
    {
        fprintf(stderr, "%s %s::%d ERROR: conflicting batch pointers for batch %d, was %p\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, batchnum, batch);
    }
    batch->ops = (struct msr_batch_op *) libmsr_calloc(*size, sizeof(struct msr_batch_op));
    int i;
    for (i = batch->numops; i < *size; i++)
    {
        batch->ops[i].err = 0;
    }
    return 0;
}

int free_batch(int batchnum)
{
    struct msr_batch_array * batch = NULL;
    static unsigned * size = NULL;
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

static int do_batch_op(int batchnum, int type)
{
    static int batchfd = 0;
    struct msr_batch_array * batch = NULL;
    if (batchfd == 0)
    {
        if ((batchfd = open(MSR_BATCH_DIR, O_RDWR)) < 0)
        {
            perror(MSR_BATCH_DIR);
            return -1;
        }
    }
    if (batch_storage(&batch, batchnum, NULL))
    {
        return -1;
    }
#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH %d: %s MSRs, numops %u\n", batchnum, (type == BATCH_READ ? "reading" : "writing"), batch->numops);
#endif

    // if current flag is the opposite type switch the flags
    if ((type == BATCH_WRITE && batch->ops[0].isrdmsr) ||
        (type == BATCH_READ && !batch->ops[0].isrdmsr))
    {
        __u8 readflag = (__u8) (type == BATCH_READ ? 1 : 0);
        int j;
        for (j = 0; j < batch->numops; j++)
        {
            batch->ops[j].isrdmsr = readflag;
        }
    }
    int res;
    res = ioctl(batchfd, X86_IOC_MSR_BATCH, batch);
    if ( res < 0)
    {
        perror("IOctl failed");
        fprintf(stderr, "ioctl returned %d\n", res);
        int i;
        for (i = 0; i < batch->numops; i++)
        {
            // Temporarily removed because err has garbage in it
            //if (batch->ops[i].err)
            //{
            //    fprintf(stderr, "CPU %d, RDMSR %x, ERR (%s)\n", batch->ops[i].cpu, batch->ops[i].msr,
            //            strerror(batch->ops[i].err));
            //}
        }
    }
#ifdef BATCH_DEBUG
    int k;
    for (k = 0; k < batch->numops; k++)
    {
        fprintf(stderr, "BATCH %d: msr 0x%x cpu %u data 0x%lx (at %p)\n", batchnum, batch->ops[k].msr,
                batch->ops[k].cpu, (uint64_t) batch->ops[k].msrdata,
                &batch->ops[k].msrdata);
    }
#endif
    return 0;
}

int create_batch_op(off_t msr, uint64_t cpu, uint64_t ** dest, const int batchnum)
{
    struct msr_batch_array * batch = NULL;
    unsigned * size = NULL;
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
    if (batch->numops >= *size)
    {
        fprintf(stderr, "%s %s::%d ERROR: batch %d is full, you probably used the wrong size\n", getenv("HOSTNAME"),
                __FILE__, __LINE__, batchnum);
        return -1; 
    }

    batch->numops++;
    batch->ops[batch->numops - 1].msr = msr;
    batch->ops[batch->numops - 1].cpu = (__u16) cpu;
    batch->ops[batch->numops - 1].isrdmsr = (__u8) 1;
    batch->ops[batch->numops - 1].err = 0;
    *dest = (uint64_t *) &batch->ops[batch->numops - 1].msrdata;
#ifdef BATCH_DEBUG
    fprintf(stderr, "BATCH: destination of msr %lx on core %lx (at %p) is %p\n", msr, cpu, 
            dest, &batch->ops[batch->numops - 1].msrdata);
#endif
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

    if (!sockets)
    {
        sockets = num_sockets();
    }
    if (*socket > sockets)
    {
        fprintf(stderr, "%s %s::%d ERROR: requested non-existent socket %d\n", 
                getenv("HOSTNAME"), file, location, *socket);
        return -1;
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
        return -1;
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
        return -1;
    }
    return 0;
}

int stat_module(char * filename, int * kerneltype, int * dev_idx)
{
	struct stat statbuf;
    if (stat(filename, &statbuf))
    {
        if (*kerneltype)
        {
            fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
            fprintf(stderr, "%s %s::%d FATAL ERROR: could not stat any valid module. Aborting...\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__);
            // could not find any msr module so exit
            return -1;
        }
        // could not find msr_safe so try the msr module
        fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s.\n", 
                getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
        *kerneltype = 1;
        // restart loading file descriptors for each device
        *dev_idx = -1;
        return 0;
    }
    if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || 
       !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
    {
        fprintf(stderr, "%s %s::%d  ERROR: Read/write permissions denied for %s.\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, filename);
        *kerneltype = 1;
        *dev_idx = -1;
        if (kerneltype)
        {
            fprintf(stderr, "%s %s::%d FATAL ERROR: could not find any valid module with correct permissions. Aborting...\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__);
            // could not find any msr module with RW permissions, so exit
            return -1;
        }
    }
    return 0;
}

// Initialize the MSR module file descriptors
int init_msr()
{
	int dev_idx;
    int * fileDescriptor = NULL;
	char filename[FILENAME_SIZE];
	static int initialized = 0;
    int kerneltype = 0; // 0 is msr_safe, 1 is msr
    uint64_t numDevs = num_devs();
    
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s Initializing %lu device(s).\n", getenv("HOSTNAME"), (numDevs));
#endif
	if( initialized ){
		return 0;
	}
    // open the file descriptor for each device's msr interface
	for (dev_idx=0; dev_idx < (numDevs); dev_idx++)
    {
        // use the msr_safe module, or default to the msr module
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
        // try to open the msr module, if you cant then return the appropriate error message
        fileDescriptor = core_fd(dev_idx);
        *fileDescriptor = open(filename, O_RDWR);
		if(*fileDescriptor == -1)
        {
            fprintf(stderr, "%s %s::%d  ERROR: could not open %s: %s.\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, filename, strerror(errno));
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d FATAL ERROR: could not open any valid module. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not open any msr module, so exit
                return -1;
            }
            kerneltype = 1;
            dev_idx = -1;
		}
	}
	initialized = 1;
	return 0;
}

int finalize_msr()
{
	int dev_idx;
    int rc;
    int * fileDescriptor = NULL;
    uint64_t numDevs = num_devs();

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: finalize_msr\n");
#endif
    
    //close the file descriptors
	for (dev_idx=0; dev_idx < (numDevs); dev_idx++)
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
    memhdlr_finalize();
    return 0;
}

int
write_msr_by_coord( unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t  val )
{
    sockets_assert(&socket, __LINE__, __FILE__);
    cores_assert(&core, __LINE__, __FILE__);
    threads_assert(&thread, __LINE__, __FILE__);
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, NULL, NULL);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s %s::%d (write_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx) val=%lu\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr, val);
    return write_msr_by_idx_and_verify( COORD_INDEXING, msr, val );
#endif
    return write_msr_by_idx( COORD_INDEXING, msr, val );
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
	return read_msr_by_idx( COORD_INDEXING, msr, val );
}

int
read_msr_by_coord_batch(  unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t ** val, int batchnum){
#ifdef BATCH_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_coord_batch) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", 
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
#ifdef BATCH_DEBUG
    fprintf(stderr, "DEBUG: passed operation on msr 0x%lx (socket %u, core %u, thread %u) to BATCH OPS with destination %p\n", 
            msr, socket, core, thread, val);
#endif
    create_batch_op(msr, COORD_INDEXING, val, batchnum);
    return 0;
}

int read_batch(const int batchnum)
{
    do_batch_op(batchnum, BATCH_READ);
    return 0;
}

int write_batch(const int batchnum)
{
    do_batch_op(batchnum, BATCH_WRITE);
    return 0;
}

int
load_socket_batch(  off_t msr, uint64_t **val , int batchnum)
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
	for(dev_idx=0, val_idx=0; dev_idx< NUM_DEVS_NEW;
        dev_idx += coresPerSocket * threadsPerCore, val_idx++ )
    {
        create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
	}
    return 0;
}

int
load_core_batch( off_t msr, uint64_t **val , int batchnum)
{
	int dev_idx, val_idx;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t sockets = 0;
    static uint64_t coretotal = 0; 
    if (coresPerSocket == 0 || threadsPerCore == 0)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        coretotal = sockets * coresPerSocket;
    }
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx< coretotal; dev_idx += threadsPerCore, val_idx++ )
    {
        create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
	}
    return 0;
}

int 
load_thread_batch( off_t msr, uint64_t **val , int batchnum)
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
	fprintf(stderr, "%s %s %s::%d (read_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<(NUM_DEVS_NEW); dev_idx++, val_idx++ )
    {
        create_batch_op(msr, dev_idx, &val[val_idx], batchnum);
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
