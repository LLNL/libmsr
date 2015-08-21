#include <stdio.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "msr_core.h"
#include "profile.h"

static struct itimerval tout_val;
static int rank;
static int size;

{{fn foo MPI_Init}}
	{{callfn}}
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	fprintf(stdout,"Greetings from rank %d of %d.", rank, size);
	tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = 0;
        tout_val.it_value.tv_usec = 5000;
if(rank==0) {
	init_msr();	
        setitimer(ITIMER_REAL, &tout_val, 0);
        signal(SIGALRM, msr_profile);
}
	
{{endfn}}


{{fn foo MPI_Finalize}}
if(rank == 0) {
	tout_val.it_interval.tv_sec = 0;
       	tout_val.it_interval.tv_usec = 0;
       	tout_val.it_value.tv_sec = 0;
       	tout_val.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tout_val, 0);
	finalize_msr();
}
	{{callfn}}
{{endfn}}

