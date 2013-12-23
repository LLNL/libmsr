/*
 * Copyright (c) 2013, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov.
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
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
		ef = ((double)2.601) * ((double)(aperf-previous_aperf[package])) / ((double)(mperf-previous_mperf[package]));
	}
	previous_mperf[package] = mperf;
	previous_aperf[package] = aperf;
	init=1;
	return ef;
}



