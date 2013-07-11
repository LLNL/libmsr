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

#ifndef SET_UP
static struct timeval startTime;
static int init = 0;
int stop=0;
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
//	for (socket = 0 ; socket <1; socket++)
	{
		for(core = 0; core < NUM_CORES_PER_SOCKET; core++)
//		for(core = 0; core < 1; core++)
		{
			gettimeofday(&currentTime, NULL);
			double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
			//if and else if put in for debugging purposes
			if(timeStamp > 0 && stop < 1)
			{
		//		printf("in first if ");
				get_therm_stat(socket, core, &s);
				dump_core_temp(socket, core, &s);
				printf(" %.2f\n", timeStamp);
				stop = 1;
			}
			else if (stop == 1) 
			{
		//		printf("third if");
				get_therm_stat(socket, core, &s);
				dump_core_temp(socket, core, &s);
				printf(" %.2f\n", timeStamp);
			}
		}
	}
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0 ;
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
