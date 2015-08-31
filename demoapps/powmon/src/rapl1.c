#include <stdio.h>
#include <stdlib.h>
#include <msr/msr_core.h>
#include <msr/msr_rapl.h>

static struct rapl_data rdat[NUM_SOCKETS];
static struct rapl_limit rlim[NUM_SOCKETS];

void read_rapl_init() {
    init_msr();
    rdat[0].flags = 0;
    rdat[1].flags = 0;
    read_rapl_data(0,NULL);
    read_rapl_data(1,NULL);
}

void read_rapl_energy_and_power(double* ret) {
    // RAPL reads
    read_rapl_data(0,&(rdat[0]));
    read_rapl_data(1,&(rdat[1]));
    get_rapl_limit(0,&(rlim[0]),NULL,NULL);
    get_rapl_limit(1,&(rlim[1]),NULL,NULL);

    ret[0] = rdat[0].pkg_delta_joules;
    ret[1] = rdat[1].pkg_delta_joules;
    ret[2] = rlim[0].watts * rdat[0].elapsed;
    ret[3] = rlim[1].watts * rdat[1].elapsed;
    ret[4] = rdat[0].pkg_delta_joules / rdat[0].elapsed;
    ret[5] = rdat[1].pkg_delta_joules / rdat[1].elapsed;
    ret[6] = rlim[0].watts;
    ret[7] = rlim[1].watts;
}

void set_rapl_power(double s0bound, double s1bound) {
    static struct rapl_limit rlim[2];
    rlim[0].watts = s0bound;
    rlim[1].watts = s1bound;
    rlim[0].bits = 0;
    rlim[1].bits = 0;
    rlim[0].seconds = 1;
    rlim[1].seconds = 1;
    set_rapl_limit(0, &(rlim[0]), &(rlim[1]), NULL);
    set_rapl_limit(1, &(rlim[0]), &(rlim[1]), NULL);
}
