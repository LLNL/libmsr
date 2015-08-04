#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "msr_core.h"
#include "msr_turbo.h"
#include "memhdlr.h"

// MSRs common to 062A and 062D.
#define MSR_MISC_ENABLE			0x1A0	// aka IA32_MISC_ENABLE
						// setting bit 38 high DISABLES turbo mode. To be done in the BIOS.
						//
#define IA32_PERF_CTL			0x199   // setting bit 32 high DISABLES turbo mode. This is the software control.

int turbo_storage(uint64_t *** val)
{
    static uint64_t ** perf_ctl = NULL;
    static int init = 1;
    static uint64_t coresPerSocket = 0, threadsPerCore = 0, sockets = 0;
    if (init)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        perf_ctl = (uint64_t **) libmsr_malloc(NUM_DEVS_NEW * sizeof(uint64_t *));
        init = 0;
    }
    if (val)
    {
        *val = perf_ctl;
    }
    return 0;
}

void
disable_turbo(){

	int j;
    static uint64_t sockets = 0;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t ** val = NULL;
    if (!sockets || !coresPerSocket || !threadsPerCore)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        val = (uint64_t **) libmsr_malloc(NUM_DEVS_NEW * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    // Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 0.
    read_batch(PERF_CTL);
	for(j=0; j<NUM_DEVS_NEW; j++){
		*val[j] |= ((uint64_t)1) << 32;
	}
    write_batch(PERF_CTL);
}


void
enable_turbo(){
	int j;
    static uint64_t sockets = 0;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    static uint64_t ** val =  NULL;
    if (!sockets || !coresPerSocket || !threadsPerCore)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
        val = (uint64_t **) libmsr_malloc(NUM_DEVS_NEW * sizeof(uint64_t *));
        turbo_storage(&val);
    }
    // Set bit 32 "IDA/Turbo DISENGAGE" of IA32_PERF_CTL to 1.
    read_batch(PERF_CTL);
	for(j=0; j<NUM_DEVS_NEW;j++){
		*val[j] &= ~(((uint64_t)1) << 32);
        fprintf(stderr, "0x%016lx\t", *val[j] & (((uint64_t)1)<<32));
	}

    write_batch(PERF_CTL);
}

void
dump_turbo(FILE * writeFile){
	int core;
    static uint64_t sockets = 0;
    static uint64_t coresPerSocket = 0;
    static uint64_t threadsPerCore = 0;
    if (!sockets || !coresPerSocket || !threadsPerCore)
    {
        core_config(&coresPerSocket, &threadsPerCore, &sockets, NULL);
    }
	uint64_t val;
    for(core=0; core<NUM_DEVS_NEW; core++){
        read_msr_by_idx(core, MSR_MISC_ENABLE, &val);
        fprintf(writeFile, "Core: %d\n", core);
        fprintf(writeFile, "0x%016lx\t", val & (((uint64_t)1)<<38));
        fprintf(writeFile, "| MSR_MISC_ENABLE | 38 (0=Turbo Available) \n");
        read_msr_by_idx(core, IA32_PERF_CTL, &val);
        fprintf(writeFile, "0x%016lx \t", val & (((uint64_t)1)<<32));
        fprintf(writeFile, "| IA32_PERF_CTL | 32 (0=Turbo Engaged) \n");
    }
}


