
#include <msr/msr_core.h>
#include <msr/msr_rapl.h>

void setAllocations(const double aw0, const double aw1) {
    struct rapl_limit rlim[2];
    rlim[0].bits = 0;
    rlim[0].watts = aw0;
    rlim[0].seconds = 1;
    rlim[1].bits = 0;
    rlim[1].watts = aw1;
    rlim[1].seconds = 1;
    set_rapl_limit(0,&rlim[0],NULL,NULL);
    set_rapl_limit(1,&rlim[1],NULL,NULL);
}

int myrapl_init() {
    return init_msr();
}
