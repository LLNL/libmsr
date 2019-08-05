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
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#include "msr_core.h"
#include "memhdlr.h"
#include "msr_thermal.h"
#include "cpuid.h"
#include "libmsr_debug.h"

/// @brief Initialize storage for MSR_TEMPERATURE_TARGET data.
///
/// @param [out] tt Data for target temperature.
static void init_temp_target(struct msr_temp_target *tt)
{
    static uint64_t sockets = 0;

    sockets = num_sockets();
    tt->raw = (uint64_t **) libmsr_malloc(sockets * sizeof(uint64_t *));
    tt->temp_target = (uint64_t *) libmsr_malloc(sockets * sizeof(uint64_t));
    allocate_batch(TEMP_TARGET, num_sockets());
    load_socket_batch(MSR_TEMPERATURE_TARGET, tt->raw, TEMP_TARGET);
}

/// @brief Initialize storage for IA32_THERM_STATUS.
///
/// @param [out] ts Data for per-core thermal status.
static void init_therm_stat(struct therm_stat *ts)
{
    uint64_t cores =  num_cores();

    ts->raw = (uint64_t **) libmsr_malloc(cores * sizeof(uint64_t *));
    ts->status = (int *) libmsr_malloc(cores * sizeof(int));
    ts->status_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->PROCHOT_or_FORCEPR_event = (int *) libmsr_malloc(cores * sizeof(int));
    ts->PROCHOT_or_FORCEPR_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->crit_temp_status = (int *) libmsr_malloc(cores * sizeof(int));
    ts->crit_temp_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->therm_thresh1_status = (int *) libmsr_malloc(cores * sizeof(int));
    ts->therm_thresh1_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->therm_thresh2_status = (int *) libmsr_malloc(cores * sizeof(int));
    ts->therm_thresh2_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->power_limit_status = (int *) libmsr_malloc(cores * sizeof(int));
    ts->power_notification_log = (int *) libmsr_malloc(cores * sizeof(int));
    ts->readout = (int *) libmsr_malloc(cores * sizeof(int));
    ts->resolution_deg_celsius = (int *) libmsr_malloc(cores * sizeof(int));
    ts->readout_valid = (int *) libmsr_malloc(cores * sizeof(int));
    allocate_batch(THERM_STAT, cores);
    load_core_batch(IA32_THERM_STATUS, ts->raw, THERM_STAT);
}

/// @brief Initialize storage for IA32_THERM_INTERRUPT.
///
/// @param [out] ti Data for per-core thermal interrupts.
static void init_therm_interrupt(struct therm_interrupt *ti)
{
    uint64_t cores =  num_cores();

    ti->raw = (uint64_t **) libmsr_malloc(cores * sizeof(uint64_t *));
    ti->high_temp_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->low_temp_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->PROCHOT_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->FORCEPR_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->crit_temp_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->thresh1_val = (int *) libmsr_malloc(cores * sizeof(int));
    ti->thresh1_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->thresh2_val = (int *) libmsr_malloc(cores * sizeof(int));
    ti->thresh2_enable = (int *) libmsr_malloc(cores * sizeof(int));
    ti->pwr_limit_notification_enable = (int *) libmsr_malloc(cores * sizeof(int));
    allocate_batch(THERM_INTERR, cores);
    load_core_batch(IA32_THERM_INTERRUPT, ti->raw, THERM_INTERR);
}

/// @brief Initialize storage for IA32_PACKAGE_THERM_STATUS.
///
/// @param [out] pts Data for package-level thermal status.
static void init_pkg_therm_stat(struct pkg_therm_stat *pts)
{
    uint64_t sockets = num_sockets();

    pts->raw = (uint64_t **) libmsr_malloc(sockets * sizeof(uint64_t *));
    pts->status = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->status_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->PROCHOT_event = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->PROCHOT_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->crit_temp_status = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->crit_temp_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->therm_thresh1_status = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->therm_thresh1_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->therm_thresh2_status = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->therm_thresh2_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->power_limit_status = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->power_notification_log = (int *) libmsr_malloc(sockets * sizeof(int));
    pts->readout = (int *) libmsr_malloc(sockets * sizeof(int));
    allocate_batch(PKG_THERM_STAT, sockets);
    load_socket_batch(IA32_PACKAGE_THERM_STATUS, pts->raw, PKG_THERM_STAT);
}

/// @brief Initialize storage for IA32_PACKAGE_THERM_INTERRUPT.
///
/// @param [out] pti Data for package-level thermal interrupts.
static void init_pkg_therm_interrupt(struct pkg_therm_interrupt *pti)
{
    uint64_t sockets = num_sockets();

    pti->raw = (uint64_t **) libmsr_malloc(sockets * sizeof(uint64_t *));
    pti->high_temp_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->low_temp_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->PROCHOT_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->crit_temp_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->thresh1_val = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->thresh1_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->thresh2_val = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->thresh2_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    pti->pwr_limit_notification_enable = (int *) libmsr_malloc(sockets * sizeof(int));
    allocate_batch(PKG_THERM_INTERR, sockets);
    load_socket_batch(IA32_PACKAGE_THERM_INTERRUPT, pti->raw, PKG_THERM_INTERR);
}

