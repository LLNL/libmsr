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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
static double watt_cap = 0.0;

static pthread_mutex_t mlock;
static int *shmseg;
static int shmid;

static int running = 1;

#include "common.c"

int main(int argc, char **argv)
{
    printf("starting wrapper\n");
    if (argc < 3)
    {
        printf("Usage: %s <watt_cap> <executable> <args>...\n", argv[0]);
        return 1;
    }

    if (highlander())
    {
        /* Start the log file. */
        int logfd;
        char hostname[64];
        gethostname(hostname, 64);

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

        read_rapl_init();

        /* Set the cap. */
        watt_cap = strtod(argv[1], NULL);
        set_rapl_power(watt_cap, watt_cap);

        /* Start power measurement thread. */
        pthread_t mthread;
        pthread_mutex_init(&mlock, NULL);
        pthread_create(&mthread, NULL, power_measurement, NULL);

        /* Fork. */
        pid_t app_pid = fork();
        if(app_pid == 0)
        {
            /* I'm the child. */
            execvp(argv[2], &argv[2]);
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
        char *msg;
        ret = asprintf(&msg, "host: %s\npid: %d\ntotal: %lf\nallocated: %lf\nmax_watts: %lf\nmin_watts: %lf\nruntime ms: %lu\n,start: %lu\nend: %lu\n", hostname, app_pid, total_joules, limit_joules, max_watts, min_watts, end-start, start, end);

        if (ret < 0)
        {
            printf("Fatal Error: Cannot allocate memory for msg\n");
            return 1;
        }
        fprintf(logfile, "%s", msg);
        fclose(logfile);
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
            execvp(argv[2], &argv[2]);
            return 1;
        }
        /* Wait. */
        waitpid(app_pid, NULL, 0);

        highlander_wait();
    }

    return 0;
}
