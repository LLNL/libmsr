#ifndef MSR_CORE_H
#define MSR_CORE_H
#include <stdint.h>
#include <sys/types.h>	// off_t
#define NUM_SOCKETS 2
#define NUM_CORES_PER_SOCKET 8
enum{
	MSR_AND,
	MSR_OR,
	MSR_XOR
};

// Depending on their function, MSRs can be addressed at either
// the socket (aka cpu) or core level, and possibly the hardware
// thread level.  
//
//  read/write_msr reads from core 0.
//  read/write_msr_all_cores_v uses a vector of values.
//  write_msr_all_cores writes all cores with a single value.
//  read/write_msr_single_core contains all of the low-level logic.
//	The rest of the functions are wrappers that call these
//	two functions.
//  
#ifdef __cplusplus 
extern "C" {
#endif

int init_msr();
void finalize_msr();
void write_msr(int socket, off_t msr, uint64_t val);
void write_msr_all_cores(int cpu, off_t msr, uint64_t val);
void write_msr_all_cores_v(int cpu, off_t msr, uint64_t *val);
void write_msr_single_core(int cpu, int core, off_t msr, uint64_t val);

void read_msr(int socket, off_t msr, uint64_t *val);
void blr_read_msr(int socket, off_t msr, uint64_t *val);
void read_msr_all_cores_v(int cpu, off_t msr, uint64_t *val);
void read_msr_single_core(int cpu, int core, off_t msr, uint64_t *val);

int blr_init_msr();
void blr_finalize_msr();
#ifdef __cplusplus 
}
#endif
#endif //MSR_CORE_H
