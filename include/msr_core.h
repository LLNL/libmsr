/*
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
#define NUM_CORES_PER_SOCKET 8 
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
void finalize_msr();
void write_msr(int socket, off_t msr, uint64_t val);
void write_msr_all_cores(int socket, off_t msr, uint64_t val);
void write_msr_all_cores_v(int socket, off_t msr, uint64_t *val);
void write_msr_single_core(int socket, int core, off_t msr, uint64_t val);

void read_msr(int socket, off_t msr, uint64_t *val);
void read_msr_all_cores_v(int socket, off_t msr, uint64_t *val);
void read_msr_single_core(int socket, int core, off_t msr, uint64_t *val);

#ifdef __cplusplus 
}
#endif
#endif //MSR_CORE_H
