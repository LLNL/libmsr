#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <stdint.h>

volatile uint64_t i,w;

int
main(int argc, char **argv){


	fprintf(stdout, "About to call MPI_Init...\n");
	MPI_Init(&argc, &argv);
	fprintf(stdout, "Returned from MPI_init...\n");

	for(i=0; i<(2048*2024LL*1024LL); i++){
		w += i;
	}
	
	MPI_Finalize();
	return 0;
}