void store_temp_target(struct msr_temp_target **tt)
{
    static struct msr_temp_target t_target;
    static int init = 0;

    if (!init)
    {
        init_temp_target(&t_target);
        init = 1;
    }
    if (tt != NULL)
    {
        *tt = &t_target;
    }
}

void store_therm_stat(struct therm_stat **ts)
{
    static struct therm_stat t_stat;
    static int init = 0;

    if (!init)
    {
        init_therm_stat(&t_stat);
        init = 1;
    }
    if (ts != NULL)
    {
        *ts = &t_stat;
    }
}

void store_therm_interrupt(struct therm_interrupt **ti)
{
    static struct therm_interrupt t_interrupt;
    static int init = 0;

    if (!init)
    {
        init_therm_interrupt(&t_interrupt);
        init = 1;
    }
    if (ti != NULL)
    {
        *ti = &t_interrupt;
    }
}

void store_pkg_therm_stat(struct pkg_therm_stat **ps)
{
    static struct pkg_therm_stat pkg_status;
    static int init = 0;

    if (!init)
    {
        init_pkg_therm_stat(&pkg_status);
        init = 1;
    }
    if (ps != NULL)
    {
        *ps = &pkg_status;
    }
}

void store_pkg_therm_interrupt(struct pkg_therm_interrupt **pi)
{
    static struct pkg_therm_interrupt pkg_interrupt;
    static int init = 0;

    if (!init)
    {
        init_pkg_therm_interrupt(&pkg_interrupt);
        init = 1;
    }
    if (pi != NULL)
    {
        *pi = &pkg_interrupt;
    }
}

void is_init(void)
{
    static int init = 0;
    static struct msr_temp_target *t_target = NULL;

    if (t_target == NULL)
    {
        store_temp_target(&t_target);
    }
    if (!init)
    {
        get_temp_target(t_target);
        init = 1;
    }
    else
    {
        return;
    }
}

void get_temp_target(struct msr_temp_target *s)
{
    static uint64_t sockets = 0;
    int i;

    if (!sockets)
    {
        sockets = num_sockets();
    }
    read_batch(TEMP_TARGET);
    for (i = 0; i < sockets; i++)
    {
        // Minimum temperature at which PROCHOT will be asserted in degree
        // Celsius (probably the TCC Activation Temperature).
        s->temp_target[i] = MASK_VAL(*s->raw[i], 23, 16);
    }
}

void get_therm_stat(struct therm_stat *s)
{
    uint64_t numCores = num_cores();
    int i;

    read_batch(THERM_STAT);
    for (i = 0; i< numCores; i++)
    {
        // Indicates whether the digital thermal sensor high-temperature output
        // signal (PROCHOT#) is currently active.
        // (1=active)
        s->status[i] = MASK_VAL(*s->raw[i], 0, 0);

        // Indicates the history of the thermal sensor high temperature output
        // signal (PROCHOT#).
        // If equals 1, PROCHOT# has been asserted since a previous RESET or
        // clear 0 by user.
        s->status_log[i] = MASK_VAL(*s->raw[i], 1, 1);

        // Indicates whether PROCHOT# or FORCEPR# is being asserted by another
        // agent on the platform.
        s->PROCHOT_or_FORCEPR_event[i] = MASK_VAL(*s->raw[i], 2, 2);

        // Indicates whether PROCHOT# or FORCEPR# has been asserted by another
        // agent on the platform since the last clearing of the bit or a reset.
        // (1=has been externally asserted)
        // (0 to clear)
        // External PROCHOT# assertions are only acknowledged if the
        // Bidirectional Prochot feature is enabled.
        s->PROCHOT_or_FORCEPR_log[i] = MASK_VAL(*s->raw[i], 3, 3);

        // Indicates whether actual temp is currently higher than or equal to
        // the value set in Thermal Thresh 1.
        // (0 then actual temp is lower)
        // (1 then equal to or higher)
        s->crit_temp_status[i] = MASK_VAL(*s->raw[i], 4, 4);

        // Sticky bit indicates whether the crit temp detector output signal
        // has been asserted since the last reset or clear
        // (0 cleared) (1 asserted)
        s->crit_temp_log[i] = MASK_VAL(*s->raw[i], 5, 5);

        // Indicates whether actual temp is currently higher than or equal to
        // the value set in Thermal Threshold 1.
        // (0 actual temp is lower)
        // (1 actual temp is greater than or equal to TT#1)
        s->therm_thresh1_status[i] = MASK_VAL(*s->raw[i], 6, 6);

        // Sticky bit indicates whether Thermal Threshold #1 has been reached
        // since last reset or clear 0.
        s->therm_thresh1_log[i] = MASK_VAL(*s->raw[i], 7, 7);

        // Same as therm_thresh1_status, except for Thermal Threshold #2.
        s->therm_thresh2_status[i] = MASK_VAL(*s->raw[i], 8, 8);

        // Same as therm_thresh1_log, except for Thermal Threshold #2.
        s->therm_thresh2_log[i] = MASK_VAL(*s->raw[i], 9, 9);

        // Indicates whether the processor is currently operating below
        // OS-requested P-state (specified in IA32_PERF_CTL), or OS-requested
        // clock modulation duty cycle (in IA32_CLOCK_MODULATION).
        // This field supported only if CPUID.06H:EAX[bit 4] = 1.
        // Package level power limit notification can be delivered
        // independently to IA32_PACKAGE_THERM_STATUS MSR.
        s->power_limit_status[i] = MASK_VAL(*s->raw[i], 10, 10);

        // Sticky bit indicates the processor went below OS-requested P-state
        // or OS-requested clock modulation duty cycle since last RESET or
        // clear 0.
        // Supported only if CPUID.06H:EAX[bit 4] = 1.
        // Package level power limit notification is indicated independently in
        // IA32_PACKAGE_THERM_STATUS MSR.
        s->power_notification_log[i] = MASK_VAL(*s->raw[i], 11, 11);

        // Digital temperature reading in 1 degree Celsius relative to the TCC
        // activation temperature.
        // (0: TCC Activation temperature)
        // (1: (TCC Activation -1)... etc.)
        s->readout[i] = MASK_VAL(*s->raw[i], 22, 16);

        // Specifies the resolution (tolerance) of the digital thermal sensor.
        // The value is in degrees Celsius. Recommended that new threshold
        // values be offset from the current temperature by at least the
        // resolution + 1 in order to avoid hysteresis of interrupt generation.
        s->resolution_deg_celsius[i] = MASK_VAL(*s->raw[i], 30, 27);

        // Indicates if the digital readout is valid (valid if = 1).
        s->readout_valid[i] = MASK_VAL(*s->raw[i], 31, 31);
    }
}

