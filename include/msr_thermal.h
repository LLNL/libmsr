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

#ifndef MSR_THERMAL_H_INCLUDE
#define MSR_THERMAL_H_INCLUDE

#include <stdio.h>

#include "master.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Structure containing data from MSR_TEMPERATURE_TARGET.
///
/// The scope of this MSR is defined as unique for Sandy Bridge. In our
/// implementation, we assume a socket-level scope.
struct msr_temp_target
{
    /// @brief Raw 64-bit value stored in MSR_TEMPERATURE_TARGET.
    uint64_t **raw;
    /// @brief Min temperature (in degree Celsius) at which PROCHOT will be
    /// asserted.
    ///
    /// This bit field (bits 23:16) is RO and is likely equal to the TCC
    /// Activation Temperature.
    uint64_t *temp_target;
};

/// @brief Structure holding data for IA32_THERM_STATUS.
///
/// The scope of this MSR is core-level for Sandy Bridge.
struct therm_stat
{
    /// @brief Raw 64-bit value stored in IA32_THERM_STATUS.
    uint64_t **raw;
    /// @brief Status (active/not active) of the digital thermal sensor
    /// high-temperature output signal (PROCHOT#) for the core.
    ///
    /// The thermal status bit indicates whether the digital thermal sensor
    /// high-temperature output signal (PROCHOT#) is currently active. If 1,
    /// PROCHOT# is active, else the feature is not active. This bit field (bit
    /// 0) is RO.
    int *status;
    /// @brief Sticky bit indicating history of the thermal sensor
    /// high-temperature output signal (PROCHOT#) since last cleared or last
    /// reset.
    ///
    /// Sticky bit indicating history of the thermal sensor high temperature
    /// output signal (PROCHOT#). If 1, PROCHOT# has been asserted since a
    /// previous RESET or the last time software cleared the bit. This bit (bit
    /// 1) is R/W, but software can only clear the bit by writing a 0.
    int *status_log;
    /// @brief Indicates if PROCHOT# or FORCEPR# is being asserted by another
    /// component on the platform.
    ///
    /// Indicates whether another component on the platform is causing
    /// high-temperature and asserting PROCHOT# or FORCEPR# as a result. This
    /// bit field (bit 2) is RO.
    int *PROCHOT_or_FORCEPR_event;
    /// @brief Sticky bit indicating history of the PROCHOT# and FORCEPR#
    /// signals since last cleared or last reset.
    ///
    /// Sticky bit indicating history of the PROCHOT# and FORCEPR# signals. If
    /// 1, PROCHOT# or FORCEPR# has been externally asserted by another agent
    /// on the platform since the last clearing of this bit or a reset.
    /// External PROCHOT# assertions are only acknowledged if the Bidirectional
    /// Prochot feature is enabled. This bit (bit 3) is R/W, but software can
    /// only clear the bit by writing a 0.
    int *PROCHOT_or_FORCEPR_log;
    /// @brief Status (active/not active) of the critical temperature detector
    /// output signal.
    ///
    /// Indicates whether the critical temperature detector output signal is
    /// currently active. If 1, the critical temperature detector output
    /// signal is currently active, else it is not active. This bit (bit 4) is
    /// RO.
    int *crit_temp_status;
    /// @brief Sticky bit indicating history of the critical temperature
    /// detector output signal since last cleared or last reset.
    ///
    /// Sticky bit indicating history of the critical temperature detector
    /// output signal. If 1, the signal has been asserted since a previous
    /// RESET or the last time software cleared the bit. This bit (bit 5) is
    /// R/W, but software can only clear the bit by writing a 0.
    int *crit_temp_log;
    /// @brief Indicates if the actual core temperature is currently higher
    /// than or equal to Thermal Threshold #1.
    ///
    /// Indicates whether the actual core temperature is currently higher than
    /// or equal to the value set in Thermal Threshold #1. If 0, the actual
    /// temperature is lower, else the actual core temperature is greater than
    /// or equal to TT#1. This bit field (bit 6) is RO.
    int *therm_thresh1_status;
    /// @brief Sticky bit indicating history of Thermal Threshold #1 (TT#1)
    /// since last cleared or last reset.
    ///
    /// Sticky bit indicating whether the Thermal Threshold #1 (TT#1) has been
    /// reached since the last clearing of this bit or a reset. If 1, TT#1 has
    /// been reached. This bit field (bit 7) is R/W, but software can only
    /// clear the bit by writing a 0.
    int *therm_thresh1_log;
    /// @brief Indicates if the actual temperature is currently higher than or
    /// equal to Thermal Threshold #2 (TT#2).
    ///
    /// Indicates whether the actual temperature is currently higher than or
    /// equal to the value set in Thermal Threshold #2. If 0, the actual
    /// temperature is lower, else the actual temperature is greater than or
    /// equal to TT#2. This bit field (bit 8) is RO.
    int *therm_thresh2_status;
    /// @brief Sticky bit indicating history of Thermal Threshold #2 (TT#2)
    /// since last cleared or last reset.
    ///
    /// Sticky bit indicating whether the Thermal Threshold #2 (TT#2) has been
    /// reached since the last clearing of this bit or a reset. If 1, TT#2 has
    /// been reached. This bit field (bit 9) is R/W, but software can only
    /// clear the bit by writing a 0.
    int *therm_thresh2_log;
    /// @brief Indicates if processor is operating below OS-requested p-state or
    /// OS-requested clock modulation duty cycle.
    ///
    /// Indicates whether the processor is currently operating below
    /// OS-requested p-state (specified in IA32_PERF_CTL) or OS-requested clock
    /// modulation duty cycle (specified in IA32_CLOCK_MODULATION). This bit
    /// (bit 10) is RO.
    int *power_limit_status;
    /// @brief Sticky bit indicating if processor went below OS-requested
    /// p-state or OS-requested clock modulation duty cycle since last cleared
    /// or last reset.
    ///
    /// Sticky bit indicating whether the processor went below the OS-requested
    /// p-state or OS-requested clock modulation duty cycle since the last
    /// clearing of this bit or a reset. This informs software of such
    /// occurrence. If 1, processor went below OS requests. This bit field (bit 11)
    /// is R/W, but software can only clear the bit by writing a 0.
    int *power_notification_log;
    /// @brief Digital temperature reading in degree Celsius relative to the
    /// TCC activation temperature.
    ///
    /// This bit field (bits 22:16) is RO.
    int *readout;
    /// @brief Specifies the resolution (or tolerance) of the digital thermal
    /// sensor in degree Celsius.
    ///
    /// This bit field (bits 30:27) is RO.
    int *resolution_deg_celsius;
    /// @brief Indicates if digital readout (bits 22:16) is valid.
    ///
    /// If 1, digital readout is valid, else it is not valid. This bit field
    /// (bit 31) is RO.
    int *readout_valid;
};

