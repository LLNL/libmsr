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
#include "checkCPUID.h"

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

void get_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2);
{
	//FIXME these should be per thread.
	uint64_t perf_global_ctrl[NUM_THREADS];
	uint64_t fixed_ctr_ctrl[NUM_THREADS];
	int idx;

	read_all_threads(IA32_PERF_GLOBAL_CTRL, &perf_global_ctrl);
	read_all_threads(IA32_FIXED_CTR_CTRL,   &fixed_ctr_ctrl);

	for(idx=0; idx<NUM_THREADS; idx++){
		if(ctr0){
			ctr0->enable[i] 		= MASK_VAL(perf_global_ctrl[i], 32, 32);
			ctr0->ring_level[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 1,0);
			ctr0->anyThread_enable[i]  	= MASK_VAL(fixed_ctr_ctrl[i], 2,2);
			ctr0->pmi_enable[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 3,3);
		}

		if(ctr1){
			ctr1->enable[i] 		= MASK_VAL(perf_global_ctrl[i], 33, 33); 
			ctr1->ring_level[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 5,4);
			ctr1->anyThread_enable[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 6,6);
			ctr1->pmi_enable[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 7,7);	
		}

		if(ctr2){
			ctr2->enable[i] 		= MASK_VAL(perf_global_ctrl[i], 34, 34);	
			ctr2->ring_level[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 9,8);
			ctr2->anyThread_enable[i] 	= MASK_VAL(fixed_ctr_ctrl[i], 10, 10);
			ctr2->pmi_enable[i] 		= MASK_VAL(fixed_ctr_ctrl[i], 11,11);
		}
	}
}

void set_fixed_ctr_ctrl(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two, int socket, int core)
{
	assert(zero->ctr0_enable == 0 || zero->ctr0_enable == 1);
	assert(one->ctr1_enable == 0 || one->ctr1_enable == 1);
	assert(two->ctr2_enable == 0 || two->ctr2_enable == 1);
	
	assert(zero->ctr0_ring_level >=0 && zero->ctr0_ring_level <=3);
	assert(one->ctr1_ring_level >=0 && one->ctr1_ring_level <=3);
	assert(two->ctr2_ring_level >=0 && two->ctr2_ring_level <=3);
	
	assert(zero->ctr0_anyThread_enable == 0 || zero->ctr0_anyThread_enable == 1);
	assert(one->ctr1_anyThread_enable == 0 || one->ctr1_anyThread_enable == 1);
	assert(two->ctr2_anyThread_enable == 0 || two->ctr2_anyThread_enable == 1);

	assert(zero->ctr0_pmi_enable == 0 || zero->ctr0_pmi_enable == 1);
	assert(one->ctr1_pmi_enable == 0 || one->ctr1_pmi_enable == 1);
	assert(two->ctr2_pmi_enable == 0 || two->ctr2_pmi_enable == 1);

	uint64_t msrVal;
	read_msr_by_coord(socket, core, 0, IA32_PERF_GLOBAL_CTRL, &msrVal);
	
	msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)32))) | ((uint64_t)zero->ctr0_enable << (uint64_t)32); 
	msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)33))) | ((uint64_t)one->ctr1_enable << (uint64_t)33); 
	msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)34))) | ((uint64_t)two->ctr2_enable << (uint64_t)34); 

	write_msr_by_coord(socket, core, 0, IA32_PERF_GLOBAL_CTRL, msrVal);	

	read_msr_by_coord(socket, core, 0, IA32_FIXED_CTR_CTRL, &msrVal);

	msrVal = (msrVal & (~(2<<0))) | (zero->ctr0_ring_level << 0);
	msrVal = (msrVal & (~(1<<2))) | (zero->ctr0_anyThread_enable << 2);
	msrVal = (msrVal & (~(1<<3))) | (zero->ctr0_pmi_enable << 3);

	msrVal = (msrVal & (~(2<<4))) | (one->ctr1_ring_level << 4);
	msrVal = (msrVal & (~(1<<6))) | (one->ctr1_anyThread_enable << 6);
	msrVal = (msrVal & (~(1<<7))) | (one->ctr1_pmi_enable << 7);

	msrVal = (msrVal & (~(2<<8))) | (two->ctr2_ring_level << 8);
	msrVal = (msrVal & (~(1<<10))) | (two->ctr2_anyThread_enable << 10);
	msrVal = (msrVal & (~(1<<11))) | (two->ctr2_pmi_enable << 11);

	write_msr_by_coord(socket, core, 0, IA32_FIXED_CTR_CTRL, msrVal);

}

void dump_fixed_ctr_ctrl(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two)
{
	fprintf(stdout, "\nGlobal Control Enable: %d %d %d\n", zero->ctr0_enable, one->ctr1_enable, two->ctr2_enable);
	fprintf(stdout, "Ring Level(s) Enabled: %d %d %d\n", zero->ctr0_ring_level, one ->ctr1_ring_level, two->ctr2_ring_level);
	fprintf(stdout, "AnyThread Enable: %d %d %d\n", zero->ctr0_anyThread_enable, one->ctr1_anyThread_enable, two->ctr2_anyThread_enable);
	fprintf(stdout, "PMI Overflow Enabled: %d %d %d\n\n", zero->ctr0_pmi_enable, one->ctr1_pmi_enable, two->ctr2_pmi_enable);
}

void get_fixed_counter_data(struct fixed_counter_data *data)
{
	data->num_counters = cpuid_num_fixed_perf_counters();
	data->width = cpuid_width_fixed_perf_counters();
}

void get_fixed_ctr_values(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two, int socket, int core)
{
	uint64_t raw;

	read_msr_by_coord(socket, core, 0, IA32_FIXED_CTR0 , &raw);
	zero->ctr0_value = raw;
	read_msr_by_coord(socket, core, 0, IA32_FIXED_CTR1 , &raw);
	one->ctr1_value = raw;
	read_msr_by_coord(socket, core, 0, IA32_FIXED_CTR2 , &raw);
	two->ctr2_value = raw;
}
