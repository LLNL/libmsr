/* msr_core.h
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

#ifndef MSR_CORE_H
#define MSR_CORE_H
#include <stdint.h>
#include <sys/types.h>
#include <linux/types.h>
#define LIBMSR_DEBUG 1
#define NUM_DEVS_NEW (sockets * coresPerSocket * threadsPerCore)
#define NUM_CORES_NEW (sockets * coresPerSocket)
// this is the same as num_devs
#define NUM_THREADS_NEW (sockets * coresPerSocket * threadsPerCore)
#define COORD_INDEXING ((thread * (sockets + 1) * coresPerSocket) + (socket * coresPerSocket) + core)

/* MASK_RANGE
 * Create a mask from bit m to n.
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(4,2) -->     (((1<<((4)-(2)+1))-1)<<(2))
 *                                   (((1<<          3)-1)<<(2))
 *                                   ((               4-1)<<(2))
 *                                   (                  3)<<(2))
 *                                   (                       24) = b11000
 */
#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))

/* MASK_VAL
 * Return the value of x after applying bitmask (m,n).
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(17,4,2) --> 17&24 = b10001 & b11000 = b10000
 */
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

enum{
	MSR_AND,
	MSR_OR,
	MSR_XOR
};

enum{
    RAPL_DATA,
    RAPL_UNIT,
    FIXED_COUNTERS_DATA,
    FIXED_COUNTERS_CTR_DATA,
    COUNTERS_DATA,
    COUNTERS_CTRL,
    CLOCKS_DATA,
    CLOCKS_CTR_DATA,
    THERM_STAT,
    THERM_INTERR,
    PKG_THERM_STAT,
    PKG_THERM_INTERR,
    TEMP_TARGET,
    PERF_CTL,
    PKG_CRES,
    CORE_CRES,
    UNCORE_EVTSEL,
    UNCORE_COUNT,
    USR_BATCH0,// Set aside for user defined use
    USR_BATCH1, // Set aside for user defined use
    USR_BATCH2, // Set aside for user defined use
    USR_BATCH3,  // Set aside for user defined use
    USR_BATCH4,
    USR_BATCH5,
    USR_BATCH6,
    USR_BATCH7,
    USR_BATCH8,
    USR_BATCH9,
    USR_BATCH10
};

enum
{
    BATCH_LOAD,
    BATCH_WRITE,
    BATCH_READ
};

// Depending on their function, MSRs can be addressed at either
// the socket (aka cpu) or core level, and possibly the hardware
// thread level.
//
//  read/write_msr reads from core 0.
//  read/write_msr_all_cores_v uses a vector of values.
//  write_msr_all_cores writes all cores with a single value.
//  read/write_msr_single_core contains all of the low-level logic.
//	The rest of the functions are wrappers that call these
//	two functions.
//

#ifdef __cplusplus
extern "C" {
#endif

#define MSR_MAX_BATCH_OPS 50
//#define X86_IOC_MSR_BATCH _IOWR('c', 0xA2, struct msr_bundle_desc)
//#define  X86_IOC_MSR_RDMSR_BATCH _IOWR('c', 0xA2, msr_batch_rdmsr_array)
#define  X86_IOC_MSR_BATCH _IOWR('c', 0xA2, msr_batch_array)
#define MSR_BATCH_DIR "/dev/cpu/msr_batch"

struct msr_batch_op {
	__u16 cpu;		/* In: CPU to execute {rd/wr}msr ins. */
	__u16 isrdmsr;		/* In: 0=wrmsr, non-zero=rdmsr */
    __s32 err;
	__u32 msr;		/* In: MSR Address to perform op */
	__u64 msrdata;		/* In/Out: Input/Result to/from operation */
	__u64 wmask;		/* Out: Write mask applied to wrmsr */
};

struct msr_batch_array {
	__u32 numops;			/* In: # of operations in ops array */
	struct msr_batch_op *ops;	/* In: Array[numops] of operations */
} msr_batch_array;

uint64_t num_cores();
uint64_t num_sockets();
uint64_t num_devs();
uint64_t cores_per_socket();

int init_msr();
int finalize_msr();

int create_batch_op(off_t msr, uint64_t cpu, uint64_t ** dest, const int batchnum);
int allocate_batch(int batchnum, size_t bsize);
int read_batch(const int batchnum);
int write_batch(const int batchnum);
int free_batch(int batchnum);

int core_config(uint64_t * coresPerSocket, uint64_t * threadsPerCore, uint64_t * sockets, int * HTenabled);

int sockets_assert(const unsigned * socket, const int location, const char * file);
int cores_assert(const unsigned * core, const int location, const char * file);
int threads_assert(const unsigned * thread, const int location, const char * file);

int write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val );
int write_msr_by_idx_and_verify( int dev_idx, off_t msr, uint64_t  val );
int read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val );

int write_msr_by_coord( unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t  val );
int read_msr_by_coord(  unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t *val );
int read_msr_by_coord_batch(  unsigned socket, unsigned core, unsigned thread, off_t msr, uint64_t **val , int batchnum);

int load_socket_batch(    off_t msr, uint64_t **val , const int batchnum);
int load_core_batch(      off_t msr, uint64_t **val , const int batchnum);
int load_thread_batch(    off_t msr, uint64_t **val , const int batchnum);

#ifdef __cplusplus
}
#endif
#endif //MSR_CORE_H
