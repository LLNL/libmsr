#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"


void saveData()
{
	struct itimerval tout_val;

	signal(SIGALRM, saveData);
	
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 1;
	tout_val.it_value.tv_usec = 0;
	
	setitimer(ITIMER_REAL, &tout_val, 0);
}

int main()
{
	struct itimerval tout_val;
	
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0;
	tout_val.it_value.tv_usec = 1;
	setitimer(ITIMER_REAL, &tout_val, 0);

	signal(SIGALRM, saveData);

	return 0;
}
