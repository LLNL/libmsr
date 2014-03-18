#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "profile.h"

void 
msr_profile() {
	static struct timeval startTime;
	struct timeval currentTime;
	static int init = 0;
	static struct itimerval tout_val;
	int socket;

	if(!init) {
		signal(SIGALRM, msr_profile);
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
		dump_clocks_terse(socket);
	}

	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