/// @brief Structure holding data for IA32_THERM_INTERRUPT.
///
/// The scope of this MSR is core-level for Sandy Bridge.
struct therm_interrupt
{
    /// @brief Raw 64-bit value stored in IA32_THERM_INTERRUPT.
    uint64_t **raw;
    /// @brief Enables the BIOS to generate an interrupt when transitioning
    /// from low temperature to a high temperature threshold.
    ///
    /// If 0 (default), interrupts are disabled, else interrupts are enabled.
    /// This bit field (bit 0) is R/W.
    int *high_temp_enable;
    /// @brief Enables the BIOS to generate an interrupt when transitioning
    /// from high temperature to low temperature (TCC deactivation).
    ///
    /// If 0 (default), interrupts are disabled, else interrupts are enabled.
    /// This bit field (bit 1) is R/W.
    int *low_temp_enable;
    /// @brief Enables the BIOS or OS to generate an interrupt when PROCHOT#
    /// has been asserted by another component on the platform and the
    /// Bidirectional Prochot feature is enabled.
    ///
    /// If 0, interrupts are disabled, else interrupts are enabled. This bit
    /// field (bit 2) is R/W.
    int *PROCHOT_enable;
    /// @brief Enables the BIOS or OS to generate an interrupt when FORCEPR#
    /// has been asserted by another component on the platform.
    ///
    /// If 0, interrupts are disabled, else interrupts are enabled. This bit
    /// field (bit 3) is R/W.
    int *FORCEPR_enable;
    /// @brief Enables generation of an interrupt when the Critical Temperature
    /// Detector detects a critical thermal condition.
    ///
    /// If 0, interrupts are disabled, else interrupts are enabled. This bit
    /// field (bit 4) is R/W.
    int *crit_temp_enable;
    /// @brief Temperature threshold, encoded relative to the TCC Activation
    /// temperature.
    ///
    /// Format of this bit field is the same as the Digital Readout. This
    /// threshold is compared against the Digital Readout and is used to
    /// generate Thermal Threshold #1 Status, Thermal Threshold #1 Log, and
    /// Threshold #1 thermal interrupt delivery. This bit field (bit 14:8) is
    /// R/W.
    int *thresh1_val;
    /// @brief Enables generation of an interrupt when the actual temperature
    /// crosses the Threshold #1 setting in any direction.
    ///
    /// If 0, interrupt is disabled, else interrupt is enabled. This bit field
    /// (bit 15) is R/W.
    int *thresh1_enable;
    /// @brief Temperature threshold, encoded relative to the Package TCC
    /// Activation temperature.
    ///
    /// Format of this bit field is the same as the Package Digital Readout.
    /// This threshold is compared against the Package Digital Readout and is
    /// used to generate Thermal Threshold #2 Status, Thermal Threshold #2 Log,
    /// and Threshold #2 thermal interrupt delivery. This bit field (bit 22:16)
    /// is R/W.
    int *thresh2_val;
    /// @brief Enables generation of an interrupt when the actual temperature
    /// crosses the Threshold #2 setting in any direction.
    ///
    /// If 0, interrupt is disabled, else interrupt is enabled. This bit field
    /// (bit 23) is R/W.
    int *thresh2_enable;
    /// @brief Enables generation of power notification events when the
    /// processor goes below OS-requested p-state or OS-requested clock
    /// modulation duty cycle.
    ///
    /// Enables the local APIC to deliver a thermal event when the processor
    /// went below OS-requested p-state or clock modulation duty cycle setting.
    /// This bit field (bit 24) is R/W.
    int *pwr_limit_notification_enable;
};

