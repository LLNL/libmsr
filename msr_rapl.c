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





