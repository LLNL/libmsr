/* msr_counters.h
 *
 *
*/

#ifndef MSR_COUNTERS_H
#define MSR_COUNTERS_H
#include <stdio.h>
#include <stdint.h>
#include "msr_core.h"

// Note: It should not matter which processor you are using because the MSRS are architectural 
// and should remain the same

struct ctr_data{
	int enable[NUM_THREADS];
	int ring_level[NUM_THREADS];
	int anyThread_enable[NUM_THREADS];
	int pmi_enable[NUM_THREADS];
	int overflow_stat[NUM_THREADS]; // no function for this yet
	uint64_t value[NUM_THREADS];
};


struct fixed_counter_data{
	int num_counters;	//this is how many fixed counter MSRs are available (i.e. IA32_FIXED_CTR0, IA32_FIXED_CTR1, IA32_FIXED_CTR2) 
	int width;		//when found, this is the bit width of the fixed counter MSRs
};

void get_fixed_ctr_ctrl(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two);
void set_fixed_ctr_ctrl(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two);
void dump_fixed_ctr_ctrl(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two); 

void get_fixed_counter_data(struct fixed_counter_data *data);

void get_fixed_ctr_values(struct ctr0_data *zero, struct ctr1_data *one, struct ctr2_data *two, int socket, int core);

#endif
