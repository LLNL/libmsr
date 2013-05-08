/* Wgremlin.w
 * 
 */
#include <stddef.h>
#include <unistd.h>

#include "msr_core.h"
#inlcude "msr_rapl.h"

static int rank=-1;
static struct rapl_data rd;

{{fn foo MPI_Init}}
	{{callfn}}
	blr_init_msr();
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank == 0){
		rapl_read_data(0, NULL);
	}
{{endfn}}


{{fn foo MPI_Finalize}}
	sleep(3);
	if(rank == 0){
		rapl_read_data(0, &rd);
		rapl_dump_data( &rd );
	}
	blr_finalize_msr();
	{{callfn}}
{{endfn}}


{{fnall foo MPI_Init MPI_Finalize}} 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	{{callfn}}
{{endfnall}}
