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

/*
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

#define MSR_PCU_PMON_EVNTSEL0   (0xC30)
#define MSR_PCU_PMON_EVNTSEL1   (0xC31)
#define MSR_PCU_PMON_EVNTSEL2   (0xC32)
#define MSR_PCU_PMON_EVNTSEL3   (0xC33)
#define MSR_PCU_PMON_CTR0   (0xC36)
#define MSR_PCU_PMON_CTR1   (0xC37)
#define MSR_PCU_PMON_CTR2   (0xC38)
#define MSR_PCU_PMON_CTR3   (0xC39)

// TODO: these all change for haswell
#define MSR_C0_PMON_BOX_CTL		0xD04
#define MSR_C0_PMON_EVNTSEL0	0xD10
#define MSR_C0_PMON_EVNTSEL1	0xD11
#define MSR_C0_PMON_EVNTSEL2	0xD12
#define MSR_C0_PMON_EVNTSEL3	0xD13
#define MSR_C0_BOX_FILTER		0xD14
#define MSR_C0_PMON_CTR0		0xD16
#define MSR_C0_PMON_CTR1		0xD17
#define MSR_C0_PMON_CTR2		0xD18
#define MSR_C0_PMON_CTR3		0xD19
#define MSR_C0_BOX_FILTER1		0xD1A

#define MSR_C1_PMON_BOX_CTL		0xD24
#define MSR_C1_PMON_EVNTSEL0	0xD30
#define MSR_C1_PMON_EVNTSEL1	0xD31
#define MSR_C1_PMON_EVNTSEL2	0xD32
#define MSR_C1_PMON_EVNTSEL3	0xD33
#define MSR_C1_BOX_FILTER		0xD34
#define MSR_C1_PMON_CTR0		0xD36
#define MSR_C1_PMON_CTR1		0xD37
#define MSR_C1_PMON_CTR2		0xD38
#define MSR_C1_PMON_CTR3		0xD39
#define MSR_C1_BOX_FILTER1		0xD3A

#define MSR_C2_PMON_BOX_CTL		0xD44
#define MSR_C2_PMON_EVNTSEL0	0xD50
#define MSR_C2_PMON_EVNTSEL1	0xD51
#define MSR_C2_PMON_EVNTSEL2	0xD52
#define MSR_C2_PMON_EVNTSEL3	0xD53
#define MSR_C2_BOX_FILTER		0xD54
#define MSR_C2_PMON_CTR0		0xD56
#define MSR_C2_PMON_CTR1		0xD57
#define MSR_C2_PMON_CTR2		0xD58
#define MSR_C2_PMON_CTR3		0xD59
#define MSR_C2_BOX_FILTER1		0xD5A

#define MSR_C3_PMON_BOX_CTL		0xD64
#define MSR_C3_PMON_EVNTSEL0	0xD70
#define MSR_C3_PMON_EVNTSEL1	0xD71
#define MSR_C3_PMON_EVNTSEL2	0xD72
#define MSR_C3_PMON_EVNTSEL3	0xD73
#define MSR_C3_BOX_FILTER		0xD74
#define MSR_C3_PMON_CTR0		0xD76
#define MSR_C3_PMON_CTR1		0xD77
#define MSR_C3_PMON_CTR2		0xD78
#define MSR_C3_PMON_CTR3		0xD79
#define MSR_C3_BOX_FILTER1		0xD7A

#define MSR_C4_PMON_BOX_CTL		0xD84
#define MSR_C4_PMON_EVNTSEL0	0xD90
#define MSR_C4_PMON_EVNTSEL1	0xD91
#define MSR_C4_PMON_EVNTSEL2	0xD92
#define MSR_C4_PMON_EVNTSEL3	0xD93
#define MSR_C4_BOX_FILTER		0xD94
#define MSR_C4_PMON_CTR0		0xD96
#define MSR_C4_PMON_CTR1		0xD97
#define MSR_C4_PMON_CTR2		0xD98
#define MSR_C4_PMON_CTR3		0xD99
#define MSR_C4_BOX_FILTER1		0xD9A

#define MSR_C5_PMON_BOX_CTL		0xDA4
#define MSR_C5_PMON_EVNTSEL0	0xDB0
#define MSR_C5_PMON_EVNTSEL1	0xDB1
#define MSR_C5_PMON_EVNTSEL2	0xDB2
#define MSR_C5_PMON_EVNTSEL3	0xDB3
#define MSR_C5_BOX_FILTER		0xDB4
#define MSR_C5_PMON_CTR0		0xDB6
#define MSR_C5_PMON_CTR1		0xDB7
#define MSR_C5_PMON_CTR2		0xDB8
#define MSR_C5_PMON_CTR3		0xDB9
#define MSR_C5_BOX_FILTER1		0xDBA

#define MSR_C6_PMON_BOX_CTL		0xDC4
#define MSR_C6_PMON_EVNTSEL0	0xDD0
#define MSR_C6_PMON_EVNTSEL1	0xDD1
#define MSR_C6_PMON_EVNTSEL2	0xDD2
#define MSR_C6_PMON_EVNTSEL3	0xDD3
#define MSR_C6_BOX_FILTER		0xDD4
#define MSR_C6_PMON_CTR0		0xDD6
#define MSR_C6_PMON_CTR1		0xDD7
#define MSR_C6_PMON_CTR2		0xDD8
#define MSR_C6_PMON_CTR3		0xDD9
#define MSR_C6_BOX_FILTER1		0xDDA

#define MSR_C7_PMON_BOX_CTL		0xDE4
#define MSR_C7_PMON_EVNTSEL0	0xDF0
#define MSR_C7_PMON_EVNTSEL1	0xDF1
#define MSR_C7_PMON_EVNTSEL2	0xDF2
#define MSR_C7_PMON_EVNTSEL3	0xDF3
#define MSR_C7_BOX_FILTER		0xDF4
#define MSR_C7_PMON_CTR0		0xDF6
#define MSR_C7_PMON_CTR1		0xDF7
#define MSR_C7_PMON_CTR2		0xDF8
#define MSR_C7_PMON_CTR3		0xDF9
#define MSR_C7_BOX_FILTER1		0xDFA

#define MSR_C8_PMON_BOX_CTL		0xE04
#define MSR_C8_PMON_EVNTSEL0	0xE10
#define MSR_C8_PMON_EVNTSEL1	0xE11
#define MSR_C8_PMON_EVNTSEL2	0xE12
#define MSR_C8_PMON_EVNTSEL3	0xE13
#define MSR_C8_BOX_FILTER		0xE14
#define MSR_C8_PMON_CTR0		0xE16
#define MSR_C8_PMON_CTR1		0xE17
#define MSR_C8_PMON_CTR2		0xE18
#define MSR_C8_PMON_CTR3		0xE19
#define MSR_C8_BOX_FILTER1		0xE1A

#define MSR_C9_PMON_BOX_CTL		0xE24
#define MSR_C9_PMON_EVNTSEL0	0xE30
#define MSR_C9_PMON_EVNTSEL1	0xE31
#define MSR_C9_PMON_EVNTSEL2	0xE32
#define MSR_C9_PMON_EVNTSEL3	0xE33
#define MSR_C9_BOX_FILTER		0xE34
#define MSR_C9_PMON_CTR0		0xE36
#define MSR_C9_PMON_CTR1		0xE37
#define MSR_C9_PMON_CTR2		0xE38
#define MSR_C9_PMON_CTR3		0xE39
#define MSR_C9_BOX_FILTER1		0xE3A

#define MSR_C10_PMON_BOX_CTL	0xE44
#define MSR_C10_PMON_EVNTSEL0	0xE50
#define MSR_C10_PMON_EVNTSEL1	0xE51
#define MSR_C10_PMON_EVNTSEL2	0xE52
#define MSR_C10_PMON_EVNTSEL3	0xE53
#define MSR_C10_BOX_FILTER		0xE54
#define MSR_C10_PMON_CTR0		0xE56
#define MSR_C10_PMON_CTR1		0xE57
#define MSR_C10_PMON_CTR2		0xE58
#define MSR_C10_PMON_CTR3		0xE59
#define MSR_C10_BOX_FILTER1		0xE5A

#define MSR_C11_PMON_BOX_CTL	0xE64
#define MSR_C11_PMON_EVNTSEL0	0xE70
#define MSR_C11_PMON_EVNTSEL1	0xE71
#define MSR_C11_PMON_EVNTSEL2	0xE72
#define MSR_C11_PMON_EVNTSEL3	0xE73
#define MSR_C11_BOX_FILTER		0xE74
#define MSR_C11_PMON_CTR0		0xE76
#define MSR_C11_PMON_CTR1		0xE77
#define MSR_C11_PMON_CTR2		0xE78
#define MSR_C11_PMON_CTR3		0xE79
#define MSR_C11_BOX_FILTER1		0xE7A

#define MSR_C12_PMON_BOX_CTL	0xE84
#define MSR_C12_PMON_EVNTSEL0	0xE90
#define MSR_C12_PMON_EVNTSEL1	0xE91
#define MSR_C12_PMON_EVNTSEL2	0xE92
#define MSR_C12_PMON_EVNTSEL3	0xE93
#define MSR_C12_BOX_FILTER		0xE94
#define MSR_C12_PMON_CTR0		0xE96
#define MSR_C12_PMON_CTR1		0xE97
#define MSR_C12_PMON_CTR2		0xE98
#define MSR_C12_PMON_CTR3		0xE99
#define MSR_C12_BOX_FILTER1		0xE9A

#define MSR_C13_PMON_BOX_CTL	0xEA4
#define MSR_C13_PMON_EVNTSEL0	0xEB0
#define MSR_C13_PMON_EVNTSEL1	0xEB1
#define MSR_C13_PMON_EVNTSEL2	0xEB2
#define MSR_C13_PMON_EVNTSEL3	0xEB3
#define MSR_C13_BOX_FILTER		0xEB4
#define MSR_C13_PMON_CTR0		0xEB6
#define MSR_C13_PMON_CTR1		0xEB7
#define MSR_C13_PMON_CTR2		0xEB8
#define MSR_C13_PMON_CTR3		0xEB9
#define MSR_C13_BOX_FILTER1		0xEBA

#define MSR_C14_PMON_BOX_CTL	0xEC4
#define MSR_C14_PMON_EVNTSEL0	0xED0
#define MSR_C14_PMON_EVNTSEL1	0xED1
#define MSR_C14_PMON_EVNTSEL2	0xED2
#define MSR_C14_PMON_EVNTSEL3	0xED3
#define MSR_C14_BOX_FILTER		0xED4
#define MSR_C14_PMON_CTR0		0xED6
#define MSR_C14_PMON_CTR1		0xED7
#define MSR_C14_PMON_CTR2		0xED8
#define MSR_C14_PMON_CTR3		0xED9
#define MSR_C14_BOX_FILTER1		0xEDA
*/

