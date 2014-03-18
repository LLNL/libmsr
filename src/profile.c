#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "profile.h"

void msr_profile(int i)
{
	static struct timeval startTime;
	struct timeval currentTime;
	static int init = 0;
	static struct itimerval tout_val;
	int socket;
	uint64_t APERF0, MPERF0, APERF1, MPERF1;

	if(!init) {
		signal(SIGALRM, printData);
		tout_val.it_interval.tv_sec = 0;
		tout_val.it_interval.tv_usec = 0;
		tout_val.it_value.tv_sec = 0;
		tout_val.it_value.tv_usec = 100000;
		init = 1;
		gettimeofday(&startTime, NULL);
	}
	
	gettimeofday(&currentTime, NULL);
	fprintf(stdout, "QQQ %lf ", (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0);
	for (socket = 0 ; socket < NUM_SOCKETS; socket++) {
		dump_clocks_terse(package);
	}

	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

