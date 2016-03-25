/* csr_core.h
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
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

#include <linux/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#define FNAMESIZE 128

struct csr_batch_op {
	uint8_t  bus;
	uint8_t  device;
	uint8_t  function;
	uint8_t  socket;
	uint16_t offset;
	int err;
	uint8_t isread;
	uint64_t csrdata;
	uint64_t wmask;
	uint8_t size;
};

struct csr_batch_array {
	uint32_t numops;
	struct csr_batch_op *ops;
};

#define CSRSAFE_8086_BATCH _IOWR('a', 0x05, struct csr_batch_array)

enum{
	CSR_IMC_MEMCTRA,
	CSR_IMC_MEMCTRR,
	CSR_IMC_MEMCTRW,
	CSR_IMC_IMCCTR,
	CSR_IMC_CTRS,
	CSR_IMC_EVTS,
	CSR_IMC_PMONUNITCTRL,
	CSR_IMC_PMONUNITSTAT,
	CSR_QPI_CTRS,
	CSR_QPI_EVTS
};

int init_csr();
int finalize_csr();
//int csr_batch_storage(struct csr_batch_array **batchsel, const int batchnum,
//	unsigned **opssize);
int allocate_csr_batch(const int batchnum, size_t bsize);
int free_csr_batch(const int batchnum);
int create_csr_batch_op(off_t csr, uint8_t bus, uint8_t device, uint8_t function,
	uint8_t socket, uint8_t isread, size_t opsize, uint64_t **dest, const int batchnum);
int do_csr_batch_op(const int batchnum);
