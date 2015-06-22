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
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "msr_core.h"
#include "cpuid.h"
#include "msr_rapl.h"
#define LIBMSR_DEBUG 1
#define RAPL_USE_DRAM 1

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

// Processor specific rapl flags (see rapl_init function)
#define MF_06_37
#define MF_06_4A
#define MF_06_5A
#define MF_06_4D
#define MF_06_4C
#define MF_06_2A // note tbl 16 and 17
#define MF_06_2D // note tbl 16 and 18
#define MF_06_3A
#define MF_06_3E
#define MF_06_3C
#define MF_06_45
#define MF_06_46
#define MF_06_3F
#define MF_06_3D
#define MF_06_47
#define MF_06_4F
#define MF_06_56
#define MF_06_4E
#define MF_06_5E
#define MF_06_57

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

// rapl flags
// 0: 606h, MSR_RAPL_POWER_UNIT
// 1: 610h, MSR_PKG_POWER_LIMIT
// 2: 611h, MSR_PKG_ENERGY_STATUS
// 3: 613h, MSR_PKG_PERF_STATUS
// 4: 614h, MSR_PKG_POWER_INFO
// 5: 618h, MSR_DRAM_POWER_LIMIT
// 6: 619h, MSR_DRAM_ENERGY_STATUS
// 7: 61Bh, MSR_DRAM_PERF_STATUS
// 8: 61Ch, MSR_DRAM_POWER_INFO
// 9: 638h, MSR_PP0_POWER_LIMIT
// 10: 639h, MSR_PP0_ENERGY_STATUS
// 11: 63Ah, MSR_PP0_POLICY
// 12: 63Bh, MSR_PP0_PERF_STATUS
// 13: 640h, MSR_PP1_POWER_LIMIT
// 14: 641h, MSR_PP1_ENERGY_STATUS
// 15: 642h, MSR_PP1_POLICY
// 16: 64Ch, ?
// 17: 690h, ?
int rapl_init(struct rapl_data ** rapl, uint64_t * rapl_flags)
{
    static int initialized = 0;
    uint64_t model;

    cpuid_get_model(&model);

    if (!initialized)
    {
        *rapl = (struct rapl_data *) calloc(NUM_SOCKETS, sizeof(struct rapl_data));
        if (!*rapl)
        {
            fprintf(stderr, "%s %s::%d ERROR: unable to allocate memory\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: rapl initialized at %p\n", *rapl);
#endif
    return 0;
}

int rapl_finalize(struct rapl_data ** rapl)
{
    free(*rapl);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: rapl finalized\n");
#endif
    return 0;
}

static void
translate( const int socket, uint64_t* bits, double* units, int type ){
	static int initialized=0;
	static struct rapl_units ru[NUM_SOCKETS];
	uint64_t val[NUM_SOCKETS];
	int i;
	if(!initialized){
		initialized=1;
        // why does this have to read all of them to translate one unit?
		read_all_sockets( MSR_RAPL_POWER_UNIT, val );
		for(i=0; i<NUM_SOCKETS; i++){
			// See figure 14-16 for bit fields.
			//  1  1 1  1 1 
			//  9  6 5  2 1  8 7  4 3  0
			//
			//  1010 0001 0000 0000 0011
			//
			//     A    1    0    0    3
			//ru[i].msr_rapl_power_unit = 0xA1003;

			ru[i].msr_rapl_power_unit = val[i];
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

	double pkg_max_power;		// watts
	double pkg_min_power;
	double pkg_max_window;		// seconds
	double pkg_therm_power;		// watts

#ifdef RAPL_USE_DRAM
	uint64_t msr_dram_power_info;
	double dram_max_power;		// watts
	double dram_min_power;
	double dram_max_window;		// seconds
	double dram_therm_power;	// watts
#endif
};

static void
get_rapl_power_info( const int socket, struct rapl_power_info *info){
	uint64_t val = 0;
	//info->msr_pkg_power_info  = 0x6845000148398;
	//info->msr_dram_power_info = 0x682d0001482d0;

	read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info) );
#ifdef RAPL_USE_DRAM
	read_msr_by_coord( socket, 0, 0, MSR_DRAM_POWER_INFO, &(info->msr_dram_power_info) );
#endif

	// Note that the same units are used in both the PKG and DRAM domains.
	
	val = MASK_VAL( info->msr_pkg_power_info,  53, 48 );
	translate( socket, &val, &(info->pkg_max_window), BITS_TO_SECONDS );
	
	val = MASK_VAL( info->msr_pkg_power_info,  46, 32 );
	translate( socket, &val, &(info->pkg_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  30, 16 );
	translate( socket, &val, &(info->pkg_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  14,  0 );
	translate( socket, &val, &(info->pkg_therm_power), BITS_TO_WATTS );
#ifdef RAPL_USE_DRAM
	val = MASK_VAL( info->msr_dram_power_info, 53, 48 );
	translate( socket, &val, &(info->dram_max_window), BITS_TO_SECONDS );

	val = MASK_VAL( info->msr_dram_power_info, 46, 32 );
	translate( socket, &val, &(info->dram_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 30, 16 );
	translate( socket, &val, &(info->dram_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 14,  0 );
	translate( socket, &val, &(info->dram_therm_power), BITS_TO_WATTS );
#endif
}

// TODO: fix this function
static int check_power_limit(uint64_t * watts_bits, char limit)
{
    uint64_t power_limit_check_mask = 0;
    int init = 0, i;
    static struct rapl_power_info rpi[NUM_SOCKETS];

    if (!init)
    {
        init = 1;
        for (i = 0; i < NUM_SOCKETS; i++)
        {
            get_rapl_power_info(i, &(rpi[NUM_SOCKETS]));
        }
    }

    // set the mask to upper or lower 32 bits of register
    if (limit == '2')
    {
        power_limit_check_mask = 0xFFFF8000FFFFFFF;
        power_max <<= 32;
        power_min <<= 32;
    }
    else
    {
        power_limit_check_mask = 0xFFFFFFFFFFFF8000;
    }

/* WAIT TO DO THIS -> once translate is fixed
    if (*watts_bits & ~power_max)
    {
        // user watts setting is over the processor maximum specification
        *watts_bits = power_max;
        fprintf(stderr, "%s %s::%d WARNING: RAPL power limit %c is over maximum. Reverting to max value\n",
                getenv("HOSTNAME"), __FILE__, __LINE__, limit);
    }
    else if (*watts_bits & ~power_min)
    {
        // user watts setting is below the processor minimum specification
        *watts_bits = power_min;
        fprintf(stderr, "%s %s::%d WARNING: RAPL power limit %c is below minimum. Reverting to min value\n",
                getenv("HOSTNAME"), __FILE__, __LINE__, limit);
    }
*/

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: watts bits is %lx\n", *watts_bits & power_limit_check_mask);
#endif
    if (*watts_bits & power_limit_check_mask)
    {
        // incorrect bits were flipped, use the complement of the mask to AND them off
        *watts_bits &= ~power_limit_check_mask;
        fprintf(stderr, "%s %s::%d ERROR: RAPL power limit %c out of bounds,\n", 
                getenv("HOSTNAME"), __FILE__, __LINE__, limit);
        return -1;
    }

    return 0;
}

// TODO: fix this function
static int check_time_limit(uint64_t * seconds_bits, char limit)
{
    uint64_t time_limit_check_mask = 0;
    uint64_t time_max = MASK_VAL(pkgmax, 53, 47) << 17;
    // set the mask to upper or lower 32 bits of register
    if (limit == '2')
    {
        time_limit_check_mask = 0xFF01FFFFFFFFFFFF;
        time_max <<= 32;
    }
    else
    {
        time_limit_check_mask = 0xFFFFFFFFFF01FFFF;
    }

/* WAIT TO DO THIS -> once translate is fixed
    if (*seconds_bits & ~time_max)
    {
        // user time window is above maximum sensible time frame
        *seconds_bits = time_max;
        fprintf(stderr, "%s %s::%d WARNING: RAPL time frame %c is over maximum. Reverting to max value\n",
                getenv("HOSTNAME"), __FILE__, __LINE__, limit);
    }
*/

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: seconds bits is %lx\n", *seconds_bits & time_limit_check_mask);
#endif
    if (*seconds_bits & time_limit_check_mask)
    {
        // incorrect bits were flipped, use the complement of the mask to AND them off
        *seconds_bits &= ~time_limit_check_mask;
        fprintf(stderr, "%s %s::%d ERROR: RAPL time frame %c out of bounds.\n", 
                getenv("HOSTNAME"), __FILE__, __LINE__, limit);
        return -1;
    }
    return 0;
}

// TODO: what is this here for?
static int read_pkg_power_limit(uint64_t * val)
{
    return read_msr_by_coord(0, 0, 0, MSR_PKG_POWER_INFO, val);
}

static void
calc_rapl_limit(const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	static struct rapl_power_info rpi[NUM_SOCKETS];
	static int initialized=0;
	uint64_t watts_bits=0, seconds_bits=0;
	int i;

	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_SOCKETS; i++){
			get_rapl_power_info(i, &(rpi[i]));
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
#ifdef LIBMSR_DEBUG
			fprintf(stderr, "Converted %lf watts into %lx bits.\n", limit1->watts, watts_bits);
			fprintf(stderr, "Converted %lf seconds into %lx bits.\n", limit1->seconds, seconds_bits);
#endif
            watts_bits <<= 0;
            check_power_limit(&watts_bits, '1');
            seconds_bits <<= 17;
            check_time_limit(&seconds_bits, '1');
			limit1->bits |= watts_bits;
			limit1->bits |= seconds_bits;
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
            watts_bits <<= 32;
            check_power_limit(&watts_bits, '2');
            seconds_bits <<= 49;
            check_time_limit(&seconds_bits, '2');
			limit2->bits |= watts_bits;
			limit2->bits |= seconds_bits;
		}
	}

#ifdef RAPL_USE_DRAM
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
            watts_bits <<= 0;
            check_power_limit(&watts_bits, 'D');
            seconds_bits <<= 17;
            check_time_limit(&seconds_bits, 'D');
			dram->bits |= watts_bits;
			dram->bits |= seconds_bits;
		}
	}
#endif
}

void
dump_rapl_limit( struct rapl_limit* L, FILE *writeFile ){
	fprintf(writeFile, "bits    = %lx\n", L->bits);
	fprintf(writeFile, "seconds = %lf\n", L->seconds);
	fprintf(writeFile, "watts   = %lf\n", L->watts);
	fprintf(writeFile, "\n");
}

void 
set_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	// Fill in whatever values are necessary.
	uint64_t pkg_limit=0;
#ifdef RAPL_USE_DRAM
	uint64_t dram_limit=0;
#endif
    
	calc_rapl_limit( socket, limit1, limit2, dram );

    // for these: why do 1 << 15 | 1 << 16 when you can just do 3 << 15?
	if(limit1){
		pkg_limit |= limit1->bits | (1LL << 15) | (1LL << 16);	// enable clamping
	}
	if(limit2){
		pkg_limit |= limit2->bits | (1LL << 47) | (1LL << 48);	// enable clamping
	}
	if(limit1 || limit2){
		write_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_LIMIT, pkg_limit );
	}
	if(dram){
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: Modifying DRAM values. DRAM bits are %lx\n", getenv("HOSTNAME"), __FILE__, __LINE__, dram->bits);
#endif
        // why was this setting bit 16? thats reserved -> caused i/o error.
		dram_limit |= dram->bits | (1LL << 15); // | (1LL << 16);	// enable clamping
        fprintf(stdout, "OUTPUT: dram_limit is %lx\n", dram_limit);
		write_msr_by_coord( socket, 0, 0, MSR_DRAM_POWER_LIMIT, dram_limit );
	}
}

