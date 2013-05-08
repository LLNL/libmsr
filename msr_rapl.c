/* msr_rapl.c
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include "msr_common.h"
#include "msr_core.h"
#include "msr_rapl.h"

// Section 35.7
// Table 35-11.  MSRs supported by Intel processors based on Intel 
// microarchitecture code name Sandy Bridge.
// Model/family 06_2A, 06_2D.
#if (USE_062A || USE_062D)
#define MSR_RAPL_POWER_UNIT 		(0x606)	// ro
#define MSR_PKG_POWER_LIMIT 		(0x610) // rw
#define MSR_PKG_ENERY_STATUS 		(0x611) // ro
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
translate( int package, uint64_t* bits, double* units, int type ){
	static int initialized=0;
	static struct rapl_units ru[NUM_PACKAGES];
	int i;
	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_PACKAGES; i++){
			// See figure 14-16 for bit fields.
#ifdef MSR_DEBUG
			ru[i].msr_rapl_power_unit = 0xA1003;
#else
			read_msr( i, MSR_RAPL_POWER_UNIT, &(ru[i].msr_rapl_power_unit) );
#endif
			// default is 1010b or 976 microseconds
			ru[i].seconds = 1.0/(double)( 2^(MASK_VAL( ru[i].msr_rapl_power_unit, 19, 16 )));
			// default is 10000b or 15.3 microjoules
			ru[i].joules  = 1.0/(double)( 2^(MASK_VAL( ru[i].msr_rapl_power_unit, 12,  8 )));
			// default is 0011b or 1/8 Watts
			ru[i].watts   = 1.0/(double)( 2^(MASK_VAL( ru[i].msr_rapl_power_unit,  3,  0 )));
		}	
	}
	switch(type){
		case BITS_TO_WATTS: 	*units = (double)(*bits)  * ru[i].watts; 	break;
		case BITS_TO_SECONDS:	*units = (double)(*bits)  * ru[i].seconds; 	break;
		case BITS_TO_JOULES:	*units = (double)(*bits)  * ru[i].joules; 	break;
		case WATTS_TO_BITS:	*bits  = (double)(*units) * ru[i].watts; 	break;
		case SECONDS_TO_BITS:	*bits  = (double)(*units) * ru[i].seconds; 	break;
		case JOULES_TO_BITS:	*bits  = (double)(*units) * ru[i].joules; 	break;
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
rapl_get_power_info(int package, struct rapl_power_info *info){
	uint64_t val = 0;
#ifdef MSR_DEBUG
	info->msr_pkg_power_info  = 0x6845000148398;
	info->msr_dram_power_info = 0x682d0001482d0;
#else
	read_msr( package, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info) );
	read_msr( package, MSR_DRAM_POWER_INFO, &(info->msr_dram_power_info) );
#endif

	// Note that the same units are used in both the PKG and DRAM domains.
	// Also note that "package", "socket" and "cpu" are being used interchangably.  This needs to be fixed.
	
	val = MASK_VAL( info->msr_pkg_power_info,  53, 48 );
	translate( package, &val, &(info->pkg_max_window), BITS_TO_SECONDS );

	val = MASK_VAL( info->msr_pkg_power_info,  46, 32 );
	translate( package, &val, &(info->pkg_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  30, 16 );
	translate( package, &val, &(info->pkg_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_pkg_power_info,  14,  0 );
	translate( package, &val, &(info->pkg_therm_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 53, 48 );
	translate( package, &val, &(info->dram_max_window), BITS_TO_SECONDS );

	val = MASK_VAL( info->msr_dram_power_info, 46, 32 );
	translate( package, &val, &(info->dram_max_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 30, 16 );
	translate( package, &val, &(info->dram_min_power), BITS_TO_WATTS );

	val = MASK_VAL( info->msr_dram_power_info, 14,  0 );
	translate( package, &val, &(info->dram_therm_power), BITS_TO_WATTS );
}

int 
rapl_set_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	static struct rapl_power_info rpi[NUM_PACKAGES];
	static int initialized=0;
	uint64_t pkg_limit=0;
	uint64_t dram_limit=0;
	uint64_t watts_bits=0, seconds_bits=0;
	double watts_val=0.0, seconds_val=0.0;
	int i;
	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_PACAKGES; i++){
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

			translate( package, &watts_bits, &limit1->watts, BITS_TO_WATTS );
			translate( package, &seconds_bits, &limit1->seconds, BITS_TO_SECONDS );

		}else{
			// We have been given watts and seconds and need to translate
			// these into bit values.
			translate( package, &watts_bits,   &limit1->watts,   WATTS_TO_BITS   );
			translate( package, &seconds_bits, &limit1->seconds, SECONDS_TO_BITS );
			limit1->bits |= watts_bits   << 0
			limit1->bits |= seconds_bits << 17
		}
		pkg_limit |= limit1->bits | (1 << 15) | (1 << 16);	// enable clamping
	}	
	if(limit2){
		if (limit2->bits){
			watts_bits   = MASK_VAL( limit2->bits, 46, 32 );
			seconds_bits = MASK_VAL( limit2->bits, 55, 49 );

			translate( package, &watts_bits, &limit2->watts, BITS_TO_WATTS );
			translate( package, &seconds_bits, &limit2->seconds, BITS_TO_SECONDS );

		}else{
			translate( package, &watts_bits,   &limit1->watts,   WATTS_TO_BITS   );
			translate( package, &seconds_bits, &limit1->seconds, SECONDS_TO_BITS );
			limit1->bits |= watts_bits   << 32
			limit1->bits |= seconds_bits << 49
		}
		pkg_limit |= limit1->bits | (1 << 47) | (1 << 48);	// enable clamping
	}
	if(dram){
		if (dram->bits){
			// We have been given the bits to be written to the msr.
			// For sake of completeness, translate these into watts 
			// and seconds.
			watts_bits   = MASK_VAL( dram->bits, 14,  0 );
			seconds_bits = MASK_VAL( dram->bits, 23, 17 );

			translate( package, &watts_bits, &dram->watts, BITS_TO_WATTS );
			translate( package, &seconds_bits, &dram->seconds, BITS_TO_SECONDS );

		}else{
			// We have been given watts and seconds and need to translate
			// these into bit values.
			translate( package, &watts_bits,   &dram->watts,   WATTS_TO_BITS   );
			translate( package, &seconds_bits, &dram->seconds, SECONDS_TO_BITS );
			dram->bits |= watts_bits   << 0
			dram->bits |= seconds_bits << 17
		}
		dram_limit |= dram->bits | (1 << 15) | (1 << 16);	// enable clamping
	}
	if(pkg_limit){
		write_msr( package, MSR_PKG_POWER_LIMIT, pkg_limit );
	}
	if(dram_limit){
		write_msr( package, MSR_DRAM_POWER_LIMIT, pkg_limit );
	}
		
	return 0;
}

int 
rapl_get_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
	package=package;
	limit1=limit1;
	limit2=limit2;
	dram=dram;
	return 0;
}
/*
void
take_delta( int package, struct delta* new_delta){
	static struct timeval then, now;
	static struct delta old_delta[NUM_PACKAGES];
	static int initialized;	
	int i;

	if(!initialized){
		initialized=1;
		gettimeofday(&then, NULL);
		// initialize the delta for each package.
		for(i=0; i<NUM_PACKAGES; i++){
			old_delta[i].seconds	= 0.0;
			old_delta[i].dram_watts	= 0.0;
			old_delta[i].dram_joules= 0.0;
			old_delta[i].pkg_watts	= 0.0;
			old_delta[i].pkg_joules	= 0.0;
		}
		new_delta->seconds 	= 0.0;
		new_delta->dram_watts	= 0.0;
		new_delta->dram_joules	= 0.0;
		new_delta->pkg_watts	= 0.0;
		new_delta->pkg_joules	= 0.0;
	}
}
*/


#ifdef MSR_DEBUG
/* To compile this use 
	gcc -DMSR_DEBUG -Wall msr_rapl.c
   and run
	./a.out
*/
int main(){
	struct rapl_power_info r;
	rapl_get_power_info(1, &r);
	return 0;		
}
#endif	
