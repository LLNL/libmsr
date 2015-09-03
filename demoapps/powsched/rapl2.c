/* rapl2.c
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

#include <msr/msr_core.h>
#include <msr/msr_rapl.h>
#include <stdio.h>

static struct rapl_data rdat[2];
static uint64_t rflags[2];

static char RINSERT[]="insert into PowerIn values('%s','%f','%f','%f','%f')";

extern char* hostname;
/*
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
*/


void setAllocations(const double aw0, const double aw1) {
    struct rapl_limit rlim[2];
    rlim[0].bits = 0;
    rlim[0].watts = aw0;
    rlim[0].seconds = 1;
    rlim[1].bits = 0;
    rlim[1].watts = aw1;
    rlim[1].seconds = 1;

    set_pkg_rapl_limit(0, &(rlim[0]), &(rlim[1]));
    set_pkg_rapl_limit(1, &(rlim[0]), &(rlim[1]));
}

int myrapl_init() {
    init_msr();
    rflags[0] = 0;
    rflags[1] = 0;
    rapl_init(&rdat, &rflags);
}
