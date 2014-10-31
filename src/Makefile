# Makefile 
#
# Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
# Produced at the Lawrence Livermore National Laboratory.
# Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
# All rights reserved.
#
# This file is part of libmsr.
#
# libmsr is free software: you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any
# later version.
#
# libmsr is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public 
# License along
# with libmsr. If not, see <http://www.gnu.org/licenses/>.
#
# This material is based upon work supported by the U.S. Department
# of Energy's Lawrence Livermore National Laboratory. Office of 
# Science, under Award number DE-AC52-07NA27344.
#
CFLAGS=-fPIC -Wall -O3 -I../include
CC=gcc

all: libmsr

libmsr: msr_core.o msr_rapl.o msr_thermal.o msr_clocks.o cpuid.o msr_counters.o profile.o msr_misc.o signalCombined.o msr_turbo.o
	mpicc -DPIC -fPIC -g -shared  -Wl,-soname,libmsr.so -o ../lib/libmsr.so $^

msr_core.o:   		Makefile msr_core.c       ../include/msr_core.h 
msr_rapl.o:   		Makefile msr_rapl.c       ../include/msr_rapl.h
msr_thermal.o: 		Makefile msr_thermal.c    ../include/msr_thermal.h
msr_clocks.o: 		Makefile msr_clocks.c     ../include/msr_clocks.h
cpuid.o: 		Makefile cpuid.c 	  ../include/cpuid.h
msr_counters.o:		Makefile msr_counters.c	  ../include/msr_counters.h
profile.o: 		Makefile profile.c        ../include/profile.h
msr_misc.o:		Makefile msr_misc.c	  ../include/msr_misc.h
signalCombined.o: 	Makefile signalCombined.c ../include/signalCombined.h
msr_turbo.o:		Makefile msr_turbo.c	 ../include/msr_turbo.h
clean:
	rm -f *.o ../lib/*.so 


	

  