void 
get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	assert(socket < NUM_SOCKETS);
	assert(socket >=0 );
	if(limit1){
		read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit1->bits) );
	}
	if(limit2){
		read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit2->bits) );
	}
#ifdef RAPL_USE_DRAM
	if(dram){
		read_msr_by_coord( socket, 0, 0, MSR_DRAM_POWER_LIMIT, &(dram->bits) );
	}
#endif
	// Fill in whatever values are necessary.
	calc_rapl_limit( socket, limit1, limit2, dram );
}

void
dump_rapl_terse_label( FILE *writeFile ){
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++){
		fprintf(writeFile,"pkgW%02d dramW%02d ", socket, socket );
	}
}

void
dump_rapl_terse( FILE * writeFile ){
	int socket;
	struct rapl_data r;
	r.flags=0;

#ifdef LIBMSR_DEBUG
        fprintf(writeFile, "%s %s::%d Writing terse label\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    // TODO:  based on how read_rapl_data works this seems wrong
	for(socket=0; socket<NUM_SOCKETS; socket++){
		read_rapl_data_old(socket, &r);
		fprintf(writeFile,"%8.4lf %8.4lf ", r.pkg_watts, r.dram_watts);
	}
}

void 
dump_rapl_data( struct rapl_data *r, FILE *writeFile ){
	static int initialized=0;
	static struct timeval start;
	struct timeval now;
	if(!initialized){
		initialized=1;
		gettimeofday( &start, NULL );
	}
#ifdef LIBMSR_DEBUG
    fprintf(writeFile, "At address %p\n", r);
    fprintf(writeFile, "pkg_bits = %8.4lx   pkg_joules= %8.4lf\n", r->pkg_bits, r->pkg_joules);
#endif
	gettimeofday( &now, NULL );
	fprintf(writeFile, "pkg_watts= %8.4lf   elapsed= %8.5lf   timestamp= %9.6lf\n", 
			r->pkg_watts,
			r->elapsed,
			now.tv_sec - start.tv_sec + (now.tv_usec - start.tv_usec)/1000000.0
			);
	fprintf(writeFile, "dram_watts= %8.4lf   elapsed= %8.5lf   timestamp= %9.6lf\n", 
			r->dram_watts,
			r->elapsed,
			now.tv_sec - start.tv_sec + (now.tv_usec - start.tv_usec)/1000000.0
			);
}

void
dump_rapl_power_info( FILE *writeFile){
	int socket;
	struct rapl_power_info info;
	for(socket = 0; socket < NUM_SOCKETS; socket++)
	{
		get_rapl_power_info(socket, &info);
		fprintf(writeFile, "Socket= %d pkg_max_power= %8.4lf pkg_min_power=%8.4lf pkg_max_window=%8.4lf pkg_therm_power=%8.4lf\n",
				socket,
				info.pkg_max_power,
				info.pkg_min_power,
				info.pkg_max_window,
				info.pkg_therm_power);
#ifdef RAPL_USE_DRAM

		fprintf(writeFile, "Socket= %d dram_max_power= %8.4lf dram_min_power=%8.4lf dram_max_window=%8.4lf dram_therm_power=%8.4lf\n",
				socket,
				info.dram_max_power,
				info.dram_min_power,
				info.dram_max_window,
				info.dram_therm_power);
#endif
	}
}

// TODO: does not work, fix later: might defer to end user
int rapl_delta_point(const int socket, struct rapl_data ** rapl, struct rapl_data * datapoint)
{
    struct rapl_data * p;

    if (!*rapl)
    {
        fprintf(stderr, "%s %s::%d ERROR: rapl init has either failed or has not been called\n", 
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    p = &((*rapl)[socket]);

    // TODO: handle wraparound
    datapoint->old_pkg_bits = datapoint->pkg_bits;
    datapoint->old_dram_bits = datapoint->dram_bits;
    datapoint->pkg_bits = p->pkg_bits;
    datapoint->dram_bits = p->dram_bits;

    delta_rapl_data(datapoint, NULL);

    
    return 0;
}

int poll_rapl_data(const int socket, struct rapl_data ** rapl, struct rapl_data * result)
{
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (poll_rapl_data) socket=%d\n", getenv("HOSTNAME"), __FILE__, __LINE__, socket);
#endif
    struct rapl_data * p;
    if (!*rapl)
    {
        fprintf(stderr, "%s %s::%d ERROR: rapl init has either failed or has not been called\n", 
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    p = &((*rapl)[socket]);
    read_rapl_data(socket, rapl); //, p);
    delta_rapl_data(socket, p, result);
    return 0;
}

// TODO: should probably be static
int delta_rapl_data(const int socket, struct rapl_data * p, struct rapl_data * result)
{
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d (delta_rapl_data)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    // where did that number come from? -> max possible energy value?
	uint64_t  maxbits=4294967296;
    // why is this zero? does not seem right
	double max_joules=0.0;

    p->elapsed = (p->now.tv_sec - p->old_now.tv_sec) 
             +
             (p->now.tv_usec - p->old_now.tv_usec)/1000000.0;
    // Get delta joules.
    // Now handles wraparound.
    if(p->pkg_joules - p->old_pkg_joules < 0)
    {
        translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
        p->pkg_delta_joules = ( p->pkg_joules + max_joules) - p->old_pkg_joules;
    } else {
        p->pkg_delta_joules  = p->pkg_joules  - p->old_pkg_joules;		
    }
#ifdef RAPL_USE_DRAM
    if(p->dram_joules - p->old_dram_joules < 0)
    {
        translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
        p->dram_delta_joules = (p->dram_joules + max_joules) - p->old_dram_joules;
    } else {
        p->dram_delta_joules = p->dram_joules - p->old_dram_joules;	
    }	
#endif
    // Get watts.
    assert(p->elapsed != 0.0);
    if(p->elapsed > 0.0){
        p->pkg_watts  = p->pkg_delta_joules  / p->elapsed;
#ifdef RAPL_USE_DRAM
        p->dram_watts = p->dram_delta_joules / p->elapsed;
#endif
    }else{
        p->pkg_watts  = -999.0;
#ifdef RAPL_USE_DRAM
        p->dram_watts = -999.0;
#endif
    }
    if (result)
    {
        result = p;
    }
    return 0;
}

// TODO: should probably be static
int read_rapl_data(const int socket, struct rapl_data ** rapl)
{
    struct rapl_data * p;
    p = &((*rapl)[socket]);

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d (read_rapl_data): socket=%d at address %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, socket, p);
#endif

    // Move current variables to "old" variables.
    p->old_pkg_bits		= p->pkg_bits;
    p->old_pkg_joules	= p->pkg_joules;
    p->old_now.tv_sec 	= p->now.tv_sec;
    p->old_now.tv_usec	= p->now.tv_usec;
    p->old_dram_perf    = p->dram_perf_count;

    // grab a timestamp
	gettimeofday( &(p->now), NULL );

    // Get raw joules
    if(read_msr_by_coord( socket, 0, 0, MSR_PKG_ENERGY_STATUS,  &(p->pkg_bits)  ))
    {
        return -1;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: pkg_bits %lx\n", p->pkg_bits);
#endif
    translate( socket, &(p->pkg_bits), &(p->pkg_joules), BITS_TO_JOULES );
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: pkg_joules %lf\n", p->pkg_joules);
    fprintf(stderr, "DEBUG: pkg_watts %lf\n", p->pkg_watts);
#endif

    /* read PP registers
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_POWER_LIMIT, &(p->pp0_power_limit)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_ENERGY_STATUS, &(p->pp0_energy_status)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_POLICY, &(p->pp0_policy)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_PERF_STATUS, &(p->pp0_perf_status)))
    {
        return -1;
    }
    */
    
#ifdef RAPL_USE_DRAM
    p->old_dram_bits	= p->dram_bits;
    p->old_dram_joules	= p->dram_joules;
    if(read_msr_by_coord( socket, 0, 0, MSR_DRAM_ENERGY_STATUS, &(p->dram_bits) ))
    {
        return -1;
    }
    translate( socket, &(p->dram_bits), &(p->dram_joules), BITS_TO_JOULES );
    if (read_msr_by_coord( socket, 0, 0, MSR_DRAM_PERF_STATUS, &(p->dram_perf_count)))
    {
        return -1;
    }
#endif
    return 0;
}

int
read_rapl_data_old( const int socket, struct rapl_data *r ){
/*
	 * If r is null, measurments are recorded into the local static struct s.           // poll rapl data
	 *
	 * If r is not null and r->flags == 0, measurements are also recorded               // read rapl data(null)
	 *   into the local static struct s and the delta is recorded in r.
	 *
	 * If r is not null and r->flags & RDF_REENTRANT, measurements are recorded in      // read rapl data(result)
	 *   r only and the delta is calculated relative to the difference in 
	 *   the values passed in with r.  old_* values are destroyed and * values
	 *   are moved to old_* values.  
	 *
	 * If r is not null and r->flags & RDF_INITIALIZE, no calculations are 
	 *   performed and deltas are set to zero.  This is intented as a convenience
	 *   function only; there is no harm in initializing without this flag and
	 *   simply ignoring the results.
	 *
	 * This functionality allows for the case where the user wishes to, e.g., 
	 *   take measurments at points A, B, C, C', B', A' using three separate
	 *   rapl_data structs.
*/
    fprintf(stderr, "\nREADING RAPL DATA\n");
    // previous state for calculating the delta?
	struct rapl_data *p;	// Where the previous state lives.
    // where did that number come from? -> max possible energy value?
	uint64_t  maxbits=4294967296;
    // why is this zero? does not seem right
	double max_joules=0.0;
	static struct rapl_data s[NUM_SOCKETS];

    // If r is null we store data locally, otherwise we put it in r. What is the point of only having it locally?
	if( (r == NULL) || !(r->flags & RDF_REENTRANT) ){
		p = &s[socket];
	} else{
		p = r;
	}


    // why doesn't this zero out old data
	// Zero out values on init.
	if(p->flags & RDF_INIT){
		p->pkg_bits=0;
		p->pkg_joules=0.0;
		p->now.tv_sec=0;
		p->now.tv_usec=0;
		p->pkg_delta_joules=0.0;
	}
	
	// Move current variables to "old" variables.
	p->old_pkg_bits		= p->pkg_bits;
	p->old_pkg_joules	= p->pkg_joules;
	p->old_now.tv_sec 	= p->now.tv_sec;
	p->old_now.tv_usec	= p->now.tv_usec;
    p->old_dram_perf    = p->dram_perf_count;

	// Get current timestamp
	gettimeofday( &(p->now), NULL );

	// Get raw joules
	if(read_msr_by_coord( socket, 0, 0, MSR_PKG_ENERGY_STATUS,  &(p->pkg_bits)  ))
    {
        return -1;
    }
	translate( socket, &(p->pkg_bits),  &(p->pkg_joules),  BITS_TO_JOULES );

    /* read PP registers
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_POWER_LIMIT, &(p->pp0_power_limit)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_ENERGY_STATUS, &(p->pp0_energy_status)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_POLICY, &(p->pp0_policy)))
    {
        return -1;
    }
    if(read_msr_by_coord(socket, 0, 0, MSR_PPO_PERF_STATUS, &(p->pp0_perf_status)))
    {
        return -1;
    }
    */
	
#ifdef RAPL_USE_DRAM
	p->old_dram_bits	= p->dram_bits;
	p->old_dram_joules	= p->dram_joules;
	if(read_msr_by_coord( socket, 0, 0, MSR_DRAM_ENERGY_STATUS, &(p->dram_bits) ))
    {
        return -1;
    }
	translate( socket, &(p->dram_bits), &(p->dram_joules), BITS_TO_JOULES );
    if (read_msr_by_coord( socket, 0, 0, MSR_DRAM_PERF_STATUS, &(p->dram_perf_count)))
    {
        return -1;
    }
#endif
	
	// Fill in the struct if present.
	if(r && !(r->flags & RDF_INIT)){
		// Get delta in seconds
		r->elapsed = (p->now.tv_sec - p->old_now.tv_sec) 
			     +
			     (p->now.tv_usec - p->old_now.tv_usec)/1000000.0;

		// Get delta joules.
		// Now handles wraparound.
		if(p->pkg_joules - p->old_pkg_joules < 0)
		{
			translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
			r->pkg_delta_joules = ( p->pkg_joules + max_joules) - p->old_pkg_joules;
		} else {
			r->pkg_delta_joules  = p->pkg_joules  - p->old_pkg_joules;		
		}

#ifdef RAPL_USE_DRAM
		if(p->dram_joules - p->old_dram_joules < 0)
		{
			translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
			r->dram_delta_joules = (p->dram_joules + max_joules) - p->old_dram_joules;
		} else {
			r->dram_delta_joules = p->dram_joules - p->old_dram_joules;	
		}	
#endif

		// Get watts.
        assert(r->elapsed != 0.0);
		if(r->elapsed > 0.0){
			r->pkg_watts  = r->pkg_delta_joules  / r->elapsed;
#ifdef RAPL_USE_DRAM
			r->dram_watts = r->dram_delta_joules / r->elapsed;
#endif
		}else{
			r->pkg_watts  = -999.0;
#ifdef RAPL_USE_DRAM
			r->dram_watts = -999.0;
#endif
		}
	}
    return 0;
}

