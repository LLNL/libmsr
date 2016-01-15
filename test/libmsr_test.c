#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "../include/cpuid.h"
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#include "../include/msr_counters.h"
#include "../include/msr_clocks.h"
#include "../include/profile.h"
#include "../include/msr_misc.h"
#include "../include/msr_turbo.h"
#include "../include/csr_core.h"
#include "../include/csr_imc.h"
#ifdef MPI
#include <mpi.h>
#endif

#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

uint64_t pp_policy = 0x5;
struct rapl_limit l1, l2, l3, l4;

int VALIDATE = 0;

void
get_limits()
{
	int i;
    uint64_t pp_result;
    static uint64_t sockets = 0;
    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    fprintf(stderr, "\nGetting limits...\n");
	for(i=0; i<sockets; i++){
        fprintf(stdout, "\nSocket %d:\n", i);
        printf("PKG\n");
        get_pkg_rapl_limit(i, &l1, &l2);
		dump_rapl_limit(&l1, stdout);
		dump_rapl_limit(&l2, stdout);
        printf("DRAM\n");
        get_dram_rapl_limit(i, &l3);
        dump_rapl_limit(&l3, stdout);
        printf("PP\n");
        get_pp_rapl_limit(i, &l4, NULL);
        dump_rapl_limit(&l4, stdout);
        get_pp_rapl_policies(i, &pp_result, NULL);
        printf("PP policy\n%ld\n", pp_result);
	}
}

void test_pkg_lower_limit(unsigned s)
{
    printf("\n Testing pkg %u lower limit\n", s);
    l1.watts = 95;
    l1.seconds = 1;
    l1.bits = 0;
    set_pkg_rapl_limit(s, &l1, NULL);
    get_limits();
}

void test_pkg_upper_limit(unsigned s)
{
    printf("\n Testing pkg %u upper limit\n", s);
    l2.watts = 120;
    l2.seconds = 9;
    l2.bits = 0;
    set_pkg_rapl_limit(s, NULL, &l2);
    get_limits();
}

void test_socket_1_limits(unsigned s)
{
    printf("\n Testing socket %u limits with new values\n", s);
    l1.watts = 100;
	l1.seconds = 2;
	l1.bits = 0;
	l2.watts =  180;
	l2.seconds =  3;
	l2.bits = 0;
    set_pkg_rapl_limit(s, &l1, &l2);
    l3.watts = 25;
    l3.seconds = 2;
    l3.bits = 0;
    set_dram_rapl_limit(s, &l3);
    l4.watts = 115;
    l4.seconds = 1;
    l4.bits = 0;
    set_pp_rapl_limit(s, &l4, NULL);
    pp_policy = 8;
    set_pp_rapl_policies(1, &pp_policy, NULL);
    get_limits();
}

void test_socket_0_limits(unsigned s)
{
    printf("\n Testing socket %u limits\n", s);
    l1.watts = 110;
	l1.seconds = 1;
	l1.bits = 0;
	l2.watts =  135;
	l2.seconds =  5;
	l2.bits = 0;
    set_pkg_rapl_limit(s, &l1, &l2);
    l3.watts = 35;
    l3.seconds = 1;
    l3.bits = 0;
    set_dram_rapl_limit(s, &l3);
    l4.watts = 132;
    l4.seconds = 2;
    l4.bits = 0;
    set_pp_rapl_limit(s, &l4, NULL);
    pp_policy = 1;
    set_pp_rapl_policies(0, &pp_policy, NULL);
    get_limits();
}

void test_all_limits()
{
    static uint64_t sockets = 0;
    if (!sockets)
    {
        core_config(NULL, NULL, &sockets, NULL);
    }
    printf("\n Testing all sockets\n");
    l1.watts = 50;
	l1.seconds = 1;
	l1.bits = 0;
	l2.watts =  180;
	l2.seconds =  3;
	l2.bits = 0;
    l3.watts = 40;
    l3.seconds = 1;
    l3.bits = 0;
    l4.watts = 110;
    l4.seconds = 8;
    l4.bits = 0;
    pp_policy = 31;
    int i;
    for (i = 0; i < sockets; i++)
    {
        set_pkg_rapl_limit(i, &l1, &l2);
        set_pp_rapl_limit(i, &l4, NULL);
        set_dram_rapl_limit(i, &l3);
        set_pp_rapl_policies(i, &pp_policy, NULL);
    }
    get_limits();
}

// TODO: test other parts of thermal
void thermal_test(){
	//dump_thermal_verbose_label(stdout);
	//fprintf(stdout, "\n");
	//dump_thermal_verbose(stdout);
	//fprintf(stdout, "\n");
    dump_therm_reading(stdout);
}

void counters_test()
{
    dump_fixed_readable(stdout);
    fprintf(stdout, "\n");
    set_all_pmc_ctrl(0x0, 0x67, 0x00, 0xC4, 1);
    set_all_pmc_ctrl(0x0, 0x67, 0x00, 0xC4, 2);
    enable_pmc();
    dump_pmc_readable(stdout);
    fprintf(stdout, "\n");
}

// TODO: test other parts of clocks
void clocks_test()
{
    dump_clocks_readable(stdout);
    fprintf(stdout, "\n");
}

void misc_test()
{
    struct misc_enable s;
    uint64_t sockets = 0;
    core_config(NULL, NULL, &sockets, NULL);
    int i;
    for (i = 0; i < sockets; i++)
    {
        get_misc_enable(i, &s);
        dump_misc_enable(&s);
    }
}

