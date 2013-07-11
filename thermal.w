#include <stdio.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "signalCode.h"

static struct itimerval tout_val;

{{fn foo MPI_Init}}
	{{callfn}}
	int taskid;
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	if(taskid == 0)
	{
	tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0;
        tout_val.it_value.tv_usec = 100000;
		init_msr();	
        	setitimer(ITIMER_REAL, &tout_val, 0);
        	signal(SIGALRM, printData);
	}
{{endfn}}


{{fn foo MPI_Finalize}}
	int taskid;
        MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	if(taskid == 0)
	{
		tout_val.it_interval.tv_sec = 0;
        	tout_val.it_interval.tv_usec = 0;
        	tout_val.it_value.tv_sec = 0;
        	tout_val.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tout_val, 0);
		finalize_msr;
	}
	{{callfn}}
{{endfn}}

