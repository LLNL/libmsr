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

#ifndef MSR_CLOCKS_H_INCLUDE
#define MSR_CLOCKS_H_INCLUDE

#include <stdint.h>

#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure containing data for IA32_CLOCK_MODULATION.
///
/// There is a bit at 0 that can be used for Extended On-Demand Clock
/// Modulation Duty Cycle. It is added with the bits 3:1. When used, the
/// granularity of clock modulation duty cycle is increased to 6.25% as opposed
/// to 12.5%. To enable this, must have CPUID.06H:EAX[Bit 5] = 1. I am not sure
/// how to check that because otherwise bit 0 is reserved.
struct clock_mod
{
    /// @brief Raw 64-bit value stored in IA32_CLOCK_MODULATION.
    uint64_t raw;
    /// @brief Enable/disable on-demand software-controlled clock modulation.
    int duty_cycle_enable;
    /// @brief On-demand clock modulation duty cycle.
    int duty_cycle;
    //   Value   | Duty Cycle
    // 000b = 0	 |  Reserved
    // 001b = 1	 |    12.5% (default)
    // 010b = 2	 |    25.0%
    // 011b = 3	 |    37.5%
    // 100b = 4	 |    50.0%
    // 101b = 5	 |    63.5%
    // 110b = 6	 |    75.0%
    // 111b = 7	 |    87.5%
};

/// @brief Structure containing data for IA32_APERF, IA32_MPERF, and
/// IA32_TIME_STAMP_COUNTER.
struct clocks_data
{
    /// @brief Raw 64-bit value stored in IA32_APERF.
    uint64_t **aperf;
    /// @brief Raw 64-bit value stored in IA32_MPERF.
    uint64_t **mperf;
    /// @brief Raw 64-bit value stored in IA32_TIME_STAMP_COUNTER.
    uint64_t **tsc;
};

/// @brief Structure containing data for IA32_PERF_STATUS and IA32_PERF_CTL.
struct perf_data
{
    /// @brief Raw 64-bit value stored in IA32_PERF_STATUS.
    uint64_t **perf_status;
    /// @brief Raw 64-bit value stored in IA32_PERF_CTL.
    uint64_t **perf_ctl;
};

/// @brief Allocate array for storing raw register data from IA32_APERF,
/// IA32_MPERF, and IA32_TIME_STAMP_COUNTER.
///
/// There are plans to use a struct to make the indirection less crazy.
///
/// @param [in] cd Pointer to clock-related data.
void clocks_storage(struct clocks_data **cd);

/// @brief Allocate array for storing raw register data from IA32_PERF_STATUS
/// and IA32_PERF_CTL.
///
/// @param [in] pd Pointer to perf-related data.
void perf_storage(struct perf_data **pd);

/// @brief Print the label for the abbreviated clocks data print out.
///
/// @param [in] writedest File stream where output will be written to.
void dump_clocks_data_terse_label(FILE *writedest);

/// @brief Print abbreviated clocks data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_clocks_data_terse(FILE *writedest);

/// @brief Print current p-state.
///
/// @param [in] writedest File stream where output will be written to.
void dump_p_state(FILE *writedest);

/// @brief Request new current p-state.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] pstate Desired p-state.
void set_p_state(unsigned socket,
                 uint64_t pstate);

/// @brief Print detailed clocks data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_clocks_data_readable(FILE *writedest);

/****************************************/
/* Software Controlled Clock Modulation */
/****************************************/

/// @brief Print clock modulation data.
///
/// @param [in] s Data for clock modulation.
///
/// @param [in] writedest File stream where output will be written to.
void dump_clock_mod(struct clock_mod *s,
                    FILE *writedest);

/// @brief Get contents of IA32_CLOCK_MODULATION.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [out] s Data for clock modulation.
void get_clock_mod(int socket,
                   int core,
                   struct clock_mod *s);

/// @brief Change value of IA32_CLOCK_MODULATION.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] core Unique core identifier.
///
/// @param [in] s Data for clock modulation.
int set_clock_mod(int socket,
                  int core,
                  struct clock_mod *s);

#ifdef __cplusplus
}
#endif
#endif
