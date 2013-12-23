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
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include "msr_core.h"
#include "msr_rapl.h"

/* 
 * Macros 
 */

/* MASK_RANGE
 * Create a mask from bit m to n. 
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(4,2) -->     (((1<<((4)-(2)+1))-1)<<(2)) 
 * 				     (((1<<          3)-1)<<(2))
 * 				     ((               4-1)<<(2))
 * 				     (                  3)<<(2))
 * 				     (                       24) = b11000
 */ 				 
#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))	// m>=n

/* MASK_VAL
 * Return the value of x after applying bitmask (m,n).
 * 63 >= m >= n >= 0
 * Example:  MASK_RANGE(17,4,2) --> 17&24 = b10001 & b11000 = b10000
 */
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

/* UNIT_SCALE 
 * Calculates x/(2^y).  
 * Example:  The RAPL interface measures power in units of 1/(2^p) watts,
 * where is expressed as bits 0:3 of the RAPL_POWER_UNIT MSR, with the
 * default value y=b011 (thus 1/8th of a Watt).  Reading a power values
 * of x=b10000 translates into UNIT_SCALE(x,y) Watts, and a power cap of 
 * x Watts translates to a representation of UNIT_DESCALE(x,y) units.
 *
 */
#define UNIT_SCALE(x,y) ((x)/(double)(1<<(y)))
#define UNIT_DESCALE(x,y) ((x)*(double)(1<<(y)))




// Section 35.7
// Table 35-11.  MSRs supported by Intel processors based on Intel 
// microarchitecture code name Sandy Bridge.
// Model/family 06_2A, 06_2D.
#if (USE_062A || USE_062D)
#define MSR_RAPL_POWER_UNIT 		(0x606)	// ro
#define MSR_PKG_POWER_LIMIT 		(0x610) // rw
#define MSR_PKG_ENERGY_STATUS 		(0x611) // ro sic;  MSR_PKG_ENERY_STATUS
#define MSR_PKG_POWER_INFO 		(0x614) // rw (text states ro)
#define MSR_PP0_POWER_LIMIT 		(0x638) // rw
#define MSR_PP0_ENERY_STATUS 		(0x639) // ro
#define MSR_PP0_POLICY 			(0x63A) // rw
#define MSR_PP0_PERF_STATUS 		(0x63B) // ro
#endif

// Section 35.7.1
// Table 35-12. MSRs supported by second generation Intel Core processors 
// (Intel microarchitecture Code Name Sandy Bridge)
// Model/family 06_2AH
#if (USE_062A)
#define MSR_PP1_POWER_LIMIT 		(0x640) // rw
#define MSR_PP1_ENERGY_STATUS 		(0x641)	// ro.  sic; MSR_PP1_ENERY_STATUS
#define MSR_PP1_POLICY 			(0x642) // rw
#endif

// Section 35.7.2
// Table 35-13. Selected MSRs supported by Intel Xeon processors E5 Family 
// (based on Intel Microarchitecture code name Sandy Bridge) 
// Model/family 06_2DH
#if (USE_062D)
#define MSR_PKG_PERF_STATUS 		(0x613) // ro
#define MSR_DRAM_POWER_LIMIT 		(0x618) // rw	
#define MSR_DRAM_ENERGY_STATUS 		(0x619)	// ro.  sic; MSR_DRAM_ENERY_STATUS
#define MSR_DRAM_PERF_STATUS 		(0x61B) // ro
#define MSR_DRAM_POWER_INFO 		(0x61C) // rw (text states ro)
#endif

// Section 35.8.1
// Table 35-15. Selected MSRs supported by Intel Xeon processors E5 Family v2 
// (based on Intel microarchitecture code name Ivy Bridge) 
// Model/family 06_3EH.  
// The Intel documentation only lists this table of msrs; this may be an error.
#if (USE_063E)
#define MSR_PKG_PERF_STATUS 		(0x613) //
#define MSR_DRAM_POWER_LIMIT 		(0x618) //
#define MSR_DRAM_ENERGY_STATUS 		(0x619)	// sic; MSR_DRAM_ENERY_STATUS
#define MSR_DRAM_PERF_STATUS 		(0x61B) //
#define MSR_DRAM_POWER_INFO 		(0x61C) //
#endif

