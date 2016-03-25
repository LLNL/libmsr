/* autoconf.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
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

// This will generate the correct header files for the target architecture

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#define FNAME_SIZE 128
#define BUFFER_SIZE 512

uint64_t detect_arch()
{
	uint64_t rax = 1, rbx = 0, rcx = 0, rdx = 0;

	asm volatile("cpuid"
					: "=a" (rax),
					  "=b" (rbx),
					  "=c" (rcx),
					  "=d" (rdx)
					: "0" (rax), "2" (rcx));

	return ((rax >> 4) & 0xF) | ((rax >> 12) & 0xF0);	
}

FILE * open_header(char * a)
{
	FILE * header = NULL;
	char fname[FNAME_SIZE];
	uint64_t arch = detect_arch();

	if (a)
	{
		snprintf(fname, FNAME_SIZE, "headers/intel%s.h", a);
	}
	else
	{
		snprintf(fname, FNAME_SIZE, "headers/intel%lX.h", arch);
	}
	header = fopen(fname, "r");
	if (header == NULL)
	{
		fprintf(stderr, "ERROR: unable to open file %s\n", fname);
		if (a)
		{
			fprintf(stderr, 
				"architecture %s may not be supported. No header files found.\n",
				a);
		}
		else
		{
			fprintf(stderr, 
				"architecture %lx may not be supported. Use -f[arch number] to force.\n",
				arch);
		}
		exit(-1);
	}
	return header;
}

FILE * open_master()
{
	FILE * master = fopen("headers/master.h", "w");
	if (master == NULL)
	{
		fprintf(stderr, "ERROR: unable to open file headers/master.h\n");
		exit(-2);
	}
	return master;
}

int copy(char * arch)
{
	FILE * header = open_header(arch);
	FILE * master = open_master();
	char * buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
	size_t bsize = BUFFER_SIZE;
	size_t ret = 0;

	fprintf(master, "// This file was generated automatically by autoconf.c\n");
	fprintf(master, "// You should not modify it unless you really know what you're doing.\n");

	while((ret = getline(&buffer, &bsize, header)) != -1)
	{
		fprintf(master, "%s", buffer);
	}

	if (header)
	{
		fclose(header);
	}
	if (master)
	{
		fclose(master);
	}
	if (buffer)
	{
		free(buffer);
	}
	return 0;
}

int main(int argc, char ** argv)
{
	char arch[3];
	if (argc < 2 || argv[1][0] != '-' || argv[1][1] != 'f')
	{
		copy(NULL);
		return 0;
	}
	arch[0] = toupper(argv[1][2]);
	arch[1] = toupper(argv[1][3]);
	arch[2] = '\0';
	copy(arch);
	return 0;
}