/// @brief Structure holding data for IA32_PACKAGE_THERM_STATUS.
///
/// The scope of this MSR is package-level for Sandy Bridge.
struct pkg_therm_stat
{
    /// @brief Raw 64-bit value stored in IA32_PACKAGE_THERM_STATUS.
    uint64_t **raw;
    /// @brief Status (active/not active) of the digital thermal sensor
    /// high-temperature output signal (PROCHOT#) for the package.
    ///
    /// The thermal status bit indicates whether the digital thermal sensor
    /// high-temperature output signal (package PROCHOT#) is currently active.
    /// If 1, package PROCHOT# is active, else the feature is not active. This
    /// bit field (bit 0) is RO.
    int *status;
    /// @brief Sticky bit indicating history of the package thermal sensor
    /// high-temperature output signal (PROCHOT#) since last cleared or last
    /// reset.
    ///
    /// Sticky bit indicating history of the package thermal sensor high
    /// temperature output signal (package PROCHOT#). If 1, package PROCHOT#
    /// has been asserted since a previous RESET or the last time software
    /// cleared the bit. This bit (bit 1) is R/W, but software can only clear
    /// the bit by writing a 0.
    int *status_log;
    /// @brief Indicates if package PROCHOT# # is being asserted by another
    /// component on the platform.
    ///
    /// Indicates whether another component on the platform is causing
    /// high-temperature and asserting package PROCHOT# as a result. This bit
    /// field (bit 2) is RO.
    int *PROCHOT_event;
    /// @brief Sticky bit indicating history of the package PROCHOT# signal
    /// since last cleared or last reset.
    ///
    /// Sticky bit indicating history of the package PROCHOT# signal. If 1
    /// 1, package PROCHOT# has been externally asserted by another agent on
    /// the platform since the last clearing of this bit or a reset. This bit
    /// (bit 3) is R/W, but software can only clear the bit by writing a 0.
    int *PROCHOT_log;
    /// @brief Status (active/not active) of the package critical temperature
    /// detector output signal.
    ///
    /// Indicates whether the package critical temperature detector output
    /// signal is currently active. If 1, the critical temperature detector output
    /// signal is currently active, else it is not active. This bit (bit 4) is
    /// RO.
    int *crit_temp_status;
    /// @brief Sticky bit indicating history of the package critical temperature
    /// detector output signal since last cleared or last reset.
    ///
    /// Sticky bit indicating history of the package critical temperature
    /// detector output signal. If 1, the signal has been asserted since a
    /// previous RESET or the last time software cleared the bit. This bit (bit
    /// 5) is R/W, but software can only clear the bit by writing a 0.
    int *crit_temp_log;
    /// @brief Indicates if the actual package temperature is currently higher
    /// than or equal to Package Thermal Threshold #1 (PTT#1).
    ///
    /// Indicates whether the actual package temperature is currently higher
    /// than or equal to the value set in Package Thermal Threshold #1 (PTT#1).
    /// If 0, the actual temperature is lower, else the actual temperature is
    /// greater than or equal to PTT#1. This bit field (bit 6) is RO.
    int *therm_thresh1_status;
    /// @brief Sticky bit indicating history of Package Thermal Threshold #1
    /// (PTT#1) since last cleared or last reset.
    ///
    /// Sticky bit indicating whether the Package Thermal Threshold #1 (PTT#1)
    /// has been reached since the last clearing of this bit or a reset. If 1,
    /// PTT#1 has been reached. This bit field (bit 7) is R/W, but software
    /// can only clear the bit by writing a 0.
    int *therm_thresh1_log;
    /// @brief Indicates if the actual package temperature is currently higher
    /// than or equal to Package Thermal Threshold #2 (PTT#2).
    ///
    /// Indicates whether the actual package temperature is currently higher
    /// than or equal to the value set in Package Thermal Threshold #2 (PTT#2).
    /// If 0, the actual temperature is lower, else the actual temperature is
    /// greater than or equal to PTT#2. This bit field (bit 8) is RO.
    int *therm_thresh2_status;
    /// @brief Sticky bit indicating history of Package Thermal Threshold #2
    /// (PTT#2) since last cleared or last reset.
    ///
    /// Sticky bit indicating whether the Package Thermal Threshold #2 (PTT#2)
    /// has been reached since the last clearing of this bit or a reset. If 1,
    /// PTT#2 has been reached. This bit field (bit 9) is R/W, but software
    /// can only clear the bit by writing a 0.
    int *therm_thresh2_log;
    /// @brief Indicates package power limit is forcing at least one processor
    /// to operate below OS-requested p-state.
    ///
    /// Indicates if package power limit is forcing one or more processors to
    /// operate below OS-requested p-state. Software can examine
    /// IA32_THERM_STATUS to determine if the cause originates from a processor
    /// core. This bit (bit 10) is RO.
    int *power_limit_status;	//Read only
    /// @brief Sticky bit indicating if any processor in the package went below
    /// OS-requested p-state or OS-requested clock modulation duty cycle since
    /// last cleared or last reset.
    ///
    /// Sticky bit indicating whether any processor in the package went below
    /// the OS-requested p-state or OS-requested clock modulation duty cycle
    /// since the last clearing of this bit or a reset. This informs software
    /// of such occurrence. If 1, processor went below OS requests. This bit
    /// field (bit 11) is R/W, but software can only clear the bit by writing a
    /// 0.
    int *power_notification_log;
    /// @brief Package digital temperature reading in degree Celsius relative
    /// to the TCC activation temperature.
    ///
    /// This bit field (bits 22:16) is RO.
    int *readout;
};

