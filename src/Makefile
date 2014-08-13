# Makefile 
#
# Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
# Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
# This file is part of libmsr and is licensed under the LGLP.  See the LICENSE file for details.
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


	

  


