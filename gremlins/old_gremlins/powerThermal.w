#include <stdio.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>

#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "msr_misc.h"
#include "msr_turbo.h"
#include "signalCombined.h"

static struct itimerval tout_val;
static int rank;
static int size;

static char hname[1025];
static struct rapl_limit P0_1, P0_2, P0_DRAM, P1_1, P1_2, P1_DRAM;
static int watts=115; 
static int retVal = -1;
static struct rapl_limit lim; 
static int turboEn=1, retVal2;

int get_env_int(const char *name, int *val);

{{fn foo MPI_Init}}
	{{callfn}}
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if(rank == 0)
	{
		tout_val.it_interval.tv_sec = 0;
        	tout_val.it_interval.tv_usec = 0;
        	tout_val.it_value.tv_sec = 5;
        	tout_val.it_value.tv_usec = 0;

		retVal = get_env_int("POWER_CAP", &watts);
		retVal2 = get_env_int("TURBO_EN", &turboEn);

		if(retVal==-1) {
			fprintf(stdout, "No power cap specified. Using default of 115W per socket\n");
		}


		if(retVal2==-1) {
			fprintf(stdout, "No turbo flag specified. Using default: TURBO IS ENABLED.\n");
		}

		init_msr();	
        
		setitimer(ITIMER_REAL, &tout_val, 0);
	
		gethostname(hname, 1024);
		fprintf(stdout, "Hostname: %s\n", hname);

		if(retVal2==0 && turboEn==0){
			//Disable Turbo Boost First.

			fprintf(stdout,"Disabling Turbo\n");	
			//dump_turbo();

			disable_turbo();

			//fprintf(stdout,"\n\nDump of core registers after Disabling Turbo\n\n\n");	
			//dump_turbo();
		}

		if(retVal ==0){
			fprintf(stdout, "Power cap specified. Setting limit1 on both sockets to %d W for minimal time window\n", watts);
		
			lim.watts = watts;
          		lim.seconds = 0.009766;
                	lim.bits = 0;
          		set_rapl_limit(0, &lim, NULL, NULL);
          		set_rapl_limit(1, &lim, NULL, NULL);
		}
	
		get_rapl_limit(0,&P0_1, &P0_2, &P0_DRAM);
		get_rapl_limit(1, &P1_1, &P1_2, &P1_DRAM);
		
		fprintf(stdout, "PKG0, Limit1\n");
		dump_rapl_limit(&P0_1);

		fprintf(stdout, "PKG1, Limit1\n");
		dump_rapl_limit(&P1_1);

		signal(SIGALRM, printData);
	}
	PMPI_Barrier(MPI_COMM_WORLD);
{{endfn}}


{{fn foo MPI_Finalize}}
	PMPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0)
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


int get_env_int(const char *name, int *val){
 	char *str=NULL;
        str = getenv(name);
        if(str ==NULL){
          *val = -1;
          return -1;
        }
        *val = (int) strtol(str, (char **)NULL, 0);
        return 0;
}


