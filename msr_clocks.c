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
read_aperf(int package, uint64_t *aperf){
	read_msr( package, MSR_IA32_APERF, aperf );
}

void
read_mperf(int package, uint64_t *mperf){
	read_msr( package, MSR_IA32_MPERF, mperf );
}

void 
read_tsc(int package, uint64_t *tsc){
	read_msr( package, IA32_TIME_STAMP_COUNTER, tsc );
}


void 
dump_clocks(){
	int package;
	uint64_t val = 99;

	for( package=0; package<NUM_PACKAGES; package++){
		read_aperf(package, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "MSR_IA32_APERF\n");

	for( package=0; package<NUM_PACKAGES; package++){
		read_mperf(package, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "MSR_IA32_MPERF\n");

	for( package=0; package<NUM_PACKAGES; package++){
		read_tsc(package, &val);
		fprintf(stdout, "%20lu ", val);
	}
	fprintf(stdout, "TSC\n");
}

double
get_effective_frequency(int package){
	static int init=0;
	static uint64_t previous_mperf[NUM_PACKAGES], previous_aperf[NUM_PACKAGES];
	uint64_t mperf, aperf;
	double ef=0.0;
	read_mperf(package, &mperf);
	read_aperf(package, &aperf);
	if(init && (mperf-previous_mperf[package])){
		ef = ((double)2.6) * ((double)(aperf-previous_aperf[package])) / ((double)(mperf-previous_mperf[package]));
	}
	previous_mperf[package] = mperf;
	previous_aperf[package] = aperf;
	init=1;
	return ef;
}



