#include <stdio.h>
#include "msr_pebs.h"

int main( int argc, char **argv ){
	argc=argc;
	argv=argv;

	fprintf(stdout,"AAA\n");
	pebs_init();
	fprintf(stdout,"BBB\n");
	//dump_pebs();
	fprintf(stdout,"CCC\n");
	return 0;
}
