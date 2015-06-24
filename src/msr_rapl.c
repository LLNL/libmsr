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
#include <math.h>
#include <tgmath.h>
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
//#if (USE_062A || USE_062D)
#define MSR_RAPL_POWER_UNIT 		(0x606)	// ro
#define MSR_PKG_POWER_LIMIT 		(0x610) // rw
#define MSR_PKG_ENERGY_STATUS 		(0x611) // ro sic;  MSR_PKG_ENERY_STATUS
#define MSR_PKG_POWER_INFO 		(0x614) // rw (text states ro)
#define MSR_PP0_POWER_LIMIT 		(0x638) // rw
#define MSR_PP0_ENERGY_STATUS 		(0x639) // ro
#define MSR_PP0_POLICY 			(0x63A) // rw
#define MSR_PP0_PERF_STATUS 		(0x63B) // ro
//#endif

// Section 35.7.1
// Table 35-12. MSRs supported by second generation Intel Core processors 
// (Intel microarchitecture Code Name Sandy Bridge)
// Model/family 06_2AH
//#if (USE_062A)
#define MSR_PP1_POWER_LIMIT 		(0x640) // rw
#define MSR_PP1_ENERGY_STATUS 		(0x641)	// ro.  sic; MSR_PP1_ENERY_STATUS
#define MSR_PP1_POLICY 			(0x642) // rw
//#endif

// Section 35.7.2
// Table 35-13. Selected MSRs supported by Intel Xeon processors E5 Family 
// (based on Intel Microarchitecture code name Sandy Bridge) 
// Model/family 06_2DH
//#if (USE_062D)
#define MSR_PKG_PERF_STATUS 		(0x613) // ro
#define MSR_DRAM_POWER_LIMIT 		(0x618) // rw	
#define MSR_DRAM_ENERGY_STATUS 		(0x619)	// ro.  sic; MSR_DRAM_ENERY_STATUS
#define MSR_DRAM_PERF_STATUS 		(0x61B) // ro
#define MSR_DRAM_POWER_INFO 		(0x61C) // rw (text states ro)
//#endif

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
// These indicate which rapl MSR's are available for a given model
#define MF_06_37 (0x407)
#define MF_06_4A (0x407)
#define MF_06_5A (0x407)
#define MF_06_4D (0x20003)
#define MF_06_4C (0x607)
#define MF_06_2A (0xFE17) 
#define MF_06_2D (0x7FF)
#define MF_06_3A (0xFE17)
#define MF_06_3E (0x7FF)
#define MF_06_3C (0x1CFE17)
#define MF_06_45 (0x1CFE17)
#define MF_06_46 (0x1CFE17)
#define MF_06_3F (0x1CE7FF)
#define MF_06_3D (0x1CFE17)
#define MF_06_47 (0x1CFE17)
#define MF_06_4F (0x1CFE17)
#define MF_06_56 (0x1CFE17)
#define MF_06_4E (0x1EFE17)
#define MF_06_5E (0x1EFE17)
#define MF_06_57 (0x507FF)

