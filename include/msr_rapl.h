/* msr_rapl.h
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
 * Modified by Scott Walker, walker91@llnl.gov
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

    // PKG
    // holds the bits previously stored in the MSR_PKG_ENERGY_STATUS register
	uint64_t old_pkg_bits;
    // holds the bits currently stored in the MSR_PKG_ENERGY_STATUS register
	uint64_t ** pkg_bits;

    // holds the bits previously stored in the MSR_DRAM_ENERGY_STATUS register 
	uint64_t old_dram_bits;
    // holds the bits currently stored in the MSR_DRAM_ENERGY_STATUS register
	uint64_t ** dram_bits;

    // this holds the previous energy value stored in MSR_PKG_ENERGY_STATUS register represented in joules
	double old_pkg_joules;
    // this holds the current energy value stored in MSR_PKG_ENERGY_STATUS register represented in joules
	double pkg_joules;

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

    uint64_t * pkg_perf_count; // pkg performance counter

    //uint64_t old_pkg_perf; // old pkg performance counter

    // this does ?
	uint64_t flags;

    // DRAM
    // this is a count of how many times dram performance was capped due to imposed limits
    uint64_t * dram_perf_count;

    // uint64_t old_dram_perf;
    
    // this represents the change in energy for DRAM between rapl data measurements
	double dram_delta_joules;
    // this represents change in power for DRAM between rapl data measurements
	double dram_watts;

    // this holds the current energy value stored in MSR_DRAM_ENERGY_STATUS register represented in joules
	double old_dram_joules;
    // this holds the current energy value stored in MSR_DRAM_ENERGY_STATUS register represented in joules
	double dram_joules;

    // PP0
    uint64_t ** pp0_bits;

    uint64_t old_pp0_bits;

    double pp0_joules;

    double old_pp0_joules;

    double pp0_delta_joules;

    uint64_t * pp0_policy;

    uint64_t * pp0_perf_count;

    double pp0_watts;

    // PP1
    uint64_t ** pp1_bits; // energy bits

    uint64_t old_pp1_bits; // old energy bits

    double  pp1_joules; // energy

    double old_pp1_joules; // old energy

    double pp1_delta_joules; // delta energy

    uint64_t * pp1_policy; // policy

    double pp1_watts;

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

int rapl_storage(struct rapl_data ** data, uint64_t ** flags);
int rapl_init(struct rapl_data ** rapl, uint64_t ** rapl_flags);
int rapl_finalize();

int set_pkg_rapl_limit(const unsigned socket, struct rapl_limit * limit1, struct rapl_limit * limit2);
int set_dram_rapl_limit(const unsigned socket, struct rapl_limit * limit);
int set_pp_rapl_limit(const unsigned socket, struct rapl_limit * limit0, struct rapl_limit * limit1);
int set_pp_rapl_policies(const unsigned socket, uint64_t * pp0, uint64_t * pp1);
int get_pp_rapl_policies(const unsigned socket, uint64_t * pp0, uint64_t * pp1);
int get_pkg_rapl_limit(const unsigned socket, struct rapl_limit * limit1, struct rapl_limit * limit2);
int get_dram_rapl_limit(const unsigned socket, struct rapl_limit * limit);
int get_pp_rapl_limit(const unsigned socket, struct rapl_limit * limit0, struct rapl_limit * limit1);
void dump_rapl_limit( struct rapl_limit *L, FILE *w );

int read_rapl_data(const unsigned socket);
int poll_rapl_data(const unsigned socket, struct rapl_data * result);
int delta_rapl_data(const unsigned socket, struct rapl_data * p, struct rapl_data * result);
int dump_rapl_data( struct rapl_data *r, FILE *w );

int dump_rapl_terse(FILE *w);
int dump_rapl_terse_label(FILE *w);
int dump_rapl_power_info(FILE *w);
#ifdef __cplusplus 
}
#endif

#endif /*MSR_RAPL_H*/
