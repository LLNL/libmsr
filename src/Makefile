CFLAGS=-fPIC -Wall -O3
CC=gcc

all: libmsr

libmsr: msr_core.o msr_rapl.o msr_thermal.o msr_clocks.o signalCombined.o 
	mpicc -DPIC -fPIC -g -shared  -Wl,-soname,libmsr.so -o ../lib/libmsr.so $^

msr_core.o:   Makefile msr_core.c   msr_core.h 
msr_rapl.o:   Makefile msr_rapl.c   msr_rapl.h
msr_thermal.o: Makefile msr_thermal.c msr_thermal.h
msr_clocks.o: Makefile msr_clocks.c msr_clocks.h
signalCombined.o: Makefile signalCombined.c signalCombined.h

clean:
	rm -f *.o *.so 


	

  


