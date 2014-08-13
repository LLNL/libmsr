/* profile.c
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "signalCombined.h"

#ifndef SET_UP
static struct timeval startTime;
static int init = 0;
int stop=0;
#endif



void printData(int i)
{
	signal(SIGALRM, printData);
	struct timeval currentTime;
	if(!init)
	{
		init = 1;
		gettimeofday(&startTime, NULL);
	}
	struct itimerval tout_val;
	
	struct rapl_data r1,r2;
	

	gettimeofday(&currentTime, NULL);
	double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
	fprintf(stdout, "Timestamp: %3.2lf\n", timeStamp);	

	fprintf(stdout, "Core Temperatures: ");
	dump_thermal_terse(stdout);	
	fprintf(stdout, "\n");

	read_rapl_data(0,&r1);
	read_rapl_data(1,&r2);
	fprintf(stdout, "Power Socket/PKG/DRAM: %d %8.4lf %8.4lf \n", 0, r1.pkg_watts, r1.dram_watts);
	fprintf(stdout, "Power Socket/PKG/DRAM: %d %8.4lf %8.4lf \n", 1, r2.pkg_watts, r2.dram_watts);

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 5;
	tout_val.it_value.tv_usec = 0;
	
	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