enum{
	BITS_TO_WATTS,
	WATTS_TO_BITS,
	BITS_TO_SECONDS,
	SECONDS_TO_BITS,
	BITS_TO_JOULES,
	JOULES_TO_BITS,
	NUM_XLATE
};

struct rapl_units{
	uint64_t msr_rapl_power_unit;	// raw msr value
	double seconds;
	double joules;
	double watts;
};

static void
translate( const int socket, uint64_t* bits, double* units, int type ){
	static int initialized=0;
	static struct rapl_units ru[NUM_SOCKETS];
	int i;
	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_SOCKETS; i++){
			// See figure 14-16 for bit fields.
			//  1  1 1  1 1 
			//  9  6 5  2 1  8 7  4 3  0
			//
			//  1010 0001 0000 0000 0011
			//
			//     A    1    0    0    3
			//ru[i].msr_rapl_power_unit = 0xA1003;

			read_msr( i, MSR_RAPL_POWER_UNIT, &(ru[i].msr_rapl_power_unit) );
			// default is 1010b or 976 microseconds
			ru[i].seconds = 1.0/(double)( 1<<(MASK_VAL( ru[i].msr_rapl_power_unit, 19, 16 )));
			// default is 10000b or 15.3 microjoules
			ru[i].joules  = 1.0/(double)( 1<<(MASK_VAL( ru[i].msr_rapl_power_unit, 12,  8 )));
			// default is 0011b or 1/8 Watts
			ru[i].watts   = ((1.0)/((double)( 1<<(MASK_VAL( ru[i].msr_rapl_power_unit,  3,  0 )))));
		}	
	}
	switch(type){
		case BITS_TO_WATTS: 	*units = (double)(*bits)  * ru[socket].watts; 			break;
		case BITS_TO_SECONDS:	*units = (double)(*bits)  * ru[socket].seconds; 		break;
		case BITS_TO_JOULES:	*units = (double)(*bits)  * ru[socket].joules; 		break;
		case WATTS_TO_BITS:	*bits  = (uint64_t)(  (*units) / ru[socket].watts    ); 	break;
		case SECONDS_TO_BITS:	*bits  = (uint64_t)(  (*units) / ru[socket].seconds  ); 	break;
		case JOULES_TO_BITS:	*bits  = (uint64_t)(  (*units) / ru[socket].joules   ); 	break;
		default: 
			fprintf(stderr, "%s:%d  Unknown value %d.  This is bad.\n", __FILE__, __LINE__, type);  
			*bits = -1;
			*units= -1.0;
			break;
	}
}

struct rapl_power_info{
	uint64_t msr_pkg_power_info;	// raw msr values
	uint64_t msr_dram_power_info;

	double pkg_max_power;		// watts
	double pkg_min_power;
	double pkg_max_window;		// seconds
	double pkg_therm_power;		// watts

	double dram_max_power;		// watts
	double dram_min_power;
	double dram_max_window;		// seconds
	double dram_therm_power;	// watts
};

