CFLAGS=-fPIC -Wall -O3 -I../include -g
CC=gcc
MYLIBDIR=../lib
WRAP=../wrap/wrap.py

all: helloWorldMPI.o libPowThermTest 

libPowThermTest: powerThermalTest.c
	mpicc $(CFLAGS) -shared -Wl,-soname,libPowThermTest.so -o ../lib/libPowThermTest.so $^

helloWorldMPI.o: helloWorld_mpi.c libPowThermTest
	mpicc $(CFLAGS) -DPIC -o helloWorldMPI -Wl,-rpath=/home/shoga1/libmsr -L ${MYLIBDIR} -lmsr -L${MYLIBDIR} -lPowThermTest helloWorld_mpi.c 

thermalTest.c: thermal.w
	$(WRAP) -g -o thermalTest.c thermal.w

powerTest.c: power.w
	$(WRAP) -g -o powerTest.c power.w

powerThermalTest.c: powerThermal.w
	$(WRAP) -g -o powerThermalTest.c powerThermal.w

clean:
	rm -f *.o *.so thermalTest.c powerTest.c powerThermalTest.c


	

  


