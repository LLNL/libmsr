/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "msr_core.h"
#include "memhdlr.h"
#include "msr_counters.h"
#include "cpuid.h"
#include "libmsr_debug.h"

void print_available_counters(void)
{
    fprintf(stdout, "IA32_FIXED_CTR_CTRL, 38Dh\nIA32_PERF_GLOBAL_CTRL, 38Fh\nIA32_PERF_GLOBAL_STATUS, 38Eh\n");
    fprintf(stdout, "IA32_PERF_GLOBAL_OVF_CTRL, 390h\nIA32_FIXED_CTR0, 309h\nIA32_FIXED_CTR1, 30Ah\n");
    fprintf(stdout, "IA32_FIXED_CTR2, 30Bh\n");
    int avail = cpuid_num_pmc();
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
}

/**************************************/
/* Programmable Performance Counters  */
/**************************************/

/// @brief Initialize storage for performance event select counter data.
///
/// @param [out] evt Data for performance event select counters.
///
/// @return 0 if successful, else -1 if number of general-purpose performance
/// counters is less than 1.
static int init_perfevtsel(struct perfevtsel *evt)
{
    uint64_t numDevs = num_devs();
    int avail = cpuid_num_pmc();

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

/// @brief Initialize storage for general-purpose performance counter data.
///
/// @param [out] p Data for general-purpose performance counters.
///
/// @return 0 if successful, else -1 if number of general-purpose performance
/// counters is less than 1.
static int init_pmc(struct pmc *p)
{
    uint64_t numDevs = num_devs();
    int avail = cpuid_num_pmc();

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

#if 0 /* For testing purposes. */
static void test_pmc_ctrl(void)
{
    uint64_t cmask = 0x0;
    uint64_t flags = 0x67;
    uint64_t umask = 0x00;
    uint64_t eventsel = 0xC4;
    set_all_pmc_ctrl(cmask, flags, umask, eventsel, 1);
}
#endif

void perfevtsel_storage(struct perfevtsel **e)
{
    static struct perfevtsel evt;
    static int init = 0;

    if (!init)
    {
        init_perfevtsel(&evt);
        init = 1;
    }
    if (e != NULL)
    {
        *e = &evt;
    }
}

void pmc_storage(struct pmc **p)
{
    static struct pmc counters;
    static int init = 0;

    if (!init)
    {
        init_pmc(&counters);
        init = 1;
    }
    if (p != NULL)
    {
        *p = &counters;
    }
}

/* IA32_PEREVTSELx MSRs
 * cmask [31:24]
 * flags [23:16]
 * umask [15:8]
 * eventsel [7:0]
 */
void set_pmc_ctrl_flags(uint64_t cmask, uint64_t flags, uint64_t umask, uint64_t eventsel, int pmcnum, unsigned thread)
{
    static struct perfevtsel *evt = NULL;
    if (evt == NULL)
    {
        perfevtsel_storage(&evt);
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
}

void set_all_pmc_ctrl(uint64_t cmask, uint64_t flags, uint64_t umask, uint64_t eventsel, int pmcnum)
{
    uint64_t numDevs = num_devs();
    int i;

    for (i = 0; i < numDevs; i++)
    {
        set_pmc_ctrl_flags(cmask, flags, umask, eventsel, pmcnum, i);
    }
}

int enable_pmc(void)
{
    static struct perfevtsel *evt = NULL;
    static int avail = 0;
    if (evt == NULL)
    {
        avail = cpuid_num_pmc();
        if (avail == 0)
        {
            return -1;
        }
        perfevtsel_storage(&evt);
    }
    //test_pmc_ctrl();
    write_batch(COUNTERS_CTRL);
    clear_all_pmc();
    return 0;
}

void clear_all_pmc(void)
{
    static struct pmc *p = NULL;
    static uint64_t numDevs = 0;
    static int avail = 0;
    int i;

    if (p == NULL)
    {
        avail = cpuid_num_pmc();
        numDevs = num_devs();
        pmc_storage(&p);
    }
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
}

int clear_pmc(int idx)
{
    static struct pmc *p = NULL;
    static int avail = 0;

    if (p == NULL)
    {
        avail = cpuid_num_pmc();
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
    //write_batch(COUNTERS_DATA);
    return 0;
}

void dump_pmc_data_readable(FILE *writedest)
{
    static struct pmc *p = NULL;
    static uint64_t numDevs = 0;
    static int avail = 0;
    int i;

    if (p == NULL)
    {
        avail = cpuid_num_pmc();
        numDevs = num_devs();
        pmc_storage(&p);
    }
    read_batch(COUNTERS_DATA);
    fprintf(writedest, "PMC Counters:\n");
    for (i = 0; i < numDevs; i++)
    {
        fprintf(writedest, "Thread %d\n", i);
        switch (avail)
        {
            case 8:
                fprintf(writedest, "\tpmc7: %lu\n", *p->pmc7[i]);
            case 7:
                fprintf(writedest, "\tpmc6: %lu\n", *p->pmc6[i]);
            case 6:
                fprintf(writedest, "\tpmc5: %lu\n", *p->pmc5[i]);
            case 5:
                fprintf(writedest, "\tpmc4: %lu\n", *p->pmc4[i]);
            case 4:
                fprintf(writedest, "\tpmc3: %lu\n", *p->pmc3[i]);
            case 3:
                fprintf(writedest, "\tpmc2: %lu\n", *p->pmc2[i]);
            case 2:
                fprintf(writedest, "\tpmc1: %lu\n", *p->pmc1[i]);
            case 1:
                fprintf(writedest, "\tpmc0: %lu\n", *p->pmc0[i]);
        }
    }
}

/*************************************/
/* Uncore PCU Performance Monitoring */
/*************************************/

/// @brief Initialize storage for uncore performance event select counter data.
///
/// @param [out] uevt Data for uncore performance event select counters.
static void init_unc_perfevtsel(struct unc_perfevtsel *uevt)
{
    static int init = 0;
    if (!init)
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
        init = 1;
    }
}

/// @brief Initialize storage for uncore general-purpose performance event
/// counter data.
///
/// @param [out] uevt Data for uncore general-purpose performance counters.
static void init_unc_counters(struct unc_counters *uc)
{
    static int init = 0;
    if (!init)
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
        init = 1;
    }
}

#if 0 /* For testing purposes. */
static int test_pcu_ctrl(void)
{
    uint64_t flags = 0x0;
    uint64_t reset = 0x0;
    uint64_t occ = 0x0;
    uint64_t eventsel = 0x0;
    set_all_pcu_ctrl(flags, reset, occ, eventsel, 1);
    return 0;
}
#endif

void unc_perfevtsel_storage(struct unc_perfevtsel **uevt)
{
    static struct unc_perfevtsel uevt_data;
    static int init = 0;
    if (!init)
    {
        init = 1;
        init_unc_perfevtsel(&uevt_data);
    }
    if (uevt != NULL)
    {
        *uevt = &uevt_data;
    }
}

void unc_counters_storage(struct unc_counters **uc)
{
    static struct unc_counters uc_data;
    static int init = 0;
    if (!init)
    {
        init = 1;
        init_unc_counters(&uc_data);
    }
    if (uc != NULL)
    {
        *uc = &uc_data;
    }
}

void set_pcu_ctrl_flags(uint64_t flags, uint64_t reset, uint64_t occ, uint64_t eventsel, int pcunum, unsigned socket)
{
    static struct unc_perfevtsel *uevt = NULL;
    flags &= 0xDF7FFFFFL; // must set bit 29 and 23 to 0
    if (uevt == NULL)
    {
        unc_perfevtsel_storage(&uevt);
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
}

void set_all_pcu_ctrl(uint64_t flags, uint64_t reset, uint64_t occ, uint64_t eventsel, int pcunum)
{
    uint64_t sockets = num_sockets();
    int i;
    for (i = 0; i < sockets; i++)
    {
        set_pcu_ctrl_flags(flags, reset, occ, eventsel, pcunum, i);
    }
}

void enable_pcu(void)
{
    static struct unc_perfevtsel *uevt = NULL;
    if (uevt == NULL)
    {
        unc_perfevtsel_storage(&uevt);
    }
    //test_pcu_ctrl();
    write_batch(UNCORE_EVTSEL);
    clear_all_pcu();
}

void clear_all_pcu(void)
{
    static struct unc_counters *uc = NULL;
    static int sockets = 0;
    int i;
    if (uc == NULL)
    {
        sockets = num_sockets();
        unc_counters_storage(&uc);
    }
    for (i = 0; i < sockets; i++)
    {
        *uc->c0[i] = 0;
        *uc->c1[i] = 0;
        *uc->c2[i] = 0;
        *uc->c3[i] = 0;
    }
    write_batch(UNCORE_COUNT);
}

int clear_pcu(int idx)
{
    static struct unc_counters *uc = NULL;
    static int sockets = 0;
    if (uc == NULL)
    {
        sockets = num_sockets();
        unc_counters_storage(&uc);
    }
    if (idx < sockets)
    {
        uc->c0[idx] = 0;
    }
    else
    {
        return -1;
    }
    //write_batch(UNCORE_COUNT);
    return 0;
}

void dump_unc_counter_data_label(FILE *writedest)
{
    fprintf(writedest, "socket c0\tc1\tc2\tc3\n");
}

void dump_unc_counter_data(FILE *writedest)
{
    struct unc_counters *uc;
    int i;
    unc_counters_storage(&uc);
    for (i = 0; i < num_sockets(); i++)
    {
        fprintf(writedest, "%d %lx\t%lx\t%lx\t%lx\n", i, *uc->c0[i], *uc->c1[i], *uc->c2[i], *uc->c3[i]);
    }
}

/*****************************************/
/* Fixed Counters Performance Monitoring */
/*****************************************/

void fixed_counter_storage(struct fixed_counter **ctr0, struct fixed_counter **ctr1, struct fixed_counter **ctr2)
{
    static struct fixed_counter c0, c1, c2;
    static int init = 0;
    if (!init)
    {
        init = 1;
        init_fixed_counter(&c0);
        init_fixed_counter(&c1);
        init_fixed_counter(&c2);
        allocate_batch(FIXED_COUNTERS_DATA, 3UL * num_devs());
        load_thread_batch(IA32_FIXED_CTR0, c0.value, FIXED_COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR1, c1.value, FIXED_COUNTERS_DATA);
        load_thread_batch(IA32_FIXED_CTR2, c2.value, FIXED_COUNTERS_DATA);
    }
    if (ctr0 != NULL)
    {
        *ctr0 = &c0;
    }
    if (ctr1 != NULL)
    {
        *ctr1 = &c1;
    }
    if (ctr2 != NULL)
    {
        *ctr2 = &c2;
    }
}

void fixed_counter_ctrl_storage(uint64_t ***perf_ctrl, uint64_t ***fixed_ctrl)
{
    static uint64_t **perf_global_ctrl = NULL;
    static uint64_t **fixed_ctr_ctrl = NULL;
    static uint64_t totalThreads = 0;
    static int init = 0;

    if (!init)
    {
        totalThreads = num_devs();
        perf_global_ctrl = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        fixed_ctr_ctrl = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        allocate_batch(FIXED_COUNTERS_CTR_DATA, 2UL * num_devs());
        load_thread_batch(IA32_PERF_GLOBAL_CTRL, perf_global_ctrl, FIXED_COUNTERS_CTR_DATA);
        load_thread_batch(IA32_FIXED_CTR_CTRL, fixed_ctr_ctrl, FIXED_COUNTERS_CTR_DATA);
        init = 1;
    }
    if (perf_ctrl != NULL)
    {
        *perf_ctrl = perf_global_ctrl;
    }
    if (fixed_ctrl != NULL)
    {
        *fixed_ctrl = fixed_ctr_ctrl;
    }
}

void init_fixed_counter(struct fixed_counter *ctr)
{
    static uint64_t totalThreads = 0;

    totalThreads = num_devs();
    ctr->enable = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: note q1, ctr->enable is at %p\n", ctr->enable);
#endif
    ctr->ring_level = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->anyThread = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->pmi = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->overflow = (uint64_t *) libmsr_malloc(totalThreads * sizeof(uint64_t));
    ctr->value = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
}

void get_fixed_ctr_ctrl(struct fixed_counter *ctr0, struct fixed_counter *ctr1, struct fixed_counter *ctr2)
{
    static uint64_t totalThreads = 0;
    static uint64_t **perf_global_ctrl =  NULL;
    static uint64_t **fixed_ctr_ctrl = NULL;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
        fixed_counter_ctrl_storage(&perf_global_ctrl, &fixed_ctr_ctrl);
    }
    read_batch(FIXED_COUNTERS_CTR_DATA);

    for (i = 0; i < totalThreads; i++)
    {
        if (ctr0 != NULL)
        {
            ctr0->enable[i] = MASK_VAL(*perf_global_ctrl[i], 32, 32);
            ctr0->ring_level[i] = MASK_VAL(*fixed_ctr_ctrl[i], 1, 0);
            ctr0->anyThread[i] = MASK_VAL(*fixed_ctr_ctrl[i], 2, 2);
            ctr0->pmi[i] = MASK_VAL(*fixed_ctr_ctrl[i], 3, 3);
        }
        if (ctr1 != NULL)
        {
            ctr1->enable[i] = MASK_VAL(*perf_global_ctrl[i], 33, 33);
            ctr1->ring_level[i] = MASK_VAL(*fixed_ctr_ctrl[i], 5, 4);
            ctr1->anyThread[i] = MASK_VAL(*fixed_ctr_ctrl[i], 6, 6);
            ctr1->pmi[i] = MASK_VAL(*fixed_ctr_ctrl[i], 7, 7);
        }
        if (ctr2 != NULL)
        {
            ctr2->enable[i] = MASK_VAL(*perf_global_ctrl[i], 34, 34);
            ctr2->ring_level[i] = MASK_VAL(*fixed_ctr_ctrl[i], 9, 8);
            ctr2->anyThread[i] = MASK_VAL(*fixed_ctr_ctrl[i], 10, 10);
            ctr2->pmi[i] = MASK_VAL(*fixed_ctr_ctrl[i], 11, 11);
        }
    }
}

void set_fixed_counter_ctrl(struct fixed_counter *ctr0, struct fixed_counter *ctr1, struct fixed_counter *ctr2)
{
    static uint64_t totalThreads = 0;
    static uint64_t **perf_global_ctrl = NULL;
    static uint64_t **fixed_ctr_ctrl = NULL;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
        fixed_counter_ctrl_storage(&perf_global_ctrl, &fixed_ctr_ctrl);
    }
    /* Don't need to read counters data, we are just zeroing things out. */
    read_batch(FIXED_COUNTERS_CTR_DATA);

    for (i = 0; i < totalThreads; i++)
    {
        *ctr0->value[i] = 0;
        *ctr1->value[i] = 0;
        *ctr2->value[i] = 0;
        *perf_global_ctrl[i] = (*perf_global_ctrl[i] & ~(1ULL<<32)) | ctr0->enable[i] << 32;
        *perf_global_ctrl[i] = (*perf_global_ctrl[i] & ~(1ULL<<33)) | ctr1->enable[i] << 33;
        *perf_global_ctrl[i] = (*perf_global_ctrl[i] & ~(1ULL<<34)) | ctr2->enable[i] << 34;

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

void get_fixed_counter_config(struct fixed_counter_config *data)
{
    data->num_counters = cpuid_num_fixed_counters();
    data->width = cpuid_width_fixed_counters();
}

void enable_fixed_counters(void)
{
    static uint64_t totalThreads = 0;
    struct fixed_counter *c0, *c1, *c2;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    fixed_counter_storage(&c0, &c1, &c2);

    for (i = 0; i < totalThreads; i++)
    {
        c0->enable[i] = c1->enable[i] = c2->enable[i] = 1;
        c0->ring_level[i] = c1->ring_level[i] = c2->ring_level[i] = 3; // usr + os
        c0->anyThread[i] = c1->anyThread[i] = c2->anyThread[i] = 1;
        c0->pmi[i] = c1->pmi[i] = c2->pmi[i] = 0;
    }
    set_fixed_counter_ctrl(c0, c1, c2);
}

void disable_fixed_counters(void)
{
    static uint64_t totalThreads = 0;
    struct fixed_counter *c0, *c1, *c2;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    fixed_counter_storage(&c0, &c1, &c2);

    for (i = 0; i < totalThreads; i++)
    {
        c0->enable[i] = c1->enable[i] = c2->enable[i] = 0;
        c0->ring_level[i] = c1->ring_level[i] = c2->ring_level[i] = 3; // usr + os
        c0->anyThread[i] = c1->anyThread[i] = c2->anyThread[i] = 1;
        c0->pmi[i] = c1->pmi[i] = c2->pmi[i] = 0;
    }
    set_fixed_counter_ctrl(c0, c1, c2);
}

void dump_fixed_counter_data_terse(FILE *writedest)
{
    static uint64_t totalThreads = 0;
    struct fixed_counter *c0, *c1, *c2;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    fixed_counter_storage(&c0, &c1, &c2);

    read_batch(FIXED_COUNTERS_DATA);
    for (i = 0; i < totalThreads; i++)
    {
        fprintf(writedest, "%lu %lu %lu ", *c0->value[i], *c1->value[i], *c2->value[i]);
    }
}

void dump_fixed_counter_data_terse_label(FILE *writedest)
{
    static uint64_t totalThreads = 0;
    int i;

    /*
     * 0 	Unhalted core cycles
     * 1	Instructions retired
     * 2	Unhalted reference cycles
     * 3	LLC Reference
     * 4	LLC Misses
     * 5	Branch Instructions Retired
     * 6	Branch Misses Retired
     */

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    for (i = 0; i < totalThreads; i++)
    {
        fprintf(writedest, "IR%02d UCC%02d URC%02d ", i, i, i);
    }
}

void dump_fixed_counter_data_readable(FILE *writedest)
{
    static uint64_t totalThreads = 0;
    struct fixed_counter *c0, *c1, *c2;
    int i;

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    fixed_counter_storage(&c0, &c1, &c2);

    read_batch(FIXED_COUNTERS_DATA);
    for (i = 0; i < totalThreads; i++)
    {
        fprintf(writedest, "IR%02d: %lu UCC%02d:%lu URC%02d:%lu\n", i, *c0->value[i], i, *c1->value[i], i, *c2->value[i]);
    }
}
