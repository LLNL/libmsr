/* msr_rapl.h
 */

#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include "msr_common.h"
#include "msr_core.h"

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
	double power;
};

static void
translate( int package, uint64_t* bits, double* units, int type ){
	static int initialized=0;
	static struct rapl_units units[NUM_PACKAGES];
	int i;
	if(!initialized){
		initialized=1;
		for(i=0; i<NUM_PACKAGES; i++){
			// See figure 14-16 for bit fields.
			read_msr( i, MSR_RAPL_POWER_UNIT, &(units[i].msr_rapl_power_unit) );
			// default is 1010b or 976 microseconds
			units[i].seconds = 1.0/(double)( 2^(MASK_VAL( units[i].msr_rapl_power_unit, 19, 16 )))
			// default is 10000b or 15.3 microjoules
			units[i].joules  = 1.0/(double)( 2^(MASK_VAL( units[i].msr_rapl_power_unit, 12,  8 )))
			// default is 0011b or 1/8 Watts
			units[i].watts   = 1.0/(double)( 2^(MASK_VAL( units[i].msr_rapl_power_unit,  3,  0 )))
		}	
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

static int
rapl_get_power_info(int package, struct rapl_power_info *info){

	read_msr( package, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info) );
	read_msr( package, MSR_DRAM_POWER_INFO, &(info->msr_DRAM_power_info) );

	// Note that the same units are used in both the PKG and DRAM domains.
	// Also note that "package", "socket" and "cpu" are being used interchangably.  This needs to be fixed.
	info->pkg_max_window  = translate( package, MASK_VAL( info->msr_pkg_power_info,  53, 48 ), BITS_TO_SECONDS );
	info->pkg_max_power   = translate( package, MASK_VAL( info->msr_pkg_power_info,  46, 32 ), BITS_TO_WATTS   );
	info->pkg_min_power   = translate( package, MASK_VAL( info->msr_pkg_power_info,  30, 16 ), BITS_TO_WATTS   );
	info->pkg_therm_power = translate( package, MASK_VAL( info->msr_pkg_power_info,  14,  0 ), BITS_TO_WATTS   );

	info->dram_max_window = translate( package, MASK_VAL( info->msr_dram_power_info, 53, 48 ), BITS_TO_SECONDS );
	info->dram_max_power  = translate( package, MASK_VAL( info->msr_dram_power_info, 46, 32 ), BITS_TO_WATTS   );
	info->dram_min_power  = translate( package, MASK_VAL( info->msr_dram_power_info, 30, 16 ), BITS_TO_WATTS   );
	info->dram_therm_power= translate( package, MASK_VAL( info->msr_dram_power_info, 14,  0 ), BITS_TO_WATTS   );
	
}

int 
rapl_set_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){

}

int 
rapl_get_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2, struct rapl_limit* dram ){
}

int
take_delta( int package, struct delta* new_delta){
	static struct timeval then, now;
	static struct delta[NUM_PACKAGES] old_delta;
	static int initialized;	
	double current_seconds, current_joules;
	int i;

	if(!initialized){
		initialized=1;
		gettimeofday(&then, NULL);
		// initialize the delta for each package.
		for(i=0; i<NUM_PACKAGES; i++){
			old_delta[i].seconds	= 0.0;
			old_delta[i].watts	= 0.0;
			old_delta[i].joules	= 0.0;
		}
		new_delta.seconds 	= 0.0;
		new_delta.watts		= 0.0;
		new_delta.joules	= 0.0;
		return 0;
	}
	
}





