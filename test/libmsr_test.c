#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

// A sleep with some CPU and DRAM activity
char buf[100*1024];
void stall(int numSecs) {
  sleep(numSecs);
  int i;
  for(i=0; i<100*1024; i++)
    buf[i]++;
}

void rapl_set_test(double limit) {
	struct rapl_limit CPULimit, DRAMLimit;
	int s;
	CPULimit.watts = limit;
	CPULimit.seconds = 1;
	DRAMLimit.watts = limit;
	DRAMLimit.seconds = 1;
	fprintf(stdout, "Capping power at %f\n", limit);
	for(s=0; s<NUM_SOCKETS; s++) {
		set_rapl_limit(s, &CPULimit, NULL, &DRAMLimit);
	}
}

void rapl_test(){
	read_rapl_data(0, NULL);	// Initialize
	stall(3);

	dump_rapl_terse_label();
	fprintf(stdout, "\n");
	dump_rapl_terse();		// Read and dump.
	fprintf(stdout, "\n");

}

void thermal_test(){
	dump_thermal_terse_label();
	fprintf(stdout, "\n");
	dump_thermal_terse();
	fprintf(stdout, "\n");
}

void perform_rapl_measurement(struct rapl_data* r) {
	int socket;
	r->flags = RDF_REENTRANT;
	for(socket=0; socket<NUM_SOCKETS; socket++)
		fprintf(stdout,"pkgW%02d\tdramW%02d\telapsed%02d\t", socket, socket, socket );
	fprintf(stdout, "\n");

	for(socket=0; socket<NUM_SOCKETS; socket++) {
		read_rapl_data(socket, r);
		fprintf(stdout,"%8.6lf\t%8.6lf\t%8.6lf\t", r->pkg_watts, r->dram_watts, r->elapsed);
	}
	fprintf(stdout, "\n");
}


void rapl_r_test(){
	// Initialize two separate state objects and read rapl data into them during overlapping time windows
	struct rapl_data r1; r1.flags = RDF_REENTRANT;
	struct rapl_data r2; r2.flags = RDF_REENTRANT;

	read_rapl_data(0, &r1);  // Initialize r1
	stall(1);

	read_rapl_data(0, &r2);  // Initialize r2
	stall(1);

	// Complete and report s2 measurement
	fprintf(stdout, "R2: ");
	perform_rapl_measurement(&r2);
	stall(1);

	// Complete and report s1 measurement
	fprintf(stdout, "R1: ");
	perform_rapl_measurement(&r1);
	stall(1);

	// Complete and report s2 measurement
	fprintf(stdout, "R2: ");
	perform_rapl_measurement(&r2);
	stall(1);

	// Complete and report s1 measurement
	fprintf(stdout, "R1: ");
	perform_rapl_measurement(&r1);
}


int main(int argc, char** argv){
	#ifdef MPI
	MPI_Init(&argc, &argv);
	#endif
	
	init_msr();
	rapl_set_test(50);
	rapl_test();
	rapl_r_test();
	rapl_set_test(100);
	rapl_test();
	rapl_r_test();
	thermal_test();

	finalize_msr();
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