void get_therm_interrupt(struct therm_interrupt *s)
{
    uint64_t numCores = num_cores();
    int i;

    read_batch(THERM_INTERR);
    for (i = 0; i < numCores; i++)
    {
        // Allows the BIOS to enable the generation of an interrupt on the
        // transition from low-temp to a high-temp threshold.
        // 0 (default) disable[i]s interrupts (1 enables interrupts).
        s->high_temp_enable[i] = MASK_VAL(*s->raw[i], 0, 0);

        // Allows the BIOS to enable generation of an interrupt on the
        // transition from high temp to low temp (TCC deactivation-activation).
        // 0 (default) disable[i]s interrupts (1 enables interrupts).
        s->low_temp_enable[i] = MASK_VAL(*s->raw[i], 1, 1);

        // Allows BIOS or OS to enable generation of an interrupt when PROCHOT
        // has been asserted by another agent on the platform and the
        // Bidirectional Prochot feature is enable[i]d.
        // (0 disable[i]s) (1 enables).
        s->PROCHOT_enable[i] = MASK_VAL(*s->raw[i], 2, 2);

        // Allows the BIOS or OS to enable generation of an interrupt when
        // FORCEPR# has been asserted by another agent on the platform.
        // (0 disable[i]s the interrupt) (2 enables).
        s->FORCEPR_enable[i] = MASK_VAL(*s->raw[i], 3, 3);

        // Enables generations of interrupt when the critical temperature
        // detector has detected a critical thermal condition.
        // Recommended response: system shutdown.
        // (0 disable[i]s interrupt) (1 enables).
        s->crit_temp_enable[i] = MASK_VAL(*s->raw[i], 4, 4);

        // A temp threshold. Encoded relative to the TCC Activation temperature
        // (same format as digital readout) used to generate
        // therm_thresh1_status and therm_thresh1_log and Threshold #1 thermal
        // interrupt delivery.
        s->thresh1_val[i] = MASK_VAL(*s->raw[i], 14, 8);

        // Enables generation of an interrupt when the actual temperature
        // crosses Threshold #1 setting in any direction.
        // (ZERO ENABLES the interrupt) (ONE DISABLES the interrupt).
        s->thresh1_enable[i] = MASK_VAL(*s->raw[i], 15, 15);

        // See above description for thresh1_val (just for thresh2).
        s->thresh2_val[i] = MASK_VAL(*s->raw[i], 22, 16);

        // See above description for thresh1_enable (just for thresh2).
        s->thresh2_enable[i] = MASK_VAL(*s->raw[i], 23, 23);

        // Enables generation of power notification events when the processor
        // went below OS-requested P-state or OS-requested clock modulation
        // duty cycle.
        // THIS FIELD SUPPORTED ONLY IF CPUID.06H:EAX[bit 4] = 1.
        // Package level power limit notification can be enable[i]d
        // independently by IA32_PACKAGE_THERM_INTERRUPT MSR.
        s->pwr_limit_notification_enable[i] = MASK_VAL(*s->raw[i], 24, 24);
    }
}

