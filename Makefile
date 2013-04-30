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
LIBDIR=${HOME}/local/research/libmsr
CFLAGS=-fPIC -Wall ${COMPILER_SPECIFIC_FLAGS}
CC=gcc

libmsr: blr_util.o msr_core.o msr_turbo.o msr_pebs.o msr_clocks.o 
	$(CC) -fPIC -shared -Wl,-soname,libmsr.so -o libmsr.so $^

libpebs: pebs.o 
	$(CC) -fPIC -shared -Wl,-soname,libpebs.so -Wl,-rpath=${LIBDIR} -L{LIBDIR} -o libpebs.so $^ -lmsr
	
app: app.o 
	$(CC) $(CFLAGS) -O0 -Wall -o $@ $^ -Wl,-rpath=${LIBDIR} -L{LIBDIR} -lpebs 


	$(CC) -Wall -o pebs pebs_harness.c -Wl,-rpath=${LIBDIR} -L${LIBDIR} -lmsr
msr_core.o:   	Makefile                       msr_core.c   msr_core.h 
msr_turbo.o:  	Makefile msr_core.o            msr_turbo.c  msr_turbo.h 
msr_clocks.o: 	Makefile msr_core.o            msr_clocks.c msr_clocks.h
blr_util.o:   	Makefile                       blr_util.c   blr_util.h 
app.o:		Makefile app.c
pebs.o:   	Makefile msr_core.o            msr_pebs.c   msr_pebs.h 
	
clean:
	rm -f *.o $(library)


