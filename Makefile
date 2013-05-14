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
LIBDIR=./
INCDIR=./
APPCFLAGS= -g
CFLAGS=-fPIC -Wall -g
CC=gcc

DEFINES=-DARCH_SANDY_BRIDGE -DPKG_PERF_STATUS_AVAILABLE 
MYLIBDIR=$(HOME)/local/research/libmsr
# Machine- and compiler-specific information goes here:
-include localConfig.d/tlcc2Config

ifneq ($(dbg),)
DEFINES +=-D_DEBUG=$(dbg) -g -pg 
else
DEFINES +=-O2
endif
DEFINES +=-DUSE_MPI

CFLAGS=-fPIC -Wall ${DEFINES} ${COMPILER_SPECIFIC_FLAGS}
CC=mpicc

all: libmsr libwg app cleanup

libmsr: blr_util.o msr_core.o msr_turbo.o msr_pebs.o msr_clocks.o msr_rapl.o
	$(CC) -fPIC -g -shared -Wl,-rpath,$(MYLIBDIR) -Wl,-soname,libmsr.so -o libmsr.so $^

msr_core.o:   Makefile                       msr_core.c   msr_core.h 
msr_pebs.o:   Makefile msr_core.o            msr_pebs.c   msr_pebs.h 
msr_turbo.o:  Makefile msr_core.o            msr_turbo.c  msr_turbo.h 
msr_clocks.o: Makefile msr_core.o            msr_clocks.c msr_clocks.h
blr_util.o:   Makefile                       blr_util.c   blr_util.h 
msr_rapl.o:   Makefile		             msr_rapl.c   msr_rapl.h
clean:
	rm -f *.o $(library)

libwg: wrap.py Wgremlin.w Makefile
	./wrap.py -f -g -o Wgremlin.c Wgremlin.w
	mpicc -c -fPIC Wgremlin.c
	mpicc -fPIC -g -shared -Wl,-rpath,$(MYLIBDIR) -Wl,-soname,libwg.so -o libwg.so Wgremlin.o

app_blr: libmsr libwg.so app.c 
	mpicc -DUSE_MPI -c -Wall app.c	
	mpicc -L$(MYLIBDIR) -o app app.o -lmsr -lwg 
app_pebs: libpebs app.o
	$(CC) $(APPCFLAGS) -O0 -Wall -o $@ app.o -L${LIBDIR} -lmsr -lpebs 

libpebs: libmsr msr_pebs.o 
	$(CC) -I$(INCDIR) -fPIC -shared -Wl,-soname,libpebs.so -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -o libpebs.so msr_pebs.o -lmsr
	
cleanup: libpebs cleanup.o
	$(CC) $(APPCFLAGS) -O0 -Wall -o $@ cleanup.o -L${LIBDIR} -lmsr -lpebs 

app.o:		app.c Makefile
	$(CC) $(APPCFLAGS) -c -I$(INCDIR) -o $@ $< 
cleanup.o:		cleanup.c Makefile
	$(CC) $(APPCFLAGS) -c -I$(INCDIR) -o $@ $< 
  


