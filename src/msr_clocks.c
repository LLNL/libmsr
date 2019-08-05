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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>

#include "msr_core.h"
#include "msr_clocks.h"
#include "memhdlr.h"
#include "cpuid.h"
#include "libmsr_debug.h"

void clocks_storage(struct clocks_data **cd)
{
    static int init = 0;
    static struct clocks_data d;
    static uint64_t totalThreads = 0;

    if (!init)
    {
        totalThreads = num_devs();
        d.aperf = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        d.mperf = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        d.tsc = (uint64_t **) libmsr_malloc(totalThreads * sizeof(uint64_t *));
        allocate_batch(CLOCKS_DATA, 3UL * num_devs());
        load_thread_batch(IA32_APERF, d.aperf, CLOCKS_DATA);
        load_thread_batch(IA32_MPERF, d.mperf, CLOCKS_DATA);
        load_thread_batch(IA32_TIME_STAMP_COUNTER, d.tsc, CLOCKS_DATA);
        init = 1;
    }
    if (cd != NULL)
    {
        *cd = &d;
    }
}

void perf_storage(struct perf_data **pd)
{
    static struct perf_data d;
    static uint64_t procs = 0;

    if (!procs)
    {
        procs = num_sockets();
        d.perf_status = (uint64_t **) libmsr_malloc(procs * sizeof(uint64_t *));
        d.perf_ctl = (uint64_t **) libmsr_malloc(procs * sizeof(uint64_t *));
        allocate_batch(PERF_DATA, 2UL * num_sockets());
        allocate_batch(PERF_CTL, 2UL * num_sockets());
        load_socket_batch(IA32_PERF_STATUS, d.perf_status, PERF_DATA);
        load_socket_batch(IA32_PERF_CTL, d.perf_ctl, PERF_CTL);
    }
    if (pd != NULL)
    {
        *pd = &d;
    }
}

void dump_clocks_data_terse_label(FILE *writedest)
{
    int thread_idx;
    static uint64_t totalThreads = 0;

    if (!totalThreads)
    {
        totalThreads = num_devs();
    }
    for (thread_idx = 0; thread_idx < totalThreads; thread_idx++)
    {
        fprintf(writedest, "aperf%02d mperf%02d tsc%02d ", thread_idx, thread_idx, thread_idx);
    }
}

void dump_clocks_data_terse(FILE *writedest)
{
    static uint64_t totalThreads = 0;
    static struct clocks_data *cd;
    int thread_idx;

    if (!totalThreads)
    {
        totalThreads = num_devs();
        clocks_storage(&cd);
    }
    read_batch(CLOCKS_DATA);
    for (thread_idx = 0; thread_idx < totalThreads; thread_idx++)
    {
        fprintf(writedest, "%20lu %20lu %20lu ", *cd->aperf[thread_idx], *cd->mperf[thread_idx], *cd->tsc[thread_idx]);
    }
}

void dump_p_state(FILE *writedest)
{
    static uint64_t procs = 0;
    static struct perf_data *cd;
    int sock_idx;

    if (!procs)
    {
        procs = num_sockets();
        perf_storage(&cd);
    }
    read_batch(PERF_DATA);
    for (sock_idx = 0; sock_idx < procs; sock_idx++)
    {
#ifdef LIBMSR_DEBUG
        printf("PERF_STATUS raw decimal %" PRIu64 "\n", *cd->perf_status[sock_idx]);
#endif
        unsigned long long perf_state = *cd->perf_status[sock_idx] & 0xFFFF;
        double pstate_actual = perf_state/256;
        fprintf(writedest, "Socket %d:\n", sock_idx);
        fprintf(writedest, "  bits            = %lx\n", *cd->perf_status[sock_idx]);
        fprintf(writedest, "  current p-state = %.0f MHz\n", pstate_actual*100);
    }
}

void set_p_state(unsigned socket, uint64_t pstate)
{
    static uint64_t procs = 0;
    static struct perf_data *cd;

    if (!procs)
    {
        procs = num_sockets();
        perf_storage(&cd);
    }
    *cd->perf_ctl[socket] = pstate;
#ifdef LIBMSR_DEBUG
    printf("PERF_CTL raw decimal %" PRIu64 "\n", *cd->perf_ctl[socket]);
#endif
    write_batch(PERF_CTL);

#ifdef LIBMSR_DEBUG
    read_batch(PERF_CTL);
    printf("---reading PERF_CTL raw decimal %" PRIu64 "\n", *cd->perf_ctl[socket]);
#endif
}

void dump_clocks_data_readable(FILE *writedest)
{
    static uint64_t totalThreads = 0;
    static struct clocks_data *cd;
    int thread_idx;

    if (!totalThreads)
    {
        totalThreads = num_devs();
        clocks_storage(&cd);
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: (clocks_readable) totalThreads is %lu\n", totalThreads);
#endif
    read_batch(CLOCKS_DATA);
    for (thread_idx = 0; thread_idx < totalThreads; thread_idx++)
    {
        fprintf(writedest, "aperf%02d:%20lu mperf%02d:%20lu tsc%02d:%20lu\n", thread_idx, *cd->aperf[thread_idx], thread_idx, *cd->mperf[thread_idx], thread_idx, *cd->tsc[thread_idx]);
    }
}

void dump_clock_mod(struct clock_mod *s, FILE *writedest)
{
    double percent = 0.0;

    if (s->duty_cycle == 0)
    {
        percent = 6.25;
    }
    else if (s->duty_cycle == 1)
    {
        percent = 12.5;
    }
    else if (s->duty_cycle == 2)
    {
        percent = 25.0;
    }
    else if (s->duty_cycle == 3)
    {
        percent = 37.5;
    }
    else if (s->duty_cycle == 4)
    {
        percent = 50.0;
    }
    else if (s->duty_cycle == 5)
    {
        percent = 63.5;
    }
    else if (s->duty_cycle == 6)
    {
        percent = 75.0;
    }
    else if (s->duty_cycle == 7)
    {
        percent = 87.5;
    }
    fprintf(writedest, "duty_cycle        = %d\n", s->duty_cycle);
    fprintf(writedest, "percentage        = %.2f\n", percent);
    fprintf(writedest, "duty_cycle_enable = %d\n", s->duty_cycle_enable);
}

void get_clock_mod(int socket, int core, struct clock_mod *s)
{
    read_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, &(s->raw));
    /* Specific encoded values for target duty cycle. */
    s->duty_cycle = MASK_VAL(s->raw, 3, 1);

    /* On-Demand Clock Modulation Enable */
    /* 1 = enabled, 0 disabled */
    s->duty_cycle_enable = MASK_VAL(s->raw, 4, 4);
}

int set_clock_mod(int socket, int core, struct clock_mod *s)
{
    uint64_t msrVal;

    read_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, &msrVal);
    if (!(s->duty_cycle > 0 && s->duty_cycle < 8))
    {
        return -1;
    }
    if (!(s->duty_cycle_enable == 0 || s->duty_cycle_enable == 1))
    {
        return -1;
    }

    msrVal = (msrVal & (~(3<<1))) | (s->duty_cycle << 1);
    msrVal = (msrVal & (~(1<<4))) | (s->duty_cycle_enable << 4);

    write_msr_by_coord(socket, core, 0, IA32_CLOCK_MODULATION, msrVal);
    return 0;
}