int print_available_counters()
{
    fprintf(stdout, "IA32_FIXED_CTR_CTRL, 38Dh\nIA32_PERF_GLOBAL_CTRL, 38Fh\nIA32_PERF_GLOBAL_STATUS, 38Eh\n");
    fprintf(stdout, "IA32_PERF_GLOBAL_OVF_CTRL, 390h\nIA32_FIXED_CTR0, 309h\nIA32_FIXED_CTR1, 30Ah\n");
    fprintf(stdout, "IA32_FIXED_CTR2, 30Bh\n");
    int avail = cpuid_PMC_num();
    if (avail > 0)
    {
        fprintf(stdout, "IA32_PMC0, C1h\nIA32_PERFEVTSEL0, 186h\n");
    }
    if (avail > 1)
    {
        fprintf(stdout, "IA32_PMC1, C2h\nIA32_PERFEVTSEL1, 187h\n");
    }
    if (avail > 2)
    {
        fprintf(stdout, "IA32_PMC2, C3h\nIA32_PERFEVTSEL2, 188h\n");
    }
    if (avail > 3)
    {
        fprintf(stdout, "IA32_PMC3, C4h\nIA32_PERFEVTSEL3, 189h\n");
    }
    if (avail > 4)
    {
        fprintf(stdout, "IA32_PMC4, C5h\nIA32_PERFEVTSEL4, 18Ah\n");
    }
    if (avail > 5)
    {
        fprintf(stdout, "IA32_PMC5, C6h\nIA32_PERFEVTSEL5, 18Bh\n");
    }
    if (avail > 6)
    {
        fprintf(stdout, "IA32_PMC6, C7h\nIA32_PERFEVTSEL6, 18Ch\n");
    }
    if (avail > 7)
    {
        fprintf(stdout, "IA32_PMC7, C8h\nIA32_PERFEVTSEL7, 18Dh\n");
    }
    return 0;
}

