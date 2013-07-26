/* FILE: signalPower.c
 *
 * Author: Kathleen Shoga
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "signalPower.h"




void printData(int i)
{
	struct itimerval tout_val;
	
	signal(SIGALRM, printData);

	struct rapl_data r,rr;

	rapl_read_data(0, &r);
	rapl_read_data(1, &rr);
	printf("QQQ 0  dram_watts= %8.4lf ", r.dram_watts);
	rapl_dump_data(&r);
	printf("QQQ 1  dram_watts= %8.4lf ", rr.dram_watts);
	rapl_dump_data(&rr);

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0 ;
	tout_val.it_value.tv_usec = 50000;
	
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
