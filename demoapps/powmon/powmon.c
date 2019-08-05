/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Daniel Ellsworth <ellsworth8@llnl.gov>,
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

#define _GNU_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <msr_core.h>
#include <msr_counters.h>
#include "ghighres.c"
#include "highlander.h"
#include "rapl.h"

/********/
/* RAPL */
/********/
static double total_joules = 0.0;
static double limit_joules = 0.0;
static double max_watts = 0.0;
static double min_watts = 1024.0;

/*************************/
/* HW Counter Structures */
/*************************/
static unsigned long start;
static unsigned long end;
static FILE *logfile = NULL;
static FILE *summaryfile = NULL;

static pthread_mutex_t mlock;
static int *shmseg;
static int shmid;

static int running = 1;

#include "common.c"

int main(int argc, char **argv)
{
    const char *usage = "\n"
                        "NAME\n"
                        "  powmon - Package and DRAM power monitor\n"
                        "SYNOPSIS\n"
                        "  %s [--help | -h] [-c] <executable> <args> ...\n"
                        "OVERVIEW\n"
                        "  Powmon is a utility for sampling and printing the\n"
                        "  power consumption (for package and DRAM) and power\n"
                        "  limit per socket for systems with two sockets.\n"
                        "OPTIONS\n"
                        "  --help | -h\n"
                        "      Display this help information, then exit.\n"
                        "  -c\n"
                        "      Remove stale shared memory.\n"
                        "\n";
    if (argc == 1 || (argc > 1 && (
                          strncmp(argv[1], "--help", strlen("--help")) == 0 ||
                          strncmp(argv[1], "-h", strlen("-h")) == 0 )))
    {
        printf(usage, argv[0]);
        return 0;
    }
    if (argc < 2)
    {
        printf(usage, argv[0]);
        return 1;
    }

    int opt;
    while ((opt = getopt(argc, argv, "c")) != -1)
    {
        switch(opt)
        {
            case 'c':
                highlander_clean();
                return 0;
            default:
                fprintf(stderr, "\nError: unknown paramater \"%c\"\n", opt);
                fprintf(stderr, usage, argv[0]);
                return -1;
        }
    }

    if (highlander())
    {
        /* Start the log file. */
        int logfd;
        char hostname[64];
        gethostname(hostname,64);

        char *fname;
        int ret = asprintf(&fname, "%s.power.dat", hostname);
        if (ret < 0)
        {
            printf("Fatal Error: Cannot allocate memory for fname.\n");
            return 1; 
        }

        logfd = open(fname, O_WRONLY|O_CREAT|O_EXCL|O_NOATIME|O_NDELAY, S_IRUSR|S_IWUSR);
        if (logfd < 0)
        {
            printf("Fatal Error: %s on %s cannot open the appropriate fd.\n", argv[0], hostname);
            return 1;
        }
        logfile = fdopen(logfd, "w");
        fprintf(logfile, "time pkg_joules0 pkg_joules1 pkg_limwatts0 pkg_limwatts1 dram_joules0 dram_joules1 instr0 instr1 core0 core1\n");

        /* Start power measurement thread. */
        pthread_t mthread;
        pthread_mutex_init(&mlock, NULL);
        pthread_create(&mthread, NULL, power_measurement, NULL);

        /* Fork. */
        pid_t app_pid = fork();
        if (app_pid == 0)
        {
            /* I'm the child. */
            execvp(argv[1], &argv[1]);
            printf("fork failure\n");
            return 1;
        }
        /* Wait. */
        waitpid(app_pid, NULL, 0);
        sleep(1);

        highlander_wait();

        /* Stop power measurement thread. */
        running = 0;
        take_measurement();
        end = now_ms();

        /* Output summary data. */
        ret = asprintf(&fname, "%s.power.summary", hostname);
        if (ret < 0)
        {
            printf("Fatal Error: Cannot allocate memory for fname.\n");
            return 1;
        }

        logfd = open(fname, O_WRONLY|O_CREAT|O_EXCL|O_NOATIME|O_NDELAY, S_IRUSR|S_IWUSR);
        if (logfd < 0)
        {
            printf("Fatal Error: %s on %s cannot open the appropriate fd.\n", argv[0], hostname);
            return 1;
        }
        summaryfile = fdopen(logfd, "w");
        char *msg;
        ret = asprintf(&msg, "host: %s\npid: %d\ntotal_joules: %lf\nallocated: %lf\nmax_watts: %lf\nmin_watts: %lf\nruntime ms: %lu\nstart: %lu\nend: %lu\n", hostname, app_pid, total_joules, limit_joules, max_watts, min_watts, end-start, start, end);
        if (ret < 0)
        {
            printf("Fatal Error: Cannot allocate memory for msg.\n");
            return 1;
        }

        fprintf(summaryfile, "%s", msg);
        fclose(summaryfile);
        close(logfd);

        shmctl(shmid, IPC_RMID, NULL);
        shmdt(shmseg);
    }
    else
    {
        /* Fork. */
        pid_t app_pid = fork();
        if (app_pid == 0)
        {
            /* I'm the child. */
            execvp(argv[1], &argv[1]);
            printf("Fork failure\n");
            return 1;
        }
        /* Wait. */
        waitpid(app_pid, NULL, 0);

        highlander_wait();
    }

    return 0;
}
