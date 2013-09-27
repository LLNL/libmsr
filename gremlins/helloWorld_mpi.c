#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main (int argc, char *argv[])
{
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );	
	fprintf(stdout, "Hello from rank %d of %d.\n", rank, size);
	sleep(10);
	fprintf(stdout, "Goodbye from rank %d of %d.\n", rank, size);
	MPI_Finalize();
	return 0;
}