enum{
	BITS_TO_WATTS,
	WATTS_TO_BITS,
	BITS_TO_SECONDS_STD,
	SECONDS_TO_BITS_STD,
	BITS_TO_JOULES,
	JOULES_TO_BITS,
	NUM_XLATE,
    BITS_TO_SECONDS_DRAM,
    SECONDS_TO_BITS_DRAM
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
// 17: 66Eh, pkg power info?
// 18: 690h, ?
// 19: 6B0h, ?
// 20: 6b1h, ?
int rapl_init(struct rapl_data ** rapl, uint64_t * rapl_flags)
{
    static int initialized = 0;
    uint64_t model = 0;

    cpuid_get_model(&model);

    switch(model)
    {
        case 0x37:
            *rapl_flags = MF_06_37;
            break;
        case 0x4A:
            *rapl_flags = MF_06_4A;
            break;
        case 0x5A:
            *rapl_flags = MF_06_5A;
            break;
        case 0x4D:
            *rapl_flags = MF_06_4D;
            break;
        case 0x4C:
            *rapl_flags = MF_06_4C;
            break;
        case 0x2A:
            *rapl_flags = MF_06_2A;
            break;
        case 0x2D:
            *rapl_flags = MF_06_2D;
            break;
        case 0x3A:
            *rapl_flags = MF_06_3A;
            break;
        case 0x3E:
            *rapl_flags = MF_06_3E;
            break;
        case 0x3C:
            *rapl_flags = MF_06_3C;
            break;
        case 0x45:
            *rapl_flags = MF_06_45;
            break;
        case 0x46:
            *rapl_flags = MF_06_46;
            break;
        case 0x3F:
            *rapl_flags = MF_06_3F;
            break;
        case 0x3D:
            *rapl_flags = MF_06_3D;
            break;
        case 0x47:
            *rapl_flags = MF_06_47;
            break;
        case 0x4F:
            *rapl_flags = MF_06_4F;
            break;
        case 0x56:
            *rapl_flags = MF_06_56;
            break;
        case 0x4E:
            *rapl_flags = MF_06_4E;
            break;
        case 0x5E:
            *rapl_flags = MF_06_5E;
            break;
        case 0x57:
            *rapl_flags = MF_06_57;
            break;
        default:
            fprintf(stderr, "%s %s::%d ERROR: model number %lx is invalid\n", getenv("HOSTNAME"),
                    __FILE__, __LINE__, model);
            return -1;
            break;
    }

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (rapl_init) model is %lx, flags are %lx\n", getenv("HOSTNAME"),
            __FILE__, __LINE__, model, *rapl_flags);
#endif

    if (!(*rapl_flags && 0x1L))
    {
        fprintf(stderr, "%s %s::%d ERROR: no rapl power unit register, rapl is probably not supported on this architecture\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

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
    if (*rapl)
    {
        free(*rapl);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: rapl finalized\n");
#endif
    return 0;
}

// TODO: fix the translate for time window
static int
translate( const int socket, uint64_t* bits, double* units, int type){
	static int initialized=0;
	static struct rapl_units ru[NUM_SOCKETS];
	uint64_t val[NUM_SOCKETS];
	int i;
    uint64_t timeval_x = 0, timeval_y = 0;

	if(!initialized){
		initialized=1;
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
		case BITS_TO_WATTS: 	
            *units = (double)(*bits)  * ru[socket].watts; 			
            break;
		case BITS_TO_SECONDS_STD:	
            *units = (double)(*bits) / 0.000976;
            break;
		case BITS_TO_JOULES:	
            *units = (double)(*bits)  * ru[socket].joules; 		
            break;
		case WATTS_TO_BITS:	
            *bits  = (uint64_t)(  (*units) / ru[socket].watts    ); 	
            break;
		case SECONDS_TO_BITS_STD:	
            // TODO: needed?
            break;
		case JOULES_TO_BITS:	
            *bits  = (uint64_t)(  (*units) / ru[socket].joules   ); 	
            break;
        case BITS_TO_SECONDS_DRAM:
            timeval_y = *bits & 0x1F;
            timeval_x = (*bits & 0x60) >> 5;
            *units = ((1 + 0.25 * timeval_x) * pow(2.0, (double) timeval_y)) * 0.000976; //ru[socket].seconds;
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: timeval_x is %lx, timeval_y is %lx, units is %lf, bits is %lx\n",
                    getenv("HOSTNAME"), __FILE__, __LINE__, timeval_x, timeval_y, *units, *bits);
#endif
            break;
        case SECONDS_TO_BITS_DRAM:
            timeval_x = (*units <= 32 ? 0 : 1);
            //tempunit = (double) (*units / ru[socket].seconds) / (*units <= 40 ? 1.25 : 1.75);
            timeval_y = (uint64_t) log2((*units / ru[socket].seconds) / (1 + 0.25 * timeval_x)); //log(tempunit) / log(2);
            *bits = (uint64_t) (timeval_y | (timeval_x << 5));
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: timeval_x is %lx, timeval_y is %lx, units is %lf, bits is %lx\n",
                    getenv("HOSTNAME"), __FILE__, __LINE__, timeval_x, timeval_y, *units, *bits);
#endif
            break;
		default: 
			fprintf(stderr, "%s:%d  Unknown value %d.  This is bad.\n", __FILE__, __LINE__, type);  
			*bits = -1;
			*units= -1.0;
			break;
	}
    return 0;
}

struct rapl_power_info{
	uint64_t msr_pkg_power_info;	// raw msr values

