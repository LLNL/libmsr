#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

struct rapl_limit l1, l2, l3;

void 
rapl_test(){
	read_rapl_data(0, NULL);	// Initialize
	sleep(3);

	dump_rapl_terse_label();
	fprintf(stdout, "\n");
	dump_rapl_terse();		// Read and dump.
	fprintf(stdout, "\n");

}

void
get_limits(){
	int i;
	for(i=0; i<NUM_SOCKETS; i++){
		fprintf(stderr, "%d\n", i);
		get_rapl_limit(i, &l1, &l2, NULL);
		dump_rapl_limit(&l1);
		dump_rapl_limit(&l2);
	}
	/*dump_rapl_limit(&l3);*/
}

void
set_limits(){
	l1.watts = 55;
	l1.seconds = 0.1;
	l1.bits = 0;
	l2.watts = 65;
	l2.watts = 0.2;
	l2.bits = 0;
	set_rapl_limit(0, &l1, &l2, NULL);
	get_limits();
}

void thermal_test(){
	dump_thermal_terse_label();
	fprintf(stdout, "\n");
	dump_thermal_terse();
	fprintf(stdout, "\n");

	dump_thermal_verbose_label();
	fprintf(stdout, "\n");
	dump_thermal_verbose();
	fprintf(stdout, "\n");
}

void perform_rapl_measurement(struct rapl_data* r) {
	read_rapl_data(0, r);

	fprintf(stdout, "old_pkg_joules=%lf pkg_joules=%lf pkg_delta_joules=%lf elapsed=%lf pkg_watts=%lf\n", 
			r->old_pkg_joules, r->pkg_joules, r->pkg_delta_joules, r->elapsed, r->pkg_watts);
}


void rapl_r_test(){
	// Initialize two separate state objects and read rapl data into them during overlapping time windows
	struct rapl_data r1; 
	struct rapl_data r2; 

	r1.flags = r2.flags = RDF_REENTRANT | RDF_INIT;

	perform_rapl_measurement(&r1);  // Initialize r1
	r1.flags = RDF_REENTRANT;
	sleep(1);

	fprintf(stdout, "R1: ");
	perform_rapl_measurement(&r1);
	sleep(1);
}


int main(int argc, char** argv){
	#ifdef MPI
	MPI_Init(&argc, &argv);
	#endif
	
	init_msr();
	set_limits();
	get_limits();
	rapl_test();
	rapl_r_test();
	finalize_msr();
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
