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

static struct ds_area *ds_area;

static const off_t IA32_DS_AREA			= 0x600;
static const off_t IA32_PEBS_ENABLE 		= 0x3f1;	// AKA MSR_PEBS_ENABLE
static const off_t IA32_PERF_GLOBAL_CTRL	= 0x38f;	
static const off_t IA32_PMC0			= 0x0c1;
static const off_t IA32_PMC1			= 0x0c2;
static const off_t IA32_PMC2			= 0x0c3;
static const off_t IA32_PMC3			= 0x0c4;
static const off_t IA32_PERFEVTSEL0		= 0x186;
static const off_t IA32_PERFEVTSEL1		= 0x187;
static const off_t IA32_PERFEVTSEL2		= 0x188;
static const off_t IA32_PERFEVTSEL3		= 0x189;

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

static void 
dump_ds_area(){
	fprintf(stdout,"ds_area->bts_buffer_base		%lx\n",ds_area->bts_buffer_base);
	fprintf(stdout,"ds_area->bts_index			%lx\n",ds_area->bts_index);
	fprintf(stdout,"ds_area->bts_absolute_maximum		%lx\n",ds_area->bts_absolute_maximum);
	fprintf(stdout,"ds_area->bts_interrupt_threshold	%lx\n",ds_area->bts_interrupt_threshold);
	fprintf(stdout,"ds_area->pebs_buffer_base		%p\n",ds_area->pebs_buffer_base);
	fprintf(stdout,"ds_area->pebs_index			%p\n",ds_area->pebs_index);
	fprintf(stdout,"ds_area->pebs_absolute_maximum		%p\n",ds_area->pebs_absolute_maximum);
	fprintf(stdout,"ds_area->pebs_interrupt_threshold	%p\n",ds_area->pebs_interrupt_threshold);
	fprintf(stdout,"ds_area->pebs_counter0_reset		%lx\n",ds_area->pebs_counter0_reset);
	fprintf(stdout,"ds_area->pebs_counter1_reset		%lx\n",ds_area->pebs_counter1_reset);
	fprintf(stdout,"ds_area->pebs_counter2_reset		%lx\n",ds_area->pebs_counter2_reset);
	fprintf(stdout,"ds_area->pebs_counter3_reset		%lx\n",ds_area->pebs_counter3_reset);
	fprintf(stdout,"ds_area->reserved			%lx\n",ds_area->reserved);
}

static void
dump_useful_msrs(){
	uint64_t tmp;
	read_msr(0, IA32_PMC0, &tmp);
	fprintf(stdout,"IA32_PMC0				%lx\n", tmp);
	read_msr(0, IA32_PMC1, &tmp);
	fprintf(stdout,"IA32_PMC1				%lx\n", tmp);
	read_msr(0, IA32_PMC2, &tmp);
	fprintf(stdout,"IA32_PMC2				%lx\n", tmp);
	read_msr(0, IA32_PMC3, &tmp);
	fprintf(stdout,"IA32_PMC3				%lx\n", tmp);
	read_msr(0, IA32_PERFEVTSEL0, &tmp);
	fprintf(stdout,"IA32_PERFEVTSEL0			%lx\n", tmp);
	read_msr(0, IA32_PERFEVTSEL1, &tmp);
	fprintf(stdout,"IA32_PERFEVTSEL1			%lx\n", tmp);
	read_msr(0, IA32_PERFEVTSEL2, &tmp);
	fprintf(stdout,"IA32_PERFEVTSEL2			%lx\n", tmp);
	read_msr(0, IA32_PERFEVTSEL3, &tmp);
	fprintf(stdout,"IA32_PERFEVTSEL3			%lx\n", tmp);
	read_msr(0, IA32_PERF_GLOBAL_CTRL, &tmp);
	fprintf(stdout,"IA32_PERF_GLOBAL_CTRL			%lx\n",	tmp);
	read_msr(0, IA32_PEBS_ENABLE, &tmp);
	fprintf(stdout,"IA32_PEBS_ENABLE			%lx\n", tmp);
	read_msr(0, IA32_DS_AREA, &tmp);
	fprintf(stdout,"IA32_DS_AREA				%lx\n", tmp);
	read_msr(0, IA32_PMC0, &tmp);
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
	struct pebs_record *p = ds_area->pebs_buffer_base;
	while(p != ds_area->pebs_index){
		write( fd, p, sizeof(struct pebs_record) );
		p++;
	}
	close(fd);
}