void turbo_test()
{
    dump_turbo(stdout);
}

// NOTE: to use this, compile a NAS parallel benchmark of your choice and modify the path below
//       you will have to compile with the -D_GNU_SOURCE flag for setaffinity 
//#define MEMTEST 1

#ifdef MEMTEST
char * args[] = {"mg.B.1"};
const char path[] = "/g/g19/walker91/NPB3.3.1/NPB3.3-MPI/bin/mg.B.1";
#endif
//#ifdef PROCTEST
//char *args[] = {"ep.B.1"};
//const char path[] = "/g/g19/walker91/NPB3.3.1/NPB3.3-MPI/bin/ep.B.1";
//#endif

// We use 24 for Catalyst, (2 sockets * 12 cores)
#define NPROCS 24 

void rapl_r_test(struct rapl_data ** rd)
{

    fprintf(stdout, "\nNEW\n\n");
    //r1 = &((*rd)[0]);
    //r2 = &((*rd)[1]);
    poll_rapl_data();
    //poll_rapl_data(0, NULL);
    //poll_rapl_data(1, NULL);
    dump_rapl_data(stdout);

#ifdef MEMTEST
   unsigned nprocs = NPROCS;
   pid_t pid[NPROCS];
   int status[NPROCS];
   cpu_set_t cpuselect;

   int i;
   for (i = 0; i < nprocs; i++)
   {   
       CPU_ZERO(&cpuselect);
       CPU_SET(i, &cpuselect);
       pid[i] = fork();
       if (pid[i] == 0)
       {   
           // this is just testing on 1 node 
           sched_setaffinity(0, sizeof(cpu_set_t), &cpuselect);
           fprintf(stderr, "executing stress test\n");
           execve(path, args, NULL);
           exit(1);
       }
    }
    fprintf(stderr, "waiting for test to complete\n");
    for (i = 0; i < nprocs; i++)
    {   
        wait(&status[i]);
    }
#endif
#ifndef MEMTEST
    sleep(10);
#endif


    poll_rapl_data();
    dump_rapl_data(stdout);
}

#define PT_INC 1500

int repeated_poll_test()
{
    usleep(1000);
    poll_rapl_data();
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    dump_rapl_data(stdout);
    usleep(PT_INC);
    poll_rapl_data();
    dump_rapl_data(stdout);
    return 0;
}

void set_to_defaults()
{
    int socket = 0;
    int numsockets = num_sockets();
    struct rapl_power_info raplinfo;
    struct rapl_limit socketlim, socketlim2, dramlim;
    fprintf(stdout, "Setting Defaults\n");
    for (socket = 0; socket < numsockets; socket++)
    {
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
        dump_rapl_limit(&socketlim, stdout);
        dump_rapl_limit(&socketlim2, stdout);
        dump_rapl_limit(&dramlim, stdout);
        set_pkg_rapl_limit(socket, &socketlim, &socketlim2);
        set_dram_rapl_limit(socket, &dramlim);
    }
}


// TODO: check if test for oversized bitfield is in place, change that warning to an error
int main(int argc, char** argv)
{
    struct rapl_data * rd = NULL;
    uint64_t * rapl_flags = NULL;
    uint64_t cores = 0, threads = 0, sockets = 0;
    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }
	#ifdef MPI
	MPI_Init(&argc, &argv);
    printf("mpi init done\n");
	#endif

#ifdef SKIPMSR
goto csrpart;
#endif

	if(init_msr())
    {
        fprintf(stderr, "ERROR: Unable to initialize libmsr\n");
        return -1;
    }
    printf("msr init done\n");
    int ri_stat = 0;
    ri_stat = rapl_init(&rd, &rapl_flags);
    if (ri_stat < 0)
    {
        fprintf(stderr, "ERROR: Unable to initialize rapl\n");
        return -1;
    }
    printf("init done\n");
    print_available_rapl();
    print_available_counters();
    enable_fixed_counters();
	get_limits();
    unsigned i;
    for(i = 0; i < sockets; i++)
    {
        fprintf(stdout, "BEGINNING SOCKET %u TEST\n", i);
	    test_pkg_lower_limit(i);
	    test_pkg_upper_limit(i);
	    test_socket_0_limits(i);
	    test_socket_1_limits(i);
        fprintf(stdout, "FINISHED SOCKET %u TEST\n", i);
    }
    fprintf(stdout, "TESTING ALL SETTINGS\n");
    test_all_limits();
    printf("set limits done\n");
	rapl_r_test(&rd);
    printf("rapl_r_test done\n");
    printf("\n\nPOWER INFO\n");
    dump_rapl_power_info(stdout);
    printf("\nEND POWER INFO\n\n");
    printf("thermal test\n");
    thermal_test();

    printf("clocks test\n");
    clocks_test();
    printf("counters test\n");
    counters_test();
    printf("turbo test\n");
    turbo_test();
    printf("misc test\n");
    misc_test();

    repeated_poll_test();
    set_to_defaults();

	finalize_msr();
	#ifdef MPI

csrpart:
	MPI_Finalize();
	#endif
    fprintf(stdout, "Test Finished Successfully\n");
    if (ri_stat)
    {
        fprintf(stdout, "Found %d locked rapl register(s)\n", ri_stat);
    }

	return 0;
}
