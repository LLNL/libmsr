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

// These are non-architectural MSRs

#define IA32_PMC0 (0xC1)
#define IA32_PMC1 (0xC2)
#define IA32_PMC2 (0xC3)
#define IA32_PMC3 (0xC4)
#define IA32_PMC4 (0xC5)
#define IA32_PMC5 (0xC6)
#define IA32_PMC6 (0xC7)
#define IA32_PMC7 (0xC8)

#define IA32_PERFEVTSEL0 (0x186)
#define IA32_PERFEVTSEL1 (0x187)
#define IA32_PERFEVTSEL2 (0x188)
#define IA32_PERFEVTSEL3 (0x189)
#define IA32_PERFEVTSEL4 (0x18A)
#define IA32_PERFEVTSEL5 (0x18B)
#define IA32_PERFEVTSEL6 (0x18C)
#define IA32_PERFEVTSEL7 (0x18D)

static int init_evtsel(struct evtsel * evt)
{
    uint64_t numDevs = num_devs();
    int avail = cpuid_PMC_num();
    if (avail < 1)
    {
        return -1;
    }
    switch (avail)
    {
        case 8:
            evt->perf_evtsel7 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 7:
            evt->perf_evtsel6 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 6:
            evt->perf_evtsel5 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 5:
            evt->perf_evtsel4 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 4:
            evt->perf_evtsel3 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 3:
            evt->perf_evtsel2 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 2:
            evt->perf_evtsel1 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 1:
            evt->perf_evtsel0 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
    }
    allocate_batch(COUNTERS_CTRL, avail * numDevs);
    switch (avail)
    {
        case 8:
            load_thread_batch(IA32_PERFEVTSEL7, evt->perf_evtsel7, COUNTERS_CTRL);
        case 7:
            load_thread_batch(IA32_PERFEVTSEL6, evt->perf_evtsel6, COUNTERS_CTRL);
        case 6:
            load_thread_batch(IA32_PERFEVTSEL5, evt->perf_evtsel5, COUNTERS_CTRL);
        case 5:
            load_thread_batch(IA32_PERFEVTSEL4, evt->perf_evtsel4, COUNTERS_CTRL);
        case 4:
            load_thread_batch(IA32_PERFEVTSEL3, evt->perf_evtsel3, COUNTERS_CTRL);
        case 3:
            load_thread_batch(IA32_PERFEVTSEL2, evt->perf_evtsel2, COUNTERS_CTRL);
        case 2:
            load_thread_batch(IA32_PERFEVTSEL1, evt->perf_evtsel1, COUNTERS_CTRL);
        case 1:
            load_thread_batch(IA32_PERFEVTSEL0, evt->perf_evtsel0, COUNTERS_CTRL);
    }
    return 0;
}

static int init_pmc(struct pmc * p)
{
    uint64_t numDevs = num_devs();
    int avail = cpuid_PMC_num();
    if (avail < 1)
    {
        return -1;
    }
    switch (avail)
    {
        case 8:
            p->pmc7 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 7:
            p->pmc6 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 6:
            p->pmc5 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 5:
            p->pmc4 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 4:
            p->pmc3 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 3:
            p->pmc2 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 2:
            p->pmc1 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
        case 1:
            p->pmc0 = (uint64_t **) libmsr_calloc(numDevs, sizeof(uint64_t *));
    }
    allocate_batch(COUNTERS_DATA, avail * numDevs);
    switch (avail)
    {
        case 8:
            load_thread_batch(IA32_PMC7, p->pmc7, COUNTERS_DATA);
        case 7:
            load_thread_batch(IA32_PMC6, p->pmc6, COUNTERS_DATA);
        case 6:
            load_thread_batch(IA32_PMC5, p->pmc5, COUNTERS_DATA);
        case 5:
            load_thread_batch(IA32_PMC4, p->pmc4, COUNTERS_DATA);
        case 4:
            load_thread_batch(IA32_PMC3, p->pmc3, COUNTERS_DATA);
        case 3:
            load_thread_batch(IA32_PMC2, p->pmc2, COUNTERS_DATA);
        case 2:
            load_thread_batch(IA32_PMC1, p->pmc1, COUNTERS_DATA);
        case 1:
            load_thread_batch(IA32_PMC0, p->pmc0, COUNTERS_DATA);
    }
    return 0;
}

static int evt_sel_storage(struct evtsel ** e)
{
    static struct evtsel evt;
    static int init = 1;
    if (init)
    {
        init_evtsel(&evt);
        init = 0;
    }
    if (e)
    {
        *e = &evt;
    }
    return 0;
}

static int pmc_storage(struct pmc ** p)
{
    static struct pmc counters;
    static int init = 1;
    if (init)
    {
        init_pmc(&counters);
        init = 0;
    }
    if (p)
    {
        *p = &counters;
    }
    return 0;
}

