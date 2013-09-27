#include <stdio.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "signalPower.h"

static struct itimerval tout_val;
static int rank;
static int size;

{{fn foo MPI_Init}}
	{{callfn}}
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0;
        tout_val.it_value.tv_usec = 5000;
if(rank==0) {
	init_msr();	
	struct rapl_limit limit1, limit2;
	rapl_get_limit(0,&limit1,&limit2,NULL); 
	rapl_dump_limit(&limit1);
	rapl_dump_limit(&limit2);
        setitimer(ITIMER_REAL, &tout_val, 0);
        signal(SIGALRM, printData);
}
	
{{endfn}}


{{fn foo MPI_Finalize}}
if(rank == 0) {
	tout_val.it_interval.tv_sec = 0;
       	tout_val.it_interval.tv_usec = 0;
       	tout_val.it_value.tv_sec = 0;
       	tout_val.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tout_val, 0);
	finalize_msr;
}
	{{callfn}}
{{endfn}}

