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
#ifndef MSR_RAPL_H
#define MSR_RAPL_H
#include <stdint.h>

// TLCC2 architecture uses 062D processors; 
// those are the only ones we care about.
#define USE_062D 1
#define USE_062A 0	
#define USE_063E 0

// Watts and seconds are actual watts and actual seconds, not
// scaled values.  The bit vector is the 64-bit values that is
// read from/written to the msr.
struct rapl_data{
	uint64_t old_pkg_bits;
	uint64_t pkg_bits;
	uint64_t old_dram_bits;
	uint64_t dram_bits;
	double pkg_joules;
	double dram_joules;
	double pkg_watts;
	double dram_watts;
	double elapsed;
};


struct rapl_limit{
	double 		watts;		// User-friendly interface.
	double	 	seconds;
	uint64_t 	bits;		// User-unfriendly interface.
};


// We're going to overload this interface a bit...
//
// rapl_set_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds 
//	and write the bit vector to the msr.
//
//	c) If the bit_vector is zero, translate the watts and seconds to the appropriate
//	bit_vector and write the bit_vector to the msr.
//
// rapl_get_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds.
//
//	c) If the bit_vector is zero, read the msr value into the bit_vector and 
//	translate into watts and seconds.
//
#ifdef __cplusplus 
extern "C" {
#endif
void rapl_set_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void rapl_get_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void rapl_dump_limit( struct rapl_limit *L );

void rapl_read_data( const int socket, struct rapl_data *r );
void rapl_dump_data( struct rapl_data *r );
#ifdef __cplusplus 
}
#endif

#endif /*MSR_RAPL_H*/
