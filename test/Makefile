libmsr_test: libmsr_test.c 
	gcc -Wall -o libmsr_test libmsr_test.c -L ../lib -lmsr

rzoz: libmsr_test
	srun -p quad_sb -n 1 --auto-affinity=start=0,verbose,cpt=1 ./libmsr_test

