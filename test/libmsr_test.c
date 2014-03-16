#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_clocks.h"

int
main(){
	init_msr();

	dump_clocks();

	finalize_msr();

	return 0;
}
