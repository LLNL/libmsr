/* power_actuator.c
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree,   rountree@llnl.gov
 *             Daniel Ellsworth, ellsworth8@llnl.gov
 *             Scott Walker,     walker91@llnl.gov
 *             Kathleen Shoga,   shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

#include "config.h"

#include <mpi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//FIXME - remove after sleep forever loop is removed
#include <unistd.h>

#include <expose/highres_timer.h>
#include <expose/cachecrack.h>
#include <expose/expose.h>
#include <expose/exposeutil.h>

#include "rapl.h"

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif
static char hostname[HOST_NAME_MAX];
static char* hostnames;
static double* allocations;
static int commsize;

static double bound;
static pthread_mutex_t lock;

static int running=1;

static struct _EXPOSE_query targetQ;

/// returns the index for a name in the hostnames list.
// len is the length of a hostname field in the array
int nameToIndex(char* name, char* hostnames, int len) {
    for(int i=0; i<commsize; i++) {
        char* h = hostnames+i*len;
        if(strncmp(h,name,len)==0) {
            return i;
        }
    }
    return -1;
}

int targetReadCallback(char* buf, int len) {
    cachecrack_t resp;
    cachecrack(&resp, buf, len);
    if(resp.cols !=3 || resp.retcode != 0) {
        printf("power target query returned bad data\n%s\n",buf);
        cachecrack_free(&resp);
        return 1;
    }
    for(int i=0;i<resp.rows; i++) {
        char* name = resp.rowdata[i][0];
        double s0 = strtod(resp.rowdata[i][1],NULL);
        double s1 = strtod(resp.rowdata[i][2],NULL);
        int idx = nameToIndex(name, hostnames, HOST_NAME_MAX);
        if(idx<0) {
            printf("bad host name %s in list\n",name);
        } else {
            allocations[idx*2]   = s0;
            allocations[idx*2+1] = s1;
        }
    }
    cachecrack_free(&resp);
}

void masterRun() {
    int err;
    printf("Master started\n");

    // Initialize local book keeping
    double socketallocations[2];
    double* lastallocations=(double*)malloc(sizeof(double)*commsize*2);
    hostnames=(char*)malloc(sizeof(char)*HOST_NAME_MAX*commsize);
    for(int i=0;i<commsize*2;i++) {
        allocations[i]=115;
        lastallocations[i]=115;
    }

    // Initialize targets query
    targetQ.intervalms = 0;
    targetQ.callback = targetReadCallback;
    targetQ.query_text = "select node,socket0alloc,socket1alloc from powertargets";
    
    MPI_Gather(hostname, HOST_NAME_MAX, MPI_CHAR, hostnames, HOST_NAME_MAX, MPI_CHAR, 0, MPI_COMM_WORLD);
    while(running) {
        pthread_mutex_lock(&lock);
        int err = EXPOSE_submit_query(&targetQ);
        if(err) {
            printf("something went wrong in the query\n");
            continue;
        }
        // compute settings
        double alloc=0;
        for(int i=0;i<commsize*2;i++) {
            alloc += allocations[i];
        }
        if(alloc>bound+.0001) {
            // The targets are bad and assume cheating, reject the targets
            printf("Targets are rejected %f exceeds %f\n",alloc,bound);
            for(int i=0;i<commsize*2;i++) {
                allocations[i] = bound/(commsize*2);
            }
        }
        for(int i=0;i<commsize*2;i++) {
            if(lastallocations[i]>allocations[i]) {
                lastallocations[i]=allocations[i];
            }
        }
        
        // Apply Settings
        printf("applying settings\n");
        MPI_Scatter(lastallocations, 2, MPI_DOUBLE, socketallocations, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        setAllocations(socketallocations[0], socketallocations[1]);
        
        memcpy(lastallocations, allocations, sizeof(double)*commsize*2);

        MPI_Scatter(allocations, 2, MPI_DOUBLE, socketallocations, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        setAllocations(socketallocations[0], socketallocations[1]);

        // wait for kick
        pthread_mutex_unlock(&lock);
        EXPOSE_blockforkick("poweractuator");
    }
}

void workerRun() {
    double socketallocations[2];
    MPI_Gather(hostname, HOST_NAME_MAX, MPI_CHAR, NULL, 0, MPI_CHAR, 0, MPI_COMM_WORLD);
    while(running) {
        MPI_Scatter(NULL, 2, MPI_DOUBLE, socketallocations, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        setAllocations(socketallocations[0], socketallocations[1]);
        MPI_Scatter(NULL, 2, MPI_DOUBLE, socketallocations, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        setAllocations(socketallocations[0], socketallocations[1]);
        EXPOSE_blockforkick("poweractuator");
    }
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int err;
    int myrank;

    if(argc != 1) {
        printf("Usage: %s\nThe env var POWERACTUATOR_BOUND sets the global bound\n",argv[0]);
        return 1;
    }
    bound = strtod(getenv("POWERACTUATOR_BOUND"),NULL);
    printf("Actuator settings:\n\tbound: %f\n",bound);

    myrapl_init();

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    err = gethostname(hostname, HOST_NAME_MAX);
    if(err) {
        printf("%s fails due to gethostname() error...\n",argv[0]);
        return 1;
    }

    err = EXPOSE_init();
    if(err) {
        printf("EXPOSE_init() failure\n");
        return 1;
    }

    int toggle=0;
    err = EXPOSE_bcasthaltwait(&running, &toggle);
    if(err) {
        printf("shutdown will not work.\n");
    }

    if(myrank==0) {
        // subscribe for kicks and control exec
        MPI_Comm_size(MPI_COMM_WORLD, &commsize);
        allocations=(double*)malloc(sizeof(double)*commsize*2);
        masterRun();
    } else {
        // enter the barrier to wait on the leader
        workerRun();
    }

    MPI_Finalize();
}
