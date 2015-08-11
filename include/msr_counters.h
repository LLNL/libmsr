/* msr_counters.h
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
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
