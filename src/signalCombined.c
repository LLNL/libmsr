/* FILE: signalCombined.c
 *
 * Author: Kathleen Shoga
 *
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
	
	struct therm_stat s;
	struct rapl_data r1,r2;
	int socket, core;
	for (socket = 0 ; socket <NUM_SOCKETS; socket++)
	{
		for(core = 0; core < NUM_CORES_PER_SOCKET; core++)
		{
			gettimeofday(&currentTime, NULL);
			double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
			get_therm_stat(socket, core, &s);
			dump_core_temp(socket, core, &s);
			printf(" %.3f NA NA\n", timeStamp);
		}
	}

	gettimeofday(&currentTime, NULL);
	double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
	rapl_read_data(0,&r1);
	rapl_read_data(1,&r2);
	uint64_t APERF0, MPERF0, APERF1, MPERF1;
	read_aperf(0, &APERF0);
	read_mperf(0, &MPERF0);
	read_aperf(1, &APERF1);
	read_mperf(1, &MPERF1);

	printf("RRR NA 0 NA %.3f %8.4lf %8.4lf %20lu %20lu\n", timeStamp, r1.pkg_watts, r1.dram_watts, APERF0, MPERF0);
	printf("RRR NA 1 NA %.3f %8.4lf %8.4lf %20lu %20lu\n", timeStamp, r2.pkg_watts, r2.dram_watts, APERF1, MPERF1);


	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0;
	tout_val.it_value.tv_usec = 100000;
	
	setitimer(ITIMER_REAL, &tout_val, 0);
	
}
/*
int main()
{
	struct itimerval tout_val;
	
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0;
	tout_val.it_value.tv_usec = 10;
	setitimer(ITIMER_REAL, &tout_val, 0);

	signal(SIGALRM, printData);

	return 0;
}*/