void get_pkg_therm_stat(struct pkg_therm_stat *s)
{
    uint64_t sockets = num_sockets();
    int i;

    read_batch(PKG_THERM_STAT);
    for (i = 0; i < sockets; i++)
    {
        // Indicates whether the digital thermal sensor high-temp output signal
        // (PROCHOT#) for the pkg currently active. (1=active)
        s->status[i] = MASK_VAL(*s->raw[i], 0, 0);

        // Indicates the history of thermal sensor high temp output signal
        // (PROCHOT#) of pkg.
        // (1= pkg PROCHOT# has been asserted since previous reset or last time
        // software cleared bit.
        s->status_log[i] = MASK_VAL(*s->raw[i], 1, 1);

        // Indicates whether pkg PROCHOT# is being asserted by another agent on
        // the platform.
        s->PROCHOT_event[i] = MASK_VAL(*s->raw[i], 2, 2);

        // Indicates whether pkg PROCHOT# has been asserted by another agent on
        // the platform since the last clearing of the bit by software or
        // reset. (1= has been externally asserted) (write 0 to clear).
        s->PROCHOT_log[i] = MASK_VAL(*s->raw[i], 3, 3);

        // Indicates whether pkg crit temp detector output signal is currently
        // active (1=active).
        s->crit_temp_status[i] = MASK_VAL(*s->raw[i], 4, 4);

        // Indicates whether pkg crit temp detector output signal been asserted
        // since the last clearing of bit or reset.
        // (1=has been asserted) (set 0 to clear).
        s->crit_temp_log[i] = MASK_VAL(*s->raw[i], 5, 5);

        // Indicates whether actual pkg temp is currently higher than or equal
        // to value set in Package Thermal Threshold #1.
        // (0=actual temp lower) (1= actual temp >= PTT#1).
        s->therm_thresh1_status[i] = MASK_VAL(*s->raw[i], 6, 6);

        // Indicates whether pkg therm threshold #1 has been reached since last
        // software clear of bit or reset. (1=reached) (clear with 0).
        s->therm_thresh1_log[i] = MASK_VAL(*s->raw[i], 7, 7);

        // Same as above (therm_thresh1_stat) except it is for threshold #2.
        s->therm_thresh2_status[i] = MASK_VAL(*s->raw[i], 8, 8);

        // Same as above (therm_thresh2_log) except it is for threshold #2
        s->therm_thresh2_log[i] = MASK_VAL(*s->raw[i], 9, 9);

        // Indicates pkg power limit forcing 1 or more processors to operate
        // below OS-requested P-state.
        // (Note: pkg power limit violation may be caused by processor cores or
        // by devices residing in the uncore - examine IA32_THERM_STATUS to
        // determine if cause from processor core).
        s->power_limit_status[i] = MASK_VAL(*s->raw[i], 10, 10);

        // Indicates any processor from package went below OS-requested P-state
        // or OS-requested clock modulation duty cycle since last clear or
        // RESET.
        s->power_notification_log[i] = MASK_VAL(*s->raw[i], 11, 11);

        // Pkg digital temp reading in 1 degree Celsius relative to the pkg TCC
        // activation temp.
        // (0 = Package TCC activation temp)
        // (1 = (PTCC Activation - 1) etc.
        // Note: lower reading actually higher temp
        s->readout[i] = MASK_VAL(*s->raw[i], 22, 16);
    }
}

void get_pkg_therm_interrupt(struct pkg_therm_interrupt *s)
{
    uint64_t sockets = num_sockets();
    int i;

    read_batch(PKG_THERM_INTERR);
    for (i = 0; i < sockets; i++)
    {
        // Allows the BIOS to enable the generation of an interrupt on
        // transition from low temp to pkg high temp threshold.
        // (0 (default)- disables interrupts) (1=enables interrupts)
        s->high_temp_enable[i] = MASK_VAL(*s->raw[i], 0, 0);

        // Allows BIOS to enable the generation of an interrupt on transition
        // from high temp to a low temp (TCC deactivation-activation).
        // (0 (default)- disables interrupts) (1=enables interrupts)
        s->low_temp_enable[i] = MASK_VAL(*s->raw[i], 1, 1);

        // Allows BIOS or OS to enable generation of an interrupt when pkg
        // PROCHOT# has been asserted by another agent on the platform and the
        // Bidirectional Prochot feature is enabled. (0 disables interrupt) (1
        // enables interrupt)
        s->PROCHOT_enable[i] = MASK_VAL(*s->raw[i], 2, 2);

        // Enables generation of interrupt when pkg crit temp detector has
        // detected a crit thermal condition. Recommended response: system shut
        // down.
        // (0 disables interrupt) (1 enables)
        s->crit_temp_enable[i] = MASK_VAL(*s->raw[i], 4, 4);

        // A temp threshold, encoded relative to the Package TCC Activation
        // temp using format as Digital Readout.
        // Compared against the Package Digital Readout and used to generate
        // Package Thermal Threshold #1 status and log bits as well as the
        // Package Threshold #1 thermal interrupt delivery.
        s->thresh1_val[i] = MASK_VAL(*s->raw[i], 14, 8);

        // Enables the generation of an interrupt when the actual temp crosses
        // the thresh1_val setting in any direction.
        // (0 enables interrupt) (1 disables interrupt)
        s->thresh1_enable[i] = MASK_VAL(*s->raw[i], 15, 15);

        // See thresh1_val.
        s->thresh2_val[i] = MASK_VAL(*s->raw[i], 22, 16);

        // See thresh1_enable.
        s->thresh2_enable[i] = MASK_VAL(*s->raw[i], 23, 23);

        // Enables generation of package power notification events.
        s->pwr_limit_notification_enable[i] = MASK_VAL(*s->raw[i], 24, 24);
    }
}

