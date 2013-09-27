/* msr_fake.c
 *
 * Provides a wrapper for:
 * init_msr(),
 * finalize_msr(), 
 * read_single_core
 * write_single_core
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>
#include <stdint.h>
#include "msr_fake.h"

int init_msr()
{
	printf("This is msr_fake.c in function: init_msr()\n");
	return 0;
}

void finalize_msr()
{
	printf("This is msr_fake.c in function finalize_msr()\n");
}

void write_msr(int socket, off_t msr, uint64_t val)
{
	printf("This is msr_fake.c in function write_msr()\n");
	write_msr_single_core(socket, 0, msr, val);
}

void write_msr_single_core(int socket, int core, off_t msr, uint64_t val)
{
	printf("This is msr_fake.c in function write_msr_single_core()\n");
}

void read_msr(int socket, off_t msr, uint64_t *val)
{
	printf("This is msr_fake.c in function read_msr()\n");
	int core = 1;
	read_msr_single_core(socket, core, msr, val);
}	

void read_msr_single_core(int socket, int core, off_t msr, uint64_t *val)
{
	printf("This is msr_fake.c in function read_msr_single_core()\n");
	*val = 56879;
}