	double pkg_max_power;		// watts
	double pkg_min_power;
	double pkg_max_window;		// seconds
	double pkg_therm_power;		// watts

	uint64_t msr_dram_power_info;
	double dram_max_power;		// watts
	double dram_min_power;
	double dram_max_window;		// seconds
	double dram_therm_power;	// watts
};

static void
get_rapl_power_info( const int socket, struct rapl_power_info *info, const uint64_t * rapl_flags){
	uint64_t val = 0;
	//info->msr_pkg_power_info  = 0x6845000148398;
	//info->msr_dram_power_info = 0x682d0001482d0;
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (get_rapl_power_info)\n", getenv("HOSTNAME"),
            __FILE__, __LINE__);
#endif
    if (*rapl_flags & 0x10L)
    {
        read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info) );
        val = MASK_VAL( info->msr_pkg_power_info,  53, 48 );
        translate( socket, &val, &(info->pkg_max_window), BITS_TO_SECONDS_DRAM );
        
        val = MASK_VAL( info->msr_pkg_power_info,  46, 32 );
        translate( socket, &val, &(info->pkg_max_power), BITS_TO_WATTS );

        val = MASK_VAL( info->msr_pkg_power_info,  30, 16 );
        translate( socket, &val, &(info->pkg_min_power), BITS_TO_WATTS );

        val = MASK_VAL( info->msr_pkg_power_info,  14,  0 );
        translate( socket, &val, &(info->pkg_therm_power), BITS_TO_WATTS );
    }
    if (*rapl_flags & 0x100L)
    {
        read_msr_by_coord( socket, 0, 0, MSR_DRAM_POWER_INFO, &(info->msr_dram_power_info) );
        // Note that the same units are used in both the PKG and DRAM domains.
	
        val = MASK_VAL( info->msr_dram_power_info, 53, 48 );
        translate( socket, &val, &(info->dram_max_window), BITS_TO_SECONDS_DRAM );

        val = MASK_VAL( info->msr_dram_power_info, 46, 32 );
        translate( socket, &val, &(info->dram_max_power), BITS_TO_WATTS );

        val = MASK_VAL( info->msr_dram_power_info, 30, 16 );
        translate( socket, &val, &(info->dram_min_power), BITS_TO_WATTS );

        val = MASK_VAL( info->msr_dram_power_info, 14,  0 );
        translate( socket, &val, &(info->dram_therm_power), BITS_TO_WATTS );
    }
}

static int calc_rapl_from_bits(const int socket, struct rapl_limit * limit, const unsigned offset)
{
	uint64_t watts_bits=0, seconds_bits=0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_rapl_from_bits)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    watts_bits   = MASK_VAL( limit->bits, 14 + offset,  0 + offset);
    seconds_bits = MASK_VAL( limit->bits, 23 + offset, 17 + offset);

    // We have been given the bits to be written to the msr.
    // For sake of completeness, translate these into watts 
    // and seconds.
    translate( socket, &watts_bits, &limit->watts, BITS_TO_WATTS );
    translate( socket, &seconds_bits, &limit->seconds, BITS_TO_SECONDS_DRAM );
    return 0;
}

static int calc_rapl_bits(const int socket, struct rapl_limit * limit, const unsigned offset)
{
	uint64_t watts_bits=0, seconds_bits=0;

    watts_bits   = MASK_VAL( limit->bits, 14 + offset,  0 + offset);
    seconds_bits = MASK_VAL( limit->bits, 23 + offset, 17 + offset);
    
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_rapl_from_bits)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    // We have been given watts and seconds and need to translate
    // these into bit values.
    translate( socket, &watts_bits,   &limit->watts,   WATTS_TO_BITS   );
    translate( socket, &seconds_bits, &limit->seconds, SECONDS_TO_BITS_DRAM );
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "Converted %lf watts into %lx bits.\n", limit->watts, watts_bits);
    fprintf(stderr, "Converted %lf seconds into %lx bits.\n", limit->seconds, seconds_bits);
