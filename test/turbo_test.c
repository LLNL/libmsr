#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cpuid.h"
#include "memhdlr.h"
#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "msr_counters.h"
#include "msr_clocks.h"
#include "profile.h"
#include "msr_misc.h"
#include "msr_turbo.h"
#include "csr_core.h"
#include "csr_imc.h"
#include "libmsr_error.h"
#ifdef MPI
#include <mpi.h>
#endif

void turbo_test()
{
    dump_turbo(stdout);

    static uint64_t sockets = 0;
#ifndef IS_ARCH_2D
    int i;
    struct turbo_activation_ratio_data tar;
    struct turbo_limit_data tl, tl2;
#endif

    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }

#ifndef IS_ARCH_2D
    fprintf(stdout, "\n--- MSR_TURBO_ACTIVATION_RATIO ---\n");
    for (i = 0; i < sockets; i++)
    {
        fprintf(stdout, "Socket %d:\n", i);
        get_max_turbo_activation_ratio(i, &tar);
    }

    fprintf(stdout, "\n--- MSR_TURBO_RATIO_LIMIT ---\n");
    for (i = 0; i < sockets; i++)
    {
        fprintf(stdout, "Socket %d:\n", i);
        get_turbo_ratio_limit(i, &tl, &tl2);
    }
#endif

    printf("\n");
}

// TODO: check if test for oversized bitfield is in place, change that warning
// to an error
int main(int argc, char **argv)
{
    struct rapl_data *rd = NULL;
    uint64_t *rapl_flags = NULL;
    uint64_t cores = 0;
    uint64_t threads = 0;
    uint64_t sockets = 0;
    int ri_stat = 0;

    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }

    if (init_msr())
    {
        libmsr_error_handler("Unable to initialize libmsr", LIBMSR_ERROR_MSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    fprintf(stdout, "\n===== MSR Init Done =====\n");

    ri_stat = rapl_init(&rd, &rapl_flags);
    if (ri_stat < 0)
    {
        libmsr_error_handler("Unable to initialize rapl", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    fprintf(stdout, "\n===== RAPL Init Done =====\n");

    fprintf(stdout, "\n===== Turbo Test =====\n");
    turbo_test();

    finalize_msr();
    fprintf(stdout, "===== MSR Finalized =====\n");

    fprintf(stdout, "\n===== Test Finished Successfully =====\n");

    return 0;
}
