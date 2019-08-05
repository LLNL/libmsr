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
    l2.seconds = 1;
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

    fprintf(stdout, "\n===== Setting Defaults =====\n");
    set_to_defaults();

    finalize_msr();
    fprintf(stdout, "===== MSR Finalized =====\n");

    fprintf(stdout, "\n===== Test Finished Successfully =====\n");
    if (ri_stat)
    {
        fprintf(stdout, "\nFound %d locked rapl register(s)\n", ri_stat);
    }

    return 0;
}
