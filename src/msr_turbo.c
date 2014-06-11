#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "msr_common.h"
#include "msr_core.h"
#include "msr_turbo.h"

// MSRs common to 062A and 062D.
#define MSR_MISC_ENABLE			0x1A0	// aka IA32_MISC_ENABLE
						// setting bit 38 high DISABLES turbo mode.
#define IA32_PERF_CTL			0x199   // setting bit 32 high DISABLES turbo mode.

void
enable_turbo(int package){
	int j;
	uint64_t val[NUM_CORES_PER_PACKAGE];
	
	// This should have been turned on by the BIOS.
	// Set bit 38 to 0.  See Intel v3C 14.3.2.1.
	// Yes, writing a 0 ENABLES turbo.
	/*
	read_msr( package, MSR_MISC_ENABLE, &val );
	val &= ((uint64_t)-1) ^ ((uint64_t)1) << 38;
	write_msr( package, MSR_MISC_ENABLE, val );
	*/

	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 0.
	read_msr_all_cores_v( package, IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_CORES_PER_PACKAGE; j++){
		val[j] &= ((uint64_t)-1) ^ ((uint64_t)1) << 32;
	}
	write_msr_all_cores_v( package, IA32_PERF_CTL, &val[0] );

}


void
disable_turbo(int package){
	int j;
	uint64_t val[NUM_CORES_PER_PACKAGE];
	/* Note that this is only supposed to be done
	 * by the BIOS.
	read_msr( package, MSR_MISC_ENABLE, &val );
	// Set bit 38 to 1.  See Intel v3C 14.3.2.1.
	// Yes, writing a 1 DISABLES turbo.
	val |= ((uint64_t)1) << 38;
	write_msr( cpu, MSR_MISC_ENABLE, val );
	*/
	
	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1.
	read_msr_all_cores_v( package, IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_CORES_PER_PACKAGE; j++){
		val[j] |= ((uint64_t)1) << 32;
	}
	
	write_msr_all_cores_v( package, IA32_PERF_CTL, &val[0] );

}

void
enable_all_turbo(){
	int package;
	for(package = 0; package<NUM_PACKAGES; package++){
		enable_turbo(package);
	}
}

void
disable_all_turbo(){
	int package;
	for(package = 0; package<NUM_PACKAGES; package++){
		disable_turbo(package);
	}
}

void
dump_turbo(){
	int package, core;
	uint64_t val;
	for(package = 0; package<NUM_PACKAGES; package++){
		for(core=0; core<NUM_CORES_PER_PACKAGE; core++){
			read_msr_single_core( package, core, MSR_MISC_ENABLE, &val );
			fprintf(stdout, "%d %d 0x%018lx \t", package, core, val & (((uint64_t)1)<<38));
			read_msr_single_core( package, core, IA32_PERF_CTL, &val );
			fprintf(stdout, "0x%018lx \t", val & (((uint64_t)1)<<32));
			fprintf(stdout, "| MSR_MISC_ENABLE | 38 (0=Turbo Available) ");
			fprintf(stdout, "| IA32_PERF_CTL | 32 (0=Turbo Engaged) \n");
		}
	}
}


