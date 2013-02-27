
rapl-dummy:
	mpicc -o rapl-dummy rapl-dummy.c -Xlinker -rpath=../lib -L../lib -lrapl
test-exec:
	mpicc -o test-exec test-exec.c -Xlinker -rpath=../lib -L../lib -lrapl

clean:
	rm -f rapl-dummy test-exec *.o

