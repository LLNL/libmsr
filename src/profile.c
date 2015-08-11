/* profile.c
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
#include "profile.h"
#include "msr_rapl.h"
#include "msr_clocks.h"
#include "msr_counters.h"
void 
msr_profile() {
	static struct timeval startTime;
	struct timeval currentTime;
	static int init = 0;
	static struct itimerval tout_val;

	if(!init) {
		signal(SIGALRM, msr_profile);
		tout_val.it_interval.tv_sec = 0;
		tout_val.it_interval.tv_usec = 0;
		tout_val.it_value.tv_sec = 0;
		tout_val.it_value.tv_usec = 100000;
		init = 1;
		gettimeofday(&startTime, NULL);

		fprintf(stdout, "QQQ gtod ");
		dump_clocks_terse_label(stdout);
		dump_rapl_terse_label(stdout);
		dump_fixed_terse_label(stdout);
		fprintf(stdout, "\n");
	}
	
	gettimeofday(&currentTime, NULL);

	fprintf(stdout, "QQQ %lf ", (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0);
	dump_clocks_terse(stdout);
	dump_rapl_terse(stdout);
	dump_fixed_terse(stdout);
	fprintf(stdout, "\n");

	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

