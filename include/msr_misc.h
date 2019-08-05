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

#ifndef MSR_MISC_H_INCLUDE
#define MSR_MISC_H_INCLUDE

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure containing enabled/disabled platform-specific features as
/// encoded in IA32_MISC_ENABLE.
struct misc_enable
{
    /// @brief Raw 64-bit value stored in IA32_MISC_ENABLE.
    uint64_t raw;
    /// @brief Enable/disable fast string operations.
    ///
    /// Fast string operations are string operations initiated with the
    /// MOVS/STOS instructions and the REP prefix. If 1 (default), stores
    /// within a single string operation may execute out of order. If 0, fast
    /// string operations are disabled. This bit field (bit 0) is R/W and is
    /// thread-level scope.
    __u8 fast_string_enable;
    /// @brief Enable/disable automatic thermal control circuit (TCC) and
    /// Thermal Monitor 1 (TM1) feature.
    ///
    /// Thermal Monitor 1 (TM1) controls the CPU temperature by modulating the
    /// duty cycle of the clock. If 1, the TCC portion of the Intel Thermal
    /// Monitor feature and TM1 are enabled, allowing the CPU to automatically
    /// reduce power consumption in response to TCC activation. If 0 (default),
    /// this feature is disabled. Disabling this feature may be ignored in
    /// critical thermal conditions. In this case, TM1, TM2, and adaptive
    /// thermal throttling will still be active. This bit field (bit 3) is R/W.
    __u8 auto_TCC_enable;
    /// @brief Performance monitoring is available or unavailable.
    ///
    /// If 1, performance monitoring is enabled, else it is disabled. This bit
    /// field (bit 7) if RO and is thread-level scope.
    __u8 performance_monitoring;
    /// @brief Branch trace storage (BTS) is supported or unsupported.
    ///
    /// The branch trace store (BTS) feature provides capability of saving
    /// branch records in a memory-resident BTS buffer. If 1, BTS is not
    /// supported by the platform, else it is supported. This bit field (bit 11)
    /// is RO and is thread-level scope.
    __u8 branch_trace_storage_unavail;
    /// @brief Processor event-based sampling (PEBS) is supported or
    /// unsupported.
    ///
    /// Processor event-based sampling (PEBS) uses an interrupt to store a set
    /// of architectural state information for the platform. If 1, PEBS is not
    /// supported by the platform, else it is supported. This bit field (bit
    /// 12) is RO and is thread-level scope.
    __u8 precise_event_based_sampling_unavail;
    /// @brief Enable/disable Thermal Monitor 2 (TM2).
    ///
    /// Thermal Monitor 2 (TM2) is an additional thermal protection feature to
    /// Thermal Monitor 1 (TM1). If TM2 encounters the CPU temperature rising
    /// above some threshold, it reduces the CPU frequency and voltage, while
    /// degrading performance less than TM1. If 1 and the thermal sensor
    /// indicates the CPU temperature has reached the pre-defined threshold,
    /// the TM2 feature is engaged. If 0, then TM2 has not been activated. This
    /// bit field (bit 13) is R/W.
    __u8 TM2_enable;
    /// @brief Enable/disable Enhanced Intel SpeedStep Technology.
    ///
    /// Intel SpeedStep Technology enables CPU power management with P-state
    /// transitions. If 1, Enhanced Intel SpeedStep Technology is enabled, else
    /// it is disabled. This bit field (bit 16) is R/W and is package-level
    /// scope.
    __u8 enhanced_Intel_SpeedStep_Tech_enable;
    /// @brief Enable/disable monitor FSM.
    ///
    /// The MONITOR and MWAIT instructions help improve thread synchronization
    /// in multi-threaded environments. If 1 (default), MONITOR/MWAIT are
    /// supported. If 0, the MONITOR feature flag is not set, indicating that
    /// MONITOR/MWAIT are not supported by the platform. This bit field (bit
    /// 18) is R/W and is thread-level scope.
    __u8 enable_monitor_fsm;
    /// @brief Limit CPUID maxval.
    ///
    /// This feature only needs to be enabled if the OS does not support
    /// specific features. CPUID retrieves information about the platform, the
    /// higher the value, the more information that can be retrieved. Before
    /// setting this bit, the BIOS must execute the CPUID.0H and examine the
    /// max value returned in EAX[7:0]. If the max is greater than 3, this
    /// feature is supported, else it is not supported. This bit field (bit 22)
    /// is R/W and is thread-level scope.
    __u8 limit_CPUID_maxval;
    /// @brief Enable/disable xTPR messages.
    ///
    /// xTPR messages are optional messages enabling the process to inform the
    /// chipset of its priority. If 1, xTPR messages are disabled. This bit
    /// field (bit 23) is R/W and is thread-level scope.
    __u8 xTPR_message_disable;
    /// @brief Enable/disable Execute Disable Bit (XD bit) feature.
    ///
    /// The Execute Disable Bit (XD bit) controls page access restriction
    /// through instruction fetches from PAE pages. If 1, the XD bit feature
    /// is disabled. If 0 (default), the feature (if available) allows the OS
    /// to enable PAE paging, leveraging data only pages. This bit field (bit
    /// 34) is R/W and is thread-level scope.
    __u8 XD_bit_disable;
    /// @brief Enable/disable Intel Dynamic Acceleration (IDA).
    ///
    /// Intel Dynamic Acceleration (IDA) leverages thermal headroom to allow a
    /// single core to run at a higher frequency when the OS demands increased
    /// performance. If 1 on platforms supporting IDA, the feature is disabled.
    /// If 0 on platforms supporting IDA, then the feature is enabled. The
    /// power on default value is used by the BIOS to detect hardware support
    /// of Turbo Boost mode. If power-on default is 1, Turbo Boost is available
    /// on the platform, else it is not available. This bit field (bit 38) is
    /// R/W and is package-level scope.
    __u8 turbo_mode_disable;
};

