/* msr_core.c
 *
 * Low-level msr interface.
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

// somethin
// stuff
#define LIBMSR_DEBUG_TAG "LIBMSR"
#define LIBMSR_DEBUG     1
#define FILENAME_SIZE 1024
static int core_fd[NUM_DEVS];

void get_groups(gid_t ** usrgroups, unsigned * grparraysize)
{
    gid_t * temp = NULL;;

    *usrgroups = (gid_t *) calloc(*grparraysize, sizeof(gid_t));
    // check the user's groups. Currently you can not be in more than 65536 groups (between 3^10 and 3^11 -> 177147)
    while(getgroups(*grparraysize, *usrgroups) == -1 && *grparraysize <= 177147)
    {
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: resizing array with size %d\n", *grparraysize);
#endif
        // expand the array exponentially to minimize calloc calls
        *grparraysize = *grparraysize * *grparraysize;
        temp = (gid_t *) calloc(*grparraysize, sizeof(gid_t));

        if (temp != NULL)
        {
            free(*usrgroups);
            *usrgroups = temp;
        }
        else
        {
            free(*usrgroups);
            fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"), __FILE__, __LINE__);
        }
    }
    free(temp);
    if (*grparraysize > 177147)
    {
        fprintf(stderr, "%s %s::%d ERROR: getgroups failed with %s\n", getenv("HOSTNAME"), __FILE__, __LINE__, strerror(errno));
    }
    if (*usrgroups == NULL)
    {
        fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"), __FILE__, __LINE__);
    }
}

// return value 1 means could not stat or open msr_safe or msr
// return value 2 means you are not in the msr group
// return value 3 means the files do not have the correct permissions
// return value 4 means that the file is owned by root
int init_msr()
{
	int dev_idx;
	char filename[1025];
	struct stat statbuf;
	static int initialized = 0;
    gid_t * usrgroups = NULL;
<<<<<<< HEAD
=======
    gid_t * temp = NULL;
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
	int retVal;
    int groupitr;
    unsigned grparraysize = 3;
    int validgroup = 0;
<<<<<<< HEAD

    get_groups(&usrgroups, &grparraysize);
=======
	int whichKernel=0; //msr_safe=0, msr=1

    usrgroups = (gid_t *) calloc(grparraysize, sizeof(gid_t));
    // check the user's groups. Currently you can not be in more than 65536 groups (between 3^10 and 3^11 -> 177147)
    while(getgroups(grparraysize, usrgroups) == -1 && grparraysize <= 177147)
    {
        // expand the array exponentially to minimize calloc calls
        grparraysize = grparraysize * grparraysize;
        temp = (gid_t *) calloc(grparraysize, sizeof(gid_t));

        if (temp != NULL)
        {
            free(usrgroups);
            usrgroups = temp;
        }
        else
        {
            free(usrgroups);
            fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"), __FILE__, __LINE__);
        }
    }
    free(temp);
    if (grparraysize > 177147)
    {
        fprintf(stderr, "%s %s::%d ERROR: getgroups failed with %s\n", getenv("HOSTNAME"), __FILE__, __LINE__, strerror(errno));
    }
    for (groupitr = 0; groupitr < grparraysize; ++groupitr)
    {
        if (statbuf.st_gid == usrgroups[groupitr])
        {
            validgroup = groupitr;
        }
    }

>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s Initializing %d device(s).\n", getenv("HOSTNAME"), NUM_DEVS);
#endif
	if( initialized ){
		return 0;
	}
	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++)
    {
		snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr_safe", dev_idx);
		retVal = stat(filename, &statbuf);
		if (retVal == -1) {
<<<<<<< HEAD
            fprintf(stderr, "%s %s::%d ERROR: could not stat /dev/cpu/%d/msr_safe: %s, check if msr_safe module is loaded\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__, dev_idx, strerror(errno));
            return init_msr_type2();
		}
        // if we haven't already found the msr group, go look for it
        if (statbuf.st_gid == 0)
        {
            fprintf(stderr, "%s %s::%d ERROR: msr_safe is owned by root. Hint: Are you running on a login node?\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__);
            return 4;
        }
        if (statbuf.st_gid != usrgroups[validgroup])
        {
            for (groupitr = 0; groupitr < grparraysize && usrgroups[validgroup] != (gid_t) 0; ++groupitr)
            {
                if (statbuf.st_gid == usrgroups[groupitr] && usrgroups[groupitr] != (gid_t) 0)
                {
                    validgroup = groupitr;
                }
            }
        }
=======
			/*
			snprintf(filename, FILENAME_SIZE, "%s %s::%d  Error: stat failed on /dev/cpu/%d/msr_safe, check if msr module is loaded\n", 
				getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
			*/
            fprintf(stderr, "%s %s::%d ERROR: could not stat /dev/cpu/%d/msr_safe, check if msr_safe module is loaded\n", 
                    getenv("HOSTNAME"), __FILE__, __LINE__, dev_idx);
			whichKernel =1;
			break; 
		}
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
#ifdef LIBMSR_DEBUG
   fprintf(stderr, "%s %s::%d DEBUG: stat of /dev/cpu/%d/msr_safe has gid %d, you have %d\n", getenv("HOSTNAME"), 
            __FILE__, __LINE__, dev_idx, statbuf.st_gid, usrgroups[validgroup]);
