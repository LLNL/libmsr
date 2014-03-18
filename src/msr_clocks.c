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
read_aperf(int socket, uint64_t *aperf){
	read_msr( socket, MSR_IA32_APERF, aperf );
}

void
read_mperf(int socket, uint64_t *mperf){
	read_msr( socket, MSR_IA32_MPERF, mperf );
}

void 
read_tsc(int socket, uint64_t *tsc){
	read_msr( socket, IA32_TIME_STAMP_COUNTER, tsc );
}

void
dump_clocks_terse_label(){
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++){
		fprintf(stdout, "aperf%02d mperf%02d tsc%02d ", 
			socket, socket, socket);
	}
}

void
dump_clocks_terse(){
	uint64_t aperf_val, mperf_val, tsc_val;
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++){
		read_aperf(socket, &aperf_val);
		read_mperf(socket, &mperf_val);
		read_tsc  (socket, &tsc_val);
		fprintf(stdout, "%20lu %20lu %20lu ", 
			aperf_val, mperf_val, tsc_val);
	}
}

void 
dump_clocks(){
	int socket;
	uint64_t val = 10101010101;

	for( socket=0; socket<NUM_SOCKETS; socket++){
		read_aperf(socket, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "MSR_IA32_APERF\n");

	for( socket=0; socket<NUM_SOCKETS; socket++){
		read_mperf(socket, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "MSR_IA32_MPERF\n");

	for( socket=0; socket<NUM_SOCKETS; socket++){
		read_tsc(socket, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "TSC\n");

}

double
get_effective_frequency(int socket){
	static int init=0;
	static uint64_t previous_mperf[NUM_SOCKETS], previous_aperf[NUM_SOCKETS];
	uint64_t mperf, aperf;
	double ef=0.0;
	read_mperf(socket, &mperf);
	read_aperf(socket, &aperf);
	if(init && (mperf-previous_mperf[socket])){
		ef = ((double)2.601) * ((double)(aperf-previous_aperf[socket])) / ((double)(mperf-previous_mperf[socket]));
	}
	previous_mperf[socket] = mperf;
	previous_aperf[socket] = aperf;
	init=1;
	return ef;
}



