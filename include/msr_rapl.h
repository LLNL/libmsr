/* msr_rapl.h
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

#ifndef MSR_RAPL_H_INCLUDE
#define MSR_RAPL_H_INCLUDE

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

// Processor specific rapl flags (see rapl_init function)
// These indicate which rapl MSRs are available for a given cpu model
// bit: address, register
//  0: 606h, MSR_RAPL_POWER_UNIT
//  1: 610h, MSR_PKG_POWER_LIMIT
//  2: 611h, MSR_PKG_ENERGY_STATUS
//  3: 613h, MSR_PKG_PERF_STATUS
//  4: 614h, MSR_PKG_POWER_INFO
//  5: 618h, MSR_DRAM_POWER_LIMIT
//  6: 619h, MSR_DRAM_ENERGY_STATUS
//  7: 61Bh, MSR_DRAM_PERF_STATUS
//  8: 61Ch, MSR_DRAM_POWER_INFO
//  9: 638h, MSR_PP0_POWER_LIMIT
// 10: 639h, MSR_PP0_ENERGY_STATUS
// 11: 63Ah, MSR_PP0_POLICY
// 12: 63Bh, MSR_PP0_PERF_STATUS
// 13: 640h, MSR_PP1_POWER_LIMIT
// 14: 641h, MSR_PP1_ENERGY_STATUS
// 15: 642h, MSR_PP1_POLICY
// 16: 64Ch, MSR_TURBO_ACTIVATION_RATIO
// 17: 66Eh, MSR_PKG_POWER_INFO
// 18: 690h, MSR_CORE_PERF_LIMIT_REASONS
// 19: 6B0h, MSR_GRAPHICS_PERF_LIMIT_REASONS
// 20: 6B1h, MSR_RING_PERF_LIMIT_REASONS
// 21: 1ADh, MSR_TURBO_RATIO_LIMIT
// 22: 1AEh, MSR_TURBO_RATIO_LIMIT1
#define MF_06_37 (0x407)
#define MF_06_4A (0x407)
#define MF_06_5A (0x407)
#define MF_06_4D (0x20003)
#define MF_06_4C (0x607)
#define MF_06_2A (0xFE17)   // Sandy Bridge
#define MF_06_2D (0x7FF)    // Sandy Bridge
#define MF_06_3A (0xFE17)
#define MF_06_3E (0x6007F7) // Ivy Bridge
#define MF_06_3C (0x1CEE17) // disabled pp0_perf_status, not on this architecture
#define MF_06_45 (0x1CFE17)
#define MF_06_46 (0x1CFE17)
#define MF_06_3F (0x1C01FF) // disabled PP registers, not on this architecture
#define MF_06_3D (0x1CFE17)
#define MF_06_47 (0x1CFE17)
#define MF_06_4F (0x177)     // Debug MSRs on quartz
#define MF_06_56 (0x1CFE17)
#define MF_06_4E (0x1EFE17)
#define MF_06_5E (0x1EFE17)
#define MF_06_55 (0x177)
#define MF_06_57 (0x507FF)

// Register flags
// These are used to check against the rapl flags (see above) to see if a
// register exists
#define POWER_UNIT             (0x1L)
#define PKG_POWER_LIMIT        (0x2L)
#define PKG_ENERGY_STATUS      (0x4L)
#define PKG_PERF_STATUS        (0x8L)
#define PKG_POWER_INFO         (0x10L)
#define DRAM_POWER_LIMIT       (0x20L)
#define DRAM_ENERGY_STATUS     (0x40L)
#define DRAM_PERF_STATUS       (0x80L)
#define DRAM_POWER_INFO        (0x100L)
#define TURBO_ACTIVATION_RATIO (0x200L)
#define TURBO_RATIO_LIMIT      (0x4000L)
#define TURBO_RATIO_LIMIT1     (0x8000L)

#define STD_ENERGY_UNIT 65536.0

/// @brief Enum encompassing unit conversion types.
enum libmsr_unit_conversions_e
{
    /// @brief Decode raw bits into Watts.
    BITS_TO_WATTS,
    /// @brief Encode Watt value to raw bits.
    WATTS_TO_BITS,
    /// @brief Decode raw bits into Joules.
    BITS_TO_JOULES,
    /// @brief Encode Joule value to raw bits.
    JOULES_TO_BITS,
    /// @brief Decode raw bits to seconds (for Sandy Bridge and Ivy Bridge).
    BITS_TO_SECONDS_STD,
    /// @brief Encode seconds value to raw bits (for Sandy Bridge and Ivy
    /// Bridge).
    SECONDS_TO_BITS_STD,
    /// @brief Decode raw bits to seconds (for Haswell).
    BITS_TO_SECONDS_HASWELL,
    /// @brief Encode seconds value to raw bits (for Haswell).
    SECONDS_TO_BITS_HASWELL,
    /// @brief Decode raw bits to Joules for DRAM.
    BITS_TO_JOULES_DRAM
};

/// @brief Structure containing units for energy, time, and power across all
/// RAPL power domains.
struct rapl_units
{
    /// @brief Raw 64-bit value stored in MSR_RAPL_POWER_UNIT.
    uint64_t msr_rapl_power_unit;
    /// @brief Energy status units (ESU) based on the multiplier 1/(2^ESU) (in
    /// Joules). ESU is encoded in bits 12:8 of MSR_RAPL_POWER_UNIT.
    double joules;
    /// @brief Time units (TU) based on the multiplier 1/(2^TU) (in seconds).
    /// TU is encoded in bits 19:16 of MSR_RAPL_POWER_UNIT.
    double seconds;
    /// @brief Power units (PU) based on the multiplier 1/(2^PU) (in Watts). PU
    /// is encoded in bits 3:0 of MSR_RAPL_POWER_UNIT.
    double watts;
};

/// @brief Structure containing data from energy, time, and power measurements
/// of various RAPL power domains.
struct rapl_data
{
    /**********/
    /* Timers */
    /**********/
    /// @brief Timestamp of the current data measurement.
    struct timeval now;
    /// @brief Timestamp of the previous data measurement.
    struct timeval old_now;
    /// @brief Amount of time elapsed between the two timestamps.
    double elapsed;

    /**************************/
    /* RAPL Power Domain: PKG */
    /**************************/
    /// @brief Raw 64-bit value stored in MSR_PKG_ENERGY_STATUS.
    uint64_t **pkg_bits;
    /// @brief Raw 64-bit value previously stored in MSR_PKG_ENERGY_STATUS.
    uint64_t *old_pkg_bits;
    /// @brief Current package-level energy usage (in Joules).
    double *pkg_joules;
    /// @brief Previous package-level energy usage (in Joules).
    double *old_pkg_joules;
    /// @brief Difference in package-level energy usage between two data
    /// measurements.
    double *pkg_delta_joules;
    /// @brief Package-level power consumption (in Watts) derived by dividing
    /// difference in package-level energy usage by time elapsed between data
    /// measurements.
    double *pkg_watts;
    /// @brief Raw 64-bit value stored in MSR_PKG_PERF_STATUS, a package-level
    /// performance counter reporting cumulative time that the package domain
    /// has throttled due to RAPL power limits.
    uint64_t **pkg_perf_count;

    /***************************/
    /* RAPL Power Domain: DRAM */
    /***************************/
    /// @brief Raw 64-bit value stored in MSR_DRAM_ENERGY_STATUS.
    uint64_t **dram_bits;
    /// @brief Raw 64-bit value previously stored in MSR_DRAM_ENERGY_STATUS.
    uint64_t *old_dram_bits;
    /// @brief Current DRAM energy usage (in Joules).
    double *dram_joules;
    /// @brief Previous DRAM energy usage (in Joules).
    double *old_dram_joules;
    /// @brief Difference in DRAM energy usage between two data measurements.
    double *dram_delta_joules;
    /// @brief DRAM power consumption (in Watts) derived by dividing difference
    /// in DRAM energy usage by time elapsed between data measurements.
    double *dram_watts;
    /// @brief Raw 64-bit value stored in MSR_DRAM_PERF_STATUS, which counts
    /// how many times DRAM performance was capped due to underlying hardware
    /// constraints.
    uint64_t **dram_perf_count;
};

