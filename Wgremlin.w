/* Wgremlin.w
 * 
 */
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

#include "msr_core.h"
#include "msr_rapl.h"

static int rank=-1;
static struct rapl_data rd;
static struct rapl_limit original_limit1, limit1;
static struct rapl_limit original_limit2, limit2;
static struct rapl_limit env_limit1;
static struct rapl_limit env_limit2;

{{fn foo MPI_Init}}
	double pkg_power_limit1=0.0;
	double pkg_power_limit2=0.0;
	double pkg_time_window1=0.0;
	double pkg_time_window2=0.0;

	char *pkg_power_limit1_str;
	char *pkg_power_limit2_str;
	char *pkg_time_window1_str;
	char *pkg_time_window2_str;

	{{callfn}}
	PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
	blr_init_msr();

	pkg_power_limit1_str = getenv("GLOBAL_PKG_POWER_LIMIT1"); 
	if(pkg_power_limit1_str){
		pkg_power_limit1 = strtod(pkg_power_limit1_str, NULL);
		limit1.watts     = pkg_power_limit1;
	}

	pkg_power_limit2_str = getenv("GLOBAL_PKG_POWER_LIMIT2"); 
	if(pkg_power_limit2_str){
		pkg_power_limit2 = strtod(pkg_power_limit2_str, NULL);
		limit2.watts     = pkg_power_limit2;
	}

	pkg_time_window1_str = getenv("GLOBAL_PKG_TIME_WINDOW1");
	if(pkg_time_window1_str){
		pkg_time_window1 = strtod(pkg_time_window1_str, NULL);
	        limit1.seconds   = pkg_time_window1;
	}

	pkg_time_window2_str = getenv("GLOBAL_PKG_TIME_WINDOW2");
	if(pkg_time_window2_str){
		pkg_time_window2 = strtod(pkg_time_window2_str, NULL);
		limit2.seconds   = pkg_time_window2;
	}


	// Assume 8 ranks per socket and 2 sockets per node.
		
	if(rank%16==0){	
		rapl_read_data(0, &rd);
		rapl_get_limit(0, &original_limit1, &original_limit2, NULL);	
		if( pkg_power_limit1_str && pkg_time_window1_str ){
			rapl_set_limit(0, &limit1, NULL, NULL);
		}
		if( pkg_power_limit2_str && pkg_time_window2_str ){
			rapl_set_limit(0, NULL, &limit2, NULL);
		}
	}
	if((rank+8)%16==0){
		rapl_read_data(1, &rd);
		rapl_get_limit(1, &original_limit1, &original_limit2, NULL);	
		if( pkg_power_limit1_str && pkg_time_window1_str ){
			rapl_set_limit(1, &limit1, NULL, NULL);
		}
		if( pkg_power_limit2_str && pkg_time_window2_str ){
			rapl_set_limit(1, NULL, &limit2, NULL);
		}
	}
{{endfn}}


{{fn foo MPI_Finalize}}
	if(rank % 16 == 0){
		rapl_read_data(0, &rd);
		rapl_dump_data( &rd );
		rapl_set_limit(0, &original_limit1, &original_limit2, NULL);	
	}
	if( (rank+8) % 16 == 0){
		rapl_read_data(1, &rd);
		rapl_dump_data( &rd );
		rapl_set_limit(1, &original_limit1, &original_limit2, NULL);	
	}
	blr_finalize_msr();
	{{callfn}}
{{endfn}}


{{fnall foo MPI_Init MPI_Finalize}} 
	if(rank % 16 == 0){
		rapl_read_data(0, &rd);
		rapl_dump_data( &rd );
	}
	if( (rank+8) % 16 == 0){
		rapl_read_data(1, &rd);
		rapl_dump_data( &rd );
	}
	{{callfn}}
{{endfnall}}

