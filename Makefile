target=msr
library=libmsr.so

DEFINES=-DARCH_SANDY_BRIDGE -DPKG_PERF_STATUS_AVAILABLE 

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

libmsr: blr_util.o msr_core.o msr_turbo.o msr_pebs.o msr_clocks.o 
	$(CC) -fPIC -shared -Wl,-soname,$(library) -o ./$(library) $^

msr_core.o:   Makefile                       msr_core.c   msr_core.h 
msr_pebs.o:   Makefile msr_core.o            msr_pebs.c   msr_pebs.h 
msr_turbo.o:  Makefile msr_core.o            msr_turbo.c  msr_turbo.h 
msr_clocks.o: Makefile msr_core.o            msr_clocks.c msr_clocks.h
blr_util.o:   Makefile                       blr_util.c   blr_util.h 
msr_rapl.o:   Makefile		             msr_rapl.c   msr_rapl.h
clean:
	rm -f *.o $(library)

rapl: Makefile libmsr msr_rapl.o
	gcc -o rapl msr_rapl.o -L. -lmsr
