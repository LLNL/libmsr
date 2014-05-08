#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_rapl.h"
#include "../include/msr_thermal.h"

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
	l1.seconds = 1;
	l1.bits = 0;
	l2.watts = 65;
	l2.watts = 0.1;
	l2.bits = 0;
	set_rapl_limit(0, &l1, &l2, NULL);
	get_limits();
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
	//get_limits();
	set_limits();
	//rapl_test();
	//thermal_test();

	finalize_msr();

	return 0;
}
