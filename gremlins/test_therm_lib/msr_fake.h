#ifndef MSR_FAKE_H
#define MSR_FAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/time.h>
#include <stdint.h>

int init_msr();
void finalize_msr();
void write_msr(int socket, off_t msr, uint64_t val);
void write_msr_single_core(int socket, int core, off_t msr, uint64_t val);
void read_msr(int socket, off_t msr, uint64_t *val);
void read_msr_single_core(int socket, int core, off_t msr, uint64_t *val);

#endif // MSR_FAKE_H