#endif
        if (statbuf.st_gid != usrgroups[validgroup])
        {
            fprintf(stderr, "%s %s::%d Error: you must be in the msr group to use libmsr.\n", getenv("HOSTNAME"), __FILE__, __LINE__);
<<<<<<< HEAD
            return 2;
        }
=======
        }
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "%s %s::%d DEBUG: read perm is %d, write perm is %d\n", getenv("HOSTNAME"), __FILE__, __LINE__, 
                !((int) statbuf.st_mode & S_IRUSR), !((int) statbuf.st_mode & S_IWUSR));
#endif
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
		if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
        {
			fprintf(stderr, "%s %s::%d  Read/write permissions denied on /dev/cpu/%d/msr_safe\n", 
				getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
<<<<<<< HEAD
            return 3;
		}
		core_fd[dev_idx] = open( filename, O_RDWR );
		if(core_fd[dev_idx] == -1){
            return init_msr_type2();
=======
			whichKernel =1;
		}
		core_fd[dev_idx] = open( filename, O_RDWR );
		if(core_fd[dev_idx] == -1){
			/*
			snprintf(filename, FILENAME_ZIE, "%s %s::%d  Error opening /dev/cpu/%d/msr_safe, check if msr module is loaded. \n", 
				getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
			perror(filename);
			*/
			whichKernel =1;
			break;
		}
	}
	if (whichKernel == 1){
        fprintf(stderr, "%s %s::%d ERROR: Unable to find msr_safe module, reverting to msr module\n", getenv("HOSTNAME"), __FILE__, __LINE__);
		for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++){
			snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr", dev_idx);
			retVal = stat(filename, &statbuf);
			if (retVal == -1) {
				fprintf(stderr, "%s %s::%d  Error: could not stat /dev/cpu/%d/msr, check if msr module is loaded\n", 
					getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
				return 1; 
			}	
			if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
            {
				fprintf(stderr, "%s %s::%d  Read/write permissions denied on /dev/cpu/%d/msr\n", 
					getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
				
				return 1;
			}
		 
			core_fd[dev_idx] = open( filename, O_RDWR );
	
			if(core_fd[dev_idx] == -1){
				fprintf(stderr, "%s %s::%d  Error opening /dev/cpu/%d/msr, check if msr module is loaded. \n", 
					getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
				return 1;
			}
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
		}
	}
    free(usrgroups);
	initialized = 1;
	return 0;
}

// return value 1 means could not stat or open msr_safe or msr
// return value 2 means you are not in the msr group
// return value 3 means the files do not have the correct permissions
// return value 4 means msr is owned by root
int init_msr_type2()
{
	int dev_idx;
	char filename[1025];
<<<<<<< HEAD
	struct stat statbuf;
	static int initialized = 0;
    gid_t * usrgroups = NULL;
	int retVal;
    int groupitr;
    unsigned grparraysize = 3;
    int validgroup = 0;
    
    get_groups(&usrgroups, &grparraysize);
    fprintf(stderr, "%s %s::%d ERROR: Unable to find msr_safe module, reverting to msr module\n", getenv("HOSTNAME"), __FILE__, __LINE__);
    for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++)
    {
        snprintf(filename, FILENAME_SIZE, "/dev/cpu/%d/msr", dev_idx);
        retVal = stat(filename, &statbuf);
        if (retVal == -1) 
        {
            fprintf(stderr, "%s %s::%d  Error: could not stat /dev/cpu/%d/msr: %s, check if msr module is loaded.\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx, strerror(errno));
            return 1; 
        }
        if (statbuf.st_gid == 0)
        {
            fprintf(stderr, "%s %s::%d ERROR: msr is owned by root. Hint: Are you running on a login node?\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__);
            return 4;    
        }
        // if we havent already found the msr group, go look for it
        if (statbuf.st_gid != usrgroups[validgroup])
        {
            for (groupitr = 0; groupitr < grparraysize && usrgroups[validgroup] != (gid_t) 0; ++groupitr)
            {
                if (statbuf.st_gid == usrgroups[groupitr] && usrgroups[groupitr] != (gid_t) 0)
                {
                    validgroup = groupitr;
                }
            }
        }
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: msr group is %d, your group is %d\n", statbuf.st_gid, usrgroups[validgroup]);
#endif
        if (statbuf.st_gid != usrgroups[validgroup])
        {
            fprintf(stderr, "%s %s::%d Error: you must be in the msr group to use libmsr.\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            return 2;
        }
        if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR) || !(statbuf.st_mode & S_IRGRP) || !(statbuf.st_mode & S_IWGRP))
        {
            fprintf(stderr, "%s %s::%d  Read/write permissions denied on /dev/cpu/%d/msr\n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx);
            return 3;
        }
        core_fd[dev_idx] = open( filename, O_RDWR );
        if(core_fd[dev_idx] == -1){
            fprintf(stderr, "%s %s::%d  Error opening /dev/cpu/%d/msr: %s, check if msr module is loaded. \n", 
                getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx, strerror(errno));
            return 1;
        }
    }
    free(usrgroups);
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
=======
    //close the file descriptors
	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++){
		if(core_fd[dev_idx]){
			rc = close(core_fd[dev_idx]);
			if( rc != 0 ){
				snprintf(filename, FILENAME_SIZE, "%s %s::%d  Error closing file /dev/cpu/%d/msr\n", 
						getenv("HOSTNAME"),__FILE__, __LINE__, dev_idx); 
				perror(filename);
			}else{
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
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
<<<<<<< HEAD
	if( rc != sizeof(uint64_t) )
    {
=======
	if( rc != sizeof(uint64_t) ){
		//snprintf( error_msg, 1024, "%s %s::%d  pread returned %d.  core_fd[%d]=%d, msr=%ld (0x%lx).  errno=%d\n", 
	//			getenv("HOSTNAME"),__FILE__, __LINE__, rc, dev_idx, core_fd[dev_idx], msr, msr, errno );
	//	perror(error_msg);
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
        fprintf(stderr, "%s %s:: %d ERROR: pread failed on core_fd[%d]=%d, msr=%ld (0x%lx) with %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
	}
}

void
write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
<<<<<<< HEAD
=======
//	char error_msg[1025];
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
#ifdef LIBMSR_DEBUG
	fprintf(stderr, "%s %s %s::%d (write_msr_by_idx) msr=%lu (0x%lx)\n", getenv("HOSTNAME"),LIBMSR_DEBUG_TAG, 
            __FILE__, __LINE__, msr, msr);
#endif
	rc = pwrite( core_fd[dev_idx], &val, (size_t)sizeof(uint64_t), msr );
<<<<<<< HEAD
	if( rc != sizeof(uint64_t) )
    {
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx) with %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
=======
	if( rc != sizeof(uint64_t) ){
		//snprintf( error_msg, 1024, "%s %s::%d  pwrite returned %d.  core_fd[%d]=%d, msr=%ld (0x%lx).  errno=%d\n", 
	//			getenv("HOSTNAME"),__FILE__, __LINE__, rc, dev_idx, core_fd[dev_idx], msr, msr, errno );
	//	perror(error_msg);
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx) with %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
#ifdef LIBMSR_DEBUG
        uint64_t test = 0;
        if(!pread(core_fd[dev_idx], (void *) test, (size_t) sizeof(uint64_t), msr))
        {
            fprintf(stderr, "DEBUG: failed to read 618 with %s\n", strerror(errno));
        }
        fprintf(stderr, "DEBUG: attempted to set value %lu and data is actually %lu\n", val, test);
#endif
>>>>>>> 7bdfc97c99db75c7379b3ec85732bffab95aacb2
	}
}

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
        fprintf(stderr, "%s %s::%d ERROR: pwrite failed on core_fd[%d]=%d, msr=%ld (0x%lx) with %s\n", getenv("HOSTNAME"), 
                __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
        if(!pread(core_fd[dev_idx], (void *) test, (size_t) sizeof(uint64_t), msr))
        {
            fprintf(stderr, "%s %s::%d ERROR: could not verify write for core_fd[%d]=%d, msr=%ld (0x%lx) with %s\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, strerror(errno));
        }
        if (val != test)
        {
            fprintf(stderr, "%s %s::%d ERROR: write core_fd[%d]=%d, msr=%ld (0x%lx) unsuccessful. %lu expected, had %lu\n", getenv("HOSTNAME"), 
                    __FILE__, __LINE__, dev_idx, core_fd[dev_idx], msr, msr, test, val);
        }
	}
}
