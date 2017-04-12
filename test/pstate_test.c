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
#include "msr_misc.h"
#include "msr_turbo.h"
#include "csr_core.h"
#include "csr_imc.h"
#include "libmsr_error.h"
#ifdef MPI
#include <mpi.h>
#endif

struct rapl_limit l1, l2, l3, l4;

void get_limits()
{
    int i;
    uint64_t pp_result;
    static uint64_t sockets = 0;

    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    for (i = 0; i < sockets; i++)
    {
        if (i != 0)
        {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "Socket %d:\n", i);
        if (get_pkg_rapl_limit(i, &l1, &l2) == 0)
        {
            fprintf(stdout, "Pkg Domain Power Lim 1 (lower lim)\n");
            dump_rapl_limit(&l1, stdout);
            fprintf(stdout, "\n");
            fprintf(stdout, "Pkg Domain Power Lim 2 (upper lim)\n");
            dump_rapl_limit(&l2, stdout);
        }
        if (get_dram_rapl_limit(i, &l3) == 0)
        {
            fprintf(stdout, "\nDRAM Domain\n");
            dump_rapl_limit(&l3, stdout);
        }
        if (get_pp_rapl_limit(i, &l4, NULL) == 0)
        {
            fprintf(stdout, "\nPP Domain\n");
            dump_rapl_limit(&l4, stdout);
        }
        if (get_pp_rapl_policies(i, &pp_result, NULL) == 0)
        {
            fprintf(stdout, "\nPP Policy\n%ld\n", pp_result);
        }
    }
}

// TODO: test other parts of clocks
void clocks_test(double *freq)
{
    fprintf(stdout, "\n--- Read IA32_APERF, IA32_MPERF, and IA32_TIME_STAMP_COUNTER ---\n");
    dump_clocks_data_readable(stdout);

    fprintf(stdout, "\n--- Reading IA32_PERF_STATUS ---\n");
    dump_p_state(stdout);
    fprintf(stdout, "\n");

    fprintf(stdout, "--- Set New CPU Frequencies ---\n");
    fprintf(stdout, "- Setting socket 0 to %.1fGHz\n", freq[0]);
    unsigned long long speedstep = (freq[0]*10)*256;
    printf("    WRITING dec=%llu hex=0x%llx\n", speedstep, speedstep);
    set_p_state(0, (uint64_t)speedstep);

    fprintf(stdout, "- Setting socket 1 to %.1fGHz\n", freq[1]);
    speedstep = (freq[1]*10)*256;
    printf("    WRITING dec=%llu hex=0x%llx\n", speedstep, speedstep);
    set_p_state(1, (uint64_t)speedstep);

    fprintf(stdout, "\n--- Confirm New Settings ---\n");
    dump_p_state(stdout);
}

void set_to_defaults()
{
    int socket = 0;
    int numsockets = num_sockets();
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;

    for (socket = 0; socket < numsockets; socket++)
    {
        if (socket != 0)
        {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "Socket %d:\n", socket);
        get_rapl_power_info(socket, &raplinfo);
        socketlim.bits = 0;
        socketlim.watts = raplinfo.pkg_therm_power;
        socketlim.seconds = 1;
        socketlim2.bits = 0;
        socketlim2.watts = raplinfo.pkg_therm_power * 1.2;
        socketlim2.seconds = 3;
        dramlim.bits = 0;
        dramlim.watts = raplinfo.dram_max_power;
        dramlim.seconds = 1;
        fprintf(stdout, "Pkg Domain Power Lim 1 (lower lim)\n");
        dump_rapl_limit(&socketlim, stdout);
        fprintf(stdout, "\n");
        fprintf(stdout, "Pkg Domain Power Lim 2 (upper lim)\n");
        dump_rapl_limit(&socketlim2, stdout);
        fprintf(stdout, "\nDRAM Domain\n");
        dump_rapl_limit(&dramlim, stdout);
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
        set_dram_rapl_limit(socket, &dramlim);
    }
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
    double *new_p_states_ghz;

    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }
    new_p_states_ghz = malloc(sockets * sizeof(double));

    if (argc == 1)
    {
        /* Default p-states to write. */
        new_p_states_ghz[0] = 1.8;
        new_p_states_ghz[1] = 1.2;
    }
    else if (argc-1 > sockets)
    {
        fprintf(stderr, "ERROR: Too many p-states (in GHz) specified.\n");
        return -1;
    }
    else if (argc-1 == sockets)
    {
        new_p_states_ghz[0] = atof(argv[1]);
        new_p_states_ghz[1] = atof(argv[2]);
    }
    else
    {
        fprintf(stderr, "ERROR: Not enough p-states (in GHz) specified.\n");
        return -1;
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

    fprintf(stdout, "\n===== Get Initial RAPL Power Limits =====\n");
    get_limits();

    fprintf(stdout, "\n===== POWER INFO =====\n");
    dump_rapl_power_info(stdout);

    fprintf(stdout, "\n===== Clocks Test =====\n");
    clocks_test(new_p_states_ghz);

    fprintf(stdout, "\n===== Setting Defaults =====\n");
    set_to_defaults();

    finalize_msr();
    fprintf(stdout, "===== MSR Finalized =====\n");

    fprintf(stdout, "\n===== Test Finished Successfully =====\n");

    return 0;
}
