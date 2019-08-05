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

#include <stdint.h>
#include <stdio.h>

#include "msr_core.h"
#include "msr_turbo.h"
#include "msr_rapl.h"
#include "memhdlr.h"
#include "libmsr_error.h"
#include "cpuid.h"
#include "libmsr_debug.h"

/* IA32_MISC_ENABLE (0x1A0)
 * Setting bit 38 high disables Turbo Boost. To be done in the BIOS.
 *
 * IA32_PERF_CTL (0x199)
 * Setting bit 32 high disables Turbo Boost. This is the software control.
 */

void turbo_storage(uint64_t ***val)
{
    static uint64_t **perf_ctl = NULL;
    static int init = 0;
    static uint64_t numDevs = 0;
    if (!init)
    {
        numDevs = num_devs();
        perf_ctl = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        init = 1;
    }
    if (val != NULL)
    {
        *val = perf_ctl;
    }
}

void enable_turbo(void)
{
    int j;
    static uint64_t numDevs = 0;
    static uint64_t **val =  NULL;
    if (!numDevs)
    {
        numDevs = num_devs();
        val = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    /* Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1. */
    read_batch(PERF_CTL);
    for (j = 0; j < numDevs; j++)
    {
        *val[j] &= ~(((uint64_t)1) << 32);
        fprintf(stderr, "0x%016lx\t", *val[j] & (((uint64_t)1)<<32));
    }
    write_batch(PERF_CTL);
}

void disable_turbo(void)
{
    int j;
    static uint64_t numDevs = 0;
    static uint64_t **val = NULL;
    if (!numDevs)
    {
        numDevs = num_devs();
        val = (uint64_t **) libmsr_malloc(numDevs * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    /* Set IDA/Turbo DISENGAGE (bit 32) of IA32_PERF_CTL to 0. */
    read_batch(PERF_CTL);
    for (j = 0; j < numDevs; j++)
    {
        *val[j] |= ((uint64_t)1) << 32;
    }
    write_batch(PERF_CTL);
}

void dump_turbo(FILE *writedest)
{
    int core;
    static uint64_t numDevs = 0;
    uint64_t val;
    if (!numDevs)
    {
        numDevs = num_devs();
    }
    for (core = 0; core < numDevs; core++)
    {
        read_msr_by_idx(core, IA32_MISC_ENABLE, &val);
        fprintf(writedest, "Core: %d\n", core);
        fprintf(writedest, "0x%016lx\t", val & (((uint64_t)1)<<38));
        fprintf(writedest, "| IA32_MISC_ENABLE | 38 (0 = Turbo Available) \n");
        read_msr_by_idx(core, IA32_PERF_CTL, &val);
        fprintf(writedest, "0x%016lx \t", val & (((uint64_t)1)<<32));
        fprintf(writedest, "|   IA32_PERF_CTL  | 32 (0 = Turbo Engaged) \n");
    }
}

#ifndef IS_ARCH_2D
void calc_max_non_turbo(const unsigned socket, struct turbo_activation_ratio_data *info)
{
    sockets_assert(&socket, __LINE__, __FILE__);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_max_non_turbo)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    printf("  bits = %lx\n", info->bits);
    info->max_non_turbo_ratio = ((double)(MASK_VAL(info->bits, 7, 0))) * 100;
    printf("  max non turbo ratio = %.0f\n", info->max_non_turbo_ratio);
}

int get_max_turbo_activation_ratio(const unsigned socket, struct turbo_activation_ratio_data *info)
{
    static uint64_t *rapl_flags = NULL;
    sockets_assert(&socket, __LINE__, __FILE__);

    if (rapl_flags == NULL)
    {
        if (rapl_storage(NULL, &rapl_flags))
        {
            return -1;
        }
    }

    /* Check if MSR_TURBO_ACTIVATION_RATIO exists on this platform. */
    if (*rapl_flags & TURBO_ACTIVATION_RATIO)
    {
        read_msr_by_coord(socket, 0, 0, MSR_TURBO_ACTIVATION_RATIO, &(info->bits));
        calc_max_non_turbo(socket, info);
    }
    else
    {
        libmsr_error_handler("get_max_turbo_activation_ratio(): MSR_TURBO_ACTIVATION_RATIO not supported", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

void calc_max_turbo_ratio(const unsigned socket, struct turbo_limit_data *info, struct turbo_limit_data *info2)
{
    sockets_assert(&socket, __LINE__, __FILE__);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_max_non_turbo)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif

    if (info != NULL)
    {
        printf("   bits = %lx\n", info->bits);
        info->max_1c = ((double)(MASK_VAL(info->bits, 7, 0))) * 100;
        info->max_2c = ((double)(MASK_VAL(info->bits, 15, 8))) * 100;
        info->max_3c = ((double)(MASK_VAL(info->bits, 23, 16))) * 100;
        info->max_4c = ((double)(MASK_VAL(info->bits, 31, 24))) * 100;
        info->max_5c = ((double)(MASK_VAL(info->bits, 39, 32))) * 100;
        info->max_6c = ((double)(MASK_VAL(info->bits, 47, 40))) * 100;
        info->max_7c = ((double)(MASK_VAL(info->bits, 55, 48))) * 100;
        info->max_8c = ((double)(MASK_VAL(info->bits, 63, 56))) * 100;
        printf("   max ratio 1C  = %.0f MHz\n", info->max_1c);
        printf("   max ratio 2C  = %.0f MHz\n", info->max_2c);
        printf("   max ratio 3C  = %.0f MHz\n", info->max_3c);
        printf("   max ratio 4C  = %.0f MHz\n", info->max_4c);
        printf("   max ratio 5C  = %.0f MHz\n", info->max_5c);
        printf("   max ratio 6C  = %.0f MHz\n", info->max_6c);
        printf("   max ratio 7C  = %.0f MHz\n", info->max_7c);
        printf("   max ratio 8C  = %.0f MHz\n", info->max_8c);
        printf("\n");
    }
    else if (info2 != NULL)
    {
        printf("   bits = %lx\n", info2->bits);
        info2->max_1c = ((double)(MASK_VAL(info2->bits, 7, 0))) * 100;
        info2->max_2c = ((double)(MASK_VAL(info2->bits, 15, 8))) * 100;
        info2->max_3c = ((double)(MASK_VAL(info2->bits, 23, 16))) * 100;
        info2->max_4c = ((double)(MASK_VAL(info2->bits, 31, 24))) * 100;
        info2->max_5c = ((double)(MASK_VAL(info2->bits, 39, 32))) * 100;
        info2->max_6c = ((double)(MASK_VAL(info2->bits, 47, 40))) * 100;
        info2->max_7c = ((double)(MASK_VAL(info2->bits, 55, 48))) * 100;
        info2->max_8c = ((double)(MASK_VAL(info2->bits, 63, 56))) * 100;
        printf("   max ratio 9C  = %.0f MHz\n", info2->max_1c);
        printf("   max ratio 10C = %.0f MHz\n", info2->max_2c);
        printf("   max ratio 11C = %.0f MHz\n", info2->max_3c);
        printf("   max ratio 12C = %.0f MHz\n", info2->max_4c);
        printf("   max ratio 13C = %.0f MHz\n", info2->max_5c);
        printf("   max ratio 14C = %.0f MHz\n", info2->max_6c);
        printf("   max ratio 15C = %.0f MHz\n", info2->max_7c);
        printf("   max ratio 16C = %.0f MHz\n", info2->max_8c);
    }
}

int get_turbo_ratio_limit(const unsigned socket, struct turbo_limit_data *info, struct turbo_limit_data *info2)
{
    static uint64_t *rapl_flags = NULL;
    sockets_assert(&socket, __LINE__, __FILE__);
    if (rapl_flags == NULL)
    {
        if (rapl_storage(NULL, &rapl_flags))
        {
            return -1;
        }
    }

    /* Check if MSR_TURBO_RATIO_LIMIT exists on this platform. */
    if (*rapl_flags & TURBO_RATIO_LIMIT)
    {
        read_msr_by_coord(socket, 0, 0, MSR_TURBO_RATIO_LIMIT, &(info->bits));
        calc_max_turbo_ratio(socket, info, NULL);
    }
    else
    {
        libmsr_error_handler("get_turbo_ratio_limit(): MSR_TURBO_RATIO_LIMIT not supported", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    /* Check if MSR_TURBO_RATIO_LIMIT1 exists on this platform. */
    if (*rapl_flags & TURBO_RATIO_LIMIT1)
    {
        read_msr_by_coord(socket, 0, 0, MSR_TURBO_RATIO_LIMIT1, &(info2->bits));
        calc_max_turbo_ratio(socket, NULL, info2);
    }
    else
    {
        libmsr_error_handler("get_turbo_ratio_limit(): MSR_TURBO_RATIO_LIMIT1 not supported", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}
#endif