/// @brief Structure holding package domain C-states residency data.
///
/// C-states (other than C0) indicate that the CPU is idle and not executing
/// any instructions. The higher the value, the deeper the CPU is in sleep
/// state.
struct pkg_cres
{
    /// @brief Raw 64-bit value stored in MSR_PKG_C2_RESIDENCY, indicating
    /// time spent by the package in C-state C2.
    uint64_t **pkg_c2;
    /// @brief Raw 64-bit value stored in MSR_PKG_C3_RESIDENCY, indicating
    /// time spent by the package in C-state C3.
    uint64_t **pkg_c3;
    /// @brief Raw 64-bit value stored in MSR_PKG_C4_RESIDENCY, indicating
    /// time spent by the package in C-state C4.
    uint64_t **pkg_c4;
    /// @brief Raw 64-bit value stored in MSR_PKG_C6_RESIDENCY, indicating
    /// time spent by the package in C-state C6.
    uint64_t **pkg_c6;
    /// @brief Raw 64-bit value stored in MSR_PKG_C7_RESIDENCY, indicating
    /// time spent by the package in C-state C7.
    uint64_t **pkg_c7;
    /// @brief Raw 64-bit value stored in MSR_PKG_C8_RESIDENCY, indicating
    /// time spent by the package in C-state C8.
    uint64_t **pkg_c8;
    /// @brief Raw 64-bit value stored in MSR_PKG_C9_RESIDENCY, indicating
    /// time spent by the package in C-state C9.
    uint64_t **pkg_c9;
    /// @brief Raw 64-bit value stored in MSR_PKG_C10_RESIDENCY, indicating
    /// time spent by the package in C-state C10.
    uint64_t **pkg_c10;
};

/// @brief Structure holding core domain C-states residency data.
///
/// C-states (other than C0) indicate that the CPU is idle and not executing
/// any instructions. The higher the value, the deeper the CPU is in sleep
/// state.
struct core_cres
{
    /// @brief Raw 64-bit value stored in MSR_CORE_C1_RESIDENCY, indicating
    /// time spent by the core in C-state C1.
    uint64_t **core_c1;
    /// @brief Raw 64-bit value stored in MSR_CORE_C3_RESIDENCY, indicating
    /// time spent by the core in C-state C3.
    uint64_t **core_c3;
    /// @brief Raw 64-bit value stored in MSR_CORE_C6_RESIDENCY, indicating
    /// time spent by the core in C-state C6.
    uint64_t **core_c6;
    /// @brief Raw 64-bit value stored in MSR_CORE_C7_RESIDENCY, indicating
    /// time spent by the core in C-state C7.
    uint64_t **core_c7;
};

/*************************/
/* Misc Enable Functions */
/*************************/

/// @brief Print list of enabled/disabled platform features as encoded in
/// IA32_MISC_ENABLE.
///
/// @param [in] s Pointer to list of platform feature data.
void dump_misc_enable(struct misc_enable *s);

/// @brief Read value of IA32_MISC_ENABLE and decode bit fields to determine
/// enabled/disabled features on platform.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [out] s Data indicating enabled/disabled features.
void get_misc_enable(unsigned socket,
                     struct misc_enable *s);

/// @brief Set value of IA32_MISC_ENABLE based on desired enabled/disabled
/// features for platform.
///
/// @param [in] socket Unique socket/package identifier.
///
/// @param [in] s Data indicating enabled/disabled features.
void set_misc_enable(unsigned socket,
                     struct misc_enable *s);

/*******************************/
/* C-State Residency Functions */
/*******************************/

/// @brief Initialize storage of package-level C-state residencies.
///
/// @param [out] pcr Pointer to package-level C-state residency data.
void pkg_cres_storage(struct pkg_cres **pcr);

/// @brief Initialize storage of core-level C-state residencies.
///
/// @param [out] ccr Pointer to core-level C-state residency data.
void core_cres_storage(struct core_cres **ccr);

/// @brief Print labels for package-level C-states.
///
/// @param [in] writedest File stream where output will be written to.
void dump_pkg_cres_label(FILE *writedest);

/// @brief Read package-level C-state residencies and print contents.
///
/// @param [in] writedest File stream where output will be written to.
void dump_pkg_cres(FILE *writedest);

/// @brief Print labels for core-level C-states.
///
/// @param [in] writedest File stream where output will be written to.
void dump_core_cres_label(FILE *writedest);

/// @brief Read core -level C-state residencies and print contents.
///
/// @param [in] writedest File stream where output will be written to.
void dump_core_cres(FILE *writedest);

#ifdef __cplusplus
}
#endif
#endif
