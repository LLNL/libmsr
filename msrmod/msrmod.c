/* msrmod.c
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Scott Walker,   walker91@llnl.gov
 *
 * All rights reserved. 
 * 
 * Command line utility for interacting with MSRs.
 *
 * msrmod is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * msrmod is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with msrmod.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
 *
 */
//#include "msr_rapl.h"
//#include "msr_counters.h"
#include <stdio.h>
#include <stdlib.h>
//#include "../libmsr-walker/include/msr_rapl.h"
//#include "../libmsr-walker/include/msr_counters.h"
#include <msr_rapl.h>
#include <msr_counters.h>

short ISVERBOSE;

void help_stuff()
{
    fprintf(stdout, "msrmod help\n");
    fprintf(stdout, "This tool is in alpha so it has lots of problems. Honestly, updates will be sparse.\n");
    fprintf(stdout, "-l, -list\n\tlist available msrs. Can refine list by adding r/rapl or c/counters\n");
    fprintf(stdout, "-p\n\tprint data, must add i/info for general rapl info or r/rapl for current rapl data\n");
    fprintf(stdout, "-s, -set\n\tset rapl msrs, can restore to defaults with d/default or set a custom setting in the format\n\t");
    fprintf(stdout, "socket limit1 limit1time limit2 limit2time\n");
    fprintf(stdout, "\tcan an an 'm' flag to set memory limits in the format limit limittime\n");
    fprintf(stdout, "-i, -interactive\n\t, currently not implemented\n");
    fprintf(stdout, "-v, -verbose\n\t, msrmod will tell you what it is doing (usually), must be first argument\n");
    fprintf(stdout, "-r, -read\n\t, read the register, msr must be given in hexadecimal\n\t");
    fprintf(stdout, "thread msr\n");
    fprintf(stdout, "-w, -write\n\t, write the register, msr and data must be given in dexadecimal\n\t");
    fprintf(stdout, "thread msr data\n");
    fprintf(stdout, "--h, --help\n\tdisplay this help information\n");
}

void generic_error(int idx, char ** argv)
{
    fprintf(stderr, "Error: invalid argument %s\n", argv[idx]);
    help_stuff();
    exit(-1);
}

char eat_whitespace(char * arg)
{
    int i = 2;
    while((arg[i] == ' ' || arg[i] == '\t' || arg[i] == '\n') && arg[i] != '\0')
    {
        i++;
    }
    return arg[i - 1];
}

int do_rapl_stuff(short isverbose)
{
    int sockets = num_sockets();
    struct rapl_limit pkg, pkg2;
    struct rapl_limit dram;
    int i;
    for (i = 0; i < sockets; i++)
    {
        fprintf(stdout, "Setting Limit on Socket %d\n", i);
        fprintf(stdout, "Package Limit(Watts): ");
        fscanf(stdout, "%d", &pkg.watts);
        fprintf(stdout, "\n");
        fprintf(stdout, "Package Timeframe(Seconds): ");
        fscanf(stdout, "%d", &pkg.seconds);
        fprintf(stdout, "\n");
        fprintf(stdout, "Package Limit 2(Watts): ");
        fscanf(stdout, "%d", &pkg2.watts);
        fprintf(stdout, "\n");
        fprintf(stdout, "Package Timeframe 2(Milleseconds): ");
        fscanf(stdout, "%d", &pkg2.seconds);
        fprintf(stdout, "\n");
        fprintf(stdout, "DRAM Limit(Watts): ");
        fscanf(stdout, "%d", &dram.watts);
        fprintf(stdout, "\n");
        fprintf(stdout, "DRAM Timeframe(Seconds): ");
        fscanf(stdout, "%d", &dram.seconds);
        fprintf(stdout, "\n");

        if (set_pkg_rapl_limit(i, &pkg, &pkg2))
        {
            fprintf(stderr, "Error setting rapl limit on pkg %d\n", i);
            return -1;
        }
        if (set_dram_rapl_limit(i, &dram))
        {
            fprintf(stderr, "Error setting rapl DRAM limit on pkg %d\n", i);
            return -1;
        }
    }
    return 0;
}

