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

	struct rapl_data r;

	rapl_read_data(0, &r);
	printf("QQQ ");
	rapl_dump_data(&r);

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
