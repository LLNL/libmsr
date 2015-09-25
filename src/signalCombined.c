/* 
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include "signalCombined.h"

#ifndef SET_UP
static struct timeval startTime;
static int init = 0;
int stop=0;
#endif



void printData(int i)
{
    static uint64_t sockets = 0;
    if (!sockets)
    {
        sockets = num_sockets();
    }
	signal(SIGALRM, printData);
	struct timeval currentTime;
	if(!init)
	{
		init = 1;
		gettimeofday(&startTime, NULL);
	}
	struct itimerval tout_val;
	
	struct rapl_data * rd = NULL;
	

	gettimeofday(&currentTime, NULL);
	double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
	fprintf(stdout, "Timestamp: %3.2lf\n", timeStamp);	

	fprintf(stdout, "Core Temperatures: ");
	dump_thermal_terse(stdout);	
	fprintf(stdout, "\n");

    // TODO: may need changes after batch is fixed
    unsigned sock_idx;
    for (sock_idx = 0; sock_idx < sockets; sock_idx++)
    {
        poll_rapl_data(sock_idx, &rd);
        fprintf(stdout, "Power Socket/PKG/DRAM: %d %8.4lf %8.4lf \n", sock_idx, *rd->pkg_watts, *rd->dram_watts);
    }

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 5;
	tout_val.it_value.tv_usec = 0;
	
	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

