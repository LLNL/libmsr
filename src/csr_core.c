/* csr_core.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC. 
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number XXXXX.
 *
 */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "csr_core.h"
#include "memhdlr.h"

//#define CSRDEBUG
#define FILENAME_SIZE 128
#define MODULE "/dev/cpu/csr_safe"

static int *csr_fd()
{
	static int csrsafe = 0;
	return &csrsafe;
}

int init_csr()
{
	int *fileDescriptor = csr_fd();
	char filename[FILENAME_SIZE];
	snprintf(filename, FILENAME_SIZE, MODULE);
	struct stat statbuf;
	static int init = 1;

	if (init) {
		init = 0;
		if (stat(filename, &statbuf)) {
			fprintf(stderr, "%s %s::%d WARNING: could not stat %s: %s\n",
				getenv("HOSTNAME"), __FILE__, __LINE__, MODULE,
				strerror(errno));
		}
		if (!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR)) {
			fprintf(stderr, "%s %s::%d WARNING: csr_safe has incorrect permissions\n",
				getenv("HOSTNAME"), __FILE__, __LINE__);
		}
		*fileDescriptor = open(filename, O_RDWR);
	} else {
		fprintf(stderr, "%s %s::%d ERROR: unable to find module %s\n",
			getenv("HOSTNAME"), __FILE__, __LINE__, MODULE);
		return -1;
	}
	return 0;
}

int finalize_csr()
{
	int * fileDescriptor = csr_fd();
	int err = 0;

	if (fileDescriptor) {
		err = close(*fileDescriptor);
	}
	if (err) {
		fprintf(stderr, "%s %s::%d ERROR: could not close csr_fd: %s\n",
			getenv("HOSTNAME"), __FILE__, __LINE__,  strerror(errno));
		return -1;
	} else {
		*fileDescriptor = 0;
	}
	return 0;
}

// CSRs have their own batch functions so that they can be used independantly of the rest of libmsr
static int csr_batch_storage(struct csr_batch_array ** batchsel, const int batchnum,
	unsigned ** opssize)
{
	static struct csr_batch_array *batch = NULL;
	static unsigned arrsize = 1;
	static unsigned * size = NULL;

	if (batch == NULL) {
#ifdef CSRDEBUG
		fprintf(stderr, "CSRBATCH: initializing batch ops\n");
#endif
		arrsize = (batchnum + 1 > arrsize ? batchnum + 1 : arrsize);
		size = (unsigned *) libmsr_calloc(arrsize, sizeof(unsigned));
		batch = (struct csr_batch_array *) libmsr_calloc(arrsize, 
			sizeof(struct csr_batch_array));
		int i;
		for (i = 0; i < arrsize; i++) {
			size[i] = 8;
			batch[i].ops = NULL;
			batch[i].numops = 0;
		}
	}
	if (batchnum + 1 > arrsize) {
#ifdef CSRDEBUG
		fprintf(stderr, "CSRBATCH: reallocating CSR batch for number %d\n",
			batchnum);
#endif
		unsigned oldsize = arrsize;
		arrsize = batchnum + 1;
		batch = (struct csr_batch_array *) libmsr_realloc(batch, arrsize *
			sizeof(struct csr_batch_array));
		size = (unsigned *) libmsr_realloc(size, arrsize * sizeof(unsigned));
		for (; oldsize < arrsize; oldsize++) {
			batch[oldsize].ops = NULL;
			batch[oldsize].numops = 0;
			size[oldsize] = 8;
		}
	}
	if (batchsel == NULL) {
		fprintf(stderr, "%s %s::%d ERROR: attempted to load uninitialized batch %d\n",
			getenv("HOSTNAME"), __FILE__, __LINE__, batchnum);
	}
	*batchsel = &batch[batchnum];
	if (opssize) {
		*opssize = &size[batchnum];
	}
	return 0;
} 

