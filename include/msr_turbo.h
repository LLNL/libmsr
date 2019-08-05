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

#ifndef MSR_TURBO_H_INCLUDE
#define MSR_TURBO_H_INCLUDE

#include <stdio.h>

#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure containing data for MSR_TURBO_ACTIVATION_RATIO.
struct turbo_activation_ratio_data
{
    /// @brief Raw 64-bit value stored in MSR_TURBO_ACTIVATION RATIO.
    uint64_t bits;
    /// @brief Max non-turbo ratio bit-field (bits [7:0]) converted to a
    /// p-state in MHz.
    double max_non_turbo_ratio;
};

/// @brief Structure holding data from the turbo ratio limit register(s).
///
/// Structure can hold data from MSR_TURBO_RATIO_LIMIT and
/// MSR_TURBO_RATIO_LIMIT1, the latter will exist depending on the number
/// of physical cores in the platform (i.e., server products).
struct turbo_limit_data
{
    /// @brief Raw 64-bit value stored in turbo ratio limit register.
    uint64_t bits;
    /// @brief Max turbo ratio (in MHz) when 1 core (or 9 cores) is/are active.
    double max_1c;
    /// @brief Max turbo ratio (in MHz) when 2/10 cores are active.
    double max_2c;
    /// @brief Max turbo ratio (in MHz) when 3/11 cores are active.
    double max_3c;
    /// @brief Max turbo ratio (in MHz) when 4/12 cores are active.
    double max_4c;
    /// @brief Max turbo ratio (in MHz) when 5/13 cores are active.
    double max_5c;
    /// @brief Max turbo ratio (in MHz) when 6/14 cores are active.
    double max_6c;
    /// @brief Max turbo ratio (in MHz) when 7/15 cores are active.
    double max_7c;
    /// @brief Max turbo ratio (in MHz) when 8/16 cores are active.
    double max_8c;
};

/// @brief Allocate array for storing raw register data from IA32_PERF_CTL.
///
/// There are plans to use a struct to make the indirection less crazy.
///
/// @param [in] val Pointer to array of raw IA32_PERF_CTL data, length equal to
///        the total number of logical processors.
void turbo_storage(uint64_t ***val);

/// @brief Enable turbo by modifying IA32_PERF_CTL on each logical processor.
///
/// Enable Intel Dynamic Acceleration (IDA) and Intel Turbo Boost Technology by
/// setting bit 32 of IA32_PERF_CTL to 0. This bit is not shared across logical
/// processors in a package, so it must be modified to the same value across
/// all logical processors in the same package.
void enable_turbo(void);

/// @brief Disable turbo by modifying IA32_PERF_CTL on each logical processor.
///
/// Disable Intel Dynamic Acceleration (IDA) and Intel Turbo Boost Technology by
/// setting bit 32 of IA32_PERF_CTL to 1. This bit is not shared across logical
/// processors in a package, so it must be modified to the same value across
/// all logical processors in the same package.
void disable_turbo(void);

/// @brief Print turbo data for each logical processor.
///
/// For each logical processor, print the unique core identifier, the value of
/// bit 32 for IA32_PERF_CTL (1 indicates IDA/Turbo Boost is enabled) and the
/// value of bit 38 for IA32_MISC_ENABLE (1 indicates support for IDA/Turbo
/// Boost by the platform).
///
/// @param [in] writedest File stream where output will be written to.
void dump_turbo(FILE *writedest);

/// @brief Convert raw bits of MSR_TURBO_ACTIVATION_RATIO to human-readable
/// values.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] info Data for turbo activation ratio.
void calc_max_non_turbo(const unsigned socket,
                        struct turbo_activation_ratio_data *info);

/// @brief Read value of the MSR_TURBO_ACTIVATION_RATIO register and translate
/// bit fields to human-readable values.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] info Data for turbo activation ratio.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int get_max_turbo_activation_ratio(const unsigned socket,
                                   struct turbo_activation_ratio_data *info);

/// @brief Convert raw bits of turbo ratio limit register to human-readable
/// values.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] info Data for turbo ratio limit.
///
/// @param [out] info2 Data for turbo ratio limit 1 (platform dependent).
void calc_max_turbo_ratio(const unsigned socket,
                          struct turbo_limit_data *info,
                          struct turbo_limit_data *info2);

/// @brief Read value of the turbo ratio limit register and translate bit
/// fields to human-readable values.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] info Data for turbo ratio limit.
///
/// @param [out] info2 Data for turbo ratio limit 1 (platform dependent).
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int get_turbo_ratio_limit(const unsigned socket,
                          struct turbo_limit_data *info,
                          struct turbo_limit_data *info2);

#ifdef __cplusplus
}
#endif
#endif
