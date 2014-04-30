/* msr_core.c
 *
 * Low-level msr interface.
 */

// Necessary for pread & pwrite.
#define _XOPEN_SOURCE 500

#include <stdio.h>	
#include <unistd.h>	
#include <sys/stat.h> 	
#include <fcntl.h>	
#include <stdint.h>	
#include <errno.h>
#include "msr_core.h"
#include "msr_counters.h"

int msr_debug;
static int core_fd[NUM_SOCKETS * NUM_CORES_PER_SOCKET * NUM_THREADS_PER_CORE];

int
init_msr(){
	int dev_idx;
	char filename[1025];
	struct stat statbuf;
	static int initialized = 0;
	int retVal;

	if( initialized ){
		return 0;
	}

	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++){

		snprintf(filename, 1024, "/dev/cpu/%d/msr_safe", dev_idx);
		retVal = stat(filename, &statbuf);

		if (retVal == -1) {
			snprintf(filename, 1024, "%s::%d  Error: stat failed on /dev/socket/%d/msr_safe, check if msr module is loaded\n", 
				__FILE__, __LINE__, dev_idx);
			return -1; 
		}	
			
		if(!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR)){
			snprintf(filename, 1024, "%s::%d  Read/write permissions denied on /dev/socket/%d/msr_safe\n", 
				__FILE__, __LINE__, dev_idx);
			
			return -1;
		}
	 
		core_fd[dev_idx] = open( filename, O_RDWR );

		if(core_fd[dev_idx] == -1){
			snprintf(filename, 1024, "%s::%d  Error opening /dev/socket/%d/msr_safe, check if msr module is loaded. \n", 
				__FILE__, __LINE__, dev_idx);
			perror(filename);
			return -1;
		}
	}
	initialized = 1;
	enable_fixed_counters();
	return 0;
}

void 
finalize_msr(){
	disable_fixed_counters();
	int dev_idx, rc;
	char filename[1025];
	for (dev_idx=0; dev_idx < NUM_DEVS; dev_idx++){
		if(core_fd[dev_idx]){
			rc = close(core_fd[dev_idx]);
			if( rc != 0 ){
				snprintf(filename, 1024, "%s::%d  Error closing file /dev/socket/%d/msr\n", 
						__FILE__, __LINE__, dev_idx); 
				perror(filename);
			}else{
				core_fd[dev_idx] = 0;
			}
		}
	}
}

void
write_msr_by_coord( int socket, int core, int thread, off_t msr, uint64_t  val ){
	write_msr_by_idx( socket * NUM_CORES_PER_SOCKET + core * NUM_THREADS_PER_CORE + thread, msr, val );
}

void
read_msr_by_coord(  int socket, int core, int thread, off_t msr, uint64_t *val ){
	read_msr_by_idx( socket * NUM_CORES_PER_SOCKET + core * NUM_THREADS_PER_CORE + thread, msr, val );
}

void
write_all_sockets(   off_t msr, uint64_t  val ){
	int dev_idx;
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET * NUM_THREADS_PER_CORE ){
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_cores(     off_t msr, uint64_t  val ){
	int dev_idx;
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE ){
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_threads(   off_t msr, uint64_t  val ){
	int dev_idx;
	for(dev_idx=0; dev_idx<NUM_DEVS; dev_idx++){
		write_msr_by_idx( dev_idx, msr, val );
	}
}

void
write_all_sockets_v( off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET*NUM_THREADS_PER_CORE, val_idx++ ){
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}

void
write_all_cores_v(   off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE, val_idx++ ){
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}

void
write_all_threads_v( off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx++, val_idx++ ){
		write_msr_by_idx( dev_idx, msr, val[val_idx] );
	}
}


void
read_all_sockets(    off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_CORES_PER_SOCKET*NUM_THREADS_PER_CORE, val_idx++ ){
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_all_cores(      off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx += NUM_THREADS_PER_CORE, val_idx++ ){
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_all_threads(    off_t msr, uint64_t *val ){
	int dev_idx, val_idx;
	for(dev_idx=0, val_idx=0; dev_idx<NUM_DEVS; dev_idx++, val_idx++ ){
		read_msr_by_idx( dev_idx, msr, &val[val_idx] );
	}
}

void
read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val ){
	int rc;
	char error_msg[1025];
	rc = pread( core_fd[dev_idx], (void*)val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) ){
		snprintf( error_msg, 1024, "%s::%d  pread returned %d.  core_fd[%d]=%d, msr=%ld (0x%lx).  errno=%d\n", 
				__FILE__, __LINE__, rc, dev_idx, core_fd[dev_idx], msr, msr, errno );
		perror(error_msg);
	}
}

void
write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val ){
	int rc;
	char error_msg[1025];
	rc = pwrite( core_fd[dev_idx], &val, (size_t)sizeof(uint64_t), msr );
	if( rc != sizeof(uint64_t) ){
		snprintf( error_msg, 1024, "%s::%d  pwrite returned %d.  core_fd[%d]=%d, msr=%ld (0x%lx).  errno=%d\n", 
				__FILE__, __LINE__, rc, dev_idx, core_fd[dev_idx], msr, msr, errno );
		perror(error_msg);
	}
}

