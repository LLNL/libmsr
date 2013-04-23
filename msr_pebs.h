/* msr_pebs.h
 */
#include <stdint.h>
#ifndef MSR_PEBS_H
#define MSR_PEBS_H

// Taken from table 18-10:  PEBS Performance Events for 
// 			    Intel Core Microarchitecture

// xx = UMASK
// yy = Event Select

// These are the PEBS-enabled counters.		  	//  xxyy 
/*
static const uint16_t INSTR_RETIRED__ANY_P		= 0x00c0;
static const uint16_t X87_OPS_RETIRED__ANY		= 0xfec1;
static const uint16_t BR_INST_RETIRED__MISPRED		= 0x00c5;
static const uint16_t SIMD_INST_RETIRED__ANY		= 0x1fc7;
static const uint16_t MEM_LOAD_RETIRED__L1D_MISS	= 0x01cb;
static const uint16_t MEM_LOAD_RETIRED__L1D_LINE_MISS	= 0x02cb;
static const uint16_t MEM_LOAD_RETIRED__L2_MISS		= 0x04cb;
static const uint16_t MEM_LOAD_RETIRED__L2_LINE_MISS	= 0x08cb;
static const uint16_t MEM_LOAD_RETIRED__DTLB_MISS	= 0x10cb;
*/

// Taken from table 18-12:  PEBS record format for Intel
// 			    Core i7 Processor Family
struct pebs_record{
	uint64_t eflags;// 0x00
	uint64_t eip;	// 0x08
	uint64_t eax;	// 0x10
	uint64_t ebx;	// 0x18
	uint64_t ecx;	// 0x20
	uint64_t edx;	// 0x28
	uint64_t esi;	// 0x30
	uint64_t edi;	// 0x38
	uint64_t ebp;	// 0x40
	uint64_t esp;	// 0x48
	uint64_t r8;	// 0x50
	uint64_t r9;	// 0x58
	uint64_t r10;	// 0x60
	uint64_t r11;	// 0x68
	uint64_t r12;	// 0x70
	uint64_t r13;	// 0x78
	uint64_t r14;	// 0x80
	uint64_t r15;	// 0x88
	uint64_t stat;	// 0x90 IA32_PERF_GLOBAL_STATUS
	uint64_t add;	// 0x98 Data Linear Address
	uint64_t enc;	// 0xa0 Data Source Encoding
	uint64_t lat;	// 0xa8 Latency value (core cycles)
			// 0xb0
};


// Taken from figure 18-16:  PEBS programming environment.
// The value in IA32_DS_AREA should point to this.
struct ds_area{
	uint64_t bts_buffer_base;			// 0x00 
	uint64_t bts_index;				// 0x08
	uint64_t bts_absolute_maximum;			// 0x10
	uint64_t bts_interrupt_threshold;		// 0x18
	struct pebs_record *pebs_buffer_base;		// 0x20	linear address of the first byte of the PEBS buffer
	struct pebs_record *pebs_index;			// 0x28	same value as the PEBS Buffer Base (updated by processor)
	struct pebs_record *pebs_absolute_maximum;	// 0x30 absolute address of the maximum length of the allocated PEBS 
							//	buffer plus the starting address of the PEBS buffer.
	struct pebs_record *pebs_interrupt_threshold;	// 0x38	linear address of the first byte of the PEBS record within 
							//	the PEBS buffer that represents the threshold record.
	uint64_t pebs_counter0_reset;			// 0x40
	uint64_t pebs_counter1_reset;			// 0x48
	uint64_t pebs_counter2_reset;			// 0x50
	uint64_t pebs_counter3_reset;			// 0x58
	uint64_t reserved;				// 0x60
};

void pebs_init(int nRecords, uint64_t *counter, uint64_t *reset_val);
void dump_pebs();

#endif // MSR_PEBS_H
