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

#ifndef MSR_COUNTERS_H_INCLUDE
#define MSR_COUNTERS_H_INCLUDE

#include <stdint.h>
#include <stdio.h>

#include "msr_core.h"
#include "master.h"

// Note: It should not matter which platform header (i.e., master header) you
// are using because the counter MSRs are architectural and should remain
// consistent across platforms.

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure containing configuration data for each fixed-function
/// performance counter as encoded in IA32_PERF_GLOBAL_CTL and
/// IA32_FIXED_CTR_CTL.
struct fixed_counter
{
    /// @brief Raw value stored in enable bit field of IA32_PERF_GLOBAL_CTL,
    /// indicating availability of fixed-function performance counter.
    uint64_t *enable;
    /// @brief Raw value stored in enable bit field of IA32_FIXED_CTR_CTL,
    /// enabling counting of events occurring in the specified ring level (i.e.,
    /// none, OS, user, both).
    uint64_t *ring_level;
    /// @brief Raw value stored in AnyThread bit field of IA32_FIXED_CTR_CTL
    /// enables all threads sharing the same processor thread (hyperthreaded
    /// scenario) to increment the counter.
    uint64_t *anyThread;
    /// @brief Raw value stored in performance monitoring interrupt (PMI)
    /// enable bit field of AI32_FIXED_CTR_CTL, allowing logical processor to
    /// generate an exception when the counter overflows.
    uint64_t *pmi;
    /// @brief Raw 64-bit value stored in IA32_FIXED_CTR[0-3].
    uint64_t **value;
    uint64_t *overflow;
};

/// @brief Structure containing general information about the fixed-function
/// performance counters on the platform.
struct fixed_counter_config
{
    /// @brief Number of fixed-function performance counters available per core
    /// (e.g., IA32_FIXED_CTR0, IA32_FIXED_CTR1, IA32_FIXED_CTR2).
    int num_counters;
    /// @brief Bit width of the fixed-function performance counters.
    int width;
};

/// @brief Structure containing data of performance event select counters.
struct perfevtsel
{
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL0.
    uint64_t **perf_evtsel0;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL1.
    uint64_t **perf_evtsel1;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL2.
    uint64_t **perf_evtsel2;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL3.
    uint64_t **perf_evtsel3;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL4.
    uint64_t **perf_evtsel4;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL5.
    uint64_t **perf_evtsel5;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL6.
    uint64_t **perf_evtsel6;
    /// @brief Raw 64-bit value stored in IA32_PERFEVTSEL7.
    uint64_t **perf_evtsel7;
};

/// @brief Structure containing data of general-purpose performance counters.
struct pmc
{
    /// @brief Raw 64-bit value stored in IA32_PMC0.
    uint64_t **pmc0;
    /// @brief Raw 64-bit value stored in IA32_PMC1.
    uint64_t **pmc1;
    /// @brief Raw 64-bit value stored in IA32_PMC2.
    uint64_t **pmc2;
    /// @brief Raw 64-bit value stored in IA32_PMC3.
    uint64_t **pmc3;
    /// @brief Raw 64-bit value stored in IA32_PMC4.
    uint64_t **pmc4;
    /// @brief Raw 64-bit value stored in IA32_PMC5.
    uint64_t **pmc5;
    /// @brief Raw 64-bit value stored in IA32_PMC6.
    uint64_t **pmc6;
    /// @brief Raw 64-bit value stored in IA32_PMC7.
    uint64_t **pmc7;
};

/// @brief Structure containing data of uncore performance event select
/// counters.
struct unc_perfevtsel
{
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PERFEVTSEL0.
    uint64_t **c0;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PERFEVTSEL1.
    uint64_t **c1;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PERFEVTSEL2.
    uint64_t **c2;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PERFEVTSEL3.
    uint64_t **c3;
};

/// @brief Structure containing data of uncore general-purpose performance
/// counters.
struct unc_counters
{
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PMC0.
    uint64_t **c0;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PMC1.
    uint64_t **c1;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PMC2.
    uint64_t **c2;
    /// @brief Raw 64-bit value stored in MSR_UNCORE_PMC3.
    uint64_t **c3;
};

/// @brief Print available general-purpose performance counters.
void print_available_counters(void);

/*************************************/
/* Programmable Performance Counters */
/*************************************/

