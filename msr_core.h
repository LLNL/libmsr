#ifndef MSR_CORE_H
#define MSR_CORE_H
#include <stdint.h>
#include <sys/types.h>	// off_t
#define NUM_PACKAGES 2
#define NUM_CORES_PER_PACKAGE 8
enum{
	MSR_AND,
	MSR_OR,
	MSR_XOR
};
int init_msr();
void finalize_msr();

void write_msr(int socket, off_t msr, uint64_t val);
void write_msr_all_cores(int cpu, off_t msr, uint64_t val);
void write_msr_all_cores_v(int cpu, off_t msr, uint64_t *val);
void write_msr_single_core(int cpu, int core, off_t msr, uint64_t val);

void read_msr(int socket, off_t msr, uint64_t *val);
void read_msr_all_cores_v(int cpu, off_t msr, uint64_t *val);
void read_msr_single_core(int cpu, int core, off_t msr, uint64_t *val);


#endif //MSR_CORE_H
