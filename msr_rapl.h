/* msr_rapl.h
 */
#include <stdint.h>

// TLCC2 architecture uses 062D processors; 
// those are the only ones we care about.
#define USE_062D 1
#define USE_062A 0	
#define USE_063E 0


// Section 35.7
// Table 35-11.  MSRs supported by Intel processors based on Intel 
// microarchitecture code name Sandy Bridge.
// Model/family 06_2A, 06_2D.
#if (USE_062A || USE_062D)
#define MSR_RAPL_POWER_UNIT 		(0x606)
#define MSR_PKG_POWER_LIMIT 		(0x610)
#define MSR_PKG_ENERY_STATUS 		(0x611)
#define MSR_PKG_POWER_INFO 		(0x614)
#define MSR_PP0_POWER_LIMIT 		(0x638)
#define MSR_PP0_ENERY_STATUS 		(0x639)
#define MSR_PP0_POLICY 			(0x63A)
#define MSR_PP0_PERF_STATUS 		(0x63B)
#endif

// Section 35.7.1
// Table 35-12. MSRs supported by second generation Intel Core processors 
// (Intel microarchitecture Code Name Sandy Bridge)
// Model/family 06_2AH
#if (USE_062A)
#define MSR_PP1_POWER_LIMIT 		(0x640)
#define MSR_PP1_ENERGY_STATUS 		(0x641)	// sic; MSR_PP1_ENERY_STATUS
#define MSR_PP1_POLICY 			(0x642)
#endif

// Section 35.7.2
// Table 35-13. Selected MSRs supported by Intel Xeon processors E5 Family 
// (based on Intel Microarchitecture code name Sandy Bridge) 
// Model/family 06_2DH
#if (USE_062D)
#define MSR_PKG_PERF_STATUS 		(0x613)
#define MSR_DRAM_POWER_LIMIT 		(0x618)
#define MSR_DRAM_ENERGY_STATUS 		(0x619)	// sic; MSR_DRAM_ENERY_STATUS
#define MSR_DRAM_ENERY_STATUS 		(0x61B)
#define MSR_DRAM_POWER_INFO 		(0x61C)
#endif

// Section 35.8.1
// Table 35-15. Selected MSRs supported by Intel Xeon processors E5 Family v2 
// (based on Intel microarchitecture code name Ivy Bridge) 
// Model/family 06_3EH.  
// The Intel documentation only lists this table of msrs; this may be an error.
#if (USE_063E)
#define MSR_PKG_PERF_STATUS 		(0x613)
#define MSR_DRAM_POWER_LIMIT 		(0x618)
#define MSR_DRAM_ENERGY_STATUS 		(0x619)	// sic; MSR_DRAM_ENERY_STATUS
#define MSR_DRAM_PERF_STATUS 		(0x61B)
#define MSR_DRAM_POWER_INFO 		(0x61C)
#endif




struct rapl_limit{
	double watts;		// User-friendly interface.
	double seconds;

	uint64_t _watts;	// User-unfriendly interface.
	uint64_t _seconds;
}

struct rapl_info{

}

int rapl_set_package_limit( int package, struct rapl_limit* limit1, struct rapl_limit* limit2 );
int rapl_set_dram_limit( int package, struct rapl_limit* limit );
int rapl_set_pp0_limit( int package, struct rapl_lmit* limit );




