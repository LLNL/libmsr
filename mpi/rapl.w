#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_opt.h"
#include "blr_util.h"

static int rank;
static char hostname[1025];
extern int msr_debug;
static FILE* f;
struct rapl_state_s *s;
char filetag[2048];
int retVal = -1; 

static int msr_rank_mod=-1;

{{fn foo MPI_Init}}
	{{callfn}}
	rank = -1;
	PMPI_Comm_rank( MPI_COMM_WORLD, &rank );
	retVal = get_env_int("MSR_RANK_MOD", &msr_rank_mod);

	if(retVal == -1){
		printf("Error: To run an MPI program, the MSR_RANK_MOD environment variable should be set.\n"); 
		exit(EXIT_FAILURE);
	}

	if(rank%msr_rank_mod == 0){
		gethostname(hostname, 1024 );
		//TP
		sprintf(filetag, "%s_rapl_%d", hostname, rank); 
		//TP
		register_sig();
//		disable_turbo(0);
//		disable_turbo(1);
		//TP
		s=rapl_init(filetag); 
	}
{{endfn}}

{{fn foo MPI_Finalize}}
	double elapsed;
	if(rank%msr_rank_mod == 0){
		rapl_finalize(s, 1);
	}
	{{callfn}}
{{endfn}}