/// @brief Structure holding data from IA32_PACKAGE_THERM_INTERRUPT.
///
/// The scope of this MSR is package-level for Sandy Bridge.
struct pkg_therm_interrupt
{
    /// @brief Raw 64-bit value stored in IA32_PACKAGE_THERM_INTERRUPT.
    uint64_t **raw;
    /// @brief Enables the BIOS to generate an interrupt when transitioning
    /// from low temperature to a package high temperature threshold.
    ///
    /// If 0 (default), interrupts are disabled, else interrupts are enabled.
    /// This bit field (bit 0) is R/W.
    int *high_temp_enable;
    /// @brief Enables the BIOS to generate an interrupt when transitioning
    /// from high temperature to package low temperature (TCC deactivation).
    ///
    /// If 0 (default), interrupts are disabled, else interrupts are enabled.
    /// This bit field (bit 1) is R/W.
    int *low_temp_enable;
    /// @brief Enables the BIOS or OS to generate an interrupt when Package
    /// PROCHOT# has been asserted by another component on the platform and the
    /// Bidirectional Prochot feature is enabled.
    ///
    /// If 0, interrupts are disabled, else interrupts are enabled. This bit
    /// field (bit 2) is R/W.
    int *PROCHOT_enable;
    /// @brief Enables generation of an interrupt when the Package Critical
    /// Temperature Detector detects a critical thermal condition.
    ///
    /// If 0, interrupts are disabled, else interrupts are enabled. This bit
    /// field (bit 4) is R/W.
    int *crit_temp_enable;
    /// @brief Temperature threshold, encoded relative to the Package TCC
    /// Activation temperature.
    ///
    /// Format of this bit field is the same as the Package Digital Readout.
    /// This threshold is compared against the Package Digital Readout and is
    /// used to generate Package Thermal Threshold #1 Status, Package Thermal
    /// Threshold #1 Log, and Package Threshold #1 thermal interrupt delivery.
    /// This bit field (bit 14:8) is R/W.
    int *thresh1_val;
    /// @brief Enables generation of an interrupt when the actual temperature
    /// crosses the Package Threshold #1 setting in any direction.
    ///
    /// If 0, interrupt is disabled, else interrupt is enabled. This bit field
    /// (bit 15) is R/W.
    int *thresh1_enable;
    /// @brief Temperature threshold, encoded relative to the Package TCC
    /// Activation temperature.
    ///
    /// Format of this bit field is the same as the Package Digital Readout.
    /// This threshold is compared against the Package Digital Readout and is
    /// used to generate Thermal Threshold #2 Status, Thermal Threshold #2 Log,
    /// and Threshold #2 thermal interrupt delivery. This bit field (bit 22:16)
    /// is R/W.
    int *thresh2_val;
    /// @brief Enables generation of an interrupt when the actual temperature
    /// crosses the Threshold #2 setting in any direction.
    ///
    /// If 0, interrupt is disabled, else interrupt is enabled. This bit field
    /// (bit 23) is R/W.
    int *thresh2_enable;
    /// @brief Enables generation of package power notification events when the
    /// any processor in the package goes below OS-requested p-state or
    /// OS-requested clock modulation duty cycle.
    ///
    /// Enables the package to deliver a thermal event when any processor in
    /// the package went below OS-requested p-state or clock modulation duty
    /// cycle setting. This bit field (bit 24) is R/W.
    int *pwr_limit_notification_enable;
};

