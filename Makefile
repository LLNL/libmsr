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
MYLIBDIR= /home/shoga1/libmsr

DEFINES=-DARCH_SANDY_BRIDGE -DPKG_PERF_STATUS_AVAILABLE 

all: helloWorldMPI.o libPowThermTest libPowerTest

#libmsr: msr_core.o msr_rapl.o msr_thermal.o signalCode.o
#libmsr: msr_core.o msr_rapl.o msr_thermal.o signalPower.o
libmsr: msr_core.o msr_rapl.o msr_thermal.o signalCombined.o 
	mpicc -DPIC -fPIC -g -shared  -Wl,-soname,libmsr.so -o libmsr.so $^

libThermTest: thermalTest.c
	mpicc -fPIC -g -shared  -Wl,-soname,libThermTest.so -o libThermTest.so $^

libPowerTest: powerTest.c
	mpicc -fPIC -g -shared -Wl,-soname,libPowerTest.so -o libPowerTest.so $^

libPowThermTest: powerThermalTest.c
	mpicc -fPIC -g -shared -Wl,-soname,libPowThermTest.so -o libPowThermTest.so $^

helloWorldMPI.o: helloWorld_mpi.c libmsr libThermTest
	mpicc -DPIC -o helloWorldMPI.o -Wl,-rpath=/home/shoga1/libmsr -L ${MYLIBDIR} -lmsr -L${MYLIBDIR} -lThermTest helloWorld_mpi.c 

msr_core.o:   Makefile msr_core.c   msr_core.h 
msr_rapl.o:   Makefile msr_rapl.c   msr_rapl.h
msr_thermal.o: Makefile msr_thermal.c msr_thermal.h
signalCode.o: Makefile signalCode.c signalCode.h
signalPower.o: Makefile signalPower.c signalPower.h
signalCombined.o: Makefile signalCombined.c signalCombined.h

thermalTest.c: thermal.w
	./wrap.py -g -o thermalTest.c thermal.w

powerTest.c: power.w
	./wrap.py -g -o powerTest.c power.w

powerThermalTest.c: powerThermal.w
	./wrap.py -g -o powerThermalTest.c powerThermal.w

clean:
	rm -f *.o *.so thermalTest.c powerTest.c powerThermalTest.c


	

  


