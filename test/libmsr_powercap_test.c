// NOTE: this code has not been updated in a while, see libmsr_test.c for working examples
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"
#ifdef MPI
#include <mpi.h>
#endif

void rapl_set_test(double limit) {
	struct rapl_limit CPULimit, DRAMLimit;
	int s;
	CPULimit.watts = limit;
	CPULimit.seconds = 0.1;
        CPULimit.bits = 0;
	DRAMLimit.watts = limit;
	DRAMLimit.seconds = 0.1;
        DRAMLimit.bits = 0;
	for(s=0; s<NUM_SOCKETS; s++) {
		struct rapl_limit measuredCPULimit;
		get_rapl_limit(s, &measuredCPULimit, NULL, NULL);
		fprintf(stdout, "Initial power cap on socket %d at %f for %f secs\n", s, measuredCPULimit.watts, measuredCPULimit.seconds);

		set_rapl_limit(s, &CPULimit, NULL, &DRAMLimit);
		fprintf(stdout, "Capping power on socket %d at %f for %f secs\n", s, CPULimit.watts, CPULimit.seconds);

		get_rapl_limit(s, &measuredCPULimit, NULL, NULL);
		fprintf(stdout, "Capped power on socket %d at %f for %f secs\n", s, measuredCPULimit.watts, measuredCPULimit.seconds);
	}
}

void perform_rapl_measurement(struct rapl_data* r) {
	int socket;
	for(socket=0; socket<NUM_SOCKETS; socket++)
		fprintf(stdout,"pkgW%02d\tpkgE%02d\tdramW%02d\tdramE%02d\telapsed%02d\t", socket, socket, socket, socket, socket );
	fprintf(stdout, "\n");

	for(socket=0; socket<NUM_SOCKETS; socket++) {
		r[socket].flags = RDF_REENTRANT;
		read_rapl_data(socket, &r[socket]);
		fprintf(stdout,"%8.6lf\t%8.6lf\t%8.6lf\t%8.6lf\t%8.6lf\t", r[socket].pkg_watts, r[socket].pkg_delta_joules, r[socket].dram_watts, r[socket].dram_delta_joules, r[socket].elapsed);
	}
	fprintf(stdout, "\n");
}

int main(int argc, char** argv){
	#ifdef MPI
	MPI_Init(&argc, &argv);
	#endif

	int power;
	init_msr();

	struct rapl_data r[NUM_SOCKETS]; 
	
        int socket;
	
	for(socket=0; socket<NUM_SOCKETS; socket++)
		r[socket].flags =  RDF_REENTRANT | RDF_INIT;
	// Initialize r
	for(socket=0; socket<NUM_SOCKETS; socket++) 
		read_rapl_data(socket, &r[socket]);

	sleep(1);	
	perform_rapl_measurement(r);
	for(power=5; power<150; power+=10) {
		rapl_set_test(power);
		sleep(10);
		perform_rapl_measurement(r);
	}

	finalize_msr();

	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
