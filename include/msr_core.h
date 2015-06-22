/* 
 * Edited by: Scott Walker
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
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
#ifndef MSR_CORE_H
#define MSR_CORE_H
#include <stdint.h>
#include <sys/types.h>	// off_t
#define NUM_SOCKETS 2
#define NUM_CORES_PER_SOCKET 12 
#define NUM_THREADS_PER_CORE 1
#define NUM_DEVS (NUM_SOCKETS * NUM_CORES_PER_SOCKET * NUM_THREADS_PER_CORE)
#define NUM_CORES (NUM_CORES_PER_SOCKET * NUM_SOCKETS)
#define NUM_THREADS (NUM_CORES * NUM_THREADS_PER_CORE)

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


int init_msr();
int finalize_msr();

int write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val );
int write_msr_by_idx_and_verify( int dev_idx, off_t msr, uint64_t  val );
int read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val );

int write_msr_by_coord( int socket, int core, int thread, off_t msr, uint64_t  val );
int read_msr_by_coord(  int socket, int core, int thread, off_t msr, uint64_t *val );

int write_all_sockets(   off_t msr, uint64_t  val  );
int write_all_sockets_v( off_t msr, uint64_t *val );
int write_all_cores(     off_t msr, uint64_t  val  );
int write_all_cores_v(   off_t msr, uint64_t *val );
int write_all_threads(   off_t msr, uint64_t  val  );
int write_all_threads_v( off_t msr, uint64_t *val );

int read_all_sockets(    off_t msr, uint64_t *val );
int read_all_cores(      off_t msr, uint64_t *val );
int read_all_threads(    off_t msr, uint64_t *val );

#ifdef __cplusplus
}
#endif
#endif //MSR_CORE_H