// probably not worth batching

int therm2_ctl_storage(uint64_t **thermctlref)
{
    static uint64_t sockets = 0;
    static uint64_t *thermctl;

    if (sockets == 0)
    {
        sockets = num_sockets();
        thermctl = (uint64_t *) libmsr_malloc(sockets * sizeof(uint64_t));
    }
    if (thermctlref != NULL)
    {
        *thermctlref = thermctl;
    }
    return 0;
}

int get_pkg_therm2_ctl(void)
{
    uint64_t sockets = num_sockets();
    uint64_t *thermctl;
    int ret;
    int i;

    therm2_ctl_storage(&thermctl);
    for (i = 0; i < sockets; i++)
    {
        ret = read_msr_by_coord(i, 0, 0, MSR_THERM2_CTL, &thermctl[i]);
        if (ret)
        {
            return ret;
        }
    }
    return 0;
}

int dump_therm2_ctl(FILE *writedest)
{
    int ret = 0;
    int i;
    uint64_t *thermctl;
    uint64_t sockets = num_sockets();

    ret = get_pkg_therm2_ctl();
    ret += therm2_ctl_storage(&thermctl);
    fprintf(writedest, "Therm2 CTL\nSocket\tValue\n");
    for (i = 0; i < sockets; i++)
    {
        fprintf(writedest, "%d\t%lx\n", i, thermctl[i]);
    }
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}

void set_therm_stat(struct therm_stat *s)
{
    uint64_t numCores = num_cores();
    int i;

    read_batch(THERM_STAT);
    for (i = 0; i < numCores; i++)
    {
        *s->raw[i] = (*s->raw[i] & (~(1<<1))) | (s->status_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<3))) | (s->PROCHOT_or_FORCEPR_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<5))) | (s->crit_temp_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<7))) | (s->therm_thresh1_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<9))) | (s->therm_thresh2_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<11))) | (s->power_notification_log[i] << 1);
    }
    write_batch(THERM_STAT);
    /* Not sure if I should update the struct here or not. */
}

void set_therm_interrupt(struct therm_interrupt *s)
{
    uint64_t numCores = num_cores();
    int i;

    read_batch(THERM_INTERR);
    for (i = 0; i < numCores; i++)
    {
        *s->raw[i] = (*s->raw[i] & (~(1<<0))) | (s->high_temp_enable[i] << 0);
        *s->raw[i] = (*s->raw[i] & (~(1<<1))) | (s->low_temp_enable[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<2))) | (s->PROCHOT_enable[i] << 2);
        *s->raw[i] = (*s->raw[i] & (~(1<<3))) | (s->FORCEPR_enable[i] << 3);
        *s->raw[i] = (*s->raw[i] & (~(1<<4))) | (s->crit_temp_enable[i] << 4);
        *s->raw[i] = (*s->raw[i] & (~(7<<8))) | (s->thresh1_val[i] << 8);
        *s->raw[i] = (*s->raw[i] & (~(1<<15))) | (s->thresh1_enable[i] << 15);
        *s->raw[i] = (*s->raw[i] & (~(7<<16))) | (s->thresh2_val[i] << 16);
        *s->raw[i] = (*s->raw[i] & (~(1<<23))) | (s->thresh2_enable[i] << 23);
        *s->raw[i] = (*s->raw[i] & (~(1<<24))) | (s->pwr_limit_notification_enable[i] << 24);
    }
    write_batch(THERM_INTERR);
}

