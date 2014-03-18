#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "profile.h"
#include "msr_rapl.h"
#include "msr_clocks.h"
void 
msr_profile() {
	static struct timeval startTime;
	struct timeval currentTime;
	static int init = 0;
	static struct itimerval tout_val;

	if(!init) {
		signal(SIGALRM, msr_profile);
		tout_val.it_interval.tv_sec = 0;
		tout_val.it_interval.tv_usec = 0;
		tout_val.it_value.tv_sec = 0;
		tout_val.it_value.tv_usec = 100000;
		init = 1;
		gettimeofday(&startTime, NULL);

		fprintf(stdout, "QQQ gtod ");
		dump_clocks_terse_label();
		dump_rapl_terse_label();
		fprintf(stdout, "\n");
	}
	
	gettimeofday(&currentTime, NULL);

	fprintf(stdout, "QQQ %lf ", (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0);
	dump_clocks_terse();
	dump_rapl_terse();
	fprintf(stdout, "\n");

	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

