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
	struct pebs_record *p = ds_area->pebs_buffer_base;
	while(p++ != ds_area->pebs_index){
		fprintf(stdout, "eflags	%lx\n",p->eflags);
		fprintf(stdout, "eip	%lx\n",   p->eip); 
		fprintf(stdout, "eax	%lx\n",   p->eax); 
		fprintf(stdout, "ebx	%lx\n",   p->ebx); 
		fprintf(stdout, "ecx	%lx\n",   p->ecx); 
		fprintf(stdout, "edx	%lx\n",   p->edx); 
		fprintf(stdout, "esi	%lx\n",   p->esi); 
		fprintf(stdout, "edi	%lx\n",   p->edi); 
		fprintf(stdout, "ebp	%lx\n",   p->ebp); 
		fprintf(stdout, "esp	%lx\n",   p->esp); 
		fprintf(stdout, "r8	%lx\n",    p->r8);  
		fprintf(stdout, "r9	%lx\n",    p->r9);  
		fprintf(stdout, "r10	%lx\n",   p->r10); 
		fprintf(stdout, "r11	%lx\n",   p->r11); 
		fprintf(stdout, "r12	%lx\n",   p->r12); 
		fprintf(stdout, "r13	%lx\n",   p->r13); 
		fprintf(stdout, "r14	%lx\n",   p->r14); 
		fprintf(stdout, "r15	%lx\n",   p->r15); 
		fprintf(stdout, "stat	%lx\n",  p->stat);
		fprintf(stdout, "add	%lx\n",   p->add); 
		fprintf(stdout, "enc	%lx\n",   p->enc); 
		fprintf(stdout, "lat	%lx\n",   p->lat); 
	}
};

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

	ds_area = mmap(
			NULL,						// let kernel choose address
			pagesize * 5,					// length in bytes
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

}

void pebs_start(){
	write_msr(0, IA32_PERF_GLOBAL_CTRL, 0xf);
}
	//stomp();


void pebs_stop(){
	write_msr(0, IA32_PERF_GLOBAL_CTRL, 0x0);
	dump_pebs();
	dump_useful_msrs();
	dump_ds_area();
}

void pebs_finalize(){	
	//Cleanup
	write_msr(0, IA32_PERF_GLOBAL_CTRL, 0x0);
	write_msr(0, IA32_PMC0, 0);
	write_msr(0, IA32_PERFEVTSEL0, 0);
	write_msr(0, IA32_DS_AREA, 0);
}



	