void set_pkg_therm_stat(struct pkg_therm_stat *s)
{
    uint64_t sockets = num_sockets();
    int i;

    read_batch(PKG_THERM_STAT);
    for (i = 0; i < sockets; i++)
    {
        *s->raw[i] = (*s->raw[i] & (~(1<<1))) | (s->status_log[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<3))) | (s->PROCHOT_log[i] << 3);
        *s->raw[i] = (*s->raw[i] & (~(1<<5))) | (s->crit_temp_log[i] << 5);
        *s->raw[i] = (*s->raw[i] & (~(1<<7))) | (s->therm_thresh1_log[i] << 7);
        *s->raw[i] = (*s->raw[i] & (~(1<<9))) | (s->therm_thresh2_log[i] << 9);
        *s->raw[i] = (*s->raw[i] & (~(1<<11))) | (s->power_notification_log[i] << 11);
    }
    write_batch(PKG_THERM_STAT);
}

void set_pkg_therm_interrupt(struct pkg_therm_interrupt *s)
{
    uint64_t sockets = num_sockets();
    read_batch(PKG_THERM_INTERR);
    int i;
    for (i = 0; i < sockets; i++)
    {
        *s->raw[i] = (*s->raw[i] & (~(1<<0))) | (s->high_temp_enable[i] << 0);
        *s->raw[i] = (*s->raw[i] & (~(1<<1))) | (s->low_temp_enable[i] << 1);
        *s->raw[i] = (*s->raw[i] & (~(1<<2))) | (s->PROCHOT_enable[i] << 2);
        *s->raw[i] = (*s->raw[i] & (~(1<<4))) | (s->crit_temp_enable[i] << 4);
        *s->raw[i] = (*s->raw[i] & (~(7<<8))) | (s->thresh1_val[i] << 8);
        *s->raw[i] = (*s->raw[i] & (~(1<<15))) | (s->thresh1_enable[i] << 15);
        *s->raw[i] = (*s->raw[i] & (~(7<<16))) | (s->thresh2_val[i] << 16);
        *s->raw[i] = (*s->raw[i] & (~(1<<23))) | (s->thresh2_enable[i] << 23);
        *s->raw[i] = (*s->raw[i] & (~(1<<24))) | (s->pwr_limit_notification_enable[i] << 24);
    }
    write_batch(PKG_THERM_INTERR);
}

void dump_therm_data_terse_label(FILE *writedest)
{
    int core;
    int socket;
    uint64_t sockets = num_sockets();
    uint64_t coresPerSocket = cores_per_socket();

    for (socket = 0; socket < sockets; socket++)
    {
        for (core = 0; core < coresPerSocket; core++)
        {
            fprintf(writedest, "TempC_%02d_%02d ", socket, core);
        }
    }
}

void dump_therm_data_terse(FILE *writedest)
{
    static struct therm_stat *t_stat = NULL;
    static struct msr_temp_target *t_target = NULL;
    int core, socket;
    int actTemp;
    uint64_t sockets, coresPerSocket;

    is_init();
    if (t_stat == NULL || t_target == NULL)
    {
        store_therm_stat(&t_stat);
        store_temp_target(&t_target);
    }
    get_therm_stat(t_stat);
    sockets = num_sockets();
    coresPerSocket = cores_per_socket();

    for (socket=0; socket < sockets; socket++)
    {
        for (core= 0; core < coresPerSocket; core++)
        {
            actTemp = t_target->temp_target[socket] - t_stat->readout[core];
            fprintf(writedest, "%d ", actTemp);
        }
    }
}

