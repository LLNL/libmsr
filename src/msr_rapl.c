/* msr_rapl.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
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
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */

#include <math.h>
#include <omp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#include "msr_core.h"
#include "memhdlr.h"
#include "cpuid.h"
#include "msr_rapl.h"
#include "msr_turbo.h"
#include "libmsr_error.h"
#include "libmsr_debug.h"

/// @brief Set the RAPL flags indicating available registers by looking up the
/// model number of the CPU.
///
/// @param [out] rapl_flags Platform-specific bit flags indicating availability
///        of RAPL MSRs.
///
/// @return 0 if successful, else -1 if platform does not have RAPL or if
/// platform does not have MSR_RAPL_POWER_UNIT.
static int setflags(uint64_t *rapl_flags)
{
    uint64_t model = 0;
    cpuid_get_model(&model);

    switch(model)
    {
        case 0x37:
            *rapl_flags = MF_06_37;
            break;
        case 0x4A:
            *rapl_flags = MF_06_4A;
            break;
        case 0x5A:
            *rapl_flags = MF_06_5A;
            break;
        case 0x4D:
            *rapl_flags = MF_06_4D;
            break;
        case 0x4C:
            *rapl_flags = MF_06_4C;
            break;
        case 0x2A:
            *rapl_flags = MF_06_2A;
            break;
        case 0x2D:
            *rapl_flags = MF_06_2D;
            break;
        case 0x3A:
            *rapl_flags = MF_06_3A;
            break;
        case 0x3E:
            *rapl_flags = MF_06_3E;
            break;
        case 0x3C:
            *rapl_flags = MF_06_3C;
            break;
        case 0x45:
            *rapl_flags = MF_06_45;
            break;
        case 0x46:
            *rapl_flags = MF_06_46;
            break;
        case 0x3F:
            *rapl_flags = MF_06_3F;
            break;
        case 0x3D:
            *rapl_flags = MF_06_3D;
            break;
        case 0x47:
            *rapl_flags = MF_06_47;
            break;
        case 0x4F:
            *rapl_flags = MF_06_4F;
            break;
        case 0x56:
            *rapl_flags = MF_06_56;
            break;
        case 0x4E:
            *rapl_flags = MF_06_4E;
            break;
        case 0x5E:
            *rapl_flags = MF_06_5E;
            break;
        case 0x55:
            *rapl_flags = MF_06_55;
            break;
        case 0x57:
            *rapl_flags = MF_06_57;
            break;
        default:
            libmsr_error_handler("setflags(): This model number does not have RAPL", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
            break;
    }

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: setflags() model is %lx, flags are %lx at %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, model, *rapl_flags, rapl_flags);
#endif

    /* Every RAPL-enabled CPU so far has this register. */
    if (!(*rapl_flags & POWER_UNIT))
    {
        libmsr_error_handler("setflags(): No RAPL power unit MSR found", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

/// @brief Check lock bit of certain registers to determine if it's writable.
///
/// @return Number of locked registers, else -1 if rapl_storage() fails.
static int check_for_locks(void)
{
    static uint64_t *rapl_flags = NULL;
    static struct rapl_data *rapl = NULL;
    struct rapl_limit rl1, rl2;
#ifndef IS_ARCH_2D
    struct turbo_activation_ratio_data tar;
#endif
    static uint64_t sockets = 0;
    int numlocked = 0;
    unsigned i;
    const uint64_t lock = 0x80000000;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: CHECKING FOR LOCKS\n");
#endif

    if (rapl_flags == NULL || rapl == NULL || sockets == 0)
    {
        sockets = num_sockets();
        if (rapl_storage(&rapl, &rapl_flags))
        {
            return -1;
        }
    }

    /// @todo Should we flip bits for pkg_limit?
    for (i = 0; i < sockets; i++)
    {
        if (*rapl_flags & PKG_POWER_LIMIT)
        {
            get_pkg_rapl_limit(i, &rl1, &rl2);
            if (rl1.bits & (lock << 32))
            {
                numlocked++;
                fprintf(stderr, "Warning: <libmsr> MSR register locked on this architecture: check_for_locks(): Power limit 1 [0:23] of MSR_PKG_POWER_LIMIT (0x610) is locked, writes will be ignored: %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
            }
        }
        if (rl2.bits & lock)
        {
            numlocked++;
            fprintf(stderr, "Warning: <libmsr> MSR register locked on this architecture: check_for_locks(): Power limit 2 [32:55] of MSR_PKG_POWER_LIMIT (0x610) is locked, writes will be ignored: %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
        }
        if (rl1.bits & lock && rl2.bits & lock)
        {
            *rapl_flags &= ~PKG_POWER_LIMIT;
        }
        if (*rapl_flags & DRAM_POWER_LIMIT)
        {
            get_dram_rapl_limit(i, &rl1);
            if (rl1.bits & lock)
            {
                numlocked++;
                fprintf(stderr, "Warning: <libmsr> MSR register locked on this architecture: check_for_locks(): MSR_DRAM_POWER_LIMIT (0x618) is locked, writes will be ignored: %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
                *rapl_flags &= ~DRAM_POWER_LIMIT;
            }
        }
#ifndef IS_ARCH_2D
        if (*rapl_flags & TURBO_ACTIVATION_RATIO)
        {
            get_max_turbo_activation_ratio(i, &tar);
            if (tar.bits & lock)
            {
                numlocked++;
                fprintf(stderr, "Warning: <libmsr> MSR register locked on this architecture: check_for_locks(): MSR_TURBO_ACTIVATION_RATIO (0x64C) is locked, writes will be ignored: %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
                *rapl_flags &= ~TURBO_ACTIVATION_RATIO;
            }
        }
#endif
    }
    return numlocked;
}

/// @brief Translate any user-desired values to the format expected in the MSRs
/// and vice versa.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] bits Raw bit field value.
///
/// @param [out] units Human-readable value.
///
/// @param [in] type libmsr_unit_conversions_e unit conversion identifier.
///
/// @return 0 upon function completion or upon converting bits to Joules for
/// DRAM RAPL power domain for 0x3F (Haswell) platform.
static int translate(const unsigned socket, uint64_t *bits, double *units, int type)
{
    static int init = 0;
    double logremainder = 0.0;
    static uint64_t sockets = 0;
    static uint64_t model = 0;
    static struct rapl_units *ru = NULL;
    uint64_t timeval_z = 0;
    uint64_t timeval_y = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: (translate) bits are at %p\n", bits);
#endif
    if (sockets == 0)
    {
        sockets = num_sockets();
    }
    if (model == 0)
    {
        cpuid_get_model(&model);
    }
    sockets_assert(&socket, __LINE__, __FILE__);

    if (!init)
    {
        init = 1;
        ru = (struct rapl_units *) libmsr_calloc(sockets, sizeof(struct rapl_units));
        get_rapl_power_unit(ru);
    }
    switch(type)
    {
        case BITS_TO_WATTS:
            *units = (double)(*bits) * ru[socket].watts;
            break;
        case BITS_TO_JOULES_DRAM:
            if (model == 0x3F || model == 0x4F || model == 0x55)
            {
                *units = (double)(*bits) / STD_ENERGY_UNIT;
#ifdef LIBMSR_DEBUG
                fprintf(stderr, "DEBUG: (translate_dram) %f is %f joules\n", (double)*bits, *units);
#endif
                return 0;
            }
            /* No break statement, if not Haswell do standard stuff. */
        case BITS_TO_JOULES:
            *units = (double)(*bits) / ru[socket].joules;
            break;
        case WATTS_TO_BITS:
            *bits  = (uint64_t)((*units) / ru[socket].watts);
            break;
        case JOULES_TO_BITS:
            /// @todo Currently unused, but if it ever is used, we need a fix for Haswell.
            *bits  = (uint64_t)((*units) * ru[socket].joules);
            break;
        case BITS_TO_SECONDS_STD:
            timeval_y = *bits & 0x1F;
            timeval_z = (*bits & 0x60) >> 5;
            /* Dividing by time unit because it's stored as (1/(2^TU))^-1. */
            *units = ((1 + 0.25 * timeval_z) * pow(2.0,(double)timeval_y)) / ru[socket].seconds;
            // Temporary fix for haswell
            //    if (model == 0x3F)
            //    {
            //        *units = *units * 2.5 + 15.0;
            //    }
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: timeval_z is %lx, timeval_y is %lx, units is %lf, bits is %lx\n", getenv("HOSTNAME"), __FILE__, __LINE__, timeval_z, timeval_y, *units, *bits);
#endif
            break;
        case SECONDS_TO_BITS_STD:
            // Temporary fix for haswell
            //    if (model == 0x3F)
            //    {
            //        *units = *units / 2.5 - 15;
            //    }
            /* Store the whole number part of the log2. */
            timeval_y = (uint64_t)log2(*units * ru[socket].seconds);
            /* Store the mantissa of the log2. */
            logremainder = (double)log2(*units * ru[socket].seconds) - (double)timeval_y;
            timeval_z = 0;
            /* Based on the mantissa, choose the appropriate multiplier. */
            if (logremainder > 0.15 && logremainder <= 0.45)
            {
                timeval_z = 1;
            }
            else if (logremainder > 0.45 && logremainder <= 0.7)
            {
                timeval_z = 2;
            }
            else if (logremainder > 0.7)
            {
                timeval_z = 3;
            }
            /* Store the bits in the Intel specified format. */
            *bits = (uint64_t)(timeval_y | (timeval_z << 5));
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: timeval_z is %lx, timeval_y is %lx, units is %lf, bits is %lx, remainder is %lf\n", getenv("HOSTNAME"), __FILE__, __LINE__, timeval_z, timeval_y, *units, *bits, logremainder);
#endif
            break;
        default:
            fprintf(stderr, "%s:%d  Unknown value %d.  This is bad.\n", __FILE__, __LINE__, type);
            *bits = -1;
            *units= -1.0;
            break;
    }
    return 0;
}

/// @brief Create the human-readable power settings if the user-supplied bits.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] limit Data for desired power limit.
///
/// @param [in] offset Power limit 1 (lower) or power limit 2 (upper)
///        identifier. Power limit 2 only applies to package domain.
///
/// @return 0 if successful, else -1 if translate() failed.
static int calc_rapl_from_bits(const unsigned socket, struct rapl_limit *limit, const unsigned offset)
{
    uint64_t watts_bits = 0;
    uint64_t seconds_bits = 0;
    int ret = 0;

    sockets_assert(&socket, __LINE__, __FILE__);

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_rapl_from_bits)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    watts_bits = MASK_VAL(limit->bits, 14+offset, 0+offset);
    seconds_bits = MASK_VAL(limit->bits, 23+offset, 17+offset);

    // We have been given the bits to be written to the msr.
    // For sake of completeness, translate these into watts and seconds.
    ret = translate(socket, &watts_bits, &limit->watts, BITS_TO_WATTS);
    // If the offset is > 31 (we are writing the upper PKG limit), then no
    // translation needed
    if (offset < 32)
    {
        ret += translate(socket, &seconds_bits, &limit->seconds, BITS_TO_SECONDS_STD);
    }
    else
    {
        limit->seconds = seconds_bits;
    }
    if (ret < 0)
    {
        libmsr_error_handler("calc_rapl_from_bits(): Translation from bits to values failed", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return ret;
    }
    return 0;
}

/// @brief Translates human-readable power settings into raw 64-bit format.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] limit Data for desired power limit.
///
/// @param [in] offset Power limit 1 (lower) or power limit 2 (upper)
///        identifier. Power limit 2 only applies to package domain.
///
/// @return 0 if successful, else -1 if rapl_storage() fails or if the Watts
/// value or seconds value overflow the bit field.
static int calc_rapl_bits(const unsigned socket, struct rapl_limit *limit, const unsigned offset)
{
    uint64_t watts_bits = 0;
    uint64_t seconds_bits = 0;
    sockets_assert(&socket, __LINE__, __FILE__);

    watts_bits = MASK_VAL(limit->bits, 14+offset, 0+offset);
    seconds_bits = MASK_VAL(limit->bits, 23+offset, 17+offset);

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_rapl_bits)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    /*
     * We have been given watts and seconds and need to translate these into
     * bit values.
     * If offset is >= 32 (we are setting the 2nd pkg limit), we don't need time
     * conversion.
     */
    if (offset >= 32)
    {
        seconds_bits = (uint64_t)limit->seconds; // unit is milliseconds
        //translate(socket, &seconds_bits, &limit->seconds, SECONDS_TO_BITS_STD);
    }
    else
    {
        translate(socket, &seconds_bits, &limit->seconds, SECONDS_TO_BITS_STD);
    }
    /* There is only 1 translation for watts (so far). */
    translate(socket, &watts_bits, &limit->watts, WATTS_TO_BITS);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "Converted %lf watts into %lx bits.\n", limit->watts, watts_bits);
    fprintf(stderr, "Converted %lf seconds into %lx bits.\n", limit->seconds, seconds_bits);
#endif
    /* Check to make sure the watts value does not overflow the bit field. */
    if (watts_bits & 0xFFFFFFFFFFFF8000)
    {
        libmsr_error_handler("calc_rapl_bits(): Translation from bits to values failed", LIBMSR_ERROR_INVAL, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    watts_bits <<= 0 + offset;
    /* Check to make sure the seconds value does not overflow the bit field. */
    if (seconds_bits & 0xFFFFFFFFFFFFFF80)
    {
        libmsr_error_handler("calc_rapl_bits(): Seconds value is too large", LIBMSR_ERROR_INVAL, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    seconds_bits <<= 17 + offset;
    limit->bits |= watts_bits;
    limit->bits |= seconds_bits;
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "calculated rapl bits\n");
#endif
    return 0;
}

/// @brief Determine how the user setup the package domain RAPL power limits
/// and setup other limit accordingly.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] limit1 Data for desired power limit 1 (lower).
///
/// @param [out] limit2 Data for desired power limit 2 (upper).
///
/// @return 0 if successful, else -1 if calc_rapl_bits() or
/// calc_rapl_from_bits() fails.
static int calc_pkg_rapl_limit(const unsigned socket, struct rapl_limit *limit1, struct rapl_limit *limit2)
{
    sockets_assert(&socket, __LINE__, __FILE__);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_pkg_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif

    /* If we have been given a lower rapl limit. */
    if (limit1 != NULL)
    {
        if (limit1->bits)
        {
            if (calc_rapl_from_bits(socket, limit1, 0))
            {
                return -1;
            }
        }
        else
        {
            if (calc_rapl_bits(socket, limit1, 0))
            {
                return -1;
            }
        }
    }
    /* If we have been given an upper rapl limit. */
    if (limit2 != NULL)
    {
        if (limit2->bits)
        {
            if (calc_rapl_from_bits(socket, limit2, 32))
            {
                return -1;
            }
        }
        else
        {
            if (calc_rapl_bits(socket, limit2, 32))
            {
                return -1;
            }
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "pkg calculated\n");
#endif
    return 0;
}

/// @brief Determine how the user setup non-package RAPL power limits and setup
/// the data for the other limit accordingly.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] limit Data for desired power limit.
///
/// @return 0 if successful, else -1 if calc_rapl_from_bits() or
/// calc_rapl_bits() fails.
static int calc_std_rapl_limit(const unsigned socket, struct rapl_limit *limit)
{
    sockets_assert(&socket, __LINE__, __FILE__);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (calc_std_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif

    if (limit->bits)
    {
        if (calc_rapl_from_bits(socket, limit, 0))
        {
            return -1;
        }
    }
    else
    {
        if (calc_rapl_bits(socket, limit, 0))
        {
            return -1;
        }
    }
    return 0;
}

/// @brief Get number of batch MSRs available on platform based on bit flags.
///
/// @param [in] rapl_flags Platform-specific bit flags indicating availability
///        of RAPL MSRs.
///
/// @return Number of available MSRs on platform.
static uint64_t rapl_data_batch_size(uint64_t *rapl_flags)
{
    uint64_t size = 0;

    if (*rapl_flags & PKG_PERF_STATUS)
    {
        size++;
    }
    if (*rapl_flags & PKG_ENERGY_STATUS)
    {
        size++;
    }
    if (*rapl_flags & DRAM_ENERGY_STATUS)
    {
        size++;
    }
    if (*rapl_flags & PKG_ENERGY_STATUS)
    {
        size++;
    }
    return size;
}

/// @brief Allocate RAPL data for batch operations.
///
/// @param [in] rapl_flags Platform-specific bit flags indicating availability
///        of RAPL MSRs.
///
/// @param [in] rapl Measurements of energy, time, and power data from a given
///        RAPL power domain.
static void create_rapl_data_batch(uint64_t *rapl_flags, struct rapl_data *rapl)
{
    uint64_t sockets = num_sockets();

    allocate_batch(RAPL_DATA, rapl_data_batch_size(rapl_flags) * sockets);
    if (*rapl_flags & PKG_ENERGY_STATUS)
    {
        rapl->pkg_bits = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        rapl->pkg_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->old_pkg_bits = (uint64_t *) libmsr_calloc(sockets, sizeof(uint64_t));
        rapl->old_pkg_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->pkg_delta_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->pkg_watts = (double *) libmsr_calloc(sockets, sizeof(double));
        load_socket_batch(MSR_PKG_ENERGY_STATUS, rapl->pkg_bits, RAPL_DATA);
    }
    if (*rapl_flags & PKG_PERF_STATUS)
    {
        rapl->pkg_perf_count = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t));
        load_socket_batch(MSR_PKG_PERF_STATUS, rapl->pkg_perf_count, RAPL_DATA);
    }
    if (*rapl_flags & DRAM_ENERGY_STATUS)
    {
        rapl->dram_bits = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        rapl->old_dram_bits = (uint64_t *) libmsr_calloc(sockets, sizeof(uint64_t));
        rapl->dram_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->old_dram_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->dram_delta_joules = (double *) libmsr_calloc(sockets, sizeof(double));
        rapl->dram_watts = (double *) libmsr_calloc(sockets, sizeof(double));
        load_socket_batch(MSR_DRAM_ENERGY_STATUS, rapl->dram_bits, RAPL_DATA);
    }
    if (*rapl_flags & DRAM_PERF_STATUS)
    {
        rapl->dram_perf_count = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t));
        load_socket_batch(MSR_DRAM_PERF_STATUS, rapl->dram_perf_count, RAPL_DATA);
    }
}

int rapl_storage(struct rapl_data **data, uint64_t **flags)
{
    static struct rapl_data *rapl = NULL;
    static uint64_t *rapl_flags = NULL;
    static uint64_t sockets = 0;
    static int init = 0;

#ifdef STORAGE_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (rapl_storage) data pointer is %p, flags pointer is %p, data is at %p, flags are %lx at %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, data, flags, rapl, (rapl_flags ? *rapl_flags : 0), rapl_flags);
#endif

    if (!init)
    {
        init = 1;
        sockets = num_sockets();

        rapl = (struct rapl_data *) libmsr_malloc(sockets * sizeof(struct rapl_data));
        rapl_flags = (uint64_t *) libmsr_malloc(sizeof(uint64_t));

        if (setflags(rapl_flags))
        {
            return -1;
        }
        if (data != NULL)
        {
            *data = rapl;
        }
        if (flags != NULL)
        {
            *flags = rapl_flags;
        }
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "%s %s::%d DEBUG: (storage) initialized rapl data at %p, flags are %lx, (flags at %p, rapl_flags at %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, rapl, **flags, flags, rapl_flags);
        fprintf(stderr, "DEBUG: socket 0 has pkg_bits at %p\n", &rapl[0].pkg_bits);
#endif
        return 0;
    }
    /* If the data pointer is not null, it should point to the rapl array. */
    if (data != NULL)
    {
        *data = rapl;
    }
    /* if the flags pointer is not null, it should point to the rapl flags. */
    if (flags != NULL)
    {
        *flags = rapl_flags;
    }
    return 0;
}

int print_available_rapl(void)
{
    uint64_t *rapl_flags = NULL;

    if (rapl_storage(NULL, &rapl_flags))
    {
        libmsr_error_handler("print_available_rapl(): Could not load RAPL data", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    if (*rapl_flags & POWER_UNIT)
    {
        fprintf(stdout, "MSR_RAPL_POWER_UNIT, 606h\n");
    }
    if (*rapl_flags & PKG_POWER_LIMIT)
    {
        fprintf(stdout, "MSR_PKG_POWER_LIMIT, 610h\n");
    }
    if (*rapl_flags & PKG_ENERGY_STATUS)
    {
        fprintf(stdout, "MSR_PKG_ENERGY_STATUS, 611h\n");
    }
    if (*rapl_flags & PKG_PERF_STATUS)
    {
        fprintf(stdout, "MSR_PKG_PERF_STATUS, 613h\n");
    }
    if (*rapl_flags & PKG_POWER_INFO)
    {
        fprintf(stdout, "MSR_PKG_POWER_INFO, 614h\n");
    }
    if (*rapl_flags & DRAM_POWER_LIMIT)
    {
        fprintf(stdout, "MSR_DRAM_POWER_LIMIT, 618h\n");
    }
    if (*rapl_flags & DRAM_ENERGY_STATUS)
    {
        fprintf(stdout, "MSR_DRAM_ENERGY_STATUS, 619h\n");
    }
    if (*rapl_flags & DRAM_PERF_STATUS)
    {
        fprintf(stdout, "MSR_DRAM_PERF_STATUS, 61Bh\n");
    }
    if (*rapl_flags & DRAM_POWER_INFO)
    {
        fprintf(stdout, "MSR_DRAM_POWER_INFO, 61Ch\n");
    }
    return 0;
}

int rapl_init(struct rapl_data **rapl, uint64_t **rapl_flags)
{
    static int init = 0;
    int ret = 0;
    if (init)
    {
        fprintf(stderr, "Warning: <libmsr> Reinitialized rapl: rapl_init(): %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    /* Can now call init more than once. */
    init = 1;
    if (rapl_storage(rapl, rapl_flags))
    {
        ret = -1;
        return ret;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "DEBUG: (init) rapl initialized at %p, flags are %lx at %p\n", *rapl, **rapl_flags, *rapl_flags);
#endif
    ret = check_for_locks();
    return ret;
}

int set_pkg_rapl_limit(const unsigned socket, struct rapl_limit *limit1, struct rapl_limit *limit2)
{
    uint64_t pkg_limit = 0;
    static uint64_t *rapl_flags = NULL;
    uint64_t currentval = 0;
    int ret = 0;
    sockets_assert(&socket, __LINE__, __FILE__);

    if (rapl_flags == NULL)
    {
        if (rapl_storage(NULL, &rapl_flags))
        {
            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (set_pkg_rapl_limit) flags are at %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, rapl_flags);
#endif

    /* Make sure the pkg power limit register exists. */
    if (*rapl_flags & PKG_POWER_LIMIT)
    {
        /* If there is only one limit, grab the other existing one. */
        if (limit1 == NULL)
        {
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: only one rapl limit, retrieving any existing power limits\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
            ret = read_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, &currentval);
            /* We want to keep the lower limit so mask off all other bits. */
            pkg_limit |= currentval & 0x00000000FFFFFFFF;
        }
        else if (limit2 == NULL)
        {
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "%s %s::%d DEBUG: only one rapl limit, retrieving any existing power limits\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
            ret = read_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, &currentval);
            /* We want to keep the upper limit so mask off all other bits. */
            pkg_limit |= currentval & 0xFFFFFFFF00000000;
        }
        if (calc_pkg_rapl_limit(socket, limit1, limit2))
        {
            return -1;
        }
        /* Enable the rapl limit (15 && 47) and turn on clamping (16 && 48). */
        if (limit1 != NULL)
        {
            pkg_limit |= limit1->bits | (1LL << 15) | (1LL << 16);
        }
        if (limit2 != NULL)
        {
            pkg_limit |= limit2->bits | (1LL << 47) | (1LL << 48);
        }
        if (limit1 != NULL || limit2 != NULL)
        {
            ret += write_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, pkg_limit);
        }
    }
    else
    {
        libmsr_error_handler("set_pkg_rapl_limit(): PKG domain RAPL limit not supported on this architecture", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "pkg set\n");
#endif
    return ret;
}

int set_dram_rapl_limit(const unsigned socket, struct rapl_limit *limit)
{
    uint64_t dram_limit = 0;
    static uint64_t *rapl_flags = NULL;

    sockets_assert(&socket, __LINE__, __FILE__);
    if (rapl_flags == NULL)
    {
        if (rapl_storage(NULL, &rapl_flags))
        {
            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (set_dram_rapl_limit)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    /* Make sure the dram power limit register exists. */
    if (*rapl_flags & DRAM_POWER_LIMIT)
    {
        if (limit != NULL)
        {
            if (calc_std_rapl_limit(socket, limit))
            {
                return -1;
            }
            dram_limit |= limit->bits | (1LL << 15);
            write_msr_by_coord(socket, 0, 0, MSR_DRAM_POWER_LIMIT, dram_limit);
        }
    }
    else
    {
        libmsr_error_handler("set_dram_rapl_limit(): DRAM domain RAPL limit not supported on this architecture", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int get_rapl_power_info(const unsigned socket, struct rapl_power_info *info)
{
    uint64_t val = 0;
    static uint64_t *rapl_flags = NULL;

    sockets_assert(&socket, __LINE__, __FILE__);

    if (rapl_flags == NULL)
    {
        if (rapl_storage(NULL, &rapl_flags))
        {
            libmsr_error_handler("get_rapl_power_info(): Cannot load rapl_flags", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (get_rapl_power_info)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (*rapl_flags & PKG_POWER_INFO)
    {
        read_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_INFO, &(info->msr_pkg_power_info));
        val = MASK_VAL(info->msr_pkg_power_info, 54, 48);
        translate(socket, &val, &(info->pkg_max_window), BITS_TO_SECONDS_STD);

        val = MASK_VAL(info->msr_pkg_power_info, 46, 32);
        translate(socket, &val, &(info->pkg_max_power), BITS_TO_WATTS);

        val = MASK_VAL(info->msr_pkg_power_info, 30, 16);
        translate(socket, &val, &(info->pkg_min_power), BITS_TO_WATTS);

        val = MASK_VAL(info->msr_pkg_power_info, 14, 0);
        translate(socket, &val, &(info->pkg_therm_power), BITS_TO_WATTS);
    }
    if (*rapl_flags & DRAM_POWER_INFO)
    {
        read_msr_by_coord(socket, 0, 0, MSR_DRAM_POWER_INFO, &(info->msr_dram_power_info));

        val = MASK_VAL(info->msr_dram_power_info, 54, 48);
        translate(socket, &val, &(info->dram_max_window), BITS_TO_SECONDS_STD);

        val = MASK_VAL(info->msr_dram_power_info, 46, 32);
        translate(socket, &val, &(info->dram_max_power), BITS_TO_WATTS);

        val = MASK_VAL(info->msr_dram_power_info, 30, 16);
        translate(socket, &val, &(info->dram_min_power), BITS_TO_WATTS);

        val = MASK_VAL(info->msr_dram_power_info, 14, 0);
        translate(socket, &val, &(info->dram_therm_power), BITS_TO_WATTS);
    }
    return 0;
}

int get_pkg_rapl_limit(const unsigned socket, struct rapl_limit *limit1, struct rapl_limit *limit2)
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

    /* Make sure the pkg power limit register exists. */
    if (*rapl_flags & PKG_POWER_LIMIT)
    {
        if (limit1 != NULL)
        {
            read_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit1->bits));
        }
        if (limit2 != NULL)
        {
            read_msr_by_coord(socket, 0, 0, MSR_PKG_POWER_LIMIT, &(limit2->bits));
        }
        calc_pkg_rapl_limit(socket, limit1, limit2);
    }
    else
    {
        libmsr_error_handler("get_pkg_rapl_limit(): PKG domain RAPL power limit not supported on this architecture", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int get_dram_rapl_limit(const unsigned socket, struct rapl_limit *limit)
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

    /* Make sure the dram power limit register exists. */
    if ((limit != NULL) && (*rapl_flags & DRAM_POWER_LIMIT))
    {
        read_msr_by_coord(socket, 0, 0, MSR_DRAM_POWER_LIMIT, &(limit->bits));
        calc_std_rapl_limit(socket, limit);
    }
    else if (limit != NULL)
    {
        libmsr_error_handler("get_dram_rapl_limit(): DRAM domain RAPL power limit not supported on this architecture", LIBMSR_ERROR_PLATFORM_NOT_SUPPORTED, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

void dump_rapl_limit(struct rapl_limit *L, FILE *writedest)
{
    fprintf(writedest, "bits    = %lx\n", L->bits);
    fprintf(writedest, "seconds = %lf\n", L->seconds);
    fprintf(writedest, "watts   = %lf\n", L->watts);
}

int dump_rapl_data_terse_label(FILE *writedest)
{
    int socket;
    static struct rapl_data *rapl = NULL;
    static uint64_t *rapl_flags = NULL;
    static uint64_t sockets = 0;

    if (rapl == NULL || rapl_flags == NULL || sockets == 0)
    {
        sockets = num_sockets();
        if (rapl_storage(&rapl, &rapl_flags))
        {
            return -1;
        }
    }
    for (socket = 0; socket < sockets; socket++)
    {
        /* Check to see what registers are available. */
        if (*rapl_flags & PKG_ENERGY_STATUS)
        {
            fprintf(writedest, "pkgW%0d ", socket);
        }
        if (*rapl_flags & DRAM_ENERGY_STATUS)
        {
            fprintf(writedest, "dramW%0d ", socket);
        }
    }
    return 0;
}

int dump_rapl_data_terse(FILE *writedest)
{
    int socket;
    static struct rapl_data *rapl = NULL;
    static uint64_t *rapl_flags = NULL;
    static uint64_t sockets = 0;

    if (rapl == NULL || rapl_flags == NULL || sockets == 0)
    {
        sockets = num_sockets();
        if (rapl_storage(&rapl, &rapl_flags))
        {
            return -1;
        }
    }

#ifdef LIBMSR_DEBUG
    fprintf(writedest, "%s %s::%d Writing terse label\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    read_rapl_data();
    delta_rapl_data();

    for (socket = 0; socket < sockets; socket++)
    {
        /* Check to see which registers are available. */
        if (*rapl_flags & PKG_ENERGY_STATUS)
        {
            fprintf(writedest, "%8.4lf ", rapl->pkg_watts[socket]);
        }
        if (*rapl_flags & DRAM_ENERGY_STATUS)
        {
            fprintf(writedest, "%8.4lf ", rapl->dram_watts[socket]);
        }
    }
    return 0;
}

int dump_rapl_data(FILE *writedest)
{
    static int init = 0;
    static uint64_t *rapl_flags = NULL;
    static struct rapl_data *r = NULL;
    static struct timeval start;
    static uint64_t sockets = 0;
    struct timeval now;

    if (!init)
    {
        sockets = num_sockets();
        init = 1;
        gettimeofday(&start, NULL);
        if (rapl_storage(&r, &rapl_flags))
        {
            return -1;
        }
    }
#ifdef LIBMSR_DEBUG
    fprintf(writedest, "pkg_bits = %8.4lx   pkg_joules= %8.4lf\n", *r->pkg_bits, r->pkg_joules);
#endif
    gettimeofday(&now, NULL);
    int s;
    for (s = 0; s < sockets; s++)
    {
        fprintf(writedest, "Socket: %d\n", s);
        if (*rapl_flags & PKG_ENERGY_STATUS)
        {
            fprintf(writedest, "pkg_watts = %8.4lf   elapsed= %8.5lf   timestamp= %9.6lf\n", r->pkg_watts[s], r->elapsed, now.tv_sec - start.tv_sec + (now.tv_usec - start.tv_usec)/1000000.0);
        }
        if (*rapl_flags & DRAM_ENERGY_STATUS)
        {
            fprintf(writedest, "dram_watts= %8.4lf   elapsed= %8.5lf   timestamp= %9.6lf\n", r->dram_watts[s], r->elapsed, now.tv_sec - start.tv_sec + (now.tv_usec - start.tv_usec)/1000000.0);
        }
    }
    return 0;
}

int dump_rapl_power_info(FILE *writedest)
{
    int socket;
    static uint64_t *rapl_flags = NULL;
    struct rapl_power_info info;
    static uint64_t sockets = 0;

    if (rapl_flags == NULL || sockets == 0)
    {
        sockets = num_sockets();
        if (rapl_storage(NULL, &rapl_flags))
        {
            return -1;
        }
    }
    for (socket = 0; socket < sockets; socket++)
    {
        get_rapl_power_info(socket, &info);
        if (*rapl_flags & PKG_POWER_INFO)
        {
            fprintf(writedest, "Socket: %d\n", socket);
            fprintf(writedest, "   pkg_max_power   (W) = %8.4lf  pkg_min_power    (W) = %8.4lf\n", info.pkg_max_power, info.pkg_min_power);
            fprintf(writedest, "   pkg_max_window  (s) = %8.4lf  pkg_therm_power  (W) = %8.4lf\n", info.pkg_max_window, info.pkg_therm_power);
        }
        if (*rapl_flags & DRAM_POWER_INFO)
        {
            fprintf(writedest, "Socket: %d\n", socket);
            fprintf(writedest, "   dram_max_power  (W) = %8.4lf  dram_min_power   (W) = %8.4lf\n", info.dram_max_power, info.dram_min_power);
            fprintf(writedest, "   dram_max_window (s) = %8.4lf  dram_therm_power (W) = %8.4lf\n", info.dram_max_window, info.dram_therm_power);
        }
    }
    return 0;
}

void get_rapl_power_unit(struct rapl_units *ru)
{
    static int init = 0;
    static uint64_t sockets = 0;
    static uint64_t **val = NULL;
    int i;

    sockets = num_sockets();
    if (!init)
    {
        init = 1;
        val = (uint64_t **) libmsr_calloc(sockets, sizeof(uint64_t *));
        allocate_batch(RAPL_UNIT, sockets);
        load_socket_batch(MSR_RAPL_POWER_UNIT, val, RAPL_UNIT);
    }
    read_batch(RAPL_UNIT);
    /* Initialize the units used for each socket. */
    for (i = 0; i < sockets; i++)
    {
        // See figure 14-16 for bit fields.
        //  1  1 1  1 1
        //  9  6 5  2 1  8 7  4 3  0
        //
        //  1010 0001 0000 0000 0011
        //
        //     A    1    0    0    3
        //ru[i].msr_rapl_power_unit = 0xA1003;

        ru[i].msr_rapl_power_unit = *val[i];
        /* Default is 1010b or 976 microseconds. */
        /* Storing (1/(2^TU))^-1 for maximum precision. */
        ru[i].seconds = (double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 19, 16)));
        /* Default is 10000b or 15.3 microjoules. */
        /* Storing (1/(2^ESU))^-1 for maximum precision. */
        ru[i].joules = (double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 12, 8)));
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: joules unit is %f register has %lx\n", ru[i].joules, ru[i].msr_rapl_power_unit);
#endif
        /* Default is 0011b or 1/8 Watts. */
        ru[i].watts = ((1.0)/((double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 3, 0)))));
#ifdef LIBMSR_DEBUG
        fprintf(stdout, "Pkg %d MSR_RAPL_POWER_UNIT\n", i);
        fprintf(stdout, "Raw: %f sec, %f J, %f watts\n", ru[i].seconds, ru[i].joules, ru[i].watts);
        fprintf(stdout, "Adjusted: %f sec, %f J, %f watts\n", 1/ru[i].seconds, 1/ru[i].joules, ru[i].watts);
#endif
    }

    /* Check consistency between packages. */
    uint64_t *tmp = (uint64_t *) libmsr_calloc(sockets, sizeof(uint64_t));
    for (i = 0; i < sockets; i++)
    {
        read_msr_by_coord(i, 0, 0, MSR_RAPL_POWER_UNIT, tmp);
        double energy = (double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 12, 8)));
        double seconds = (double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 19, 16)));
        double power = ((1.0)/((double)(1 << (MASK_VAL(ru[i].msr_rapl_power_unit, 3, 0)))));
        if (energy != ru[i].joules || power != ru[i].watts || seconds != ru[i].seconds)
        {
            libmsr_error_handler("get_rapl_power_unit(): Inconsistent rapl power units across packages", LIBMSR_ERROR_RUNTIME, getenv("HOSTNAME"), __FILE__, __LINE__);
        }
    }
}

void dump_rapl_power_unit(FILE *writedest)
{
    int socket;
    struct rapl_units *r;
    static uint64_t sockets = 0;

    sockets = num_sockets();
    r = (struct rapl_units *) libmsr_calloc(sockets, sizeof(struct rapl_units));
    get_rapl_power_unit(r);

    for (socket = 0; socket < sockets; socket++)
    {
        fprintf(writedest, "Socket: %d\n", socket);
        fprintf(writedest, "   RAW power unit (W) = %8.4lf  energy unit (J^-1) = %8.4lf  time unit (s^-1) = %8.4lf\n", r[socket].watts, r[socket].joules, r[socket].seconds);
        fprintf(writedest, "   ADJ power unit (W) = %f  energy unit (J)    = %f    time unit (s)    = %f\n", r[socket].watts, 1/r[socket].joules, 1/r[socket].seconds);
    }
}

int poll_rapl_data(void)
{
    static struct rapl_data *rapl = NULL;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (poll_rapl_data) socket=%lu\n", getenv("HOSTNAME"), __FILE__, __LINE__, num_sockets());
#endif

    if (rapl == NULL)
    {
        if (rapl_storage(&rapl, NULL))
        {
            return -1;
        }
    }

    if (rapl == NULL)
    {
        libmsr_error_handler("poll_rapl_data(): RAPL init failed or has not yet been called", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    read_rapl_data();
    delta_rapl_data();

    return 0;
}

int delta_rapl_data(void)
{
    /* The energy status register holds 32 bits, this is max unsigned int. */
    static double max_joules = UINT_MAX / 65536; // This fixed wraparound problem
    static int init = 0;
    static uint64_t sockets = 0;
    static uint64_t *rapl_flags;
    static struct rapl_data *rapl;
    int s = 0;

#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (delta_rapl_data)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    if (!init)
    {
        sockets = num_sockets();
        if (rapl_storage(&rapl, &rapl_flags))
        {
            return -1;
        }
        for (s = 0; s < sockets; s++)
        {
            if (*rapl_flags & PKG_ENERGY_STATUS)
            {
                rapl->pkg_watts[s] = 0.0;
            }
            if (*rapl_flags & DRAM_ENERGY_STATUS)
            {
                rapl->dram_watts[s] = 0.0;
            }
        }
        init = 1;
        rapl->elapsed = 0;
        return 0;
    }
    /*
     * Get delta joules.
     * Now handles wraparound.
     * Make sure the pkg energy status register exists
     */
    for (s = 0; s < sockets; s++)
    {
        if (*rapl_flags & PKG_ENERGY_STATUS)
        {
            /* Check to see if there was wraparound and use corresponding translation. */
            if (rapl->pkg_joules[s] - rapl->old_pkg_joules[s] < 0)
            {
                //fprintf(stderr, "DEBUG: pkg wrap\n");
                //fprintf(stderr, "DEBUG: pkg_joules[%d] %lf, old_pkg_joules[%d] %lf, max_joules %lf\n", s, rapl->pkg_joules[s], s, rapl->old_pkg_joules[s], max_joules);
                rapl->pkg_delta_joules[s] = (rapl->pkg_joules[s] + max_joules) - rapl->old_pkg_joules[s];
            }
            else
            {
                //fprintf(stderr, "DEBUG: pkg_joules[%d] %lf, old_pkg_joules[%d] %lf\n", s, rapl->pkg_joules[s], s, rapl->old_pkg_joules[s]);
                rapl->pkg_delta_joules[s] = rapl->pkg_joules[s] - rapl->old_pkg_joules[s];
            }
        }
        /* Make sure the dram energy status register exists. */
        if (*rapl_flags & DRAM_ENERGY_STATUS)
        {
            /* Check to see if there was wraparound and use corresponding translation. */
            if (rapl->dram_joules[s] - rapl->old_dram_joules[s] < 0)
            {
                rapl->dram_delta_joules[s] = (rapl->dram_joules[s] + max_joules) - rapl->old_dram_joules[s];
            }
            else
            {
                rapl->dram_delta_joules[s] = rapl->dram_joules[s] - rapl->old_dram_joules[s];
            }
        }
        /* Get watts. */
        if (rapl->elapsed > 0.0L)
        {
            /* Make sure the pkg power limit register exists. */
            if (*rapl_flags & PKG_ENERGY_STATUS)
            {
                rapl->pkg_watts[s]  = rapl->pkg_delta_joules[s]  / rapl->elapsed;
                //fprintf(stderr, "DEBUG: pkg_watts[%d] %lf\n", s, rapl->pkg_watts[s]);
                //fprintf(stderr, "DEBUG: pkg_delta_joules[%d] %lf, elapsed %lf\n", s, rapl->pkg_delta_joules[s], rapl->elapsed);
            }
            if (*rapl_flags & DRAM_ENERGY_STATUS)
            {
                rapl->dram_watts[s] = rapl->dram_delta_joules[s] / rapl->elapsed;
            }
        }
        else
        {
            rapl->pkg_watts[s] = 0.0;
            /* Make sure the dram power limit register exists. */
            if (*rapl_flags & DRAM_ENERGY_STATUS)
            {
                rapl->dram_watts[s] = 0.0;
            }
        }
    }
    return 0;
}

int read_rapl_data(void)
{
    static struct rapl_data *rapl = NULL;
    static uint64_t *rapl_flags = NULL;
    static short init = 0;
    static uint64_t sockets = 0;
    int s;

    if (!init)
    {
        sockets = num_sockets();
        if (rapl_storage(&rapl, &rapl_flags))
        {
            return -1;
        }
        create_rapl_data_batch(rapl_flags, rapl);
        rapl->now.tv_sec = 0;
        rapl->now.tv_usec = 0;
        rapl->old_now.tv_sec = 0;
        rapl->old_now.tv_usec = 0;
        rapl->elapsed = 0;
        for (s = 0; s < sockets; s++)
        {
            rapl->pkg_joules[s] = 0;
            rapl->old_pkg_joules[s] = 0;
            rapl->dram_joules[s] = 0;
            rapl->old_dram_joules[s] = 0;
        }
    }
    //p = &rapl[socket];
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (read_rapl_data): socket=%lu at address %p\n", getenv("HOSTNAME"), __FILE__, __LINE__, num_sockets(), rapl);
#endif
    /* Move current variables to "old" variables. */
    rapl->old_now.tv_sec = rapl->now.tv_sec;
    rapl->old_now.tv_usec = rapl->now.tv_usec;
    /* Grab a timestamp. */
    gettimeofday(&(rapl->now), NULL);
    if (init)
    {
        rapl->elapsed = (rapl->now.tv_sec - rapl->old_now.tv_sec) +
                        (rapl->now.tv_usec - rapl->old_now.tv_usec)/1000000.0;
        for (s = 0; s < sockets; s++)
        {
            /* Make sure the pkg energy status register exists. */
            if (*rapl_flags & PKG_ENERGY_STATUS)
            {
#ifdef LIBMSR_DEBUG
                fprintf(stderr, "DEBUG: socket %lu msr 0x611 has destination %p\n", sockets, rapl->pkg_bits);
#endif
                rapl->old_pkg_bits[s] = *rapl->pkg_bits[s];
                rapl->old_pkg_joules[s] = rapl->pkg_joules[s];
            }
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "DEBUG: (read_rapl_data): made it to 1st mark\n");
#endif
            /* Make sure the pkg perf status register exists. */
            if (*rapl_flags & PKG_PERF_STATUS)
            {
                libmsr_error_handler("read_rapl_data(): MSR_PKG_PERF_STATUS not yet implemented", LIBMSR_ERROR_NOT_IMPLEMENTED_YET, getenv("HOSTNAME"), __FILE__, __LINE__);
            }
            /* Make sure the dram energy status register exists. */
            if (*rapl_flags & DRAM_ENERGY_STATUS)
            {
                rapl->old_dram_bits[s]	= *rapl->dram_bits[s];
                rapl->old_dram_joules[s] = rapl->dram_joules[s];
            }
            /* Make sure the dram perf status register exists. */
            if (*rapl_flags & DRAM_PERF_STATUS)
            {
                libmsr_error_handler("read_rapl_data(): MSR_DRAM_PERF_STATUS not yet implemented", LIBMSR_ERROR_NOT_IMPLEMENTED_YET, getenv("HOSTNAME"), __FILE__, __LINE__);
            }
        }
    }
    read_batch(RAPL_DATA);
    for (s = 0; s < sockets; s++)
    {
        if (*rapl_flags & DRAM_ENERGY_STATUS)
        {
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "DEBUG: (read_rapl_data): translating dram\n");
#endif
            translate(s, rapl->dram_bits[s], &rapl->dram_joules[s], BITS_TO_JOULES_DRAM);
        }
        if (*rapl_flags & PKG_ENERGY_STATUS)
        {
#ifdef LIBMSR_DEBUG
            fprintf(stderr, "DEBUG: (read_rapl_data): translating pkg\n");
#endif
            translate(s, rapl->pkg_bits[s], &rapl->pkg_joules[s], BITS_TO_JOULES);
        }
#ifdef LIBMSR_DEBUG
        fprintf(stderr, "DEBUG: socket %d\n", s);
        fprintf(stderr, "DEBUG: elapsed %f\n", rapl->elapsed);
        fprintf(stderr, "DEBUG: pkg_bits %lx\n", *rapl->pkg_bits[s]);
        fprintf(stderr, "DEBUG: pkg_joules %lf\n", rapl->pkg_joules[s]);
        fprintf(stderr, "DEBUG: pkg_watts %lf\n", rapl->pkg_watts[s]);
        fprintf(stderr, "DEBUG: delta_joules %lf\n", rapl->pkg_delta_joules[s]);
#endif
    }
    init = 1;
    return 0;
}
