/* csr_core.c
 *
 * Copyright (c) 2011-2015, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
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

#include "csr_core.h"

#define CSRDEBUG

int memhandle()
{
    static int init = 1;
    static int handle;
    if (init)
    {
        char fname[FNAMESIZE];
        snprintf(fname, FNAMESIZE, "/dev/mem");
        handle = open(fname, O_RDONLY);
        if (handle < 0)
        {
            fprintf(stderr, "ERROR: could not open %s. Are you root?\n", fname);
            return -1;
        }
        init = 0;
    }
    return handle;
}

int openMCFG()
{
    static int init = 1;
    static int mcfg;
    if (init)
    {
        char fname[FNAMESIZE];
        snprintf(fname, FNAMESIZE, "/sys/firmware/acpi/tables/MCFG");
        int mcfg = open(fname, O_RDONLY);
        if (mcfg < 0)
        {
            fprintf(stderr, "ERROR: could not open %s. Are you root?\n", fname);
            return -1;
        }
    }
    return mcfg;
}

uint64_t getBAR()
{
    static int init = 1;
    static int mcfg;
    static struct MCFGRecord record;
    if (init)
    {
        mcfg = openMCFG();
        int result = pread(mcfg, &record, sizeof(struct MCFGRecord), sizeof(struct MCFGHeader));
        if (result != sizeof(struct MCFGRecord))
        {
            fprintf(stderr, "ERROR: could not read BAR. pread returned %d\n", result);
            return -1;
        }
#ifdef CSRDEBUG
    printf("base address %llx\n", record.baseAddress);
#endif
        init = 0;
    }
    return record.baseAddress;
}

int csr_finalize()
{
    int handle = memhandle();
    if (handle)
    {
        close(handle);
    }
    int mcfg = getBAR();
    if (mcfg)
    {
        close(mcfg);
    }
    return 0;
}

char * pcieMap(uint32_t bus, uint32_t device, uint32_t function)
{
    uint64_t base_addr = 0;
    int handle;
    //unsigned char max_bus = 0;
    //result = pread(mcfg, &max_bus, sizeof(unsigned char), 55);
    //if (bus > max_bux)
    //{
    //    fprintf(stderr, "ERROR: bus %d is too large\n", bus);
    //    return NULL;
    //}
    handle = memhandle();
    if (handle < 0)
    {
        return NULL;
    }
    base_addr = getBAR(); 
    if (base_addr < 0)
    {
        return NULL;
    }
    base_addr += (bus * 1024 * 1024 + device * 32 * 1024 + function * 4 * 1024);
    // bring in a whole CSR config space 
    char * mmapAddr = (char *) mmap(NULL, 4096, PROT_READ, MAP_SHARED, handle, base_addr);

    return mmapAddr;
}

int pcieRead32(char * mmapAddr, uint32_t offset, uint32_t * value)
{
    if (mmapAddr != NULL)
    {
        *value = *(mmapAddr + offset);
        return 0;
    }
    return -1;
}

int pcieRead8(char * mmapAddr, uint32_t offset, uint8_t * value)
{
    if (mmapAddr != NULL)
    {
        *value = *(mmapAddr + offset);
        return 0;
    }
    return -1;

}