void type2_list(int * idx, int argc, char ** argv)
{
    switch(argv[++*idx][0])
    {
        case 'a':
            // print everything available
            print_available_counters();
            print_available_rapl();
            break;
        case 'r':
            //print available rapl stuff
            print_available_rapl();
            break;
        case 'c':
            // print available counter stuff
            print_available_counters();
            break;
        default:
            generic_error(*idx, argv);
    }
}

void list_functions(int * idx, int argc, char ** argv)
{
    switch(argv[*idx][2])
    {
        case '\0':
        case ' ':
        case 'i':
            if (*idx + 1 >= argc)
            {
                print_available_counters();
                print_available_rapl();
                return;
            }
            type2_list(idx, argc, argv);            
            break;
        case 'a':
            print_available_counters();
            print_available_rapl();
            break;
        case 'r':
            print_available_rapl();
            break;
        case 'c':
            print_available_counters();
            break;
        default:
            generic_error(*idx, argv);
    }
}

void set_to_defaults()
{
    int socket = 0;
    int numsockets = num_sockets();
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;
    fprintf(stdout, "Setting Defaults\n");
    for (socket = 0; socket < numsockets; socket++)
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
            dump_rapl_limit(&socketlim, stdout);
            dump_rapl_limit(&socketlim2, stdout);
            dump_rapl_limit(&dramlim, stdout);
        }
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
        set_dram_rapl_limit(socket, &dramlim);
    }
}

void set_mem_limit(int * idx, int argc, char ** argv)
{
    struct rapl_limit dramlim; 
    int socket = 0;
    int numsockets = num_sockets();
    if (argc - *idx < 2)
    {
        fprintf(stderr, "Error: invalid number of parameters for dram power limit\n");
        generic_error(*idx, argv);
    }
    dramlim.bits = 0;
    dramlim.watts = 0;
    dramlim.seconds = 0;
    socket = atoi(argv[++*idx]);
    dramlim.watts = strtod(argv[++*idx], NULL);
    dramlim.seconds = strtod(argv[++*idx], NULL);
    if (ISVERBOSE) dump_rapl_limit(&dramlim, stdout);
    set_dram_rapl_limit(socket, &dramlim);
}

void type2_set(int * idx, int argc, char ** argv)
{
    int socket = 0;
    int numsockets = num_sockets();
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;
    socketlim.bits = 0;
    socketlim.watts = 0;
    socketlim.seconds = 0;
    socketlim2.bits = 0;
    socketlim2.watts = 0;
    socketlim2.seconds = 0;
    switch (argv[++*idx][0])
    {
        case 'd':
            set_to_defaults();            
            break;
        case 'm':
            // set a memory limit
            set_mem_limit(idx, argc, argv);
            break;
        default:
            // -s socket watts seconds
            //fprintf(stdout, "%s\n", argv[i]);
            if (*idx >= argc)
            {
                generic_error(*idx, argv);
            }
            if (argc - *idx < 5)
            {
                fprintf(stderr, "Error: invalid number of parameters for power limit\n");
                generic_error(*idx, argv);
            }
            socket = atoi(argv[*idx]);
            socketlim.watts = strtod(argv[++*idx], NULL);
            socketlim.seconds = strtod(argv[++*idx], NULL);
            if (ISVERBOSE) fprintf(stdout, "Socket %d\n", socket);
            if (ISVERBOSE) dump_rapl_limit(&socketlim, stdout);
            socketlim2.watts = strtod(argv[++*idx], NULL);
            socketlim2.seconds = strtod(argv[++*idx], NULL);
            if (ISVERBOSE) dump_rapl_limit(&socketlim2, stdout);
            set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
            break;
    }
}

void set_functions(int * idx, int argc, char ** argv)
{
    switch (argv[*idx][2])
    {
        case '\0':
        case ' ':
        case 'e':
            if (*idx + 1 >= argc)
            {
                generic_error(*idx, argv);
                return;
            }
            type2_set(idx, argc, argv);
            break;
        case 'm':
            set_mem_limit(idx, argc, argv);
            break;
        case 'd':
            set_to_defaults();
            break;
        default:
            generic_error(*idx, argv);
    }
}