void dump_therm_data_verbose_label(FILE *writedest)
{
    int socket;
    int core;
    uint64_t sockets = num_sockets();
    uint64_t coresPerSocket = cores_per_socket();
    for (socket = 0; socket < sockets; socket++)
    {
        /* Registers that are socket granularity. */

        /* Package-Level Thermal Status Dump */
        fprintf(writedest, "socket_status_%02d ", socket);
        fprintf(writedest, "socket_log_%02d ", socket);
        fprintf(writedest, "socket_PROCHOT_event_%02d ", socket);
        fprintf(writedest, "socket_PROCHOT_log_%02d ", socket);
        fprintf(writedest, "socket_crit_temp_status_%02d ", socket);
        fprintf(writedest, "socket_crit_temp_log_%02d ", socket);
        fprintf(writedest, "socket_therm_thresh1_status_%02d ", socket);
        fprintf(writedest, "socket_therm_thresh1_log_%02d ", socket);
        fprintf(writedest, "socket_therm_thresh2_status_%02d ", socket);
        fprintf(writedest, "socket_therm_thresh2_log_%02d ", socket);
        fprintf(writedest, "socket_power_limit_status_%02d ", socket);
        fprintf(writedest, "socket_power_notification_log_%02d ", socket);
        fprintf(writedest, "socket_readout_%02d ", socket);
        fprintf(writedest, "socket_TempC_%02d ", socket);
        /* Package-Level Thermal Interrupt Dump */
        fprintf(writedest, "socket_high_temp_enable_%02d " , socket);
        fprintf(writedest, "socket_low_temp_enable_%02d " , socket);
        fprintf(writedest, "socket_PROCHOT_enable_%02d " , socket);
        fprintf(writedest, "socket_crit_temp_enable_%02d " , socket);
        fprintf(writedest, "socket_thresh1_val_%02d " , socket);
        fprintf(writedest, "socket_thresh1_actual_tempC_%02d " , socket);
        fprintf(writedest, "socket_thresh1_enable_%02d " , socket);
        fprintf(writedest, "socket_thresh2_val_%02d " , socket);
        fprintf(writedest, "socket_thresh2_actual_tempC_%02d " , socket);
        fprintf(writedest, "socket_thresh2_enable_%02d " , socket);
        fprintf(writedest, "socket_pwr_limit_notification_enable_%02d " , socket);

        /* Registers that are core granularity */
        for (core= 0; core < coresPerSocket; core++)
        {
            /* Per-Core Thermal Status Dump */
            fprintf(writedest, "core_status_%02d_%02d " , socket, core);
            fprintf(writedest, "core_status_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_PROCHOT_or_FORCEPR_event_%02d_%02d " , socket, core);
            fprintf(writedest, "core_PROCHOT_or_FORCEPR_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_crit_temp_status_%02d_%02d " , socket, core);
            fprintf(writedest, "core_crit_temp_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_therm_thresh1_status_%02d_%02d " , socket, core);
            fprintf(writedest, "core_therm_thresh1_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_therm_thresh2_status_%02d_%02d " , socket, core);
            fprintf(writedest, "core_therm_thresh2_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_power_limit_status_%02d_%02d ", socket, core);
            fprintf(writedest, "core_power_notification_log_%02d_%02d " , socket, core);
            fprintf(writedest, "core_readout_%02d_%02d " , socket, core);
            fprintf(writedest, "core_TempC_%02d_%02d " , socket, core);
            fprintf(writedest, "core_resolution_deg_celsius_%02d_%02d " , socket, core);
            fprintf(writedest, "core_readout_valid_%02d_%02d " , socket, core);
            /* Per-Core Thermal Interrupt Dump */
            fprintf(writedest, "core_high_temp_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_low_temp_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_PROCHOT_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_FORCEPR_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_crit_temp_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh1_val_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh1_actual_tempC_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh1_enable_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh2_val_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh2_actual_tempC_%02d_%02d " , socket, core);
            fprintf(writedest, "core_thresh2_enable_%02d_%02d ", socket, core);
            fprintf(writedest, "core_pwr_limit_notification_enable_%02d_%02d " , socket, core);
        }
    }
}

void dump_therm_temp_reading(FILE *writedest)
{
    static struct therm_stat *t_stat = NULL;
    static struct therm_interrupt *t_interrupt = NULL;
    static struct pkg_therm_stat *pkg_stat = NULL;
    static struct msr_temp_target *t_target = NULL;
    static struct pkg_therm_interrupt *pkg_interrupt = NULL;
    int actTemp = 0;
    int i, j;
    uint64_t sockets, cores;

    is_init();
    if (pkg_stat == NULL)
    {
        store_therm_stat(&t_stat);
        store_therm_interrupt(&t_interrupt);
        store_pkg_therm_stat(&pkg_stat);
        store_temp_target(&t_target);
        store_pkg_therm_interrupt(&pkg_interrupt);
    }
    get_therm_stat(t_stat);
    get_therm_interrupt(t_interrupt);
    get_pkg_therm_stat(pkg_stat);
    get_pkg_therm_interrupt(pkg_interrupt);
    get_temp_target(t_target);
    sockets = num_sockets();
    cores = num_cores();
    for (i = 0; i < sockets; i++)
    {
        fprintf(writedest, "socket reading actual TCC\n");
        fprintf(writedest, "%6d", i);
        fprintf(writedest, "%8d", pkg_stat->readout[i]);
        actTemp = (t_target->temp_target[i] - pkg_stat->readout[i]);
        fprintf(writedest, "%7d", actTemp);
        fprintf(writedest, "%4lu\n", t_target->temp_target[i]);
        fprintf(writedest, "core reading actual valid\n");
        for (j = 0; j < cores; j++)
        {
            fprintf(writedest, "%4d%8d", j, t_stat->readout[j]);
            actTemp = (t_target->temp_target[i]-t_stat->readout[j]);
            fprintf(writedest, "%7d", actTemp);
            fprintf(writedest, "%6d\n", t_stat->readout_valid[j]);
        }
    }
}

