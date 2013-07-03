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

{{fn foo MPI_Init}}
	struct itimerval tout_val;

        tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0;
        tout_val.it_value.tv_usec = 10;

	{{callfn}}
	init_msr();	
        setitimer(ITIMER_REAL, &tout_val, 0);
        signal(SIGALRM, printData);
{{endfn}}


{{fn foo MPI_Finalize}}
	struct itimerval tout_val;
	
        tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0;
        tout_val.it_value.tv_usec = 0;
	{{callfn}}
	setitimer(ITIMER_REAL, &tout_val, 0);
	finalize_msr();
{{endfn}}