/// @brief Structure containing power limit data for a given RAPL power domain.
struct rapl_limit
{
    /// @brief Raw 64-bit value stored in the power limit register.
    uint64_t bits;
    /// @brief Power limit in Watts.
    double watts;
    /// @brief Time window in seconds.
    double seconds;
};

/// @brief Structure containing power range info for RAPL usage for various
/// RAPL power domains.
struct rapl_power_info
{
    /**************************/
    /* RAPL Power Domain: PKG */
    /**************************/
    /// @brief Raw 64-bit value stored in MSR_PKG_POWER_INFO.
    uint64_t msr_pkg_power_info;
    /// @brief Max power (in Watts) derived from electrical specifications of
    /// the package domain.
    double pkg_max_power;
    /// @brief Min power (in Watts) derived from electrical specifications of
    /// the package domain.
    double pkg_min_power;
    /// @brief Max time (in seconds) that can be set in either time window field
    /// of the package domain.
    double pkg_max_window;
    /// @brief Thermal specification power (in Watts) of the package domain.
    double pkg_therm_power;

    /***************************/
    /* RAPL Power Domain: DRAM */
    /***************************/
    /// @brief Raw 64-bit value stored in MSR_DRAM_POWER_INFO.
    uint64_t msr_dram_power_info;
    /// @brief Max power (in Watts) derived from electrical specifications of
    /// the DRAM domain.
    double dram_max_power;
    /// @brief Min power (in Watts) derived from electrical specifications of
    /// the DRAM domain.
    double dram_min_power;
    /// @brief Max time (in seconds) that can be set in the time window field of
    /// the DRAM domain.
    double dram_max_window;
    /// @brief Thermal specification power (in Watts) of the DRAM domain.
    double dram_therm_power;
};

