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

struct ctr_data
{
    uint64_t * enable;
    uint64_t * ring_level;
    uint64_t * anyThread;
    uint64_t * pmi;
    uint64_t * overflow;
    uint64_t ** value;
    //uint64_t * old;
    //uint64_t * delta;
};

struct fixed_counter_data{
	int num_counters;	//this is how many fixed counter MSRs are available (i.e. IA32_FIXED_CTR0, IA32_FIXED_CTR1, IA32_FIXED_CTR2)
	int width;		//when found, this is the bit width of the fixed counter MSRs
};

#ifdef __cplusplus
extern "C" {
#endif

void init_ctr_data(struct ctr_data * ctr);
void get_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2);
void set_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2);
void get_fixed_counter_data(struct fixed_counter_data *data);
void get_fixed_ctr_values(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2);
void enable_fixed_counters();
void disable_fixed_counters();
void dump_fixed_terse(FILE *w);
void dump_fixed_terse_label(FILE *w);
void dump_fixed_readable(FILE *w);

#ifdef __cplusplus
}
#endif

#endif // MSR_COUNTERS_H