// cmask [31:24]
// flags [23:16]
// umask [15:8]
// eventsel [7:0]
// set a pmc event on a single thread
int set_pmc_ctrl_flags(uint64_t cmask, uint64_t flags, uint64_t umask, uint64_t eventsel, int pmcnum, unsigned thread)
{
    static struct evtsel * evt = NULL;
    if (evt == NULL)
    {
        evt_sel_storage(&evt);
    }
    switch(pmcnum)
    {
        case 8:
            *evt->perf_evtsel7[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 7:
            *evt->perf_evtsel6[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 6:
            *evt->perf_evtsel5[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 5:
            *evt->perf_evtsel4[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 4:
            *evt->perf_evtsel3[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 3:
            *evt->perf_evtsel2[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 2:
            *evt->perf_evtsel1[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
        case 1:
            *evt->perf_evtsel0[thread] = 0UL | (cmask << 24) | (flags << 16) | (umask << 8) | eventsel;
            break;
    }
    return 0;
}

// set a pmc to the same event on all threads
int set_all_pmc_ctrl(uint64_t cmask, uint64_t flags, uint64_t umask, uint64_t eventsel, int pmcnum)
{
    uint64_t numDevs = num_devs();
    int i;
    for (i = 0; i < numDevs; i++)
    {
        set_pmc_ctrl_flags(cmask, flags, umask, eventsel, pmcnum, i);
    }
    return 0;
}

/*
static int test_pmc_ctrl()
{
    uint64_t cmask = 0x0;
    uint64_t flags = 0x67;
    uint64_t umask = 0x00;
    uint64_t eventsel = 0xC4;
    set_all_pmc_ctrl(cmask, flags, umask, eventsel, 1);
    return 0;
}
*/

int enable_pmc()
{
    static struct evtsel * evt = NULL;
    static uint64_t numDevs = 0;
    static int avail = 0;
    if (evt == NULL)
    {
        avail = cpuid_PMC_num();
        numDevs = num_devs();
        evt_sel_storage(&evt);        
    }
    //test_pmc_ctrl();
    write_batch(COUNTERS_CTRL);
    clear_pmc();
    return 0;
}

int clear_pmc()
{
    static struct pmc * p = NULL;
    static uint64_t numDevs = 0;
    static int avail = 0;
    if (p == NULL)
    {
         avail = cpuid_PMC_num();
         numDevs = num_devs();
         pmc_storage(&p);
    }
    int i;
    for (i = 0; i < numDevs; i++)
    {
        switch (avail)
        {
            case 8:
                *p->pmc7[i] = 0;
            case 7:
                *p->pmc6[i] = 0;
            case 6:
                *p->pmc5[i] = 0;
            case 5:
                *p->pmc4[i] = 0;
            case 4:
                *p->pmc3[i] = 0;
            case 3:
                *p->pmc2[i] = 0;
            case 2:
                *p->pmc1[i] = 0;
            case 1:
                *p->pmc0[i] = 0;
        }
    }
    write_batch(COUNTERS_DATA);
    return 0;
}

//#define COUNTERS_DEBUG

int dump_pmc_readable(FILE * writefile)
{
    static struct pmc * p = NULL;
    static uint64_t numDevs = 0;
    static int avail = 0;
    if (p == NULL)
    {
        avail = cpuid_PMC_num();
        numDevs = num_devs();
        pmc_storage(&p);
    }
    read_batch(COUNTERS_DATA);
    fprintf(writefile, "PMC Counters:\n");
    int i;
    for (i = 0; i < numDevs; i++)
    {
        fprintf(writefile, "Thread %d\n", i);
        switch (avail)
        {
            case 8:
                fprintf(writefile, "\tpmc7: %lu\n", *p->pmc7[i]);
            case 7:
                fprintf(writefile, "\tpmc6: %lu\n", *p->pmc6[i]);
            case 6:
                fprintf(writefile, "\tpmc5: %lu\n", *p->pmc5[i]);
            case 5:
                fprintf(writefile, "\tpmc4: %lu\n", *p->pmc4[i]);
            case 4:
                fprintf(writefile, "\tpmc3: %lu\n", *p->pmc3[i]);
            case 3:
                fprintf(writefile, "\tpmc2: %lu\n", *p->pmc2[i]);
            case 2:
                fprintf(writefile, "\tpmc1: %lu\n", *p->pmc1[i]);
            case 1:
                fprintf(writefile, "\tpmc0: %lu\n", *p->pmc0[i]);
        }
    }
    return 0;
}

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
        allocate_batch(FIXED_COUNTERS_DATA, 3UL * num_devs());
        load_thread_batch(IA32_FIXED_CTR0, c0.value, FIXED_COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR1, c1.value, FIXED_COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR2, c2.value, FIXED_COUNTERS_DATA);
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
        allocate_batch(FIXED_COUNTERS_CTR_DATA, 2UL * num_devs());
        load_thread_batch(IA32_PERF_GLOBAL_CTRL, perf_global_ctrl, FIXED_COUNTERS_CTR_DATA);
        load_thread_batch(IA32_FIXED_CTR_CTRL, fixed_ctr_ctrl, FIXED_COUNTERS_CTR_DATA);
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
    read_batch(FIXED_COUNTERS_CTR_DATA);


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
    read_batch(FIXED_COUNTERS_CTR_DATA);

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
    write_batch(FIXED_COUNTERS_CTR_DATA);
    write_batch(FIXED_COUNTERS_DATA);
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
    read_batch(FIXED_COUNTERS_DATA);
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
    read_batch(FIXED_COUNTERS_DATA);
	for(i=0; i<totalThreads; i++){
		fprintf(writeFile, "IR%02d: %lu UCC%02d:%lu URC%02d:%lu\n", i, *c0->value[i], i, *c1->value[i], i, *c2->value[i]);
	}

}