/// @brief Store the RAPL data and flags on the heap.
///
/// This data will be used by nearly all other RAPL APIs. A user can retrieve
/// only RAPL data or only RAPL flags by passing a NULL argument.
///
/// @param [out] data Pointer to measurements of energy, time, and power data
///        from a given RAPL power domain.
///
/// @param [out] flags Pointer to RAPL flags indicating available registers on
///        a given platform.
///
/// @return 0 if successful, else -1 if setflags() fails.
int rapl_storage(struct rapl_data **data,
                 uint64_t **flags);

/// @brief Print available RAPL registers based on platform-dependent flags.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int print_available_rapl(void);

/// @brief Initialize storage for Running Average Power Limit (RAPL) data and flags.
///
/// Be sure to put this function before any other RAPL functions.
///
/// @param [out] rapl Pointer to storage for energy, time, and power data
///        measurements from a given RAPL power domain.
///
/// @param [out] rapl_flags Pointer to storage for flags indicating available on
///        registers on platform.
///
/// @return Number of locked registers, else -1 if rapl_storage() fails.
int rapl_init(struct rapl_data **rapl,
              uint64_t **rapl_flags);

/// @brief Determine the steps necessary to set the user-supplied package-level
/// power limit(s).
///
/// If a pointer is null, do nothing. If the bit vector is nonzero, translate
/// the bit vector to watts and seconds and write the bit vector to the msr. If
/// the bit vector is zero, translate the watts and seconds to the appropriate
/// bit vector and write the bit vector to the msr.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] limit1 Data for lower power limit 1.
///
/// @param [in] limit2 Data for upper power limit 2.
///
/// @return 0 if successful, else -1 if rapl_storage() fails or if package
/// RAPL domain power limit is not supported on the platform.
int set_pkg_rapl_limit(const unsigned socket,
                       struct rapl_limit *limit1,
                       struct rapl_limit *limit2);

