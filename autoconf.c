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

// This code will generate the correct header files for the target
// architecture.

#include <ctype.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNAME_SIZE  128
#define BUFFER_SIZE 512

uint64_t detect_arch(void)
{
    uint64_t rax = 1;
    uint64_t rbx = 0;
    uint64_t rcx = 0;
    uint64_t rdx = 0;

    asm volatile(
        "cpuid"
        : "=a" (rax), "=b" (rbx), "=c" (rcx), "=d" (rdx)
        : "0" (rax), "2" (rcx));

    return ((rax >> 4) & 0xF) | ((rax >> 12) & 0xF0);
}

FILE *open_header(char *a)
{
    FILE *header = NULL;
    char fname[FNAME_SIZE];
    uint64_t arch = detect_arch();

    if (a != NULL) {
        snprintf(fname, FNAME_SIZE, "platform_headers/Intel%s.h", a);
    }
    else {
        snprintf(fname, FNAME_SIZE, "platform_headers/Intel%lX.h", arch);
    }
    header = fopen(fname, "r");
    if (header == NULL) {
        fprintf(stderr, "ERROR: unable to open file %s\n", fname);
        if (a != NULL) {
            fprintf(stderr, "Model %s may not be supported. No matching header files found in platform_headers/.\n", a);
        }
        else {
            fprintf(stderr, "Model %lx may not be supported. Use -f to force.\n", arch);
        }
        exit(-1);
    }
    return header;
}

FILE *open_master(void)
{
    FILE *master = fopen("include/master.h", "w");

    if (master == NULL) {
        fprintf(stderr, "ERROR: unable to open file include/master.h\n");
        exit(-2);
    }
    return master;
}

int copy(char *arch)
{
    FILE *header = open_header(arch);
    FILE *master = open_master();
    char *buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
    size_t bsize = BUFFER_SIZE;
    size_t ret = 0;

    fprintf(master, "// This file was generated automatically by autoconf.c\n");
    fprintf(master, "// You should not modify it unless you really know what you're doing.\n");

    while ((ret = getline(&buffer, &bsize, header)) != -1) {
        fprintf(master, "%s", buffer);
    }

    if (header != NULL) {
        fclose(header);
    }
    if (master != NULL) {
        fclose(master);
    }
    if (buffer != NULL) {
        free(buffer);
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *usage = "\n"
                        "NAME\n"
                        "  autoconf - Detect architecture model for MSR definitions\n"
                        "SYNOPSIS\n"
                        "  %s [--help | -h] [-f]\n"
                        "OVERVIEW\n"
                        "  Autoconf was built to determine the architecture model for\n"
                        "  which libmsr is being built on. The architecture determines\n"
                        "  the offsets for several MSRs, which may be architecture-specific.\n"
                        "OPTIONS\n"
                        "  --help | -h\n"
                        "      Display this help information, then exit.\n"
                        "  -f\n"
                        "      Target architecture model.\n"
                        "\n";
    if (argc > 1 && (strncmp(argv[1], "--help", strlen("--help")) == 0 ||
                     strncmp(argv[1], "-h", strlen("-h")) == 0 )) {
        printf(usage, argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc > 3) {
        printf(usage, argv[0]);
        return EXIT_FAILURE;
    }
    /* Auto-detect architecture model. */
    if (argc == 1) {
        copy(NULL);
        return EXIT_SUCCESS;
    }

    int opt;
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch(opt) {
            case 'f':
                copy(optarg);
                break;
            default:
                fprintf(stderr, "\nError: unknown parameter \"%c\"\n", opt);
                fprintf(stderr, usage, argv[0]);
                return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
