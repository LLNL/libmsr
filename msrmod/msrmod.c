/* msrmod.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory
 * Written by  Scott Walker,   walker91@llnl.gov
 *
 * All rights reserved.
 *
 * Command line utility for interacting with MSRs.
 *
 * msrmod is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msrmod is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msrmod. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <msr_rapl.h>
#include <msr_counters.h>

short ISVERBOSE;

int list_functions(char *type)
{
    if (init_msr())
    {
        fprintf(stderr, "Error: unable to initialize msr.\n");
        return -1;
    }

    if (strncmp(type, "rapl", strlen("rapl")) == 0)
    {
        /* Print available rapl stuff. */
        fprintf(stdout, "\n=== RAPL MSRs ===\n");
        print_available_rapl();
        fprintf(stdout, "\n");
    }
    else if (strncmp(type, "counters", strlen("counters")) == 0)
    {
        /* Print available counter stuff. */
        fprintf(stdout, "\n=== Counter MSRs ===\n");
        print_available_counters();
        fprintf(stdout, "\n");
    }
    else if (strncmp(type, "all", strlen("all")) == 0)
    {
        /* Print everything available. */
        fprintf(stdout, "\n=== All MSRs ===");
        fprintf(stdout, "\n- RAPL MSRs -\n");
        print_available_rapl();
        fprintf(stdout, "\n- Counter MSRs -\n");
        print_available_counters();
        fprintf(stdout, "\n");
    }
    else
    {
        fprintf(stderr, "Error: unknown list_functions() parameter \"%s\"\n", type);
        return -1;
    }
    return 0;
}

void set_to_defaults(void)
{
    int socket;
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;

    fprintf(stdout, "Setting Defaults\n");
    for (socket = 0; socket < num_sockets(); socket++)
    {
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
        if (ISVERBOSE)
        {
            fprintf(stdout, "\n=== Socket %d ===\n", socket);
            fprintf(stdout, "- Power Limit 1 -\n");
            dump_rapl_limit(&socketlim, stdout);
            fprintf(stdout, "- Power Limit 2 -\n");
            dump_rapl_limit(&socketlim2, stdout);
            fprintf(stdout, "- DRAM Power Limit -\n");
            dump_rapl_limit(&dramlim, stdout);
        }
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
        set_dram_rapl_limit(socket, &dramlim);
    }
}

void set_mem_limit(int socket, double watts, double seconds)
{
    struct rapl_limit dramlim;

    dramlim.bits = 0;
    dramlim.watts = 0;
    dramlim.seconds = 0;
    dramlim.watts = watts;
    dramlim.seconds = seconds;
    if (ISVERBOSE)
    {
        fprintf(stdout, "\n=== Socket %d ===\n", socket);
        fprintf(stdout, "- DRAM Power Limit -\n");
        dump_rapl_limit(&dramlim, stdout);
    }
    set_dram_rapl_limit(socket, &dramlim);
}

int set_functions(char *type, int socket, double watts1, double seconds1, double watts2, double seconds2)
{
    if (init_msr())
    {
        fprintf(stderr, "Error: unable to initialize msr.\n");
        return -1;
    }

    if (strncmp(type, "default", strlen("default")) == 0)
    {
        set_to_defaults();
    }
    else if (strncmp(type, "memory", strlen("memory")) == 0)
    {
        set_mem_limit(socket, watts1, seconds1);
    }
    else if (strncmp(type, "package", strlen("package")) == 0)
    {
        struct rapl_limit socketlim, socketlim2;

        socketlim.bits = 0;
        socketlim.watts = 0;
        socketlim.seconds = 0;
        socketlim2.bits = 0;
        socketlim2.watts = 0;
        socketlim2.seconds = 0;

        socketlim.watts = watts1;
        socketlim.seconds = seconds1;
        if (ISVERBOSE)
        {
            fprintf(stdout, "\n=== Socket %d ===\n", socket);
            fprintf(stdout, "- Power Limit 1 -\n");
            dump_rapl_limit(&socketlim, stdout);
        }
        socketlim2.watts = watts2;
        socketlim2.seconds = seconds2;
        if (ISVERBOSE)
        {
            fprintf(stdout, "- Power Limit 2 -\n");
            dump_rapl_limit(&socketlim2, stdout);
        }
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
    }
    else
    {
        fprintf(stderr, "Error: unknown set_functions() parameter \"%s\"\n", type);
        return -1;
    }
    return 0;
}

