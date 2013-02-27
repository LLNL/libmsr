#include <unistd.h>
#include <mpi.h>

int
main(int argc, char **argv){
	int rank, size;
	MPI_Init(&argc, &argv);
	sleep(10);
	MPI_Finalize();
	return 0;
}

