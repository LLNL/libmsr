#include <unistd.h>
#include <stdio.h>
#include "../include/msr_core.h"
#include "../include/msr_clocks.h"
#include "../include/msr_rapl.h"

static struct rapl_limit pkg1_limit, pkg2_limit, dram_limit;
static struct rapl_data all_data;

void
rapl_test(){
	rapl_get_limit(0, &pkg1_limit, &pkg2_limit, &dram_limit);	
	rapl_dump_limit(&pkg1_limit);
	rapl_dump_limit(&pkg2_limit);
	rapl_dump_limit(&dram_limit);

	rapl_read_data(0, &all_data);
	sleep(3);
	rapl_read_data(0, &all_data);

	rapl_dump_data( &all_data );

}

int
main(){
	init_msr();

	dump_clocks();

	rapl_test();

	finalize_msr();

	return 0;
}
