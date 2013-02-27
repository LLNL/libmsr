/*Tapasya Patki
 * How to run sequential or openMP code
 * or another binary with librapl*/

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int
main( int argc, char **argv ){
 int return_val=0;


 char *const args[] = 
   {"/bin/ls", "-l", NULL};
 
  int child_status;
  pid_t child, cpid;
 
  MPI_Init(&argc, &argv);

  child = fork();
  if(child == 0){ /*I'm in the child process*/

  	return_val=execvp("/bin/ls", args);

 	if(return_val == -1){
    		printf("\nCould not execute binary");
	} 
  }  
  else{ 
	/*I'm in the parent process */
	/*Wait for th child to terminate*/
	do {
	  cpid = wait(&child_status);
	} while(cpid != child);
 
	MPI_Finalize();
 	return child_status;
    }
}