static void
rapl_get_power_info( const int socket, struct rapl_power_info *info){
	uint64_t val = 0;
	//info->msr_pkg_power_info  = 0x6845000148398;
	//info->msr_dram_power_info = 0x682d0001482d0;

	read_msr( socket, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info) );
	read_msr( socket, MSR_DRAM_POWER_INFO, &(info->msr_dram_power_info) );

	// Note that the same units are used in both the PKG and DRAM domains.
	
	val = MASK_VAL( info->msr_pkg_power_info,  53, 48 );
	translate( socket, &val, &(info->pkg_max_window), BITS_TO_SECONDS );
	
	val = MASK_VAL( info->msr_pkg_power_info,  46, 32 );
	translate( socket, &val, &(info->pkg_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  30, 16 );
	translate( socket, &val, &(info->pkg_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  14,  0 );
	translate( socket, &val, &(info->pkg_therm_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 53, 48 );
	translate( socket, &val, &(info->dram_max_window), BITS_TO_SECONDS );

	val = MASK_VAL( info->msr_dram_power_info, 46, 32 );
	translate( socket, &val, &(info->dram_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 30, 16 );
	translate( socket, &val, &(info->dram_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 14,  0 );
	translate( socket, &val, &(info->dram_therm_power), BITS_TO_WATTS );
}

static void
rapl_limit_calc(const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	static struct rapl_power_info rpi[NUM_SOCKETS];
	static int initialized=0;
	uint64_t watts_bits=0, seconds_bits=0;
	int i;
	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_SOCKETS; i++){
			rapl_get_power_info(i, &(rpi[i]));
		}
	}
	if(limit1){
		if (limit1->bits){
			// We have been given the bits to be written to the msr.
			// For sake of completeness, translate these into watts 
			// and seconds.
			watts_bits   = MASK_VAL( limit1->bits, 14,  0 );
			seconds_bits = MASK_VAL( limit1->bits, 23, 17 );

			translate( socket, &watts_bits, &limit1->watts, BITS_TO_WATTS );
			translate( socket, &seconds_bits, &limit1->seconds, BITS_TO_SECONDS );

		}else{
			// We have been given watts and seconds and need to translate
			// these into bit values.
			translate( socket, &watts_bits,   &limit1->watts,   WATTS_TO_BITS   );
			translate( socket, &seconds_bits, &limit1->seconds, SECONDS_TO_BITS );
			limit1->bits |= watts_bits   << 0;
			limit1->bits |= seconds_bits << 17;
		}
	}	
	if(limit2){
		if (limit2->bits){
			watts_bits   = MASK_VAL( limit2->bits, 46, 32 );
			seconds_bits = MASK_VAL( limit2->bits, 55, 49 );

			translate( socket, &watts_bits, &limit2->watts, BITS_TO_WATTS );
			translate( socket, &seconds_bits, &limit2->seconds, BITS_TO_SECONDS );

		}else{
			translate( socket, &watts_bits,   &limit2->watts,   WATTS_TO_BITS   );
			translate( socket, &seconds_bits, &limit2->seconds, SECONDS_TO_BITS );
			limit2->bits |= watts_bits   << 32;
			limit2->bits |= seconds_bits << 49;
		}
	}
	if(dram){
		if (dram->bits){
			// We have been given the bits to be written to the msr.
			// For sake of completeness, translate these into watts 
			// and seconds.
			watts_bits   = MASK_VAL( dram->bits, 14,  0 );
			seconds_bits = MASK_VAL( dram->bits, 23, 17 );

			translate( socket, &watts_bits, &dram->watts, BITS_TO_WATTS );
			translate( socket, &seconds_bits, &dram->seconds, BITS_TO_SECONDS );

		}else{
			// We have been given watts and seconds and need to translate
			// these into bit values.
			translate( socket, &watts_bits,   &dram->watts,   WATTS_TO_BITS   );
			translate( socket, &seconds_bits, &dram->seconds, SECONDS_TO_BITS );
			dram->bits |= watts_bits   << 0;
			dram->bits |= seconds_bits << 17;
		}
	}
}

void
rapl_dump_limit( struct rapl_limit* L ){
	fprintf(stdout, "bits    = %lx\n", L->bits);
	fprintf(stdout, "seconds = %lf\n", L->seconds);
	fprintf(stdout, "watts   = %lf\n", L->watts);
	fprintf(stdout, "\n");
}

void 
rapl_set_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	// Fill in whatever values are necessary.
	uint64_t pkg_limit=0;
	uint64_t dram_limit=0;

	rapl_limit_calc( socket, limit1, limit2, dram );

	if(limit1){
		pkg_limit |= limit1->bits | (1LL << 15) | (1LL << 16);	// enable clamping
	}
	if(limit2){
		pkg_limit |= limit2->bits | (1LL << 47) | (1LL << 48);	// enable clamping
	}
	if(limit1 || limit2){
		write_msr( socket, MSR_PKG_POWER_LIMIT, pkg_limit );
	}
	if(dram){
		dram_limit |= dram->bits | (1LL << 15) | (1LL << 16);	// enable clamping
		write_msr( socket, MSR_DRAM_POWER_LIMIT, dram_limit );
	}
}

void 
rapl_get_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	if(limit1){
		read_msr( socket, MSR_PKG_POWER_LIMIT, &(limit1->bits) );
	}
	if(limit2){
		read_msr( socket, MSR_PKG_POWER_LIMIT, &(limit2->bits) );
	}
	if(dram){
		read_msr( socket, MSR_DRAM_POWER_LIMIT, &(dram->bits) );
	}
	// Fill in whatever values are necessary.
	rapl_limit_calc( socket, limit1, limit2, dram );
}