void dump_therm_data_verbose(FILE *writedest)
{
    static struct therm_stat *t_stat = NULL;
    static struct therm_interrupt *t_interrupt = NULL;
    static struct pkg_therm_interrupt *pkg_interrupt = NULL;
    static struct pkg_therm_stat *pkg_stat = NULL;
    static struct msr_temp_target *t_target = NULL;
    int core, socket, actTemp;
    uint64_t sockets, coresPerSocket;

    is_init();
    if (t_stat == NULL || pkg_stat == NULL || t_interrupt == NULL || pkg_interrupt == NULL)
    {
        store_therm_stat(&t_stat);
        store_therm_interrupt(&t_interrupt);
        store_pkg_therm_interrupt(&pkg_interrupt);
        store_pkg_therm_stat(&pkg_stat);
        store_temp_target(&t_target);
    }
    get_therm_stat(t_stat);
    get_therm_interrupt(t_interrupt);
    get_pkg_therm_stat(pkg_stat);
    get_pkg_therm_interrupt(pkg_interrupt);
    get_temp_target(t_target);
#ifdef LIBMSR_DEBUG
    fprintf(stderr, "%s %s::%d DEBUG: (dump_therm_data_verbose)\n", getenv("HOSTNAME"), __FILE__, __LINE__);
#endif
    sockets = num_sockets();
    coresPerSocket = cores_per_socket();
    for (socket = 0; socket < sockets; socket++)
    {
        /* Registers that are socket granularity */

        /* Package-Level Thermal Status Dump */
        fprintf(writedest, "%d ", pkg_stat->status[socket]);
        fprintf(writedest, "%d ", pkg_stat->status_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->PROCHOT_event[socket]);
        fprintf(writedest, "%d ", pkg_stat->PROCHOT_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->crit_temp_status[socket]);
        fprintf(writedest, "%d ", pkg_stat->crit_temp_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->therm_thresh1_status[socket]);
        fprintf(writedest, "%d ", pkg_stat->therm_thresh1_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->therm_thresh2_status[socket]);
        fprintf(writedest, "%d ", pkg_stat->therm_thresh2_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->power_limit_status[socket]);
        fprintf(writedest, "%d ", pkg_stat->power_notification_log[socket]);
        fprintf(writedest, "%d ", pkg_stat->readout[socket]);
        actTemp = (t_target->temp_target[socket] - pkg_stat->readout[socket]);
        fprintf(writedest, "%d ", actTemp);
        /* Package-Level Thermal Interrupt Dump */
        fprintf(writedest, "%d ", pkg_interrupt->high_temp_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->low_temp_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->PROCHOT_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->crit_temp_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->thresh1_val[socket]);
        actTemp = (t_target->temp_target[socket]-pkg_interrupt->thresh1_val[socket]);
        fprintf(writedest, "%d ", actTemp);
        fprintf(writedest, "%d ", pkg_interrupt->thresh1_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->thresh1_val[socket]);
        actTemp = (t_target->temp_target[socket]-pkg_interrupt->thresh2_val[socket]);
        fprintf(writedest, "%d ", actTemp);
        fprintf(writedest, "%d ", pkg_interrupt->thresh2_enable[socket]);
        fprintf(writedest, "%d ", pkg_interrupt->pwr_limit_notification_enable[socket]);

        /* Registers that are core granularity */
        for (core = 0; core < coresPerSocket; core++)
        {
            /* Per-Core Thermal Status Dump */
            fprintf(writedest, "%d ", t_stat->status[core]);
            fprintf(writedest, "%d ", t_stat->status_log[core]);
            fprintf(writedest, "%d ", t_stat->PROCHOT_or_FORCEPR_event[core]);
            fprintf(writedest, "%d ", t_stat->PROCHOT_or_FORCEPR_log[core]);
            fprintf(writedest, "%d ", t_stat->crit_temp_status[core]);
            fprintf(writedest, "%d ", t_stat->crit_temp_log[core]);
            fprintf(writedest, "%d ", t_stat->therm_thresh1_status[core]);
            fprintf(writedest, "%d ", t_stat->therm_thresh1_log[core]);
            fprintf(writedest, "%d ", t_stat->therm_thresh2_status[core]);
            fprintf(writedest, "%d ", t_stat->therm_thresh2_log[core]);
            fprintf(writedest, "%d ", t_stat->power_limit_status[core]);
            fprintf(writedest, "%d ", t_stat->power_notification_log[core]);
            fprintf(writedest, "%d ", t_stat->readout[core]);
            actTemp = (t_target->temp_target[socket]-t_stat->readout[core]);
            fprintf(writedest, "%d ", actTemp);
            fprintf(writedest, "%d ", t_stat->resolution_deg_celsius[core]);
            fprintf(writedest, "%d ", t_stat->readout_valid[core]);
            /* Per-Core Thermal Interrupt Dump */
            fprintf(writedest, "%d ", t_interrupt->high_temp_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->low_temp_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->PROCHOT_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->FORCEPR_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->crit_temp_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->thresh1_val[core]);
            actTemp = (t_target->temp_target[socket]-t_interrupt->thresh1_val[core]);
            fprintf(writedest, "%d ", actTemp);
            fprintf(writedest, "%d ", t_interrupt->thresh1_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->thresh2_val[core]);
            actTemp = (t_target->temp_target[socket]-t_interrupt->thresh2_val[core]);
            fprintf(writedest, "%d ", actTemp);
            fprintf(writedest, "%d ", t_interrupt->thresh2_enable[core]);
            fprintf(writedest, "%d ", t_interrupt->pwr_limit_notification_enable[core]);
        }
    }
}
