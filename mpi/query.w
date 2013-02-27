/****************************************************************************************************/
/****************************************************************************************************/
/* end2end.c automatically created by end2end.w							    */
/****************************************************************************************************/
/****************************************************************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include "msr_core.h"
#include "msr_rapl.h"
#include "blr_util.h"

static struct power_units units;
static struct power_info info[NUM_DOMAINS];
static double joules[NUM_DOMAINS];
static struct power_limit limit[NUM_DOMAINS];
static struct timeval start, finish;
static double status_sec;
static uint64_t policy;
static int rank;
static char hostname[1025];
extern int msr_debug;

{{fn foo MPI_Init}}
	gethostname( hostname, 1024 );
	{{callfn}}
	rank = -1;
	PMPI_Comm_rank( MPI_COMM_WORLD, &rank );
	usleep(1000*rank);
	fprintf(stderr, "rank %3d on %10s\n", rank, hostname);

	//void get_power_info( 	int cpu, int domain,    struct power_info *info,        struct power_units *units);	PKG	DRAM
	//void get_power_limit(	int cpu, int domain,    struct power_limit *limit,      struct power_units *units);	PKG	DRAM	PP0	PP1
	//void get_perf_status(	int cpu, int domain,    double *pstatus_sec,            struct power_units *units);	PKG	DRAM
	//void get_policy(     	int cpu, int domain,    uint64_t *ppolicy                                        );			PP0	PP1

	sleep(rank);
	init_msr();
	disable_turbo(0);
	disable_turbo(1);
	fprintf(stderr, ">>> hostname=%10s\n", hostname);
	msr_debug=1;
	fprintf(stderr, "\nALL_DOMAINS get_rapl_power_unit\n");
	get_rapl_power_unit(		0, 					&units);
	fprintf(stderr, "\nPKG_DOMAIN  get_power_info\n");
	get_power_info( 		0, PKG_DOMAIN, 	&info[PKG_DOMAIN], 	&units);
	fprintf(stderr, "\nDRAM_DOMAIN get_power_info\n");
	get_power_info( 		0, DRAM_DOMAIN,	&info[PKG_DOMAIN], 	&units);

	fprintf(stderr, "\nPKG_DOMAIN  get_power_limit\n");
	get_power_limit( 		0, PKG_DOMAIN, 	&limit[PKG_DOMAIN], 	&units);
	fprintf(stderr, "\nPP0_DOMAIN  get_power_limit\n");
	get_power_limit( 		0, PP0_DOMAIN, 	&limit[PKG_DOMAIN], 	&units);
	fprintf(stderr, "\nDRAM_DOMAIN get_power_limit\n");
	get_power_limit( 		0, DRAM_DOMAIN, &limit[PKG_DOMAIN], 	&units);

	fprintf(stderr, "\nPKG_DOMAIN  get_perf_status\n");
	get_perf_status(		0, PKG_DOMAIN,  &status_sec,		&units);
	fprintf(stderr, "\nDRAM_DOMAIN get_perf_status\n");
	get_perf_status(		0, DRAM_DOMAIN,  &status_sec,		&units);

	fprintf(stderr, "\nPP0_DOMAIN  get_policy\n");
	get_policy(			0, PP0_DOMAIN, 	&policy			      );
	msr_debug=0;
	fprintf(stderr, "<<< hostname=%10s\n", hostname);
{{endfn}}

{{fn foo MPI_Finalize}}
	{{callfn}}
{{endfn}}