//pebs_init(int nRecords, uint64_t *counter, uint64_t *reset_val ){
void
pebs_init(){
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
	
	
	// I think we can only have one mapping per process, so put the 
	// ds_area on the first page and the pebs records on the second
	// and successive pages.

	init_stomp();	
	ds_area = mmap(
			NULL,						// let kernel choose address
			pagesize * 3,					// length in bytes
			PROT_READ | PROT_WRITE, 			// make memory r/w
			MAP_ANONYMOUS | MAP_LOCKED | MAP_PRIVATE,	// no file | no swap | not visible
			-1,						// dummy file descriptor
			0);						// offset (ignored).
	if(ds_area == (void*)-1){
		perror("mmap for ds_area failed.");
		assert(0);
	}

	dump_useful_msrs();
	dump_ds_area();

	// pebs records start one page after ds_area
	struct pebs_record *ppebs = (struct pebs_record*) ( (uint64_t)ds_area + pagesize );

	ds_area->bts_buffer_base 		= 0;
	ds_area->bts_index			= 0;
	ds_area->bts_absolute_maximum		= 0;
	ds_area->bts_interrupt_threshold	= 0;
	ds_area->pebs_buffer_base		= ppebs;
	ds_area->pebs_index			= ppebs;
	ds_area->pebs_absolute_maximum		= (struct pebs_record*)( (uint64_t) ppebs + pagesize );
	ds_area->pebs_interrupt_threshold	= (struct pebs_record*)( (uint64_t) ppebs + pagesize );
	ds_area->pebs_counter0_reset		= -1000;	//reset_val[0];
	ds_area->pebs_counter1_reset		= 0;		//reset_val[1];
	ds_area->pebs_counter2_reset		= 0;		//reset_val[2];
	ds_area->pebs_counter3_reset		= 0;		//reset_val[3];
	ds_area->reserved			= 0;

	write_msr(0, IA32_PERF_GLOBAL_CTRL, 0);			// known good state.
	write_msr(0, IA32_DS_AREA, (uint64_t)ds_area);
	write_msr(0, IA32_PEBS_ENABLE, 0xf | ((uint64_t)0xf << 32) );	// Figure 18-14.

	write_msr(0, IA32_PMC0, -1000);
	//write_msr(0, IA32_PMC0, reset_val[0]);
	//write_msr(1, IA32_PMC1, reset_val[1]);
	//write_msr(2, IA32_PMC2, reset_val[2]);
	//write_msr(3, IA32_PMC3, reset_val[3]);

	write_msr(0, IA32_PERFEVTSEL0, 0x410000 | 0xc0 );	// Retired instructions.
	//write_msr(0, IA32_PERFEVTSEL0, 0x410000 | counter[0]);
	//write_msr(0, IA32_PERFEVTSEL1, 0x410000 | counter[1]);
	//write_msr(0, IA32_PERFEVTSEL2, 0x410000 | counter[2]);
	//write_msr(0, IA32_PERFEVTSEL3, 0x410000 | counter[3]);

	if(0){
		write_msr(0, IA32_PERF_GLOBAL_CTRL, 0xf);

		stomp();

		write_msr(0, IA32_PERF_GLOBAL_CTRL, 0x0);
	}

	dump_useful_msrs();
	dump_ds_area();
	
	//Cleanup
	write_msr(0, IA32_PERF_GLOBAL_CTRL, 0x0);
	write_msr(0, IA32_PMC0, 0);
	write_msr(0, IA32_PERFEVTSEL0, 0);
	write_msr(0, IA32_DS_AREA, 0);
}



	

