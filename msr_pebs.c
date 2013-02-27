/* msr_pebs.c
 */
#include <unistd.h>
#include <stdlib.h>	// calloc, exit
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h> 	// NULL
#include <errno.h>
#include "msr_pebs.h"
#include "msr_core.h"

static struct ds_area *pds_area;

static const off_t IA32_DS_AREA		= 0x600;
static const off_t IA32_PEBS_ENABLE 	= 0x3f1;
static const off_t PERF_GLOBAL_CTRL	= 0x38f;	
static const off_t PMC0			= 0x0c1;
static const off_t PMC1			= 0x0c2;
static const off_t PMC2			= 0x0c3;
static const off_t PMC3			= 0x0c4;
static const off_t IA32_PERFEVTSEL0	= 0x186;
static const off_t IA32_PERFEVTSEL1	= 0x187;
static const off_t IA32_PERFEVTSEL2	= 0x188;
static const off_t IA32_PERFEVTSEL3	= 0x189;

static uint64_t *stomp_buf;
static const uint64_t stomp_sz		= (uint64_t)1024 * 1024 * 64;

static void
init_stomp(){
	// 512MB
	stomp_buf = calloc( (size_t)stomp_sz, sizeof(uint64_t) );
	assert(stomp_buf);
}

static void
stomp(){
	uint64_t i,j;
	
	for(j=0; j<1024; j++){
		for(i=stomp_sz-4097; i>0; i=i-4097){
			stomp_buf[i+j] += i+j;
		}
	}
}

void 
dump_pebs(){
	static int initialized = 0;
	static int fd = 0;
	if(!initialized){
		initialized = 1;
		fd = open("./pebs.out", O_WRONLY | O_APPEND | O_CREAT );
		if(fd == -1){
			fprintf(stderr, "%s::%d file error.\n", __FILE__, __LINE__ );
			perror("Bye!\n");
			exit(-1);
		}
	}
	struct pebs *p = pds_area->pebs_buffer_base;
	while(p != pds_area->pebs_index){
		write( fd, p, sizeof(struct pebs) );
	}
	close(fd);
}

void
pebs_init(int nRecords, uint64_t *counter, uint64_t *reset_val ){
	// 1. Set up the precise event buffering utilities.
	// 	a.  Place values in the
	// 		i.   precise event buffer base,
	//		ii.  precise event index
	//		iii. precise event absolute maximum,
	//		iv.  precise event interrupt threshold,
	//		v.   and precise event counter reset fields
	//	    of the DS buffer management area.
	//
	// 2.  Enable PEBS.  Set the Enable PEBS on PMC0 flag 
	// 	(bit 0) in IA32_PEBS_ENABLE_MSR.
	//
	// 3.  Set up the IA32_PMC0 performance counter and 
	// 	IA32_PERFEVTSEL0 for an event listed in Table 
	// 	18-10.
	
	// IA32_DS_AREA points to 0x58 bytes of memory.  
	// (11 entries * 8 bytes each = 88 bytes.)
	
	// Each PEBS record is 0xB0 byes long.
	int pagesize = getpagesize();
	
	init_stomp();	
	
	// I think we can only have one mapping per process, so put the 
	// pds_area on the first page and the pebs records on the second
	// and successive pages.

	pds_area = mmap(
			NULL,						// let kernel choose address
			pagesize + 
				(pagesize*(((sizeof(struct pebs)*nRecords)/pagesize)+1)),	// keep ds and records separate.
			PROT_READ | PROT_WRITE, 
			MAP_ANONYMOUS | MAP_LOCKED | MAP_PRIVATE,
			-1,						// dummy file descriptor
			0);						// offset (ignored).
	if(pds_area == (void*)-1){
		perror("mmap for pds_area failed.");
		assert(0);
	}

	struct pebs *ppebs = (struct pebs*) ( (uint64_t)pds_area + pagesize );

	pds_area->bts_buffer_base 		= 0;
	pds_area->bts_index			= 0;
	pds_area->bts_absolute_maximum		= 0;
	pds_area->bts_interrupt_threshold	= 0;
	pds_area->pebs_buffer_base		= ppebs;
	pds_area->pebs_index			= ppebs;
	pds_area->pebs_absolute_maximum		= ppebs + (nRecords-1) * sizeof(struct pebs);
	pds_area->pebs_interrupt_threshold	= ppebs + (nRecords+1) * sizeof(struct pebs);
	pds_area->pebs_counter0_reset		= reset_val[0];
	pds_area->pebs_counter1_reset		= reset_val[1];
	pds_area->pebs_counter2_reset		= reset_val[2];
	pds_area->pebs_counter3_reset		= reset_val[3];
	pds_area->reserved			= 0;

	write_msr(0, PERF_GLOBAL_CTRL, 0);			// known good state.
	write_msr(0, IA32_DS_AREA, (uint64_t)pds_area);
	write_msr(0, IA32_PEBS_ENABLE, 0xf | ((uint64_t)0xf << 32) );	// Figure 18-14.

	write_msr(0, PMC0, reset_val[0]);
	write_msr(1, PMC1, reset_val[1]);
	write_msr(2, PMC2, reset_val[2]);
	write_msr(3, PMC3, reset_val[3]);

	write_msr(0, IA32_PERFEVTSEL0, 0x410000 | counter[0]);
	write_msr(0, IA32_PERFEVTSEL1, 0x410000 | counter[1]);
	write_msr(0, IA32_PERFEVTSEL2, 0x410000 | counter[2]);
	write_msr(0, IA32_PERFEVTSEL3, 0x410000 | counter[3]);

	write_msr(0, PERF_GLOBAL_CTRL, 0xf);

	stomp();

	write_msr(0, PERF_GLOBAL_CTRL, 0x0);
}



	