void get_rapl_stuff()
{
    struct rapl_limit l1, l2, l3, l4;
    get_pkg_rapl_limit(0, &l1, &l2);
    fprintf(stdout, "Socket 0 Limit 0\n");
    dump_rapl_limit(&l1, stdout);
    fprintf(stdout, "Socket 0 Limit 1\n");
    dump_rapl_limit(&l2, stdout);
    get_pkg_rapl_limit(1, &l1, &l2);
    fprintf(stdout, "Socket 1 Limit 0\n");
    dump_rapl_limit(&l1, stdout);
    fprintf(stdout, "Socket 1 Limit 1\n");
    dump_rapl_limit(&l2, stdout);
    fprintf(stdout, "Socket 0 DRAM Limit\n");
    get_dram_rapl_limit(0, &l3);
    dump_rapl_limit(&l3, stdout);
    fprintf(stdout, "Socket 1 DRAM Limit\n");
    get_dram_rapl_limit(1, &l3);
    dump_rapl_limit(&l3, stdout);
    fprintf(stdout, "Socket 0 PP0 Limit\n");
    get_pp_rapl_limit(0, &l4, NULL);
    dump_rapl_limit(&l4, stdout);
    fprintf(stdout, "Socket 1 PP0 Limit\n");
    get_pp_rapl_limit(1, &l4, NULL);
    dump_rapl_limit(&l4, stdout);

}

void type2_print(int * idx, int argc, char ** argv)
{
    switch  (argv[++*idx][0])
    {
        case 'r':        
            get_rapl_stuff();            
            break;
        case 'i':
            dump_rapl_power_info(stdout);
            break;
        default:
            generic_error(*idx, argv);
    }
}

void printing_functions(int * idx, int argc, char ** argv)
{
    uint64_t * rapl_flags = NULL;
    struct rapl_data * rd = NULL, *r1 = NULL, *r2 = NULL;
    switch(argv[*idx][2])
    {
        rapl_init(&rd, &rapl_flags);
        case '\0':
            if (*idx >= argc)
            {
                generic_error(*idx, argv);
            }
            type2_print(idx, argc, argv);
            break;
        case 'r':
            get_rapl_stuff();
            break;
        case 'i':
            dump_rapl_power_info(stdout);
            break;
        default:
            generic_error(*idx, argv);
    }
}

int main(int argc, char ** argv)
{
    short israpl = 0;
    char next = '\0';
    ISVERBOSE = 0;
    init_msr();
    uint64_t msr_data;
    uint64_t msr;
    unsigned thread;
    int i;
    for (i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'v':
                    // do verbose stuff
                    ISVERBOSE = 1;
                    break;
                case 'r':
                    // read an msr
                    if (argc - i < 3)
                    {
                        fprintf(stderr, "Error: not enough parameters to read flag\n");
                        generic_error(i, argv);
                    }
                    thread = atoi(argv[++i]);
                    msr = strtol(argv[++i], NULL, 16);
                    read_msr_by_idx(thread, msr, &msr_data);
                    fprintf(stdout, "%lx\n", msr_data);
                    break;
                case 'w':
                    // write an msr
                    if (argc - i < 4)
                    {
                        fprintf(stderr, "Error: not enough parameters to write flag\n");
                        generic_error(i, argv);
                    }
                    thread = atoi(argv[++i]);
                    msr = strtol(argv[++i], NULL, 16);
                    msr_data = strtol(argv[++i], NULL, 16);
                    write_msr_by_idx(thread, msr, msr_data);
                    break;
                case 'i':
                    // do interactive stuff
                    fprintf(stdout, "Interactive mode is not yet available\n");
                    break;
                case '-':
                    // go deeper down the rabbit hole
                    switch(argv[i][2])
                    {
                        case 'h':
                            // print help stuff
                            help_stuff();
                            break;
                    }
                    return 0;
                case 'p':
                    // print out stuff
                    printing_functions(&i, argc, argv);
                    break;
                case 's':
                    // set a socket limit or set to default
                    set_functions(&i, argc, argv);
                    break;
                case 'l':
                    // list available events
                    list_functions(&i, argc, argv);
                    
                    return 0;
            }
        }
    }
 //   if (israpl)
 //   {
//        do_rapl_stuff(isverbose);
 //   }
    return 0;
}