/// @brief Store the target temperature data on the heap.
///
/// @param [out] tt Pointer to data for target temperature.
void store_temp_target(struct msr_temp_target **tt);

/// @brief Store the per-core thermal status data on the heap.
///
/// @param [out] ts Pointer to data for per-core thermal status.
void store_therm_stat(struct therm_stat **ts);

/// @brief Store the per-core thermal interrupt data on the heap.
///
/// @param [out] ti Pointer to data for per-core thermal interrupts.
void store_therm_interrupt(struct therm_interrupt **ti);

/// @brief Store the package-level thermal status data on the heap.
///
/// @param [out] ps Pointer to data for package-level thermal status.
void store_pkg_therm_stat(struct pkg_therm_stat **ps);

/// @brief Store the package-level thermal interrupt data on the heap.
///
/// @param [out] pi Pointer to data for package-level thermal interrupts.
void store_pkg_therm_interrupt(struct pkg_therm_interrupt **pi);

/// @brief Initialize temperature target data -- store variable on the heap and
/// read current temperature target.
void is_init(void);

/************************************/
/* MSR_TEMPERATURE_TARGET Functions */
/************************************/

/// @brief Retrieve current minimum temperature (in degree Celsius) at which
/// PROCHOT will be asserted.
///
/// @param [out] s Data for target temperature.
void get_temp_target(struct msr_temp_target *s);

