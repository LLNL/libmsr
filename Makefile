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

DEFINES=-DARCH_SANDY_BRIDGE -DPKG_PERF_STATUS_AVAILABLE 
MYLIBDIR=$(HOME)/local/research/libmsr

all: libmsr 

libmsr: blr_util.o msr_core.o msr_turbo.o msr_pebs.o msr_clocks.o msr_rapl.o
	$(CC) -fPIC -g -shared -Wl,-rpath,$(MYLIBDIR) -Wl,-soname,libmsr.so -o libmsr.so $^

msr_core.o:   Makefile                       msr_core.c   msr_core.h 
msr_pebs.o:   Makefile msr_core.o            msr_pebs.c   msr_pebs.h 
msr_turbo.o:  Makefile msr_core.o            msr_turbo.c  msr_turbo.h 
msr_clocks.o: Makefile msr_core.o            msr_clocks.c msr_clocks.h
blr_util.o:   Makefile                       blr_util.c   blr_util.h 
msr_rapl.o:   Makefile		             msr_rapl.c   msr_rapl.h
clean:
	rm -f *.o *.so


	

  


