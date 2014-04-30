#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"


void
rapl_test(){
	read_rapl_data(0, NULL);	// Initialize
	sleep(3);

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

int
main(){
	init_msr();
	rapl_test();
	thermal_test();

	finalize_msr();

	return 0;
}
