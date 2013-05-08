target=msr
library=libmsr.so

DEFINES=-DARCH_SANDY_BRIDGE -DPKG_PERF_STATUS_AVAILABLE 
MYLIBDIR=$(HOME)/local/research/libmsr
# Machine- and compiler-specific information goes here:
-include localConfig.d/tlcc2Config

ifneq ($(dbg),)
DEFINES +=-D_DEBUG=$(dbg) -g -pg
else
DEFINES +=-O2
endif

CFLAGS=-fPIC -Wall ${DEFINES} ${COMPILER_SPECIFIC_FLAGS}
CC=gcc

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
	rm -f *.o $(library)

libwg.so: wrap.py Wgremlin.w Makefile
	./wrap.py -f -g -o Wgremlin.c Wgremlin.w
	mpicc -c -fPIC Wgremlin.c
	mpicc -fPIC -g -shared -Wl,-rpath,$(MYLIBDIR) -Wl,-soname,libwg.so -o libwg.so Wgremlin.o

app: libmsr libwg.so app.c 
	mpicc -c -Wall app.c	
	mpicc -L$(MYLIBDIR) -o app app.o -lmsr -lwg 

