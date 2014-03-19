/* msr_counters.c
 *
 * 
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <assert.h>
#include "msr_core.h"
#include "msr_counters.h"
#include "cpuid.h"

/*
 * Macros
 */

// Two defines below from Barry Rountree 
#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

//These defines are from the Architectural MSRs (Should not change between models)

#define IA32_FIXED_CTR_CTRL		(0x38D)	// Controls for fixed ctr0, 1, and 2 
#define IA32_PERF_GLOBAL_CTRL		(0x38F)	// Enables for fixed ctr0,1,and2 here
#define IA32_PERF_GLOBAL_STATUS		(0x38E)	// Overflow condition can be found here
#define IA32_PERF_GLOBAL_OVF_CTRL	(0x390)	// Can clear the overflow here
#define IA32_FIXED_CTR0			(0x309)	// (R/W) Counts Instr_Retired.Any
#define IA32_FIXED_CTR1			(0x30A)	// (R/W) Counts CPU_CLK_Unhalted.Core
#define IA32_FIXED_CTR2			(0x30B)	// (R/W) Counts CPU_CLK_Unhalted.Ref

static struct ctr_data c0, c1, c2;

void 
get_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2){
	uint64_t perf_global_ctrl[NUM_THREADS];
	uint64_t fixed_ctr_ctrl[NUM_THREADS];
	int i;

	read_all_threads(IA32_PERF_GLOBAL_CTRL, perf_global_ctrl);
	read_all_threads(IA32_FIXED_CTR_CTRL,   fixed_ctr_ctrl);

	for(i=0; i<NUM_THREADS; i++){
		if(ctr0){
			ctr0->enable[i] 	= MASK_VAL(perf_global_ctrl[i], 32, 32);
			ctr0->ring_level[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 1,0);
			ctr0->anyThread[i]  	= MASK_VAL(fixed_ctr_ctrl[i], 2,2);
			ctr0->pmi[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 3,3);
		}

		if(ctr1){
			ctr1->enable[i] 	= MASK_VAL(perf_global_ctrl[i], 33, 33); 
			ctr1->ring_level[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 5,4);
			ctr1->anyThread[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 6,6);
			ctr1->pmi[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 7,7);	
		}

		if(ctr2){
			ctr2->enable[i] 	= MASK_VAL(perf_global_ctrl[i], 34, 34);	
			ctr2->ring_level[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 9,8);
			ctr2->anyThread[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 10, 10);
			ctr2->pmi[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 11,11);
		}
	}
}

void 
set_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2) {
	uint64_t perf_global_ctrl[NUM_THREADS];
	uint64_t fixed_ctr_ctrl[NUM_THREADS];
	int i;

	read_all_threads( IA32_PERF_GLOBAL_CTRL, perf_global_ctrl);
	read_all_threads( IA32_FIXED_CTR_CTRL,   fixed_ctr_ctrl);

	for(i=0; i<NUM_THREADS; i++){	
		perf_global_ctrl[i] =  ( perf_global_ctrl[i] & ~(1ULL<<32) ) | ctr0->enable[i] << 32; 
		perf_global_ctrl[i] =  ( perf_global_ctrl[i] & ~(1ULL<<33) ) | ctr1->enable[i] << 33; 
		perf_global_ctrl[i] =  ( perf_global_ctrl[i] & ~(1ULL<<34) ) | ctr2->enable[i] << 34; 

		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(3ULL<<0))) | (ctr0->ring_level[i] << 0);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<2))) | (ctr0->anyThread[i] << 2);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<3))) | (ctr0->pmi[i] << 3);

		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(3ULL<<4))) | (ctr1->ring_level[i] << 4);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<6))) | (ctr1->anyThread[i] << 6);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<7))) | (ctr1->pmi[i] << 7);

		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(3ULL<<8)))  | (ctr2->ring_level[i] << 8);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<10))) | (ctr2->anyThread[i] << 10);
		fixed_ctr_ctrl[i] = (fixed_ctr_ctrl[i] & (~(1ULL<<11))) | (ctr2->pmi[i] << 11);
	}

	write_all_threads_v( IA32_PERF_GLOBAL_CTRL, perf_global_ctrl);	
	write_all_threads_v( IA32_FIXED_CTR_CTRL,   fixed_ctr_ctrl);

	write_all_threads(IA32_FIXED_CTR0, 0);
	write_all_threads(IA32_FIXED_CTR1, 0);
	write_all_threads(IA32_FIXED_CTR2, 0);
}

void 
get_fixed_counter_data(struct fixed_counter_data *data)
{
	data->num_counters = cpuid_num_fixed_perf_counters();
	data->width = cpuid_width_fixed_perf_counters();
}

void 
get_fixed_ctr_values(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2){
	read_all_threads(IA32_FIXED_CTR0, ctr0->value);
	read_all_threads(IA32_FIXED_CTR1, ctr1->value);
	read_all_threads(IA32_FIXED_CTR2, ctr2->value);
}

// These four funcitons use the static structs defined a the top of the file.

void
enable_fixed_counters(){
	int i;
	for(i=0; i<NUM_THREADS; i++){
		c0.enable[i] 		= c1.enable[i] 		= c2.enable[i] 		= 1;
		c0.ring_level[i] 	= c1.ring_level[i] 	= c2.ring_level[i] 	= 3; // usr + os	
		c0.anyThread[i] 	= c1.anyThread[i] 	= c2.anyThread[i] 	= 1; 
		c0.pmi[i] 		= c1.pmi[i] 		= c2.pmi[i] 		= 0;
	}
	set_fixed_ctr_ctrl( &c0, &c1, &c2 );	
}

void
disable_fixed_counters(){
	int i;
	for(i=0; i<NUM_THREADS; i++){
		c0.enable[i] = c1.enable[i] = c2.enable[i] = 0;
		c0.ring_level[i] = c1.ring_level[i] = c2.ring_level[i] = 3; // usr + os	
		c0.anyThread[i] = c1.anyThread[i] = c2.anyThread[i] = 1; 
		c0.pmi[i] = c1.pmi[i] = c2.pmi[i] = 0;
	}
	set_fixed_ctr_ctrl( &c0, &c1, &c2 );	
}

void
dump_fixed_terse(){
	int i;
	get_fixed_ctr_values( &c0, &c1, &c2 );
	for(i=0; i<NUM_THREADS; i++){
		fprintf(stdout, "%lu %lu %lu ", c0.value[i], c1.value[i], c2.value[i]);
	}

}

void
dump_fixed_terse_label(){
	/*
	 * 0 	Unhalted core cycles
	 * 1	Instructions retired
	 * 2	Unhalted reference cycles
	 * 3	LLC Reference
	 * 4	LLC Misses
	 * 5	Branch Instructions Retired
	 * 6	Branch Misses Retired
	 */
	int i;
	for(i=0; i<NUM_THREADS; i++){
		fprintf(stdout, "UCC%02d IR%02d URC%02d ", i, i, i);
	}
}
