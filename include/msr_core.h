#ifndef MSR_CORE_H
#define MSR_CORE_H
#include <stdint.h>
#include <sys/types.h>	// off_t
#define NUM_SOCKETS 4
#define NUM_CORES_PER_SOCKET 8 
#define NUM_THREADS_PER_CORE 1
#define NUM_DEVS (NUM_SOCKETS * NUM_CORES_PER_SOCKET * NUM_THREADS_PER_CORE)
#define NUM_CORES (NUM_CORES_PER_SOCKET * NUM_SOCKETS)
#define NUM_THREADS (NUM_CORES * NUM_THREADS_PER_CORE)

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

void write_msr_by_idx( int dev_idx, off_t msr, uint64_t  val );
void read_msr_by_idx(  int dev_idx, off_t msr, uint64_t *val );

void write_msr_by_coord( int socket, int core, int thread, off_t msr, uint64_t  val );
void read_msr_by_coord(  int socket, int core, int thread, off_t msr, uint64_t *val );

void write_all_sockets(   off_t msr, uint64_t  val  );
void write_all_sockets_v( off_t msr, uint64_t *val );
void write_all_cores(     off_t msr, uint64_t  val  );
void write_all_cores_v(   off_t msr, uint64_t *val );
void write_all_threads(   off_t msr, uint64_t  val  );
void write_all_threads_v( off_t msr, uint64_t *val );

void read_all_sockets(    off_t msr, uint64_t *val );
void read_all_cores(      off_t msr, uint64_t *val );
void read_all_threads(    off_t msr, uint64_t *val );

#ifdef __cplusplus 
}
#endif
#endif //MSR_CORE_H
