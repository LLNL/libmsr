#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

#define MASK_RANGE(m,n) ((((uint64_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint64_t)(x)&MASK_RANGE((m),(n)))>>(n))

uint64_t pp_policy = 0x5;
struct rapl_limit l1, l2, l3, l4;

void
get_limits()
{
	int i;
    uint64_t pp_result;
    fprintf(stderr, "\nGetting limits...\n");
	for(i=0; i<NUM_SOCKETS; i++){
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
    printf("\n Testing all sockets\n");
    l1.watts = 120;
	l1.seconds = 4;
	l1.bits = 0;
	l2.watts =  155;
	l2.seconds =  6;
	l2.bits = 0;
    l3.watts = 50;
    l3.seconds = 6;
    l3.bits = 0;
    l4.watts = 110;
    l4.seconds = 8;
    l4.bits = 0;
    pp_policy = 31;
    int i;
    for (i = 0; i < NUM_SOCKETS; i++)
    {
        set_pkg_rapl_limit(i, &l1, &l2);
        set_pp_rapl_limit(i, &l4, NULL);
        set_dram_rapl_limit(i, &l3);
        set_pp_rapl_policies(i, &pp_policy, NULL);
    }
    get_limits();
}

void thermal_test(){
	dump_thermal_terse_label(stdout);
	fprintf(stdout, "\n");
	dump_thermal_terse(stdout);
	fprintf(stdout, "\n");

	dump_thermal_verbose_label(stdout);
	fprintf(stdout, "\n");
	dump_thermal_verbose(stdout);
	fprintf(stdout, "\n");
}

void rapl_r_test(struct rapl_data ** rd)
{
	// Initialize two separate state objects and read rapl data into them during overlapping time windows
    struct rapl_data * r1;// = (struct rapl_data *) malloc(sizeof(struct rapl_data));

    fprintf(stdout, "\nNEW\n\n");
    r1 = &((*rd)[0]);
    poll_rapl_data(0, r1);
    dump_rapl_data(r1, stdout);
    sleep(1);


    poll_rapl_data(0, r1);
    dump_rapl_data(r1, stdout);
    sleep(1);
}


// TODO: check if test for oversized bitfield is in place, change that warning to an error
int main(int argc, char** argv)
{
    struct rapl_data * rd = NULL;
    uint64_t * rapl_flags = NULL;
	#ifdef MPI
	MPI_Init(&argc, &argv);
    printf("mpi init done\n");
	#endif

	if(init_msr())
    {
        return -1;
    }
    printf("msr init done\n");
    if (rapl_init(&rd, &rapl_flags))
    {
        return -1;
    }
    printf("init done\n");
	get_limits();
    unsigned i;
    for(i = 0; i < NUM_SOCKETS; i++)
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
    rapl_finalize(&rd);
	finalize_msr();
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
