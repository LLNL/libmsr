/* FILE: signalCode.c
 *
 * Author: Kathleen Shoga
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "signalCode.h"

#ifndef SET_TIMEOFDAY
static struct timeval startTime;
static int init = 0;
#endif



void printData()
{
	struct timeval currentTime;
	if(!init)
	{
		init = 1;
		gettimeofday(&startTime, NULL);
	}
	struct itimerval tout_val;

	signal(SIGALRM, printData);

	struct therm_stat s;
	int socket, core;
	for (socket = 0 ; socket <NUM_SOCKETS; socket++)
	{
		for(core = 0; core < NUM_CORES_PER_SOCKET; core++)
		{
			get_therm_stat(socket, core, &s);
			dump_core_temp(socket, core, &s);
			gettimeofday(&currentTime, NULL);
			double elapsed = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
			printf(" %.2f\n", elapsed);
		}
	}

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0 ;
	tout_val.it_value.tv_usec = 10;
	
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
