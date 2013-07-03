/* Example code from Blaise Barney
 *
 * 
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  MASTER		0

int main (int argc, char *argv[])
{
int   numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];

MPI_Init(&argc, &argv);
printf("Hello world!");
MPI_Finalize();
return 0;

}
