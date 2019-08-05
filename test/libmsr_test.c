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
#ifdef MPI
#include <mpi.h>
#endif

struct rapl_limit l1, l2, l3, l4;

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

void test_pkg_lower_limit(unsigned s)
{
    struct rapl_power_info raplinfo;
    double power_cap_percentage = 0.7;
    get_rapl_power_info(s, &raplinfo);
    
    double power_cap = raplinfo.pkg_therm_power * power_cap_percentage;
    printf("Setting socket %d PL1 to %.1fW (%.0f%%)\n", s, power_cap, power_cap_percentage*100);
    l1.watts = power_cap;
    l1.seconds = 1;
    l1.bits = 0;
    set_pkg_rapl_limit(s, &l1, NULL);
    get_limits();
}

void test_pkg_upper_limit(unsigned s)
{
    struct rapl_power_info raplinfo;
    double power_cap_percentage = 0.9;
    get_rapl_power_info(s, &raplinfo);

    double power_cap = raplinfo.pkg_therm_power * power_cap_percentage;
    printf("Setting socket %d PL2 to %.1fW (%.0f%%)\n", s, power_cap, power_cap_percentage*100);
    l2.watts = power_cap;
    l2.seconds = 9;
    l2.bits = 0;
    set_pkg_rapl_limit(s, NULL, &l2);
    get_limits();
}

void test_all_limits()
{
    static uint64_t sockets = 0;
    int i;
    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    struct rapl_power_info raplinfo;
    double power_cap_percentage = 0.8;

    for (i = 0; i < sockets; i++)
    {
        get_rapl_power_info(i, &raplinfo);
        l1.watts = raplinfo.pkg_therm_power * power_cap_percentage;
        l1.seconds = 1;
        l1.bits = 0;
        l2.watts = raplinfo.pkg_max_power * power_cap_percentage;
        l2.seconds =  3;
        l2.bits = 0;
        l3.watts = raplinfo.dram_max_power * power_cap_percentage;
        l3.seconds = 1;
        l3.bits = 0;
        printf("Applying %.0f%% power cap to socket %d --- PL1 = %.1fW, PL2 = %.1fW, DRAM = %.1fW.\n", power_cap_percentage*100, i, l1.watts, l2.watts, l3.watts);

        set_pkg_rapl_limit(i, &l1, &l2);
        set_dram_rapl_limit(i, &l3);
    }
    get_limits();
}

// TODO: test other parts of thermal
void thermal_test()
{
    //dump_thermal_verbose_label(stdout);
    //fprintf(stdout, "\n");
    //dump_thermal_verbose(stdout);
    //fprintf(stdout, "\n");
    dump_therm_temp_reading(stdout);
}

void counters_test()
{
    fprintf(stdout, "\n--- Fixed Performance Counters (Instr Ret, Unhalt Core Cyc, Unhalt Ref Cyc) ---\n");
    dump_fixed_counter_data_readable(stdout);

    fprintf(stdout, "\n--- Performance Counters ---\n");
    dump_pmc_data_readable(stdout);
}