/// @brief Store the performance event select counter data on the heap.
///
/// @param [out] e Data for performance event select counters.
void perfevtsel_storage(struct perfevtsel **e);

/// @brief Store the general-purpose performance counter data on the heap.
///
/// @param [out] p Data for general-purpose performance counters.
void pmc_storage(struct pmc **p);

/// @brief Set a performance event select counter on a single logical processor.
///
/// @param [in] cmask Count multiple event occurrences per cycle.
///
/// @param [in] flags Toggle additional options, such as user mode vs. OS mode,
///        enable PMI upon counter overflow, invert cmask, edge detection of
///        event occurrence, etc.
///
/// @param [in] umask Condition to be detected by the event logic unit.
///
/// @param [in] eventsel Unique event logic unit identifier.
///
/// @param [in] pmcnum Unique performance event select counter identifier.
///
/// @param [in] thread Unique logical processor identifier.
void set_pmc_ctrl_flags(uint64_t cmask,
                        uint64_t flags,
                        uint64_t umask,
                        uint64_t eventsel,
                        int pmcnum,
                        unsigned thread);

/// @brief Set a performance event select counter to the same event on all
/// logical processors.
///
/// @param [in] cmask Count multiple event occurrences per cycle.
///
/// @param [in] flags Toggle additional options, such as user mode vs. OS mode,
///        enable PMI upon counter overflow, invert cmask, edge detection of
///        event occurrence, etc.
///
/// @param [in] umask Condition to be detected by the event logic unit.
///
/// @param [in] eventsel Unique event logic unit identifier.
///
/// @param [in] pmcnum Unique performance event select counter identifier.
void set_all_pmc_ctrl(uint64_t cmask,
                      uint64_t flags,
                      uint64_t umask,
                      uint64_t eventsel,
                      int pmcnum);

/// @brief Allocate storage for performance counters and reset values.
///
/// @return 0 if successful, else -1 if no general-purpose performance counters
/// are available.
int enable_pmc(void);

/// @brief Reset all performance counters for each logical processor.
void clear_all_pmc(void);

/// @brief Reset performance counter on a specific logical processor.
///
/// @param [in] idx Unique logical processor identifier.
///
/// @return 0 if successful, else -1 if logical processor is invalid.
int clear_pmc(int idx);

/// @brief Print out detailed performance counter data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_pmc_data_readable(FILE *writedest);

/*************************************/
/* Uncore PCU Performance Monitoring */
/*************************************/

/// @brief Store the uncore performance event select counter data on the heap.
///
/// @param [out] uevt Pointer to data for uncore performance event select
///        counters.
void unc_perfevtsel_storage(struct unc_perfevtsel **uevt);

/// @brief Store the uncore general-purpose performance counter data on the
/// heap.
///
/// @param [out] uc Pointer to data for uncore general-purpose performance
///        counters.
void unc_counters_storage(struct unc_counters **uc);

/// @brief Set an uncore performance event select counter to the same event on
/// a single socket.
///
/// @param [in] flags Toggle additional options, such as user mode vs. OS mode,
///        enable PMI upon counter overflow, invert cmask, edge detection of
///        event occurrence, etc.
///
/// @param [in] reset Enable reset of queue occupancy counter.
///
/// @param [in] occ Counter for queue occupancy.
///
/// @param [in] eventsel Unique event logic unit identifier.
///
/// @param [in] pcunum Unique performance event select counter identifier.
///
/// @param [in] socket Unique socket/package identifier.
void set_pcu_ctrl_flags(uint64_t flags,
                        uint64_t reset,
                        uint64_t occ,
                        uint64_t eventsel,
                        int pcunum,
                        unsigned socket);

/// @brief Set an uncore performance event select counter to the same event on
/// all sockets.
///
/// Can probably consolidate event select into a single function.
///
/// @param [in] flags Toggle additional options, such as user mode vs. OS mode,
///        enable PMI upon counter overflow, invert cmask, edge detection of
///        event occurrence, etc.
///
/// @param [in] reset
///
/// @param [in] occ Reset queue occupancy counter
///
/// @param [in] eventsel Unique event logic unit identifier.
///
/// @param [in] pcunum Unique performance event select counter identifier.
void set_all_pcu_ctrl(uint64_t flags,
                      uint64_t reset,
                      uint64_t occ,
                      uint64_t eventsel,
                      int pcunum);

