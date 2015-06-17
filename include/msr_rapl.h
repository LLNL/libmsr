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
#include <sys/time.h>
// TLCC2 architecture uses 062D processors; 
// those are the only ones we care about.
#define USE_062D 1
#define USE_062A 0	
#define USE_063E 0

// Watts and seconds are actual watts and actual seconds, not
// scaled values.  The bit vector is the 64-bit values that is
// read from/written to the msr.
struct rapl_data{
    // holds the bits previously stored in the MSR_PKG_ENERGY_STATUS register
	uint64_t old_pkg_bits;
    // holds the bits currently stored in the MSR_PKG_ENERGY_STATUS register
	uint64_t pkg_bits;

    // holds the bits previously stored in the MSR_DRAM_ENERGY_STATUS register 
	uint64_t old_dram_bits;
    // holds the bits currently stored in the MSR_DRAM_ENERGY_STATUS register
	uint64_t dram_bits;

    // this holds the previous energy value stored in MSR_PKG_ENERGY_STATUS register represented in joules
	double old_pkg_joules;
    // this holds the current energy value stored in MSR_PKG_ENERGY_STATUS register represented in joules
	double pkg_joules;

    // this holds the current energy value stored in MSR_DRAM_ENERGY_STATUS register represented in joules
	double old_dram_joules;
    // this holds the current energy value stored in MSR_DRAM_ENERGY_STATUS register represented in joules
	double dram_joules;

    // this holds the timestamp of the previous rapl data measurement
	struct timeval old_now;
    // this holds the timestamp of the current rapl data measurement
	struct timeval now;

    // this holds the amount of time elapsed between the two timestamps
	double elapsed;
    // this represents the change in energy for PKG between rapl data measurements
	double pkg_delta_joules;
    // this represents the change in power for PKG between rapl data measurements
	double pkg_watts;
    // this represents the change in energy for DRAM between rapl data measurements
	double dram_delta_joules;
    // this represents change in power for DRAM between rapl data measurements
	double dram_watts;


    // this does ?
	uint64_t flags;

    // DRAM
    // this is a count of how many times dram performance was capped due to imposed limits
    uint64_t dram_perf_count;

    uint64_t old_dram_perf;
    
    // PKG
    uint64_t pkg_perf_count;

    uint64_t old_pkg_perf;

    // PP
    uint64_t pp0_power_limit;

    uint64_t pp1_power_limit;

    uint64_t pp0_energy_status;

    uint64_t pp1_energy_status;

    uint64_t pp0_policy;

    uint64_t pp1_policy;

    uint64_t pp0_perf_status;

    uint64_t pp1_perf_status;
    
};

enum rapl_data_flags{
	RDF_REENTRANT	= 0x01,
	RDF_INIT        = 0x02
};

struct rapl_limit{
	double 		watts;		// User-friendly interface.
	double	 	seconds;
	uint64_t 	bits;		// User-unfriendly interface.
};


// We're going to overload this interface a bit...
//
// set_rapl_limit()
//
//	a) If a pointer is null, do nothing.
//
//	b) If the bit_vector is nonzero, translate the bit_vector to watts and seconds 
//	and write the bit vector to the msr.
//
//	c) If the bit_vector is zero, translate the watts and seconds to the appropriate
//	bit_vector and write the bit_vector to the msr.
//
// get_rapl_limit()
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

void set_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram );
void dump_rapl_limit( struct rapl_limit *L, FILE *w );

int read_rapl_data( const int socket, struct rapl_data *r );
void dump_rapl_data( struct rapl_data *r, FILE *w );

void dump_rapl_terse(FILE *w);
void dump_rapl_terse_label(FILE *w);
void dump_rapl_power_info(FILE *w);
#ifdef __cplusplus 
}
#endif

#endif /*MSR_RAPL_H*/