// TODO: test other parts of clocks
void clocks_test()
{
    fprintf(stdout, "\n--- Read IA32_APERF, IA32_MPERF, and IA32_TIME_STAMP_COUNTER ---\n");
    dump_clocks_data_readable(stdout);

    fprintf(stdout, "\n--- Reading IA32_PERF_STATUS ---\n");
    dump_p_state(stdout);
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

// NOTE: to use this, compile a NAS parallel benchmark of your choice and
// modify the path below you will have to compile with the -D_GNU_SOURCE flag
// for setaffinity

//#define MEMTEST 1
#ifdef MEMTEST
char *args[] = {"mg.B.1"};
const char path[] = "/g/g19/walker91/NPB3.3.1/NPB3.3-MPI/bin/mg.B.1";
#endif

// We use 24 for Catalyst, (2 sockets * 12 cores)
#define NPROCS 24

void rapl_r_test(struct rapl_data **rd)
{
    //r1 = &((*rd)[0]);
    //r2 = &((*rd)[1]);
    poll_rapl_data();
    //poll_rapl_data(0, NULL);
    //poll_rapl_data(1, NULL);
    fprintf(stdout, "Sample #1:\n");
    dump_rapl_data(stdout);

#ifdef MEMTEST
    unsigned nprocs = NPROCS;
    pid_t pid[NPROCS];
    int status[NPROCS];
    cpu_set_t cpuselect;

    int i;
    for (i = 0; i < nprocs; i++)
    {
        CPU_ZERO(&cpuselect);
        CPU_SET(i, &cpuselect);
        pid[i] = fork();
        if (pid[i] == 0)
        {
            // this is just testing on 1 node
            sched_setaffinity(0, sizeof(cpu_set_t), &cpuselect);
            fprintf(stderr, "executing stress test\n");
            execve(path, args, NULL);
            exit(1);
        }
    }
    fprintf(stderr, "waiting for test to complete\n");
    for (i = 0; i < nprocs; i++)
    {
        wait(&status[i]);
    }
#endif
#ifndef MEMTEST
    int time = 5;
    fprintf(stdout, "\nSleeping for %d seconds\n", time);
    sleep(time);
#endif

    poll_rapl_data();
    fprintf(stdout, "\nSample #2:\n");
    dump_rapl_data(stdout);
}

#define PT_INC 100000

int repeated_poll_test()
{
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "Sample #1:\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #2:\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #3:\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #4:\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #5:\n");
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    fprintf(stdout, "\nSample #6:\n");
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
    unsigned i;

    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }
#ifdef MPI
    MPI_Init(&argc, &argv);
    fprintf(stdout, "===== MPI Init Done =====\n");
#endif

#ifdef SKIPMSR
    goto csrpart;
#endif

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

    fprintf(stdout, "\n===== Available Performance Counters =====\n");
    print_available_counters();

    fprintf(stdout, "\n===== POWER INFO =====\n");
    dump_rapl_power_info(stdout);

    fprintf(stdout, "\n===== POWER UNIT =====\n");
    dump_rapl_power_unit(stdout);

    fprintf(stdout, "\n===== Get Initial RAPL Power Limits =====\n");
    get_limits();

    for (i = 0; i < sockets; i++)
    {
        fprintf(stdout, "\n===== Start Socket %u RAPL Power Limit Test =====\n", i);
        fprintf(stdout, "\n--- Testing Pkg Domain Lower Limit ---\n");
        test_pkg_lower_limit(i);
        fprintf(stdout, "\n--- Testing Pkg Domain Upper Limit ---\n");
        test_pkg_upper_limit(i);
        fprintf(stdout, "\n===== End Socket %u RAPL Power Limit Test =====\n", i);
    }

    fprintf(stdout, "\n===== Testing All RAPL Power Limits All Sockets =====\n");
    test_all_limits();
    fprintf(stdout, "===== End Testing All RAPL Power Limits All Sockets =====\n");

    fprintf(stdout, "\n===== Poll RAPL Data 2X =====\n");
    rapl_r_test(&rd);

    fprintf(stdout, "\n===== Thermal Test =====\n");
    thermal_test();

    fprintf(stdout, "\n===== Clocks Test =====\n");
    clocks_test();

    fprintf(stdout, "\n===== Counters Test =====\n");
    counters_test();

    fprintf(stdout, "\n===== Turbo Test =====\n");
    turbo_test();

    fprintf(stdout, "\n===== Read IA32_MISC_ENABLE =====\n");
    misc_test();

    fprintf(stdout, "\n===== Repeated RAPL Polling Test =====\n");
    repeated_poll_test();

    fprintf(stdout, "\n===== Setting Defaults =====\n");
    set_to_defaults();

    finalize_msr();
    fprintf(stdout, "===== MSR Finalized =====\n");

#ifdef MPI
csrpart:
    MPI_Finalize();
#endif

    fprintf(stdout, "\n===== Test Finished Successfully =====\n");
    if (ri_stat)
    {
        fprintf(stdout, "\nFound %d locked rapl register(s)\n", ri_stat);
    }

    return 0;
}