#endif
    if (watts_bits & 0xFFFFFFFFFFFF8000)
    {
        fprintf(stderr, "%s %s::%d ERROR: watts value is too large\n", getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    watts_bits <<= 0 + offset;
    if (seconds_bits & 0xFFFFFFFFFFFFFF80)
    {
        fprintf(stderr, "%s %s::%d ERROR: seconds value is too large\n", getenv("HOSTNAME"), __FILE__,
                __LINE__);
        return -1;
    }
    seconds_bits <<= 17 + offset;
    limit->bits |= watts_bits;
    limit->bits |= seconds_bits;
    return 0;
}

static int calc_pkg_rapl_limit(const int socket, struct rapl_limit * limit1, struct rapl_limit * limit2,
                               const uint64_t * rapl_flags)
{
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_pkg_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (limit1->bits)
    {
        calc_rapl_from_bits(socket, limit1, 0);
    }
    else
    {
        calc_rapl_bits(socket, limit1, 0);
    }
    if (limit2->bits)
    {
        calc_rapl_from_bits(socket, limit2, 32);
    }
    else
    {
        calc_rapl_bits(socket, limit2, 32);
    }
    return 0;
}

static int calc_std_rapl_limit(const int socket, struct rapl_limit * limit, const uint64_t * rapl_flags)
{
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_std_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (limit->bits)
    {
        calc_rapl_from_bits(socket, limit, 0);
    }
    else
    {
        calc_rapl_bits(socket, limit, 0);
    }
    return 0;
}

// TODO: check against LIMIT registers when translate is fixed
int set_pkg_rapl_limit(const int socket, struct rapl_limit * limit1, struct rapl_limit * limit2,
                       const uint64_t * rapl_flags)
{
    uint64_t pkg_limit = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (set_pkg_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif

    if (*rapl_flags & 0x2L)
    {
        calc_pkg_rapl_limit(socket, limit1, limit2, rapl_flags);
        if (limit1)
        {
            pkg_limit |= limit1->bits | (1LL << 15) | (1LL << 16);
        }
        if (limit2)
        {
            pkg_limit |= limit2->bits | (1LL << 47) | (1LL << 48);
        }
        if (limit1 || limit2)
        {
            write_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, pkg_limit); 
        }
    }
    else
    {
        fprintf(stderr, "%s %s::%d ERROR: pkg rapl limit not supported on this architecture\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int set_dram_rapl_limit(const int socket, struct rapl_limit * limit, const uint64_t * rapl_flags)
{
    uint64_t dram_limit = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (set_dram_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (*rapl_flags & 0x20L)
    {
        if (limit)
        {
            calc_std_rapl_limit(socket, limit, rapl_flags);
            dram_limit |= limit->bits | (1LL << 15);
            write_msr_by_coord(socket, 0, 0, MSR_DRAM_POWER_LIMIT, dram_limit); 
        }
    }
    else
    {
        fprintf(stderr, "%s %s::%d ERROR: dram rapl limit not supported on this architecture\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int set_pp_rapl_limit(const int socket, struct rapl_limit * limit0, struct rapl_limit * limit1,
                const uint64_t * rapl_flags)
{
    uint64_t pp0_limit = 0;
    uint64_t pp1_limit = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (set_pp_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (limit0 && (*rapl_flags & 0x200L))
    {
        calc_std_rapl_limit(socket, limit0, rapl_flags);
        pp0_limit |= limit0->bits | (1LL << 15);
        write_msr_by_coord(socket, 0, 0, MSR_PP0_POWER_LIMIT, pp0_limit);
    }
    else if (limit0)
    {
        fprintf(stderr, "%s %s::%d ERROR: pp0 rapl limit not supported on this architecture\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (limit1 && (*rapl_flags & 0x2000L))
    {
        calc_std_rapl_limit(socket, limit1, rapl_flags);
        pp1_limit |= limit1->bits | (1LL << 15);
        write_msr_by_coord(socket, 0, 0, MSR_PP1_POWER_LIMIT, pp1_limit);
    }
    else if (limit1)
    {
        fprintf(stderr, "%s %s::%d ERROR: pp1 rapl limit not supported on this architecture\n",
                getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int 
get_rapl_limit( const int socket, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram,
                const uint64_t * rapl_flags){
	assert(socket < NUM_SOCKETS);
	assert(socket >=0 );
	if(limit1){
		read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit1->bits) );
	}
	if(limit2){
		read_msr_by_coord( socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit2->bits) );
	}
	if(dram){
		read_msr_by_coord( socket, 0, 0, MSR_DRAM_POWER_LIMIT, &(dram->bits) );
	}
	// Fill in whatever values are necessary.
	//calc_rapl_limit( socket, limit1, limit2, dram , rapl_flags);
    return 0;
}

void
dump_rapl_limit( struct rapl_limit* L, FILE *writeFile ){
	fprintf(writeFile, "bits    = %lx\n", L->bits);
	fprintf(writeFile, "seconds = %lf\n", L->seconds);
	fprintf(writeFile, "watts   = %lf\n", L->watts);
	fprintf(writeFile, "\n");
}

void
dump_rapl_terse_label( FILE *writeFile ){
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++){
		fprintf(writeFile,"pkgW%02d dramW%02d ", socket, socket );
	}
}

void
dump_rapl_terse( FILE * writeFile, struct rapl_data ** rapl, const uint64_t * rapl_flags ){
	int socket;

#ifdef LIBMSR_DEBUG
        fprintf(writeFile, "%s %s::%d Writing terse label\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    // TODO: fix this up
	for(socket=0; socket<NUM_SOCKETS; socket++){
		read_rapl_data(socket, rapl, rapl_flags);
		fprintf(writeFile,"%8.4lf %8.4lf ", (*rapl)[socket].pkg_watts, (*rapl)[socket].dram_watts);
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
dump_rapl_power_info( FILE *writeFile, const uint64_t * rapl_flags){
	int socket;
	struct rapl_power_info info;
	for(socket = 0; socket < NUM_SOCKETS; socket++)
	{
		get_rapl_power_info(socket, &info, rapl_flags);
		fprintf(writeFile, "Socket= %d pkg_max_power= %8.4lf pkg_min_power=%8.4lf pkg_max_window=%8.4lf pkg_therm_power=%8.4lf\n",
				socket,
				info.pkg_max_power,
				info.pkg_min_power,
				info.pkg_max_window,
				info.pkg_therm_power);

        if (*rapl_flags & 0x20L)
        {
            fprintf(writeFile, 
                    "Socket= %d dram_max_power= %8.4lf dram_min_power=%8.4lf dram_max_window=%8.4lf dram_therm_power=%8.4lf\n",
                    socket,
                    info.dram_max_power,
                    info.dram_min_power,
                    info.dram_max_window,
                    info.dram_therm_power);
        }
	}
}

int poll_rapl_data(const int socket, struct rapl_data ** rapl, struct rapl_data * result, const uint64_t * rapl_flags)
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
    read_rapl_data(socket, rapl, rapl_flags); //, p);
    delta_rapl_data(socket, p, result, rapl_flags);
    return 0;
}

int delta_rapl_data(const int socket, struct rapl_data * p, struct rapl_data * result, const uint64_t * rapl_flags)
{
    // where did that number come from? -> max possible energy value?
	uint64_t  maxbits=4294967296;
    // why is this zero? does not seem right
	double max_joules=0.0;
    static int first = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d (delta_rapl_data)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (first)
    {
        p->elapsed = 0;
        first = 0;
    }
    else
    {
        p->elapsed = (p->now.tv_sec - p->old_now.tv_sec) 
                 +
                 (p->now.tv_usec - p->old_now.tv_usec)/1000000.0;
    }
    // Get delta joules.
    // Now handles wraparound.
    if(p->pkg_joules - p->old_pkg_joules < 0)
    {
        translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
        p->pkg_delta_joules = ( p->pkg_joules + max_joules) - p->old_pkg_joules;
    } else {
        p->pkg_delta_joules  = p->pkg_joules  - p->old_pkg_joules;		
    }
    if (*rapl_flags & 0x20L)
    {
        if(p->dram_joules - p->old_dram_joules < 0)
        {
            translate(socket, &maxbits, &max_joules, BITS_TO_JOULES); 
            p->dram_delta_joules = (p->dram_joules + max_joules) - p->old_dram_joules;
        } else {
            p->dram_delta_joules = p->dram_joules - p->old_dram_joules;	
        }	
    }
    // Get watts.
    assert(p->elapsed != 0.0 && !first);
    if(p->elapsed > 0.0){
        p->pkg_watts  = p->pkg_delta_joules  / p->elapsed;
        if (*rapl_flags & 0x20L)
        {
            p->dram_watts = p->dram_delta_joules / p->elapsed;
        }
    }else{
        p->pkg_watts  = -999.0;
        if (*rapl_flags & 0x20L)
        {
            p->dram_watts = -999.0;
        }
    }
    if (result)
    {
        result = p;
    }
    return 0;
}

int read_rapl_data(const int socket, struct rapl_data ** rapl, const uint64_t * rapl_flags)
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

    // PKG
    if (*rapl_flags & 0x4L) // there is pkg energy status
    {
        // Get raw joules
        p->old_pkg_bits = p->pkg_bits;
        p->old_pkg_joules = p->pkg_joules;
        if(read_msr_by_coord( socket, 0, 0, MSR_PKG_ENERGY_STATUS,  &(p->pkg_bits)  ))
        {
            return -1;
        }
        translate( socket, &(p->pkg_bits), &(p->pkg_joules), BITS_TO_JOULES );
    }
    if (*rapl_flags & 0x8L) // there is pkg perf status
    {
        if (read_msr_by_coord(socket, 0, 0, MSR_PKG_PERF_STATUS, &(p->pkg_perf_count)))
        {
//            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: pkg_bits %lx\n", p->pkg_bits);
#endif
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: pkg_joules %lf\n", p->pkg_joules);
    fprintf(stderr, "DEBUG: pkg_watts %lf\n", p->pkg_watts);
#endif

    // PP0
    if (*rapl_flags & 0x400L) // there is pp0 energy status
    {
        p->old_pp0_joules = p->pp0_joules;
        p->old_pp0_bits   = p->pp0_bits;
        if(read_msr_by_coord(socket, 0, 0, MSR_PP0_ENERGY_STATUS, &(p->pp0_bits)))
        {
            return -1;
        }
        translate(socket, &(p->pp0_bits), &(p->pp0_joules), BITS_TO_JOULES);
    }
    if (*rapl_flags & 0x1000L) // there is pp0 perf status
    {
        if (read_msr_by_coord(socket, 0, 0, MSR_PP0_PERF_STATUS, &(p->pp0_perf_status)))
        {
            return -1;
        }
    }
    if (*rapl_flags & 0x800L)
    {
        if (read_msr_by_coord(socket, 0, 0, MSR_PP0_POLICY, &(p->pp0_policy)))
        {
            return -1;
        }
    }

    // PP1
    if (*rapl_flags & 0x4000L) // there is pp1 energy status
    {
        // pp1
        p->old_pp1_bits   = p->pp1_bits;
        p->old_pp1_joules = p->pp1_joules;
        if(read_msr_by_coord(socket, 0, 0, MSR_PP1_ENERGY_STATUS, &(p->pp0_bits)))
        {
            return -1;
        }
        translate(socket, &(p->pp1_bits), &(p->pp1_joules), BITS_TO_JOULES);
    }
    if (*rapl_flags & 0x8000L)
    {
        if(read_msr_by_coord(socket, 0, 0, MSR_PP1_POLICY, &(p->pp1_policy)))
        {
            return -1;
        }
    }

    // DRAM   
    if (*rapl_flags & 0x40L) // there is dram energy status
    {
        p->old_dram_bits	= p->dram_bits;
        p->old_dram_joules	= p->dram_joules;
        if(read_msr_by_coord( socket, 0, 0, MSR_DRAM_ENERGY_STATUS, &(p->dram_bits) ))
        {
            return -1;
        }
        translate( socket, &(p->dram_bits), &(p->dram_joules), BITS_TO_JOULES );
    }
    if (*rapl_flags & 0x80) // there is dram perf status
    {
        if (read_msr_by_coord( socket, 0, 0, MSR_DRAM_PERF_STATUS, &(p->dram_perf_count)))
        {
            return -1;
        }
    }
    return 0;
}
