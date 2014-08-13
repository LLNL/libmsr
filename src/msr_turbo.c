/* msr_turbo.c
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "msr_core.h"
#include "msr_turbo.h"

// MSRs common to 062A and 062D.
#define MSR_MISC_ENABLE			0x1A0	// aka IA32_MISC_ENABLE
						// setting bit 38 high DISABLES turbo mode. To be done in the BIOS. 
						//
#define IA32_PERF_CTL			0x199   // setting bit 32 high DISABLES turbo mode. This is the software control. 

void
disable_turbo(){

	int j;	
	uint64_t val[NUM_DEVS];
	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 0.
	read_all_cores(IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_DEVS; j++){
		val[j] |= ((uint64_t)1) << 32;
	}
	write_all_cores_v(IA32_PERF_CTL, &val[0] );


}


void
enable_turbo(){
	int j;
	uint64_t val[NUM_DEVS];
	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1.
	read_all_cores(IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_DEVS;j++){
		val[j] &= ((uint64_t)1) ^ ((uint64_t)1) << 32;
	}
	
	write_all_cores_v(IA32_PERF_CTL, &val[0] );
}

void
dump_turbo(){
	int core;
	uint64_t val;
		for(core=0; core<NUM_DEVS; core++){
			read_msr_by_idx(core, MSR_MISC_ENABLE, &val);
			fprintf(stderr, "Core: %d\n", core);
			fprintf(stderr, "0x%016lx\t", val & (((uint64_t)1)<<38));
			fprintf(stderr, "| MSR_MISC_ENABLE | 38 (0=Turbo Available) \n");
			read_msr_by_idx(core, IA32_PERF_CTL, &val);
			fprintf(stderr, "0x%016lx \t", val & (((uint64_t)1)<<32));
			fprintf(stderr, "| IA32_PERF_CTL | 32 (0=Turbo Engaged) \n");
		}
}


