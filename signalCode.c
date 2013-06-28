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

	signal(SIGALRM, sayhello);

	FILE * fp;
	fp = fopen ("linpack_msrValues.out", "a");
	// write thermal and rapl reads to file 
	fclose(fp);
	//reset the signal
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 1;
	tout_val.it_value.tv_usec = 0;
	
	setitimer(ITIMER_REAL, &tout_val, 0);
}

int main(int argc, char **argv)
{
	int flags, opt, seconds, tfind;
	flags = 0;	

	while ((opt = getopt(argc, argv, "nt:")) != 1)
	{
		switch(opt)
		{
			case 't':
				seconds = atoi(optarg);
				tfind = 1;
				break;
			default: 
				printf("Usage: %s [-t seconds] name\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if(optind >= argc)
	{
		printf("Expected argument after options\n");
		exit(EXIT_FAILURE);
	}

	// Making a new .out file that will erase
	// any old ones
	FILE *fp;
	fp = fopen("linpack_msrValues.out", "w");
	fclose(fp);
	struct itimerval tout_val;
	
	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 0;
	tout_val.it_value.tv_usec = 1;
	setitimer(ITIMER_REAL, &tout_val, 0);

	signal(SIGALRM, saveData);

	return 0;
}
