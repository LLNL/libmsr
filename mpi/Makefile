ICC_FLAGS=-diag-disable 177 -diag-disable 279 -diag-disable 181
GNU_FLAGS=

rapl-icc:
	mkdir -p ../lib
	./wrap.py -f -g -o rapl.c rapl.w
	mpicc $(ICC_FLAGS) -DPIC -fPIC -I.. -DARCH_SANDY_BRIDGE -DARCH_062D -c rapl.c -o rapl.o
	mpicc -shared -Wl,-soname,librapl.so -o ../lib/librapl.so ../msr_core.o ../msr_rapl.o ../msr_turbo.o ../blr_util.o ../msr_opt.o ../msr_clocks.o rapl.o ../msr_signal.o
rapl-gnu:
	mkdir -p ../lib
	./wrap.py -f -g -o rapl.c rapl.w
	mpicc $(GNU_FLAGS) -DPIC -fPIC -I.. -DARCH_SANDY_BRIDGE -DARCH_062D -c rapl.c -o rapl.o
	mpicc -shared -Wl,-soname,librapl.so -o ../lib/librapl.so ../msr_core.o ../msr_rapl.o ../msr_turbo.o ../blr_util.o ../msr_opt.o ../msr_clocks.o rapl.o ../msr_signal.o
clean:
	rm -f *.o rapl.c* 


