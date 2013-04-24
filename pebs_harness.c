#include <stdio.h>
#include "msr_core.h"
#include "msr_pebs.h"

int main( int argc, char **argv ){
	argc=argc;
	argv=argv;

	init_msr();

	fprintf(stdout,"AAA\n");
	pebs_init();
	fprintf(stdout,"BBB\n");
	//dump_pebs();
	fprintf(stdout,"CCC\n");

	finalize_msr();

	return 0;
}
