CFLAGS=-fPIC -Wall -O3 -I../include -g
CC=gcc
MYLIBDIR=${HOME}/libmsr/lib
WRAP=../wrap/wrap.py

all: helloWorldMPI.o libPowThermTest 

libPowThermTest: exampleFile.c
	mpicc $(CFLAGS) -shared -Wl,-soname,libPowThermTest.so -o ../lib/libPowThermTest.so $^

helloWorldMPI.o: helloWorld_mpi.c libPowThermTest
	mpicc $(CFLAGS) -DPIC -o helloWorldMPI -Wl,-rpath=${MYLIBDIR} -L${MYLIBDIR} -lmsr -L${MYLIBDIR} -lPowThermTest helloWorld_mpi.c 

thermalTest.c: thermal.w
	$(WRAP) -g -o thermalTest.c thermal.w

powerTest.c: power.w
	$(WRAP) -g -o powerTest.c power.w

powerThermalTest.c: powerThermal.w
	$(WRAP) -f -g -o powerThermalTest.c powerThermal.w

exampleFile.c: exampleFile.w
	$(WRAP) -f -g -o exampleFile.c exampleFile.w
profile: 
	$(WRAP) -g -o PMPI_profile.c PMPI_profile.w
	mpicc -fPIC -Wall -c -I../include PMPI_profile.c
	mpicc $(CFLAGS) -shared -Wl,-soname,libgaramond.so -o ../lib/libgaramond.so -L../lib -lmsr PMPI_profile.o

clean:
	rm -f *.o *.so helloWorldMPI thermalTest.c powerTest.c powerThermalTest.c exampleFile.c
