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
CFLAGS=-fPIC -Wall 
CC=gcc

all: app 

libmsr: blr_util.o msr_core.o msr_turbo.o msr_clocks.o 
	$(CC) -I$(INCDIR) -fPIC -shared -Wl,-soname,libmsr.so -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -o libmsr.so $^

libpebs: libmsr msr_pebs.o 
	$(CC) -I$(INCDIR) -fPIC -shared -Wl,-soname,libpebs.so -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -o libpebs.so msr_pebs.o -lmsr
	
app: libpebs app.o
	$(CC) $(CFLAGS) -O0 -Wall -o $@ app.o -Wl,-rpath=$(LIBDIR) -L${LIBDIR} -lmsr -lpebs 

msr_core.o:   	Makefile msr_core.c msr_core.h 
msr_turbo.o:  	Makefile msr_core.o msr_turbo.c  msr_turbo.h 
msr_clocks.o: 	Makefile msr_core.o msr_clocks.c msr_clocks.h
blr_util.o:   	Makefile blr_util.c blr_util.h 
msr_pebs.o:   	Makefile msr_core.o msr_pebs.c msr_pebs.h 

app.o:		app.c Makefile
	$(CC) -c -I$(INCDIR) -o $@ $< 
  
clean:
	rm -f *.o *.so


