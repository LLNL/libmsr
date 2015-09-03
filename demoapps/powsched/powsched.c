/* powsched.c
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

#include <stdlib.h>
#include <stdio.h>

#include <expose/expose.h>
#include <expose/exposeutil.h>
#include <expose/cachecrack.h>
#include <expose/highres_timer.h>
#include <adts/hashmap.h>

static double q=5.0;
static double bound=180.0;
static double rfact=0.75;

#define CPU_MAX_WATTS 115.0
#define CPU_MIN_WATTS 35.0

struct dpair {
    double s0;
    double s1;
};

struct bpair {
    char s0;
    char s1;
};

static HashMap* C;
static HashMap* A;
static HashMap* M;

#define BUFLEN 1024
static struct _EXPOSE_query consumptionQ;
static char* injectTarget;

static double min(double a, double b) {
    return a<b ? a : b;
}
static double max(double a, double b) {
    return a<b ? b : a;
}

int consumptionReadCallback(char* buf, int len) {
    int rc;
    struct dpair* c;

    cachecrack_t resp;
    cachecrack(&resp, buf, len);
    if(resp.cols != 6 || resp.retcode != 0) {
        printf("consumption query returned bad data\n%s\n",buf);
        return 1;
    }
    for(int i=0;i<resp.rows; i++) {
        char* name = resp.rowdata[i][1];
        rc = hm_get(C, name, (void**)&c);
        if(rc == 0) {
            c = (struct dpair*)malloc(sizeof(struct dpair));
            hm_put(C, name, c, NULL);
        }
        c->s0 = strtod(resp.rowdata[i][2], NULL);
        c->s1 = strtod(resp.rowdata[i][3], NULL);
    }
    return 0;
}

int schedule() {
    int rc,err;

    struct dpair* c;
    struct dpair* a;
    struct bpair* m;

    double allocsum;

    // Read
    EXPOSE_submit_query(&consumptionQ);

    // AllocDown
    long int nodecnt=hm_size(C);
    int numdown = 0;
    char** keys = hm_keyArray(C, &nodecnt);

    allocsum = 0.0;
    // AllocDown via yield
    for(int i=0; i<nodecnt; i++) {
        // get right data
        rc = hm_get(C, keys[i], (void**)&c);
        rc = hm_get(A, keys[i], (void**)&a);
        if(rc == 0) {
            a = (struct dpair*)malloc(sizeof(struct dpair));
            rc = hm_put(A, keys[i], a, NULL);
            a->s0 = 0;
            a->s1 = 0;
        }
        rc = hm_get(M, keys[i], (void**)&m);
        if(rc == 0) {
            m = (struct bpair*)malloc(sizeof(struct bpair));
            rc = hm_put(M, keys[i], m, NULL);
        }

        if(c->s0 < a->s0 - q) {
            a->s0 = max(c->s0+q, CPU_MIN_WATTS);
            m->s0 = 0;
            numdown++;
        } else {
            m->s0 = 1;
        }
        if(c->s1 < a->s1 - q) {
            a->s1 = max(c->s1+q, CPU_MIN_WATTS);
            m->s1 = 0;
            numdown++;
        } else {
            m->s1 = 1;
        }
        allocsum += a->s0 + a->s1;
    }
    printf("allocdown adjusts %d and spends %f of %f\n",numdown,allocsum,bound);

    // AllocDown via take
    if(numdown==0 && allocsum >= bound) {
        allocsum = 0.0;
        for(int i=0; i<nodecnt; i++) {
            rc = hm_get(A, keys[i], (void**)&a);
            rc = hm_get(M, keys[i], (void**)&m);
            if(a->s0 > bound/(nodecnt*2)) {
                a->s0 -= (a->s0 - bound/(nodecnt*2)) * (1-rfact);
                m->s0 = 1;
            }
            if(a->s1 > bound/(nodecnt*2)) {
                a->s1 -= (a->s1 - bound/(nodecnt*2)) * (1-rfact);
                m->s1 = 1;
            }
            allocsum += a->s0 + a->s1;
        }
        printf("power stealing to %f of %f\n",allocsum,bound);
    }
    // AllocUp
    if(numdown==nodecnt*2) {
        // Everybody had too much power
        printf("no alloc up\n");
    } else {
        printf("alloc up\n");
        double u = (bound - allocsum)/(nodecnt*2-numdown);
        if(u<0) {
            // no power for redistribution
            printf("no power for redistribution, no adjustments made\n");
            return 0;
        }
        for(int i=0; i<nodecnt; i++) {
            rc = hm_get(A, keys[i], (void**)&a);
            rc = hm_get(M, keys[i], (void**)&m);
            if(m->s0) {
                a->s0 = min(a->s0+u, CPU_MAX_WATTS);
            }
            if(m->s1) {
                a->s1 = min(a->s1+u, CPU_MAX_WATTS);
            }
        }
    }
    allocsum = 0;
    for(int i=0; i<nodecnt; i++) {
        rc = hm_get(A, keys[i], (void**)&a);
        // Send the update
        snprintf(injectTarget,BUFLEN,"insert into powertargets values ('%s','%f','%f') on duplicate key update",keys[i],a->s0,a->s1);
        EXPOSE_raw_sql(injectTarget);
        allocsum += a->s0 + a->s1;
    }

    // kick the actuators
    printf("kicking poweractuator: power total is %f of %f\n", allocsum,bound);
    EXPOSE_sendkick("poweractuator");
    return 0;
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int err;

    bound = strtod(getenv("POWSCHED_BOUND"),NULL);
    q = strtod(getenv("POWSCHED_THRESHOLD"),NULL);
    rfact = strtod(getenv("POWSCHED_RECLAIM"),NULL);
    printf("POWsched configured with:\n\tbound: %f\n\tq: %f\n\trfact: %f\n",bound, q, rfact);

    // init
    consumptionQ.intervalms = 0;
    consumptionQ.callback = consumptionReadCallback;
    consumptionQ.query_text = "select * from poweractuals";

    C = hm_create(4, 0.0);
    A = hm_create(4, 0.0);
    M = hm_create(4, 0.0);

    injectTarget = (char*)malloc(sizeof(char)*BUFLEN);

    EXPOSE_init();

    // request consumption readings
    EXPOSE_monitor_all("PowerIn",1000);

    // run the timer loop
    int running=1;
    int toggle =0;
    EXPOSE_bcasthaltwait(&running,&toggle);
    struct mstimer timer;
    init_rt_msTimer(&timer, 1000, 0);
    while(running) {
        timer_sleep_rt(&timer);
        schedule();
    }

    EXPOSE_finalize();
    printf("scheduler all done\n");
}

