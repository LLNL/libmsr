# Based off of libWhen makefile

LIBDIR=./
INCDIR=./
APPCFLAGS= -g
CFLAGS=-fPIC -Wall -g
CC=g++

all: libTherm MSRthermal.o

libTherm: msr_fake.o 
	$(CC) -I$(INCDIR) -fPIC -shared -Wl,-soname,libTherm.so -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -ldl -o libTherm.so $^

msr_fake.o:	msr_fake.h Makefile msr_fake.c 

MSRthermal.o:	msrThermal.c Makefile
	gcc -Wall -o MSRthermal.o msrThermal.c
  
clean:
	rm -f *.o *.so


