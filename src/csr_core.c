/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "csr_core.h"
#include "memhdlr.h"
#include "cpuid.h"
#include "libmsr_debug.h"
#include "libmsr_error.h"

/// @brief Retrieve file descriptor for uncore register.
///
/// @return Unique file descriptor, else NULL.
static int *csr_fd(void)
{
    static int csrsafe = 0;
    return &csrsafe;
}

int init_csr(void)
{
    int *fileDescriptor = csr_fd();
    char filename[CSR_FILENAME_SIZE];
    struct stat statbuf;
    static int init = 0;

    snprintf(filename, CSR_FILENAME_SIZE, CSR_MODULE);
    if (!init)
    {
        init = 1;
        if (stat(filename, &statbuf))
        {
            fprintf(stderr, "Warning: <libmsr> Could not stat %s: init_csr(): %s: %s:%s::%d\n", CSR_MODULE, strerror(errno), getenv("HOSTNAME"), __FILE__, __LINE__);
        }
        if (!(statbuf.st_mode & S_IRUSR) || !(statbuf.st_mode & S_IWUSR))
        {
            fprintf(stderr, "Warning: <libmsr> Incorrect permissions on csr_safe: init_csr(): %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
        }
        *fileDescriptor = open(filename, O_RDWR);
    }
    else
    {
        libmsr_error_handler("init_csr(): Unable to find csr_safe module", LIBMSR_ERROR_MSR_MODULE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int finalize_csr(void)
{
    int *fileDescriptor = csr_fd();
    int err = 0;

    if (fileDescriptor)
    {
        err = close(*fileDescriptor);
    }
    if (err)
    {
        libmsr_error_handler("finalize_csr(): Could not close csr_fd", LIBMSR_ERROR_MSR_CLOSE, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }
    else
    {
        *fileDescriptor = 0;
    }
    return 0;
}

// CSRs have their own batch functions so that they can be used independently
// of the rest of libmsr
int csr_batch_storage(struct csr_batch_array **batchsel, const int batchnum, unsigned **opssize)
{
    static struct csr_batch_array *batch = NULL;
    static unsigned arrsize = 1;
    static unsigned *size = NULL;

    if (batch == NULL)
    {
#ifdef CSRDEBUG
        fprintf(stderr, "CSRBATCH: initializing batch ops\n");
#endif
        arrsize = (batchnum + 1 > arrsize ? batchnum + 1 : arrsize);
        size = (unsigned *) libmsr_calloc(arrsize, sizeof(unsigned));
        batch = (struct csr_batch_array *) libmsr_calloc(arrsize, sizeof(struct csr_batch_array));
        int i;
        for (i = 0; i < arrsize; i++)
        {
            size[i] = 8;
            batch[i].ops = NULL;
            batch[i].numops = 0;
        }
    }
    if (batchnum + 1 > arrsize)
    {
        unsigned oldsize = arrsize;
#ifdef CSRDEBUG
        fprintf(stderr, "CSRBATCH: reallocating CSR batch for number %d\n", batchnum);
#endif
        arrsize = batchnum + 1;
        batch = (struct csr_batch_array *) libmsr_realloc(batch, arrsize * sizeof(struct csr_batch_array));
        size = (unsigned *) libmsr_realloc(size, arrsize * sizeof(unsigned));
        for (; oldsize < arrsize; oldsize++)
        {
            batch[oldsize].ops = NULL;
            batch[oldsize].numops = 0;
            size[oldsize] = 8;
        }
    }
    if (batchsel == NULL)
    {
        libmsr_error_handler("csr_batch_storage(): Loading uninitialized batch", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    *batchsel = &batch[batchnum];
    if (opssize)
    {
        *opssize = &size[batchnum];
    }
    return 0;
}

int allocate_csr_batch(const int batchnum, size_t bsize)
{
    unsigned *size = NULL;
    struct csr_batch_array *batch = NULL;
    struct csr_batch_op *op;
#ifdef CSRDEBUG
    fprintf(stderr, "CSRBATCH: allocating batch %d\n", batchnum);
#endif
    if (csr_batch_storage(&batch, batchnum, &size))
    {
        return -1;
    }
    *size = bsize;
    if (batch->ops != NULL)
    {
        libmsr_error_handler("allocate_csr_batch(): Conflicting batch pointers", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
    }
    batch->ops = (struct csr_batch_op *) libmsr_calloc(*size, sizeof(struct csr_batch_op));
    for (op = batch->ops; op < op + batch->numops; op++)
    {
        op->err = 0;
    }
    return 0;
}

int free_csr_batch(const int batchnum)
{
    struct csr_batch_array *batch = NULL;
    static unsigned *size = NULL;

    if (batch == NULL)
    {
        if (csr_batch_storage(&batch, batchnum, &size))
        {
            return -1;
        }
    }
    size[batchnum] = 0;
    libmsr_free(batch[batchnum].ops);
    batch[batchnum].ops = NULL;
    return 0;
}

int create_csr_batch_op(off_t csr, uint8_t bus, uint8_t device, uint8_t function, uint8_t socket, uint8_t isread, size_t opsize, uint64_t **dest, const int batchnum)
{
    struct csr_batch_array *batch = NULL;
    unsigned *size = NULL;

#ifdef CSRDEBUG
    fprintf(stderr, "CSRBATCH: batch %d is at %p\n", batchnum, batch);
#endif
    if (csr_batch_storage(&batch, batchnum, &size))
    {
        return -1;
    }
#ifdef CSRDEBUG
    fprintf(stderr, "CSRBATCH: batch %d is at %p\n", batchnum, batch);
#endif
    if (batch->numops >= *size)
    {
        libmsr_error_handler("create_csr_batch_op(): Batch is full, you likely used the wrong size", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
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
    fprintf(stderr, "CSRBATCH: op %d: destination of csr %x on b%dd%df%ds%d is %p\n", batch->numops, op->offset, op->bus, op->device, op->function, op->socket, &op->csrdata);
#endif
    return 0;
}

int do_csr_batch_op(const int batchnum)
{
    static int batchfd = 0;
    struct csr_batch_array *batch = NULL;
    int res;

    if (!batchfd)
    {
        batchfd = *csr_fd();
    }
    if (csr_batch_storage(&batch, batchnum, NULL))
    {
        return -1;
    }
#ifdef CSRDEBUG
    fprintf(stderr, "CSRBATCH %d: numops %u\n", batchnum, batch->numops);
#endif
    if (batch->numops <= 0)
    {
        libmsr_error_handler("do_csr_batch_op(): Using empty batch", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    res = ioctl(batchfd, CSRSAFE_8086_BATCH, batch);
    if (res < 0)
    {
        libmsr_error_handler("do_csr_batch_op(): IOctl failed, does /dev/cpu/csr_batch exist?", LIBMSR_ERROR_MSR_BATCH, getenv("HOSTNAME"), __FILE__, __LINE__);
        fprintf(stderr, "ioctl returned %d\n", res);
        struct csr_batch_op *op;
        int i = 0;
        for (op = batch->ops; op < op + batch->numops; op++)
        {
            i++;
            if (i >= batch->numops)
            {
                break;
            }
            if (op->err)
            {
                fprintf(stderr, "CSR %x, b%dd%df%ds%d, ERR %d\n", op->offset, op->bus, op->device, op->function, op->socket, op->err);
            }
        }
    }
    /// @todo Debug stuff here.
    return 0;
}
