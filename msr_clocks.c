/* msr_clocks.c
 *
 * MPERF, APERF and friends.
 */

#include <stdio.h>
#include "msr_core.h"
#include "msr_clocks.h"


#define MSR_IA32_MPERF 		0x000000e7
#define MSR_IA32_APERF 		0x000000e8
#define IA32_TIME_STAMP_COUNTER 0x00000010

void 
read_aperf(const int socket, uint64_t *aperf){
	read_msr( socket, MSR_IA32_APERF, aperf );
}

void
read_mperf(const int socket, uint64_t *mperf){
	read_msr( socket, MSR_IA32_MPERF, mperf );
}

void 
read_tsc(const int socket, uint64_t *tsc){
	read_msr( socket, IA32_TIME_STAMP_COUNTER, tsc );
}


void 
dump_clocks(const int socket){
	uint64_t val;

	read_aperf(socket, &val);
	fprintf(stdout, "MSR_IA32_APERF.%d= %20lu  ", socket, val);

	read_mperf(socket, &val);
	fprintf(stdout, "MSR_IA32_MPERF.%d= %20lu  ", socket, val);

	read_tsc(socket, &val);
	fprintf(stdout, "TSC.%d= %20lu\n", socket, val);
}

double
get_effective_frequency(const int socket){
	static int init=0;
	static uint64_t previous_mperf[NUM_SOCKETS], previous_aperf[NUM_SOCKETS];
	uint64_t mperf, aperf;
	double ef=0.0;
	read_mperf(socket, &mperf);
	read_aperf(socket, &aperf);
	// FIXME:  document where 2.6GHz figure comes from.
	if(init && (mperf-previous_mperf[socket])){
		ef = ((double)2.6) * ((double)(aperf-previous_aperf[socket])) / ((double)(mperf-previous_mperf[socket]));
	}
	previous_mperf[socket] = mperf;
	previous_aperf[socket] = aperf;
	init=1;
	return ef;
}



