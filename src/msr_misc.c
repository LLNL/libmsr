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
#include "msr_misc.h"
#include "memhdlr.h"
#include "cpuid.h"

/// @brief Allocate data structures to store package-level C-state
/// residencies.
///
/// @param [out] pcr Pointer to package-level C-state residency data.
static void init_pkg_cres(struct pkg_cres *pcr)
{
    int sockets = num_sockets();
    pcr->pkg_c2 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
    pcr->pkg_c3 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
    pcr->pkg_c6 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
    pcr->pkg_c7 = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
    allocate_batch(PKG_CRES, 4 * sockets);
    load_socket_batch(MSR_PKG_C2_RESIDENCY, pcr->pkg_c2, PKG_CRES);
    load_socket_batch(MSR_PKG_C3_RESIDENCY, pcr->pkg_c3, PKG_CRES);
    load_socket_batch(MSR_PKG_C6_RESIDENCY, pcr->pkg_c6, PKG_CRES);
    load_socket_batch(MSR_PKG_C7_RESIDENCY, pcr->pkg_c7, PKG_CRES);
}

/// @brief Allocate data structures to store core-level C-state residencies.
///
/// @param [out] ccr Pointer to core-level C-state residency data.
static void init_core_cres(struct core_cres *ccr)
{
    int cores = num_cores();
    ccr->core_c3 = (uint64_t **) libmsr_calloc(cores, sizeof(uint64_t *));
    ccr->core_c6 = (uint64_t **) libmsr_calloc(cores, sizeof(uint64_t *));
    ccr->core_c7 = (uint64_t **) libmsr_calloc(cores, sizeof(uint64_t *));
    allocate_batch(CORE_CRES, 3 * cores);
    load_core_batch(MSR_CORE_C3_RESIDENCY, ccr->core_c3, CORE_CRES);
    load_core_batch(MSR_CORE_C6_RESIDENCY, ccr->core_c6, CORE_CRES);
    load_core_batch(MSR_CORE_C7_RESIDENCY, ccr->core_c7, CORE_CRES);
}

void dump_misc_enable(struct misc_enable *s)
{
    fprintf(stdout, "fast_string_enable                   = %u\n", s->fast_string_enable);
    fprintf(stdout, "auto_TCC_enable                      = %u\n", s->auto_TCC_enable);
    fprintf(stdout, "performance_monitoring               = %u\n", s->performance_monitoring);
    fprintf(stdout, "branch_trace_storage_unavail         = %u\n", s->branch_trace_storage_unavail);
    fprintf(stdout, "precise_event_based_sampling_unavail = %u\n", s->precise_event_based_sampling_unavail);
    fprintf(stdout, "TM2_enable                           = %u\n", s->TM2_enable);
    fprintf(stdout, "enhanced_Intel_SpeedStep_Tech_enable = %u\n", s->enhanced_Intel_SpeedStep_Tech_enable);
    fprintf(stdout, "enable_monitor_fsm                   = %u\n", s->enable_monitor_fsm);
    fprintf(stdout, "limit_CPUID_maxval                   = %u\n", s->limit_CPUID_maxval);
    fprintf(stdout, "xTPR_message_disable                 = %u\n", s->xTPR_message_disable);
    fprintf(stdout, "XD_bit_disable                       = %u\n", s->XD_bit_disable);
    fprintf(stdout, "turbo_mode_disable                   = %u\n", s->turbo_mode_disable);
}

void get_misc_enable(unsigned socket, struct misc_enable *s)
{
    sockets_assert(&socket, __LINE__, __FILE__);
    read_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, &(s->raw));

    s->fast_string_enable = MASK_VAL(s->raw, 0, 0);
    s->auto_TCC_enable = MASK_VAL(s->raw, 3, 3);
    s->performance_monitoring = MASK_VAL(s->raw, 7, 7);
    s->branch_trace_storage_unavail = MASK_VAL(s->raw, 11, 11);
    s->precise_event_based_sampling_unavail = MASK_VAL(s->raw, 12, 12);
    s->TM2_enable = MASK_VAL(s->raw, 13, 13);
    s->enhanced_Intel_SpeedStep_Tech_enable = MASK_VAL(s->raw, 16, 16);
    s->enable_monitor_fsm = MASK_VAL(s->raw, 18, 18);
    s->limit_CPUID_maxval = MASK_VAL(s->raw, 22, 22);
    s->xTPR_message_disable = MASK_VAL(s->raw, 23, 23);
    s->XD_bit_disable = MASK_VAL(s->raw, 34, 34);
    s->turbo_mode_disable = MASK_VAL(s->raw, 38, 38);
}

