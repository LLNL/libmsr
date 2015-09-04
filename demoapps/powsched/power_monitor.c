/* power_monitor.c
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

#include <stdlib.h>
#include <stdio.h>

//FIXME - remove after sleep forever loop is removed
#include <unistd.h>

#include <expose/highres_timer.h>
#include <expose/expose.h>

/// Structure defining a metric for EXPOSE
static struct _EXPOSE_metric powerMetric;

/// Structure defining a query for EXPOSE
static struct _EXPOSE_query  settingsQuery;
static char CTABLE[]="create table PowerIn (node varchar(128), sock0con real, sock1con real, sock0alloc real, sock1alloc real)";
static char RINSERT[]="insert into PowerIn values('%s','%f','%f','%f','%f')";
static char METRIC_NAME[]="PowerIn";
static char METRIC_DESC[]="Power readings/settings on sockets";

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif
char hostname[HOST_NAME_MAX];

/* newer libmsr */
#ifdef HAVE_LIBMSR2
#include <msr/msr_rapl.h>
#include <msr/msr_core.h>
static struct rapl_data rdat[2];
static uint64_t rflags[2];
int getReadings(char* buf, int buflen) {
    printf("calling getReadings\n");
    struct rapl_limit rlim[2];
    double cw0, cw1, aw0, aw1;
    
    poll_rapl_data(0,NULL);
    poll_rapl_data(1,NULL);
    get_pkg_rapl_limit(0,&(rlim[0]),NULL);
    get_pkg_rapl_limit(1,&(rlim[1]),NULL);

    get_pkg_rapl_limit(0,&(rlim[0]),NULL);
    get_pkg_rapl_limit(1,&(rlim[1]),NULL);

    cw0 = rdat[0].pkg_watts;
    cw1 = rdat[1].pkg_watts;
    aw0 = rlim[0].watts;
    aw1 = rlim[1].watts;
    int len = snprintf(buf,buflen,RINSERT,hostname,cw0,cw1,aw0,aw1);
    if(len>buflen) {
        printf("Buffer not long enough for write. Halt and catch fire\n");
        return 1;
    }
    return 0;
}
void myrapl_init() {
    init_msr();
    rflags[0] = 0;
    rflags[1] = 0;
    rapl_init(&rdat, &rflags);
}
#endif
#ifdef HAVE_LIBMSR1
#include <msr/msr_rapl.h>
#include <msr/msr_core.h>
static struct rapl_data rdat[2];
int getReadings(char* buf, int buflen) {
    struct rapl_limit rlim[2];
    double cw0, cw1, aw0, aw1;
    read_rapl_data(0,&(rdat[0]));
    read_rapl_data(1,&(rdat[1]));
    get_rapl_limit(0,&(rlim[0]),NULL,NULL);
    get_rapl_limit(1,&(rlim[1]),NULL,NULL);
    cw0 = rdat[0].pkg_watts;
    cw1 = rdat[1].pkg_watts;
    aw0 = rlim[0].watts;
    aw1 = rlim[1].watts;
    int len = snprintf(buf,buflen,RINSERT,hostname,cw0,cw1,aw0,aw1);
    if(len>buflen) {
        printf("Buffer not long enough for write. Halt and catch fire\n");
        return 1;
    }
    return 0;
}
void myrapl_init() {
    init_msr();
}
#endif

int preparePowerMetricForAnnounce() {
    myrapl_init();

    // name the metric
    powerMetric.name=METRIC_NAME;
    // describe the metric
    powerMetric.desc=METRIC_DESC;
    // define the metric table
    powerMetric.create_table=CTABLE;
    // provide the callback function
    powerMetric.callback=getReadings;
    return 0;
}
int preparePowerSettingForSubscribe() {
    return 0;
}

int main(int argc, char** argv) {
    int err;
    fprintf(stderr, "monitor starting\n");

    err = gethostname(hostname, HOST_NAME_MAX);
    if(err) {
        fprintf(stderr, "%s fails due to gethostname() error...\n",argv[0]);
        return 1;
    }

    err = EXPOSE_init();
    if(err) {
        fprintf(stderr, "EXPOSE_init() failure\n");
        return 1;
    }

    fprintf(stderr, "preparing the metric structure\n");
    // Prepare the metric structure
    err = preparePowerMetricForAnnounce();
    fprintf(stderr, "announcing the metric structure\n");
    EXPOSE_announce_metric(&powerMetric);
    fprintf(stderr, "sleeping forever\n");

    // wait forever
    while (1) {
        sleep(10);
    }
}