void 
rapl_dump_data( struct rapl_data *r ){
	// Don't bother printing if joules < 0.01 or elapsed < 0.001.
	static int initialized=0;
	static struct timeval start;
	struct timeval now;
	if(!initialized){
		initialized=1;
		gettimeofday( &start, NULL );
	}
	gettimeofday( &now, NULL );
	if(r->pkg_joules > 0.01 && r->elapsed > 0.0050){
		fprintf(stdout, "pkg_watts= %8.4lf   elapsed= %8.5lf   timestamp= %9.6lf\n", 
				r->pkg_watts,
				r->elapsed,
				now.tv_sec - start.tv_sec + (now.tv_usec - start.tv_usec)/1000000.0
				);
	}
}

void
rapl_read_data( const int socket, struct rapl_data *r ){
	static double pkg_joules[NUM_SOCKETS] = {0.0};  
	static double old_pkg_joules[NUM_SOCKETS] = {0.0}; 
	static double dram_joules[NUM_SOCKETS] = {0.0}; 
	static double old_dram_joules[NUM_SOCKETS] = {0.0};
	static uint64_t pkg_bits[NUM_SOCKETS]; 
	static uint64_t dram_bits[NUM_SOCKETS];
	static uint64_t old_pkg_bits[NUM_SOCKETS]; 
	static uint64_t old_dram_bits[NUM_SOCKETS];
	static struct timeval old_now[NUM_SOCKETS];
	static struct timeval now[NUM_SOCKETS];
	uint64_t  maxbits=4294967296;
	double max_joules=0.0;

	// Copy previous now timestamp to old_now.
	old_now[socket].tv_sec  = now[socket].tv_sec;
	old_now[socket].tv_usec = now[socket].tv_usec;

	// Copy previous raw msr values into the old buckets.	
	old_pkg_joules[socket]  = pkg_joules[socket];
	old_dram_joules[socket] = dram_joules[socket];

	// Copy off old bits (only need this for debugging)
	old_pkg_bits[socket] = pkg_bits[socket];
	old_dram_bits[socket] = dram_bits[socket];

	// Get current timestamp
	gettimeofday( &(now[socket]), NULL );

	// Get raw joules
	read_msr( socket, MSR_PKG_ENERGY_STATUS,  &pkg_bits[socket]  );
	read_msr( socket, MSR_DRAM_ENERGY_STATUS, &dram_bits[socket] );
	
	// get normalized joules
	translate( socket, &pkg_bits[socket],  &(pkg_joules[socket]),  BITS_TO_JOULES );
	translate( socket, &dram_bits[socket], &(dram_joules[socket]), BITS_TO_JOULES );
	
	// Fill in the struct if present.
	if(r){
		// Get delta in seconds
		r->elapsed = (now[socket].tv_sec - old_now[socket].tv_sec) 
			     +
			     (now[socket].tv_usec - old_now[socket].tv_usec)/1000000.0;

		// Get delta joules.
		// Now handles wraparound.
		if(pkg_joules [socket] - old_pkg_joules[socket] < 0)
		{
			translate(socket,&maxbits,&max_joules, BITS_TO_JOULES); 
			r->pkg_joules = (pkg_joules[socket] + max_joules) - old_pkg_joules[socket];
		}
		else
		{
			r->pkg_joules  = pkg_joules[socket]  - old_pkg_joules[socket];		
		}

		if(dram_joules [socket] - old_dram_joules[socket] < 0)
		{
			translate(socket,&maxbits,&max_joules, BITS_TO_JOULES); 
			r->dram_joules = (dram_joules[socket] + max_joules) - old_dram_joules[socket];
		}
		else
		{
			r->dram_joules = dram_joules[socket] - old_dram_joules[socket];	
		}	

		// Get watts.
		// Does not check for div by 0.
		r->pkg_watts  = r->pkg_joules  / r->elapsed;
		r->dram_watts = r->dram_joules / r->elapsed;

		// Save off bits for debugging.
		r->old_pkg_bits = old_pkg_bits[socket];
		r->old_dram_bits = old_dram_bits[socket];

		r->pkg_bits = pkg_bits[socket];
		r->dram_bits = dram_bits[socket];
	}
}