//-------------------- Programmable Performance Counters ----------

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

int evt_sel_storage(struct evtsel ** e)
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

int pmc_storage(struct pmc ** p)
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
    static int avail = 0;
    if (evt == NULL)
    {
        avail = cpuid_PMC_num();
        if (avail == 0)
        {
            return -1;
        }
        evt_sel_storage(&evt);        
    }
    //test_pmc_ctrl();
    write_batch(COUNTERS_CTRL);
    clear_all_pmc();
    return 0;
}

int clear_all_pmc()
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

// TODO: doesnt do anything, need to use by_idx
int clear_pmc(int idx)
{
    static struct pmc * p = NULL;
    static int avail = 0;
    if (p == NULL)
    {
         avail = cpuid_PMC_num();
         pmc_storage(&p);
    }
    if (idx < avail)
    {
        switch (idx)
        {
            case 8:
                *p->pmc7[idx] = 0;
            case 7:
                *p->pmc6[idx] = 0;
            case 6:
                *p->pmc5[idx] = 0;
            case 5:
                *p->pmc4[idx] = 0;
            case 4:
                *p->pmc3[idx] = 0;
            case 3:
                *p->pmc2[idx] = 0;
            case 2:
                *p->pmc1[idx] = 0;
            case 1:
                *p->pmc0[idx] = 0;
        }
    }
    else
    {
        return -1;
    }
//    write_batch(COUNTERS_DATA);
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

//-------------------- End Programmable Counters ---------- 

//-------------- Uncore PCU performance monitoring ------------------
static int init_uncore_evtsel(struct uncore_evtsel * uevt)
{
    static int init = 1;
    if (init)
    {
        int sockets = num_sockets();
        uevt->c0 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uevt->c1 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uevt->c2 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uevt->c3 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        allocate_batch(UNCORE_EVTSEL, 4 * sockets);
        load_socket_batch(MSR_PCU_PMON_EVNTSEL0, uevt->c0, UNCORE_EVTSEL);
        load_socket_batch(MSR_PCU_PMON_EVNTSEL1, uevt->c1, UNCORE_EVTSEL);
        load_socket_batch(MSR_PCU_PMON_EVNTSEL2, uevt->c2, UNCORE_EVTSEL);
        load_socket_batch(MSR_PCU_PMON_EVNTSEL3, uevt->c3, UNCORE_EVTSEL);
        init = 0;
    }
    return 0;
}

static int init_uncore_counters(struct uncore_counters * uc)
{
    static int init = 1;
    if (init)
    {
        int sockets = num_sockets();
        uc->c0 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uc->c1 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uc->c2 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        uc->c3 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        allocate_batch(UNCORE_COUNT, 4 * sockets);
        load_socket_batch(MSR_PCU_PMON_CTR0, uc->c0, UNCORE_COUNT);
        load_socket_batch(MSR_PCU_PMON_CTR1, uc->c1, UNCORE_COUNT);
        load_socket_batch(MSR_PCU_PMON_CTR2, uc->c2, UNCORE_COUNT);
        load_socket_batch(MSR_PCU_PMON_CTR3, uc->c3, UNCORE_COUNT);
        init = 0;
    }
    return 0;
}

int uncore_evtsel_storage(struct uncore_evtsel ** uevt)
{
    static struct uncore_evtsel uevt_data;
    static int init = 1;
    if (init)
    {
        init = 0;
        init_uncore_evtsel(&uevt_data);
    }
    if (uevt)
    {
        *uevt = &uevt_data;
    }
    return 0;
}

int uncore_counters_storage(struct uncore_counters ** uc)
{
    static struct uncore_counters uc_data;
    static int init = 1;
    if (init)
    {
        init = 0;
        init_uncore_counters(&uc_data);
    }
    if (uc)
    {
        *uc = &uc_data;
    }
    return 0;
}

int set_pcu_ctrl_flags(uint64_t flags, uint64_t reset, uint64_t occ, uint64_t eventsel, int pcunum, unsigned socket)
{
    static struct uncore_evtsel * uevt = NULL;
    flags &= 0xDF7FFFFFL; // must set bit 29 and 23 to 0
    if (uevt == NULL)
    {
        uncore_evtsel_storage(&uevt);
    }
    switch(pcunum)
    {
        case 4:
            *uevt->c3[socket] = 0UL | (flags << 18) | (reset << 17) | (occ << 14) | eventsel;
            break;
        case 3:
            *uevt->c2[socket] = 0UL | (flags << 18) | (reset << 17) | (occ << 14) | eventsel;
            break;
        case 2:
            *uevt->c1[socket] = 0UL | (flags << 18) | (reset << 17) | (occ << 14) | eventsel;
            break;
        case 1:
            *uevt->c0[socket] = 0UL | (flags << 18) | (reset << 17) | (occ << 14) | eventsel;
            break;
    }
    return 0;
}

int set_all_pcu_ctrl(uint64_t flags, uint64_t reset, uint64_t occ, uint64_t eventsel, int pcunum)
{
    uint64_t sockets = num_sockets();
    int i;
    for (i = 0; i < sockets; i++)
    {
        set_pcu_ctrl_flags(flags, reset, occ, eventsel, pcunum, i);
    }
    return 0;
}

/*
static int test_pcu_ctrl()
{
    uint64_t flags = 0x0;
    uint64_t reset = 0x0;
    uint64_t occ = 0x0;
    uint64_t eventsel = 0x0;
    set_all_pcu_ctrl(flags, reset, occ, eventsel, 1);
    return 0;
}
*/

int enable_pcu()
{
    static struct uncore_evtsel * uevt = NULL;
    if (uevt == NULL)
    {
        uncore_evtsel_storage(&uevt);        
    }
 //   test_pcu_ctrl();
    write_batch(UNCORE_EVTSEL);
    clear_all_pcu();
    return 0;
}

int clear_all_pcu()
{
    static struct uncore_counters * uc = NULL;
    static int sockets = 0;
    if (uc == NULL)
    {
         sockets = num_sockets();
         uncore_counters_storage(&uc);
    }
    int i;
    for (i = 0; i < sockets; i++)
    {
        *uc->c0[i] = 0;
        *uc->c1[i] = 0;
        *uc->c2[i] = 0;
        *uc->c3[i] = 0;
    }
    write_batch(UNCORE_COUNT);
    return 0;
}

// TODO: doesnt do anything, need to use by_idx
// Deprecated, just use the reset bit
int clear_pcu(int idx, int pcu)
{
    static struct uncore_counters * uc = NULL;
    static int sockets = 0;
    if (uc == NULL)
    {
         sockets = num_sockets();
         uncore_counters_storage(&uc);
    }
    if (idx < sockets)
    {
        uc->c0[idx] = 0;
    }
    else
    {
        return -1;
    }
//    write_batch(UNCORE_COUNT);
    return 0;
}

int dump_uncore_counters_label(FILE * writedest)
{
    fprintf(writedest, "uncore counters\ncore c0\tc1\tc2\tc3\n");
    return 0;
}

int dump_uncore_counters(FILE * writedest)
{
    struct uncore_counters * uc;
    uncore_counters_storage(&uc);
    int i;
    for (i = 0; i < num_sockets(); i++)
    {
        fprintf(writedest, "%d %lx\t%lx\t%lx\t%lx\n", i, *uc->c0[i], *uc->c1[i], *uc->c2[i], *uc->c3[i]);
    }
    return 0;
}

//-------------- end Uncore PCU performance monitoring
//-------------- Fixed Counters performance monitoring -------------------

int fixed_ctr_storage(struct ctr_data ** ctr0, struct ctr_data ** ctr1, struct ctr_data ** ctr2)
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

int fixed_ctr_ctrl_storage(uint64_t *** perf_ctrl, uint64_t *** fixed_ctrl)
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
// ------------------ End Fixed Counters ---------------