int allocate_csr_batch(const int batchnum, size_t bsize)
{
	unsigned * size = NULL;
	struct csr_batch_array * batch = NULL;
#ifdef CSRDEBUG
	fprintf(stderr, "CSRBATCH: allocating batch %d\n", batchnum);
#endif
	if (csr_batch_storage(&batch, batchnum, &size)) {
		return -1;
	}
	*size = bsize;
	if (batch->ops != NULL) {
		fprintf(stderr, "%s %s::%d ERROR: conflicting batch pointers for batch %d, was %p\n",
			getenv("HOSTNAME"), __FILE__, __LINE__, batchnum, batch);
	}
	batch->ops = (struct csr_batch_op *) libmsr_calloc(*size, sizeof(struct csr_batch_op));
	struct csr_batch_op *op;
	for (op = batch->ops; op < op + batch->numops; op++) {
		op->err = 0;
	}
	return 0;
}

int free_csr_batch(const int batchnum)
{
	struct csr_batch_array *batch = NULL;
	static unsigned *size = NULL;
	if (batch == NULL) {
		if (csr_batch_storage(&batch, batchnum, &size)) {
			return -1;
		}
	}
	size[batchnum] = 0;
	libmsr_free(batch[batchnum].ops);
	batch[batchnum].ops = NULL;
	return 0;
}

int create_csr_batch_op(off_t csr, uint8_t bus, uint8_t device, uint8_t function, uint8_t socket,
	uint8_t isread, size_t opsize, uint64_t **dest, const int batchnum)
{
	struct csr_batch_array * batch = NULL;
	unsigned *size = NULL;
#ifdef CSRDEBUG
	fprintf(stderr, "CSRBATCH: batch %d is at %p\n", batchnum, batch);
#endif
	if (csr_batch_storage(&batch, batchnum, &size)) {
		return -1;
	}
#ifdef CSRDEBUG
	fprintf(stderr, "CSRBATCH: batch %d is at %p\n", batchnum, batch);
#endif
	if (batch->numops >= *size) {
		fprintf(stderr, "%s %s::%d ERROR: batch %d is full, you probably used the wrong size\n",
			getenv("HOSTNAME"), __FILE__, __LINE__, batchnum);
		return -1;
	}

	batch->numops++;
	struct csr_batch_op *op = &batch->ops[batch->numops - 1];
	op->offset = csr;
	op->bus = bus;
	op->device = device;
	op->function = function;
	op->socket = socket;
	op->isread = isread;
	op->err = 0;
	op->size = opsize;
	*dest = (uint64_t *) &op->csrdata;
#ifdef CSRDEBUG
	fprintf(stderr, "CSRBATCH: op %d: destination of csr %x on b%dd%df%ds%d is %p\n",
		batch->numops, op->offset, op->bus, op->device, op->function, op->socket, &op->csrdata);
#endif
	return 0;
}

int do_csr_batch_op(const int batchnum) {
	static int batchfd = 0;
	struct csr_batch_array * batch = NULL;
	if (!batchfd) {
		batchfd = *csr_fd();
	}
	if (csr_batch_storage(&batch, batchnum, NULL)) {
		return -1;
	}
#ifdef CSRDEBUG
//	fprintf(stderr, "CSRBATCH %d: numops %u\n", batchnum, batch->numops); 
#endif
	if (batch->numops <= 0) {
		fprintf(stderr, "%s %s::%d ERROR: attempted to use empty batch.\n",
			getenv("HOSTNAME"), __FILE__, __LINE__);
		return -1;
	}

	int res;
	res = ioctl(batchfd, CSRSAFE_8086_BATCH, batch);
	if (res < 0) {
		perror("IOctl failed, does /dev/cpu/csr_batch exist?");
		fprintf(stderr, "ioctl returned %d\n", res);
		struct csr_batch_op *op;
		int i = 0;
		for (op = batch->ops; op < op + batch->numops; op++) {
			i++;
			if (i >= batch->numops)
			{
				break;
			}
			if (op->err) {
				fprintf(stderr, "CSR %x, b%dd%df%ds%d, ERR %d\n",
					op->offset, op->bus, op->device, op->function,
					op->socket, op->err);
			}
		}
	}
	// DEBUG STUFF HERE
	return 0;
}
