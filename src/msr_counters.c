/* msr_counters.c
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

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>
#include <assert.h>
#include "msr_core.h"
#include "memhdlr.h"
#include "msr_counters.h"
#include "cpuid.h"

//These defines are from the Architectural MSRs (Should not change between models)

#define IA32_FIXED_CTR_CTRL		(0x38D)	// Controls for fixed ctr0, 1, and 2 
#define IA32_PERF_GLOBAL_CTRL		(0x38F)	// Enables for fixed ctr0,1,and2 here
#define IA32_PERF_GLOBAL_STATUS		(0x38E)	// Overflow condition can be found here
#define IA32_PERF_GLOBAL_OVF_CTRL	(0x390)	// Can clear the overflow here
#define IA32_FIXED_CTR0			(0x309)	// (R/W) Counts Instr_Retired.Any
#define IA32_FIXED_CTR1			(0x30A)	// (R/W) Counts CPU_CLK_Unhalted.Core
#define IA32_FIXED_CTR2			(0x30B)	// (R/W) Counts CPU_CLK_Unhalted.Ref


static int fixed_ctr_storage(struct ctr_data ** ctr0, struct ctr_data ** ctr1, struct ctr_data ** ctr2)
{
    static struct ctr_data c0, c1, c2;
    static int init = 1;
    if (init)
    {
        init = 0;
        init_ctr_data(&c0);
        init_ctr_data(&c1);
        init_ctr_data(&c2);
        specify_batch_size(COUNTERS_DATA, 3UL * num_devs());
        load_thread_batch(IA32_FIXED_CTR0, c0.value, COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR1, c1.value, COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR2, c2.value, COUNTERS_DATA);
    }
    if (ctr0)
    {
        *ctr0 = &c0;
    }
    if (ctr1)
    {
        *ctr1 = &c1;
    }
    if (ctr2)
    {
        *ctr2 = &c2;
    }
    return 0;
}

static int fixed_ctr_ctrl_storage(uint64_t *** perf_ctrl, uint64_t *** fixed_ctrl)
{
    static uint64_t ** perf_global_ctrl = NULL, ** fixed_ctr_ctrl = NULL;
    static uint64_t totalThreads = 0;
    int init = 1;
    if (init)
    {
        totalThreads = num_devs();
        perf_global_ctrl = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        fixed_ctr_ctrl   = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        specify_batch_size(COUNTERS_CTR_DATA, 2UL * num_devs());
        load_thread_batch(IA32_PERF_GLOBAL_CTRL, perf_global_ctrl, COUNTERS_CTR_DATA);
        load_thread_batch(IA32_FIXED_CTR_CTRL, fixed_ctr_ctrl, COUNTERS_CTR_DATA);
        init = 0;
    }
    if (perf_ctrl)
    {
        *perf_ctrl = perf_global_ctrl;
    }
    if (fixed_ctrl)
    {
        *fixed_ctrl = fixed_ctr_ctrl;
    }
    return 0;
}

void init_ctr_data(struct ctr_data * ctr)
{
    static uint64_t totalThreads = 0;
    totalThreads = num_devs();
    ctr->enable = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: note q1, ctr->enable is at %p\n", ctr->enable);
#endif
    ctr->ring_level = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->anyThread  = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->pmi        = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->overflow   = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->value      = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));

}

void 
get_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2){
    static uint64_t totalThreads = 0;
    static uint64_t ** perf_global_ctrl =  NULL;
    static uint64_t ** fixed_ctr_ctrl = NULL;
    if (!totalThreads)
    {
        totalThreads = num_devs();
        fixed_ctr_ctrl_storage(&perf_global_ctrl, &fixed_ctr_ctrl);
    }
    read_batch(COUNTERS_CTR_DATA);


	int i;
	for(i=0; i<totalThreads; i++){
		if(ctr0){
			ctr0->enable[i] 	= MASK_VAL(*perf_global_ctrl[i], 32, 32);
			ctr0->ring_level[i] 	= MASK_VAL(*fixed_ctr_ctrl[i], 1,0);
			ctr0->anyThread[i]  	= MASK_VAL(*fixed_ctr_ctrl[i], 2,2);
			ctr0->pmi[i] 		= MASK_VAL(*fixed_ctr_ctrl[i], 3,3);
		}

		if(ctr1){
			ctr1->enable[i] 	= MASK_VAL(*perf_global_ctrl[i], 33, 33); 
			ctr1->ring_level[i] 	= MASK_VAL(*fixed_ctr_ctrl[i], 5,4);
			ctr1->anyThread[i] 	= MASK_VAL(*fixed_ctr_ctrl[i], 6,6);
			ctr1->pmi[i] 		= MASK_VAL(*fixed_ctr_ctrl[i], 7,7);	
		}

		if(ctr2){
			ctr2->enable[i] 	= MASK_VAL(*perf_global_ctrl[i], 34, 34);	
			ctr2->ring_level[i] 	= MASK_VAL(*fixed_ctr_ctrl[i], 9,8);
			ctr2->anyThread[i] 	= MASK_VAL(*fixed_ctr_ctrl[i], 10, 10);
			ctr2->pmi[i] 		= MASK_VAL(*fixed_ctr_ctrl[i], 11,11);
		}
	}
}

void 
set_fixed_ctr_ctrl(struct ctr_data *ctr0, struct ctr_data *ctr1, struct ctr_data *ctr2) {
    static uint64_t totalThreads = 0;
    static uint64_t ** perf_global_ctrl = NULL,
                    ** fixed_ctr_ctrl   = NULL;
    if (!totalThreads)
    {
        totalThreads = num_devs();
        fixed_ctr_ctrl_storage(&perf_global_ctrl, &fixed_ctr_ctrl);
    }
    // dont need to read counters data, we are just zeroing things out
    read_batch(COUNTERS_CTR_DATA);

	int i;
    
	for(i=0; i<totalThreads; i++){	
        *ctr0->value[i] = 0;
        *ctr1->value[i] = 0;
        *ctr2->value[i] = 0;
		*perf_global_ctrl[i] =  ( *perf_global_ctrl[i] & ~(1ULL<<32) ) | ctr0->enable[i] << 32; 
		*perf_global_ctrl[i] =  ( *perf_global_ctrl[i] & ~(1ULL<<33) ) | ctr1->enable[i] << 33; 
		*perf_global_ctrl[i] =  ( *perf_global_ctrl[i] & ~(1ULL<<34) ) | ctr2->enable[i] << 34; 

		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(3ULL<<0))) | (ctr0->ring_level[i] << 0);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<2))) | (ctr0->anyThread[i] << 2);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<3))) | (ctr0->pmi[i] << 3);

		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(3ULL<<4))) | (ctr1->ring_level[i] << 4);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<6))) | (ctr1->anyThread[i] << 6);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<7))) | (ctr1->pmi[i] << 7);

		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(3ULL<<8)))  | (ctr2->ring_level[i] << 8);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<10))) | (ctr2->anyThread[i] << 10);
		*fixed_ctr_ctrl[i] = (*fixed_ctr_ctrl[i] & (~(1ULL<<11))) | (ctr2->pmi[i] << 11);
	}
    write_batch(COUNTERS_CTR_DATA);
    write_batch(COUNTERS_DATA);
}

void 
get_fixed_counter_data(struct fixed_counter_data *data)
{
	data->num_counters = cpuid_num_fixed_perf_counters();
	data->width = cpuid_width_fixed_perf_counters();
}

// These four functions use the structs defined in fixed_ctr_storage.

void
enable_fixed_counters(){
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    struct ctr_data * c0, * c1, * c2;
    fixed_ctr_storage(&c0, &c1, &c2);

	int i;
	for(i=0; i<totalThreads; i++){
		c0->enable[i] 		= c1->enable[i] 		= c2->enable[i] 		= 1;
		c0->ring_level[i] 	= c1->ring_level[i] 	= c2->ring_level[i] 	= 3; // usr + os	
		c0->anyThread[i] 	= c1->anyThread[i] 	    = c2->anyThread[i] 	= 1; 
		c0->pmi[i] 		    = c1->pmi[i] 		    = c2->pmi[i] 		= 0;
	}
	set_fixed_ctr_ctrl( c0, c1, c2 );	
}

void
disable_fixed_counters(){
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    struct ctr_data * c0, * c1, * c2;
    fixed_ctr_storage(&c0, &c1, &c2);

	int i;
	for(i=0; i<totalThreads; i++){
		c0->enable[i] = c1->enable[i] = c2->enable[i] = 0;
		c0->ring_level[i] = c1->ring_level[i] = c2->ring_level[i] = 3; // usr + os	
		c0->anyThread[i] = c1->anyThread[i] = c2->anyThread[i] = 1; 
		c0->pmi[i] = c1->pmi[i] = c2->pmi[i] = 0;
	}
	set_fixed_ctr_ctrl( c0, c1, c2 );	
}

void
dump_fixed_terse(FILE *writeFile){
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    struct ctr_data * c0, * c1, * c2;
    fixed_ctr_storage(&c0, &c1, &c2);

	int i;
    read_batch(COUNTERS_DATA);
	for(i=0; i<totalThreads; i++){
		fprintf(writeFile, "%lu %lu %lu ", *c0->value[i], *c1->value[i], *c2->value[i]);
	}
}

void
dump_fixed_terse_label(FILE *writeFile){
	/*
	 * 0 	Unhalted core cycles
	 * 1	Instructions retired
	 * 2	Unhalted reference cycles
	 * 3	LLC Reference
	 * 4	LLC Misses
	 * 5	Branch Instructions Retired
	 * 6	Branch Misses Retired
	 */
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }

	int i;
	for(i=0; i<totalThreads; i++){
		fprintf(writeFile, "IR%02d UCC%02d URC%02d ", i, i, i);
	}
}

void dump_fixed_readable(FILE * writeFile)
{
    static uint64_t totalThreads = 0;
    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    struct ctr_data * c0, * c1, * c2;
    fixed_ctr_storage(&c0, &c1, &c2);

	int i;
    read_batch(COUNTERS_DATA);
	for(i=0; i<totalThreads; i++){
		fprintf(writeFile, "IR%02d: %lu UCC%02d:%lu URC%02d:%lu\n", i, *c0->value[i], i, *c1->value[i], i, *c2->value[i]);
	}

}
