#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_clocks.h"
#include "../include/msr_rapl.h"

static struct rapl_limit pkg1_limit, pkg2_limit, dram_limit;
static struct rapl_data all_data;

void
rapl_test(){
	get_rapl_limit(0, &pkg1_limit, &pkg2_limit, &dram_limit);	
	dump_rapl_limit(&pkg1_limit);
	dump_rapl_limit(&pkg2_limit);
	dump_rapl_limit(&dram_limit);

	read_rapl_data(0, &all_data);
	sleep(3);
	read_rapl_data(0, &all_data);

	dump_rapl_data( &all_data );

}

int
main(){
	init_msr();

	rapl_test();

	finalize_msr();

	return 0;
}
