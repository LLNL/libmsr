/* msr_core.c
 *
 * Low-level msr interface.
 * Edited by Scott Walker
 */

// Necessary for pread & pwrite.
#define _XOPEN_SOURCE 500

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>	
#include <sys/stat.h> 	
#include <fcntl.h>	
#include <stdint.h>	
#include <errno.h>
#include <assert.h>
#include "msr_core.h"
#include "msr_counters.h"
#include <string.h>

#define LIBMSR_DEBUG_TAG "LIBMSR"
#define LIBMSR_DEBUG     1
#define FILENAME_SIZE 1024
static int core_fd[NUM_DEVS];

// return value 1 means could not stat or open msr_safe or msr
// return value 2 means the files do not have the correct permissions
// return value 3 means the file could not be opened
int init_msr()
{
	int dev_idx;
	char filename[1025];
	struct stat statbuf;
	static int initialized = 0;
	int retVal;
    int kerneltype = 0; // 0 is msr_safe, 1 is msr

#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s Initializing %d device(s).\n", getenv("HOSTNAME"), NUM_DEVS);
#endif
	if( initialized ){
		return 0;
	}
    // open the file descriptor for each device's msr interface
	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++)
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
        // check if the module is there, if not return the appropriate error message.
		retVal = stat(filename, &statbuf);
		if (retVal == -1)
        {
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
                fprintf(stderr, "%s %s::%d ERROR: could not stat any valid module. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not find any msr module so exit
                return 1;
            }
            // could not find msr_safe so try the msr module
            fprintf(stderr, "%s %s::%d ERROR: could not stat %s: %s, Reverting to the msr module.\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__, filename, strerror(errno));
            kerneltype = 1;
            // restart loading file descriptors for each device
            dev_idx = -1;
            continue;
		}
        // check to see if the module has the correct permissions
		if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
        {
			fprintf(stderr, "%s %s::%d  ERROR: Read/write permissions denied for %s, Reverting to the msr module.\n", 
				    getenv("HOSTNAME"),__FILE__, __LINE__, filename);
            kerneltype = 1;
            dev_idx = -1;
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not find any valid module with correct permissions. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not find any msr module with RW permissions, so exit
                return 2;
            }
            continue;
		}
        // try to open the msr module, if you cant then return the appropriate error message
		core_fd[dev_idx] = open( filename, O_RDWR );
		if(core_fd[dev_idx] == -1)
        {
            fprintf(stderr, "%s %s::%d  ERROR: could not open %s: %s.\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, filename, strerror(errno));
            if (kerneltype)
            {
                fprintf(stderr, "%s %s::%d ERROR: could not open any valid module. Aborting...\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__);
                // could not open any msr module, so exit
                return 3;
            }
            kerneltype = 1;
            dev_idx = -1;
		}
	}
	initialized = 1;
	return 0;
}

void finalize_msr()
{
	int dev_idx;
    int rc;
    //close the file descriptors
	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++)
    {
		if(core_fd[dev_idx])
        {
			rc = close(core_fd[dev_idx]);
			if( rc != 0 )
            {
                fprintf(stderr, "%s %s::%d Error closing file /dev/cpu/%d/msr or msr_safe: %s\n", 
                        getenv("HOSTNAME"), __FILE__, __LINE__, dev_idx, strerror(errno));
			}
            else
            {
				core_fd[dev_idx] = 0;
			}
		}
	}
}

void
write_msr_by_coord( int socket, int core, int thread, off_t msr, uint64_t  val ){
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx) val=%lu\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr, val);
#endif
	write_msr_by_idx( socket * NUM_CORES_PER_SOCKET + core * NUM_THREADS_PER_CORE + thread, msr, val );
}

void
read_msr_by_coord(  int socket, int core, int thread, off_t msr, uint64_t *val ){
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_coord) socket=%d core=%d thread=%d msr=%lu (0x%lx)\n", 
            getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, __FILE__, __LINE__, socket, core, thread, msr, msr);
#endif
	assert(socket < NUM_SOCKETS);
	assert(core   < NUM_CORES_PER_SOCKET);
	assert(thread < NUM_THREADS_PER_CORE);
	assert(socket >= 0 );
	assert(core   >= 0 );
	assert(thread >= 0 );
	read_msr_by_idx( socket * NUM_CORES_PER_SOCKET + core * NUM_THREADS_PER_CORE + thread, msr, val );
}

void
write_all_sockets(   off_t msr, uint64_t  val )
{
	int dev_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_sockets) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET * NUM_THREADS_PER_CORE )
    {
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_cores(     off_t msr, uint64_t  val )
{
	int dev_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE )
    {
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_threads(   off_t msr, uint64_t  val )
{
	int dev_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx++)
    {
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_sockets_v( off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_sockets_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET*NUM_THREADS_PER_CORE, val_idx++ )
    {
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}

void
write_all_cores_v(   off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_cores_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE, val_idx++ )
    {
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}

void
write_all_threads_v( off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_all_threads_v) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx++, val_idx++ )
    {
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}


void
read_all_sockets(    off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_sockets) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET*NUM_THREADS_PER_CORE, val_idx++ )
    {
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_all_cores(      off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_cores) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE, val_idx++ )
    {
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_all_threads(    off_t msr, uint64_t *val )
{
	int dev_idx, val_idx;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_all_threads) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx++, val_idx++ )
    {
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val )
{
	int rc;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (read_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pread( core_fd[dev_idx], (void*)val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s:: %d ERROR: pread failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
	}
}

void
write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pwrite( core_fd[dev_idx], &val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
	}
}

// This function does a read right after writing the msr to see if the write put in the correct data
void
write_msr_by_idx_and_verify( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
    uint64_t test = 0;
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pwrite( core_fd[dev_idx], &val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
        if(!pread(core_fd[dev_idx], (void *) test, (size_t) sizeof(uint64_t), msr))
        {
            fprintf(stderr, "%s %s::%d ERROR: could not verify write for core_fd[%d]=%d, msr=%ld (0x%lx): %s\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
        }
        if (val != test)
        {
            fprintf(stderr, "%s %s::%d ERROR: write core_fd[%d]=%d, msr=%ld (0x%lx) unsuccessful. %lu expected, had %lu\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, test, val);
        }
	}
}
