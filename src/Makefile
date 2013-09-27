# General makefile notes:
# n.o is made automatically from n.c with a recipe of the form $(CC) $(CPPFLAGS) $(CFLAGS). 
# n is made automatically from n.o by running the linker (usually called ld) via the C compiler. The precise recipe used is 
#	$(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS)
# $^ The names of all the prerequisites, with spaces between them.
# $@ The file name of the target.
#
# Note:  Sandy Bridge Core is -DARCH_062A
#        Sandy Bridge Xeon is -DARCH_062D
#
CFLAGS=-fPIC -Wall -O3
CC=gcc

all: libmsr

libmsr: msr_core.o msr_rapl.o msr_thermal.o msr_clocks.o signalCombined.o 
	mpicc -DPIC -fPIC -g -shared  -Wl,-soname,libmsr.so -o libmsr.so $^

msr_core.o:   Makefile msr_core.c   msr_core.h 
msr_rapl.o:   Makefile msr_rapl.c   msr_rapl.h
msr_thermal.o: Makefile msr_thermal.c msr_thermal.h
msr_clocks.o: Makefile msr_clocks.c msr_clocks.h
signalCode.o: Makefile signalCode.c signalCode.h
signalPower.o: Makefile signalPower.c signalPower.h
signalCombined.o: Makefile signalCombined.c signalCombined.h

clean:
	rm -f *.o *.so 


	

  


