libmsr_test:
	gcc -o libmsr_test libmsr_test.c -L ../lib -lmsr

rzoz: libmsr_test
	srun -p quad_sb -n 1 ./libmsr_test

