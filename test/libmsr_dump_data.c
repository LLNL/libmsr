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

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cpuid.h"
#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "msr_counters.h"
#include "msr_clocks.h"
#include "msr_misc.h"
#include "msr_turbo.h"
#include "csr_core.h"
#include "csr_imc.h"
#include "libmsr_error.h"

struct rapl_limit l1, l2, l3;

void get_limits()
{
    int i;
    static uint64_t sockets = 0;

    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    for (i = 0; i < sockets; i++)
    {
        if (i != 0)
        {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "Socket %d:\n", i);
        if (get_pkg_rapl_limit(i, &l1, &l2) == 0)
        {
            fprintf(stdout, "Pkg Domain Power Lim 1 (lower lim)\n");
            dump_rapl_limit(&l1, stdout);
            fprintf(stdout, "\n");
            fprintf(stdout, "Pkg Domain Power Lim 2 (upper lim)\n");
            dump_rapl_limit(&l2, stdout);
        }
        if (get_dram_rapl_limit(i, &l3) == 0)
        {
            fprintf(stdout, "\nDRAM Domain\n");
            dump_rapl_limit(&l3, stdout);
        }
    }
}

// TODO: test other parts of thermal
void thermal_test()
{
    dump_therm_temp_reading(stdout);
    fprintf(stdout, "\n* Thermal Terse Printout *\n");
    dump_therm_data_verbose_label(stdout);
    dump_therm_data_verbose(stdout);
    fprintf(stdout, "\n");
}

void counters_test()
{
    fprintf(stdout, "\n--- Fixed Performance Counters (Instr Ret, Unhalt Core Cyc, Unhalt Ref Cyc) ---\n");
    dump_fixed_counter_data_readable(stdout);

    fprintf(stdout, "\n* Fixed Perf Counters Terse Printout *\n");
    dump_fixed_counter_data_terse_label(stdout);
    dump_fixed_counter_data_terse(stdout);
    fprintf(stdout, "\n");

    fprintf(stdout, "\n--- Performance Counters ---\n");
    dump_pmc_data_readable(stdout);
}

// TODO: test other parts of clocks
void clocks_test()
{
    uint64_t sockets = 0;
    int i, j;
    struct clock_mod t;
    uint64_t coresPerSocket;

    sockets = num_sockets();
    coresPerSocket = cores_per_socket();

    fprintf(stdout, "\n--- Read IA32_APERF, IA32_MPERF, and IA32_TIME_STAMP_COUNTER ---\n");
    dump_clocks_data_readable(stdout);

    fprintf(stdout, "\n* Clocks Terse Printout *\n");
    dump_clocks_data_terse_label(stdout);
    dump_clocks_data_terse(stdout);
    fprintf(stdout, "\n");

    fprintf(stdout, "\n--- Reading IA32_PERF_STATUS ---\n");
    dump_p_state(stdout);

    fprintf(stdout, "\n--- Reading IA32_CLOCK_MODULATION ---\n");
    for (i = 0; i < sockets; i++)
    {
        for (j = 0; j < coresPerSocket; j++)
        {
            get_clock_mod(i, j, &t);
            fprintf(stdout, "Socket %d, Core %2d\n", i, j);
            dump_clock_mod(&t, stdout);
            fprintf(stdout, "\n");
        }
    }
}

void misc_test()
{
    struct misc_enable s;
    uint64_t sockets = 0;
    int i;
    core_config(NULL, NULL, &sockets, NULL);
    for (i = 0; i < sockets; i++)
    {
        get_misc_enable(i, &s);
        dump_misc_enable(&s);
    }
}

void turbo_test()
{
    dump_turbo(stdout);
}

#define PT_INC 100000

void rapl_r_test(struct rapl_data **rd)
{
    //r1 = &((*rd)[0]);
    //r2 = &((*rd)[1]);
    poll_rapl_data();
    //poll_rapl_data(0, NULL);
    //poll_rapl_data(1, NULL);
    fprintf(stdout, "Sample #1\n");
    dump_rapl_data(stdout);

    usleep(PT_INC);

    poll_rapl_data();
    fprintf(stdout, "\nSample #2\n");
    dump_rapl_data(stdout);
}

int repeated_poll_test()
{
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "Sample #1\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #2\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #3\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #4\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #5\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #6\n");
    dump_rapl_data(stdout);
    return 0;
}

void set_to_defaults()
{
    int socket = 0;
    int numsockets = num_sockets();
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;

    for (socket = 0; socket < numsockets; socket++)
    {
        if (socket != 0)
        {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "Socket %d:\n", socket);
        get_rapl_power_info(socket, &raplinfo);
        socketlim.bits = 0;
        socketlim.watts = raplinfo.pkg_therm_power;
        socketlim.seconds = 1;
        socketlim2.bits = 0;
        socketlim2.watts = raplinfo.pkg_therm_power * 1.2;
        socketlim2.seconds = 3;
        dramlim.bits = 0;
        dramlim.watts = raplinfo.dram_max_power;
        dramlim.seconds = 1;
        fprintf(stdout, "Pkg Domain Power Lim 1 (lower lim)\n");
        dump_rapl_limit(&socketlim, stdout);
        fprintf(stdout, "\n");
        fprintf(stdout, "Pkg Domain Power Lim 2 (upper lim)\n");
        dump_rapl_limit(&socketlim2, stdout);
        fprintf(stdout, "\nDRAM Domain\n");
        dump_rapl_limit(&dramlim, stdout);
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
        set_dram_rapl_limit(socket, &dramlim);
    }
}

// TODO: check if test for oversized bitfield is in place, change that warning
// to an error
int main(int argc, char **argv)
{
    struct rapl_data *rd = NULL;
    uint64_t *rapl_flags = NULL;
    uint64_t cores = 0;
    uint64_t threads = 0;
    uint64_t sockets = 0;
    int ri_stat = 0;

    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }

    if (init_msr())
    {
        libmsr_error_handler("Unable to initialize libmsr", LIBMSR_ERROR_MSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    fprintf(stdout, "\n===== MSR Init Done =====\n");

    ri_stat = rapl_init(&rd, &rapl_flags);
    if (ri_stat < 0)
    {
        libmsr_error_handler("Unable to initialize rapl", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    fprintf(stdout, "\n===== RAPL Init Done =====\n");

    fprintf(stdout, "\n===== Available RAPL Registers =====\n");
    print_available_rapl();

    fprintf(stdout, "\n===== Get Initial RAPL Power Limits =====\n");
    get_limits();

    fprintf(stdout, "\n===== Thermal Test =====\n");
    thermal_test();

    fprintf(stdout, "\n===== Clocks Test =====\n");
    clocks_test();

    fprintf(stdout, "===== Read IA32_MISC_ENABLE =====\n");
    misc_test();

    fprintf(stdout, "\n===== Available Performance Counters =====\n");
    print_available_counters();

    fprintf(stdout, "\n===== Counters Test =====\n");
    counters_test();

    fprintf(stdout, "\n===== Poll RAPL Data 2X =====\n");
    rapl_r_test(&rd);

    fprintf(stdout, "\n===== Repeated RAPL Polling Test =====\n");
    repeated_poll_test();

    fprintf(stdout, "\n===== Setting Defaults =====\n");
    set_to_defaults();

    finalize_msr();
    fprintf(stdout, "\n===== MSR Finalized =====\n");

    fprintf(stdout, "\n===== Test Finished Successfully =====\n");
    if (ri_stat)
    {
        fprintf(stdout, "\nFound %d locked rapl register(s)\n", ri_stat);
    }

    return 0;
}