/// @brief Determine the steps necessary to set the user-supplied DRAM limit.
///
/// If a pointer is null, do nothing. If the bit vector is nonzero, translate
/// the bit vector to watts and seconds and write the bit vector to the msr. If
/// the bit vector is zero, translate the watts and seconds to the appropriate
/// bit vector and write the bit vector to the msr.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] limit RAPL power limit data for DRAM power domain.
///
/// @return 0 if successful, else -1 if rapl_storage() fails, if
/// calc_std_rapl_limit() fails, or if DRAM RAPL domain power limit is not
/// supported on the platform.
int set_dram_rapl_limit(const unsigned socket,
                        struct rapl_limit *limit);

/// @brief Get power info data for all RAPL power domains.
///
///	If a pointer is null, do nothing. If the bit vector is nonzero, translate
/// the bit vector to watts and seconds. If the bit vector is zero, read the
/// msr value into the bit vector and translate into watts and seconds.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] info Data for domain-specific power range info for RAPL usage.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int get_rapl_power_info(const unsigned socket,
                        struct rapl_power_info *info);

/// @brief Get RAPL power limit for the package domain.
///
///	If a pointer is null, do nothing. If the bit vector is nonzero, translate
/// the bit vector to watts and seconds. If the bit vector is zero, read the
/// msr value into the bit vector and translate into watts and seconds.
///
/// @param [in] socket Identifier of socket to read
///
/// @param [out] limit1 Data for package domain RAPL power limit 1 (lower).
///
/// @param [out] limit2 Data for package domain RAPL power limit 2 (upper).
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int get_pkg_rapl_limit(const unsigned socket,
                       struct rapl_limit *limit1,
                       struct rapl_limit *limit2);

/// @brief Get RAPL power limit for the DRAM domain.
///
///	If a pointer is null, do nothing. If the bit vector is nonzero, translate
/// the bit vector to watts and seconds. If the bit vector is zero, read the
/// msr value into the bit vector and translate into watts and seconds.
///
/// @param [in] socket Identifier of socket to read
///
/// @param [out] limit Data for DRAM domain RAPL power limit.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int get_dram_rapl_limit(const unsigned socket,
                        struct rapl_limit *limit);

/// @brief Print out RAPL power limit.
///
/// @param [in] L RAPL power limit for a given domain.
///
/// @param [in] writedest File stream where output will be written to.
void dump_rapl_limit(struct rapl_limit *L,
                     FILE *writedest);

/// @brief Print the label for the abbreviated RAPL data print out.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int dump_rapl_data_terse_label(FILE *writedest);

/// @brief Print abbreviated RAPL data.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int dump_rapl_data_terse(FILE *writedest);

/// @brief Print out a specified RAPL data item.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int dump_rapl_data(FILE *writedest);

/// @brief Print out only RAPL power data.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int dump_rapl_power_info(FILE *writedest);

/// @brief Read the current RAPL data and calculate the deltas from the
/// previous call to this function.
///
/// If this function has not been called at least once, then the data will be
/// initialized to zeros.
/// NOTE: This is now what you use instead of read_rapl_data().
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int poll_rapl_data(void);

/// @brief Check how much the RAPL data has changed overtime to derive
/// time-based values, such as power.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int delta_rapl_data(void);

/// @brief Read all available RAPL data for a given socket.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
int read_rapl_data(void);

/// @brief Get units for RAPL power data.
///
/// @param [out] ru Data for RAPL power units.
void get_rapl_power_unit(struct rapl_units *ru);

/// @brief Print out RAPL power units (power, energy, time).
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else -1 if rapl_storage() fails.
void dump_rapl_power_unit(FILE *writedest);

#ifdef __cplusplus
}
#endif
#endif
