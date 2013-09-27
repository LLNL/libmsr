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
MYLIBDIR=../lib
WRAP=../wrap/wrap.py

all: helloWorldMPI.o libPowThermTest libPowerTest

libThermTest: thermalTest.c
	mpicc -fPIC -g -shared  -Wl,-soname,libThermTest.so -o libThermTest.so $^

libPowerTest: powerTest.c
	mpicc -fPIC -g -shared -Wl,-soname,libPowerTest.so -o libPowerTest.so $^

libPowThermTest: powerThermalTest.c
	mpicc -fPIC -g -shared -Wl,-soname,libPowThermTest.so -o libPowThermTest.so $^

helloWorldMPI.o: helloWorld_mpi.c libmsr libThermTest
	mpicc -DPIC -o helloWorldMPI.o -Wl,-rpath=/home/shoga1/libmsr -L ${MYLIBDIR} -lmsr -L${MYLIBDIR} -lThermTest helloWorld_mpi.c 

thermalTest.c: thermal.w
	$(WRAP) -g -o thermalTest.c thermal.w

powerTest.c: power.w
	$(WRAP) -g -o powerTest.c power.w

powerThermalTest.c: powerThermal.w
	$(WRAP) -g -o powerThermalTest.c powerThermal.w

clean:
	rm -f *.o *.so thermalTest.c powerTest.c powerThermalTest.c


	

  