/// @brief Allocate storage for uncore performance counters and reset values.
void enable_pcu(void);

/// @brief Reset all uncore performance counters for each socket.
void clear_all_pcu(void);

/// @brief Reset uncore performance counter on a specific socket.
///
/// @param [in] idx Unique socket/package identifier.
///
/// @return 0 if successful, else -1 if socket is invalid.
int clear_pcu(int idx);

/// @brief Print the label for the uncore performance counter data print out.
///
/// @param [in] writedest File stream where output will be written to.
void dump_unc_counter_data_label(FILE *writedest);

/// @brief Print out uncore performance counter data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_unc_counter_data(FILE *writedest);

/*****************************************/
/* Fixed Counters Performance Monitoring */
/*****************************************/

/// @brief Initialize storage for fixed-function performance counter data, and
/// store it on the heap.
///
/// @param [out] ctr0 Data for fixed-function performance counter for any
///        instructions retired.
/// @param [out] ctr1 Data for fixed-function performance counter for core
///        unhalted clock cycles.
/// @param [out] ctr2 Data for fixed-function performance counter for unhalted
///        reference clock cycles.
void fixed_counter_storage(struct fixed_counter **ctr0,
                           struct fixed_counter **ctr1,
                           struct fixed_counter **ctr2);

/// @brief Initialize storage for performance global control and fixed-function
/// performance control data, and store it on the heap.
///
/// There are plans to use a struct to make the indirection less crazy.
///
/// @param [out] perf_ctrl Data for controlling counting of each performance
///        counter.
///
/// @param [out] fixed_ctrl Pointer to data for controlling the operations of a
///        fixed-function performance counter.
void fixed_counter_ctrl_storage(uint64_t ***perf_ctrl,
                                uint64_t ***fixed_ctrl);

/// @brief Initialize storage for fixed-function performance counter data.
///
/// @param [out] ctr Data for fixed-function performance counters.
void init_fixed_counter(struct fixed_counter *ctr);

/// @brief Retrieve value of all fixed-function performance counters for each
/// logical processor.
///
/// @param [out] ctr0 Data for fixed-function performance counter for any
///        instructions retired.
/// @param [out] ctr1 Data for fixed-function performance counter for core
///        unhalted clock cycles.
/// @param [out] ctr2 Data for fixed-function performance counter for unhalted
///        reference clock cycles.
void get_fixed_counter_ctrl(struct fixed_counter *ctr0,
                            struct fixed_counter *ctr1,
                            struct fixed_counter *ctr2);

/// @brief Set value of IA32_FIXED_CTR_CTL and IA32_PERF_GLOBAL_CTL and reset
/// all fixed-function performance counters on all logical processors.
///
/// @param [in] ctr0 Data for fixed-function performance counter for any
///        instructions retired.
/// @param [in] ctr1 Data for fixed-function performance counter for core
///        unhalted clock cycles.
/// @param [in] ctr2 Data for fixed-function performance counter for unhalted
///        reference clock cycles.
void set_fixed_counter_ctrl(struct fixed_counter *ctr0,
                            struct fixed_counter *ctr1,
                            struct fixed_counter *ctr2);

/// @brief Retrieve number of fixed-function performance counters on the
/// platform and the bit width of these counters.
///
/// @param [out] data Data for general information about fixed-function
///        performance counters.
void get_fixed_counter_config(struct fixed_counter_config *data);

/// @brief Enable fixed-function counters by setting enable bit in
/// IA32_FIXED_CTR_CTL.
void enable_fixed_counters(void);

/// @brief Disable fixed-function counters by clearing enable bit in
/// IA32_FIXED_CTR_CTL.
void disable_fixed_counters(void);

/// @brief Print abbreviated fixed-function performance counter data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_fixed_counter_data_terse(FILE *writedest);

/// @brief Print the label for the abbreviated fixed-function performance
/// counter data print out.
///
/// @param [in] writedest File stream where output will be written to.
void dump_fixed_counter_data_terse_label(FILE *writedest);

/// @brief Print detailed fixed-function performance counter data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_fixed_counter_data_readable(FILE *writedest);

#ifdef __cplusplus
}
#endif
#endif