void get_rapl_stuff(void)
{
    struct rapl_limit l1, l2, l3;

    get_pkg_rapl_limit(0, &l1, &l2);
    fprintf(stdout, "=== Pkg Domain ===\n");
    fprintf(stdout, "- Socket 0 Limit 0 -\n");
    dump_rapl_limit(&l1, stdout);
    fprintf(stdout, "- Socket 0 Limit 1 -\n");
    dump_rapl_limit(&l2, stdout);
    get_pkg_rapl_limit(1, &l1, &l2);
    fprintf(stdout, "- Socket 1 Limit 0 -\n");
    dump_rapl_limit(&l1, stdout);
    fprintf(stdout, "- Socket 1 Limit 1 -\n");
    dump_rapl_limit(&l2, stdout);

    fprintf(stdout, "\n=== DRAM Domain ===\n");
    fprintf(stdout, "- Socket 0 -\n");
    get_dram_rapl_limit(0, &l3);
    dump_rapl_limit(&l3, stdout);
    fprintf(stdout, "- Socket 1 -\n");
    get_dram_rapl_limit(1, &l3);
    dump_rapl_limit(&l3, stdout);
}


int printing_functions(char *type)
{
    uint64_t *rapl_flags = NULL;
    struct rapl_data *rd = NULL;

    if (init_msr())
    {
        fprintf(stderr, "Error: unable to initialize msr.\n");
        return -1;
    }

    int ret = rapl_init(&rd, &rapl_flags);
    if (ret < 0)
    {
        fprintf(stderr, "Error: unable to initialize rapl.\n");
        return -1;
    }

    if (strncmp(type, "rapl", strlen("rapl")) == 0)
    {
        get_rapl_stuff();
    }
    else if (strncmp(type, "info", strlen("info")) == 0)
    {
        dump_rapl_power_info(stdout);
    }
    else
    {
        fprintf(stderr, "Error: unknown printing_functions() parameter \"%s\"\n", type);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *usage = "\n"
                        "NAME\n"
                        "  msrmod - quick access to msr functions\n"
                        "\n"
                        "SYNOPSIS\n"
                        "   %s [--help | -h]\n"
                        "      [-v] [-i] [-p data_type] [-l msr_type]\n"
                        "      [-r msr_hex] [-w msr_hex] [-s set_type]\n"
                        "      [-c socket_id] [-t thread_id] [-d value_hex]\n"
                        "      [-a power1] [-b time1] [-e power2] [-f time2]\n"
                        "\n"
                        "OVERVIEW\n"
                        "  MSRMOD is designed for quick access to common\n"
                        "  libmsr functionalities, such as restoring the\n"
                        "  RAPL power limits to default values and enforcing\n"
                        "  a package domain RAPL power limit on a socket.\n"
                        "  This tool is in alpha and is not fully tested.\n"
                        "  Updates may be sparse.\n"
                        "\n"
                        "OPTIONS\n"
                        "  --help | -h \n"
                        "      Display this help information, then exit.\n"
                        "  -v\n"
                        "      Verbose mode.\n"
                        "  -i\n"
                        "      Interactive mode currently not implemented.\n"
                        "  -p data_type\n"
                        "      Print data. Valid values for data_type are\n"
                        "      'info' for general rapl info or 'rapl' for\n"
                        "      current rapl data.\n"
                        "  -l msr_type\n"
                        "      List available MSRs. Valid values for msr_type\n"
                        "      are 'all', 'rapl', or 'counters'.\n"
                        "  -r msr_hex\n"
                        "      Read the register. MSR must be given in\n"
                        "      hexadecimal. Used in conjunction with the -t\n"
                        "      flag.\n"
                        "  -w msr_hex\n"
                        "      Write the register. MSR must be given in\n"
                        "      hexadecimal. Used in conjunction with the -t\n"
                        "      and -d flags.\n"
                        "  -s set_type\n"
                        "      Set rapl MSRs. Valid values for set_type are\n"
                        "      'default' or 'package' or 'memory'. Used in\n"
                        "      conjunction with the -c, -a, -b, -e, and -f\n"
                        "      flags.\n"
                        "  -c socket_id\n"
                        "      Socket ID where power limit will be enforced.\n"
                        "      Default is socket 0.\n"
                        "  -t thread_id\n"
                        "      Thread ID where read/write MSR will be\n"
                        "      performed. Default is thread 0.\n"
                        "  -d value_hex\n"
                        "      Value to write to MSR in hexadecimal.\n"
                        "  -a power1_watts\n"
                        "      Power limit 1 for RAPL package domain.\n"
                        "  -b time1_sec\n"
                        "      Time window 1 for RAPL package domain.\n"
                        "  -e power2_watts\n"
                        "      Power limit 2 for RAPL package domain.\n"
                        "  -f time2_sec\n"
                        "      Time window 2 for RAPL package domain.\n"
                        "\n";

    if (argc == 1 || (argc > 1 && (
                          strncmp(argv[1], "--help" , strlen("--help")) == 0 ||
                          strncmp(argv[1], "-h" , strlen("-h")) == 0 )))
    {
        printf(usage, argv[0]);
        return 0;
    }

    ISVERBOSE = 0;
    char *print_data_type = '\0';
    char *print_events = '\0';
    char *set_msr_type= '\0';
    uint64_t msr_data = 0;
    uint64_t msr = 0;
    int set_msr_data = 0;
    unsigned thread = 0;
    unsigned socket = 0;
    int isread = 0;
    int iswrite = 0;
    int isset = 0;
    int power_lim1 = 0;
    int power_lim2 = 0;
    int time_lim1 = 0;
    int time_lim2 = 0;
    int opt;

    while ((opt = getopt(argc, argv, "vip:l:r:w:s:c:t:d:a:b:e:f:")) != -1)
    {
        switch (opt)
        {
            case 'v':
                /* Do verbose stuff. */
                ISVERBOSE = 1;
                break;
            case 'i':
                /* Do interactive stuff. */
                fprintf(stdout, "Interactive mode is not yet available\n");
                break;
            case 'p':
                /* Print out stuff. */
                print_data_type = optarg;
                printing_functions(print_data_type);
                break;
            case 'l':
                /* List available events. */
                print_events = optarg;
                list_functions(print_events);
                break;
            case 'r':
                /* Read an msr. */
                isread = 1;
                msr = strtol(optarg, NULL, 16);
                break;
            case 'w':
                /* Write an msr. */
                iswrite = 1;
                msr = strtol(optarg, NULL, 16);
                break;
            case 's':
                /* Set a socket limit or restore to defaults. */
                set_msr_type = optarg;
                isset = 1;
                break;
            case 'c':
                /* Set socket ID. */
                socket = atoi(optarg);
                break;
            case 't':
                /* Set location where read/write will be performed. */
                thread = atoi(optarg);
                break;
            case 'd':
                /* Set value to write to msr. */
                set_msr_data = 1;
                msr_data = strtol(optarg, NULL, 16);
                break;
            case 'a':
                /* Power (in Watts) for RAPL limit 1. */
                power_lim1 = atof(optarg);
                break;
            case 'b':
                /* Time (in seconds) for RAPL limit 1. */
                time_lim1 = atof(optarg);
                break;
            case 'e':
                /* Power (in Watts) for RAPL limit 2. */
                power_lim2 = atof(optarg);
                break;
            case 'f':
                /* Time (in seconds) for RAPL limit 2. */
                time_lim2 = atof(optarg);
                break;
            default:
                fprintf(stderr, "\nError: unknown parameter \"%c\"\n", optopt);
                fprintf(stderr, usage, argv[0]);
                return -1;
                break;
        }
    }

    if (isread && iswrite)
    {
        fprintf(stderr, "\nError: can only perform read or write: -r OR -w.\n");
        fprintf(stderr, usage, argv[0]);
        return -1;
    }

    if (isread)
    {
        if (init_msr())
        {
            fprintf(stderr, "Error: unable to initialize msr.\n");
            return -1;
        }
        read_msr_by_idx(thread, msr, &msr_data);
        fprintf(stdout, "Reading MSR 0x%lx on CPU %d: %lx\n", msr, thread, msr_data);
    }
    else if (iswrite && set_msr_data)
    {
        if (init_msr())
        {
            fprintf(stderr, "Error: unable to initialize msr.\n");
            return -1;
        }
        write_msr_by_idx(thread, msr, msr_data);
        fprintf(stdout, "Writing 0x%lx to MSR 0x%lx on CPU %d\n", msr_data, msr, thread);
    }
    else if (iswrite && !set_msr_data)
    {
        fprintf(stderr, "\nError: missing value to write (-d).\n");
        fprintf(stderr, usage, argv[0]);
        return -1;
    }
    else if (isset)
    {
        set_functions(set_msr_type, socket, power_lim1, time_lim1, power_lim2, time_lim2);
    }
    return 0;
}