void set_misc_enable(unsigned socket, struct misc_enable *s)
{
    uint64_t msrVal;
    sockets_assert(&socket, __LINE__, __FILE__);
    read_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, &msrVal);
#if 0 /* For testing purposes. */
    assert(s->fast_string_enable == 0 || s->fast_string_enable == 1);
    assert(s->auto_TCC_enable == 0 || s->auto_TCC_enable == 1);
    assert(s->TM2_enable == 0 || s->TM2_enable == 1);
    assert(s->enhanced_Intel_SpeedStep_Tech_enable == 0 || s->enhanced_Intel_SpeedStep_Tech_enable == 1);
    assert(s->limit_CPUID_maxval == 0 || s->limit_CPUID_maxval == 1);
    assert(s->xTPR_message_disable == 0 || s->xTPR_message_disable == 1);
    assert(s->XD_bit_disable == 0 || s->XD_bit_disable == 1);
    assert(s->turbo_mode_disable == 0 || s->turbo_mode_disable == 1);
#endif

    msrVal = (msrVal & (~(1<< 0))) | (s->fast_string_enable << 0);
    msrVal = (msrVal & (~(1<< 3))) | (s->auto_TCC_enable << 3);
    msrVal = (msrVal & (~(1<< 13))) | (s->TM2_enable << 13);
    msrVal = (msrVal & (~(1<< 16))) | (s->enhanced_Intel_SpeedStep_Tech_enable << 16);
    msrVal = (msrVal & (~(1<< 22))) | (s->limit_CPUID_maxval << 22);
    msrVal = (msrVal & (~(1<< 23))) | (s->xTPR_message_disable << 23);
    msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)34))) | ((uint64_t)s->XD_bit_disable << (uint64_t)34);
    msrVal = (msrVal & (~((uint64_t)1<< (uint64_t)38))) | ((uint64_t)s->turbo_mode_disable << (uint64_t)38);

    write_msr_by_coord(socket, 0, 0, IA32_MISC_ENABLE, msrVal);
}

void pkg_cres_storage(struct pkg_cres **pcr)
{
    static struct pkg_cres pkg_cres_data;
    static int init = 0;
    if (!init)
    {
        init = 1;
        init_pkg_cres(&pkg_cres_data);
    }
    if (pcr != NULL)
    {
        *pcr = &pkg_cres_data;
    }
}

void core_cres_storage(struct core_cres **ccr)
{
    static struct core_cres core_cres_data;
    static int init = 0;
    if (!init)
    {
        init = 1;
        init_core_cres(&core_cres_data);
    }
    if (ccr != NULL)
    {
        *ccr = &core_cres_data;
    }
}

void dump_pkg_cres_label(FILE *writedest)
{
    fprintf(writedest, "PKG:c2\tc3\tc6\tc7\n");
}

void dump_pkg_cres(FILE *writedest)
{
    struct pkg_cres *pcr;
    int i;

    pkg_cres_storage(&pcr);
    read_batch(PKG_CRES);
    for (i = 0; i < num_sockets(); i++)
    {
        fprintf(writedest, "%lx\t", *pcr->pkg_c2[i]);
        fprintf(writedest, "%lx\t", *pcr->pkg_c3[i]);
        fprintf(writedest, "%lx\t", *pcr->pkg_c6[i]);
        fprintf(writedest, "%lx\t", *pcr->pkg_c7[i]);
        fprintf(writedest, "\n");
    }
}

void dump_core_cres_label(FILE *writedest)
{
    fprintf(writedest, "CORE:c3\tc6\tc7\n");
}

void dump_core_cres(FILE *writedest)
{
    struct core_cres *ccr;
    int i = 0;

    core_cres_storage(&ccr);
    read_batch(CORE_CRES);
    for (i = 0; i < num_cores(); i++)
    {
        fprintf(writedest, "%lx\t", *ccr->core_c3[i]);
        fprintf(writedest, "%lx\t", *ccr->core_c6[i]);
        fprintf(writedest, "%lx\t", *ccr->core_c7[i]);
        fprintf(writedest, "\n");
    }
}