/*********************************************/
/* Thermal Functions (Status and Interrupts) */
/*********************************************/

/// @brief Read value of the IA32_THERM_STATUS register and translate bit
/// fields to human-readable values.
///
/// @param [out] s Data for per-core thermal status.
void get_therm_stat(struct therm_stat *s);

/// @brief Read value of the IA32_THERM_INTERRUPT register and translate bit
/// fields to human-readable values.
///
/// @param [out] s Data for per-core thermal interrupts.
void get_therm_interrupt(struct therm_interrupt *s);

/// @brief Read value of the IA32_PACKAGE_THERM_STATUS register and translate
/// bit fields to human-readable values.
///
/// @param [out] s Data for package-level thermal status.
void get_pkg_therm_stat(struct pkg_therm_stat *s);

/// @brief Read value of the IA32_PACKAGE_THERM_INTERRUPT register and
/// translate bit fields to human-readable values.
///
/// @param [out] s Data for package-level thermal interrupts.
void get_pkg_therm_interrupt(struct pkg_therm_interrupt *s);

/// @brief Store the package-level thermal control data on the heap.
///
/// @param [out] thermctlref Pointer to data for package-level thermal control.
///
/// @return 0 if successful.
int therm2_ctl_storage(uint64_t **thermctlref);

/// @brief Read value of the MSR_THERM2_CTL register.
///
/// @return 0 if successful, else 1 if read_msr_by_coord() fails.
int get_pkg_therm2_ctl(void);

/// @brief Print out control settings for Thermal Monitor #2.
///
/// @param [in] writedest File stream where output will be written to.
///
/// @return 0 if successful, else return a value less than 0 if
/// therm2_ctl_storage() fails.
int dump_therm2_ctl(FILE *writedest);

/// @brief Set value for IA32_THERM_STATUS across all cores.
///
/// @param [in] s Data for per-core thermal status.
void set_therm_stat(struct therm_stat *s);

/// @brief Set value for IA32_THERM_INTERRUPT across all cores.
///
/// @param [in] s Data for per-core thermal interrupts.
void set_therm_interrupt(struct therm_interrupt *s);

/// @brief Set value for IA32_PACKAGE_THERM_STATUS across all sockets.
///
/// @param [in] s Data for package-level thermal status.
void set_pkg_therm_stat(struct pkg_therm_stat *s);

/// @brief Set value for IA32_PACKAGE_THERM_INTERRUPT across all sockets.
///
/// @param [in] s Data for package-level thermal interrupts.
void set_pkg_therm_interrupt(struct pkg_therm_interrupt *s);

/// @brief Print the label for the abbreviated thermal status and interrupt
/// data printout.
///
/// @param [in] writedest File stream where output will be written to.
void dump_therm_data_terse_label(FILE *writedest);

/// @brief Print abbreviated thermal status and interrupt data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_therm_data_terse(FILE *writedest);

/// @brief Print the label for the detailed thermal status and interrupt data
/// printout.
///
/// @param [in] writedest File stream where output will be written to.
void dump_therm_data_verbose_label(FILE *writedest);

/// @brief Print only temperature data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_therm_temp_reading(FILE *writedest);

/// @brief Print detailed thermal status and interrupt data.
///
/// @param [in] writedest File stream where output will be written to.
void dump_therm_data_verbose(FILE *writedest);

#ifdef __cplusplus
}
#endif
#endif
