#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "msr_core.h"
#include "msr_turbo.h"

#define IA32_PERF_CTL			0x199   // setting bit 32 high DISABLES turbo mode.

void
enable_turbo(const int socket){
	int j;
	uint64_t val[NUM_CORES_PER_SOCKET];
	
	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 0.
	read_msr_all_cores_v( socket, IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_CORES_PER_SOCKET; j++){
		val[j] &= ((uint64_t)-1) ^ ((uint64_t)1) << 32;
	}
	write_msr_all_cores_v( socket, IA32_PERF_CTL, &val[0] );

}

void
disable_turbo(const int socket){
	int j;
	uint64_t val[NUM_CORES_PER_SOCKET];
	
	// Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1.
	read_msr_all_cores_v( socket, IA32_PERF_CTL, &val[0] );
	for(j=0; j<NUM_CORES_PER_SOCKET; j++){
		val[j] |= ((uint64_t)1) << 32;
	}
	write_msr_all_cores_v( socket, IA32_PERF_CTL, &val[0] );

}

void
enable_all_turbo(){
	int socket;
	for(socket = 0; socket<NUM_SOCKETS; socket++){
		enable_turbo(socket);
	}
}

void
disable_all_turbo(){
	int socket;
	for(socket = 0; socket<NUM_SOCKETS; socket++){
		disable_turbo(socket);
	}
}

void
dump_turbo(){
	int socket, core;
	uint64_t val;
	fprintf(stdout,"Per-socket, per-core turbo state (1=DISengaged)\n");
	for(socket = 0; socket<NUM_SOCKETS; socket++){
		fprintf(stdout, "Socket %d:  ", socket);
		for(core=0; core<NUM_CORES_PER_SOCKET; core++){
			read_msr_single_core( socket, core, IA32_PERF_CTL, &val );
			fprintf(stdout, "%lu", val & (((uint64_t)1)<<32));
		}
	}
}


