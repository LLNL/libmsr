/* msr_clocks.c
 *
 * MPERF, APERF and friends.
 */

#include <stdio.h>
#include "msr_core.h"
#include "msr_clocks.h"


#define MSR_IA32_MPERF 		0x000000e7
#define MSR_IA32_APERF 		0x000000e8
#define IA32_TIME_STAMP_COUNTER 0x00000010

void 
read_all_aperf(uint64_t *aperf){
	read_all_threads( MSR_IA32_APERF, aperf );
}

void
read_all_mperf(uint64_t *mperf){
	read_all_threads( MSR_IA32_MPERF, mperf );
}

void 
read_all_tsc(uint64_t *tsc){
	read_all_threads( IA32_TIME_STAMP_COUNTER, tsc );
}

void
dump_clocks_terse_label(){
	int thread_idx;
	for(thread_idx=0; thread_idx<NUM_THREADS; thread_idx++){
		fprintf(stdout, "aperf%02d mperf%02d tsc%02d ", 
			thread_idx, thread_idx, thread_idx);
	}
}

void
dump_clocks_terse(){
	uint64_t aperf_val[NUM_THREADS], mperf_val[NUM_THREADS], tsc_val[NUM_THREADS];
	int thread_idx;
	read_all_aperf(aperf_val);
	read_all_mperf(mperf_val);
	read_all_tsc  (tsc_val);
	for(thread_idx=0; thread_idx<NUM_THREADS; thread_idx++){
		fprintf(stdout, "%20lu %20lu %20lu ", 
			aperf_val[thread_idx], mperf_val[thread_idx], tsc_val[thread_idx]);
	}
}

