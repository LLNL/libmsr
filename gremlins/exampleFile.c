
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */

#ifdef MPICH_HAS_C2F
_EXTERN_C_ void *MPIR_ToPointer(int);
#endif // MPICH_HAS_C2F

#ifdef PIC
/* For shared libraries, declare these weak and figure out which one was linked
   based on which init wrapper was called.  See mpi_init wrappers.  */
#pragma weak pmpi_init
#pragma weak PMPI_INIT
#pragma weak pmpi_init_
#pragma weak pmpi_init__
#endif /* PIC */

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

static int in_wrapper = 0;
#include <stdio.h>
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <sched.h>
#include <string.h>
#include <errno.h>

#include "msr_core.h"
#include "msr_rapl.h"
#include "msr_thermal.h"
#include "msr_misc.h"
#include "msr_turbo.h"
#include "msr_counters.h"
#include "msr_thermal.h"
#include "msr_clocks.h"

static struct timeval startTime;
static int init = 0;

static struct itimerval tout_val;
static int rank;
static int size;

static char hname[1025];
static struct rapl_limit P0_1, P0_2, P0_DRAM, P1_1, P1_2, P1_DRAM;
static int watts=115; 
static int retVal = -1;
static struct rapl_limit lim; 
long dummy=0;
int cpuid, procsPerPackage;
char entry[3];


/*These need to be global as we are taking deltas*/
static struct rapl_data r1,r2,r3,r4;

int get_env_int(const char *name, int *val);
void get_cpuinfo_entry(int processor, char key[], char value[]);
void printData(int i);
void printDataUninterrupted(char *foo);
FILE* getFileID();

static int fortran_init = 0;
/* ================== C Wrappers for MPI_Init ================== */
_EXTERN_C_ int PMPI_Init(int *arg_0, char ***arg_1);
_EXTERN_C_ int MPI_Init(int *arg_0, char ***arg_1) { 
    int _wrap_py_return_val = 0;
    if (in_wrapper) return PMPI_Init(arg_0, arg_1);
    in_wrapper = 1;

	    if (fortran_init) {
#ifdef PIC
        if (!PMPI_INIT && !pmpi_init && !pmpi_init_ && !pmpi_init__) {
            fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init function.  Link against static library instead.\n");
            exit(1);
        }        switch (fortran_init) {
        case 1: PMPI_INIT(&_wrap_py_return_val);   break;
        case 2: pmpi_init(&_wrap_py_return_val);   break;
        case 3: pmpi_init_(&_wrap_py_return_val);  break;
        case 4: pmpi_init__(&_wrap_py_return_val); break;
        default:
            fprintf(stderr, "NO SUITABLE FORTRAN MPI_INIT BINDING\n");
            break;
        }
#else /* !PIC */
        pmpi_init_(&_wrap_py_return_val);
#endif /* !PIC */
    } else {
        _wrap_py_return_val = PMPI_Init(arg_0, arg_1);
    }

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	r1.flags=0;
	r2.flags=0;
	r3.flags=0;
	r4.flags=0;	

	retVal = get_env_int("PROCS_PER_PACKAGE",&procsPerPackage);

        //cpuid = sched_getcpu();
/*	if(retVal<0){
	        get_cpuinfo_entry(cpuid,"siblings",entry);
		procsPerPackage = atoi(entry); //value of siblings is stored as procsPerPackage
		fprintf(stderr,"PROCS_PER_PACKAGE not set! Assuming %d processor per package. Set environment vaiable!\n",procsPerPackage);
		
        }
*/
	if(rank % procsPerPackage == 0)
	{
		tout_val.it_interval.tv_sec = 0;
        	tout_val.it_interval.tv_usec = 0;
        	tout_val.it_value.tv_sec = 30;
        	tout_val.it_value.tv_usec = 0;

		FILE *writeFile = getFileID();
		fprintf(writeFile, "Hostname: %s\n", hname);
	
        struct rapl_data * rd;
        uint64_t * rapl_flags;
		init_msr();	
        rapl_init(&rd, &rapl_flags);

        printf("qqq1\n");
		retVal = get_env_int("POWER_CAP", &watts);
		if(retVal==-1) {
			fprintf(writeFile, "No power cap specified. Using default of 115W per socket\n");
		}

		if(retVal ==0){
			fprintf(writeFile, "Power cap specified. Setting limit1 on both sockets to %d W for minimal time window\n", watts);
		
            fprintf(writeFile, "Setting limit to:\n");
            dump_rapl_limit(&lim, writeFile);
			lim.watts = watts;
            lim.seconds = 1;
            lim.bits = 0;
            set_pkg_rapl_limit(0, &lim, NULL);
            set_pkg_rapl_limit(1, &lim, NULL);
		}
	

        printf("qqq2\n");
        get_pkg_rapl_limit(1, &P1_1, &P1_2);
        get_pkg_rapl_limit(0, &P0_1, &P0_2);
		//get_rapl_limit(0,&P0_1, &P0_2, &P0_DRAM);
		//get_rapl_limit(1, &P1_1, &P1_2, &P1_DRAM, &rapl_flags);
		
        printf("qqq2.1\n");
		fprintf(writeFile, "PKG0, Limit1\n");
		dump_rapl_limit(&P0_1, writeFile);

        printf("qqq2.2\n");
		fprintf(writeFile, "PKG1, Limit1\n");
		dump_rapl_limit(&P1_1, writeFile);

        printf("qqq2.3\n");
        poll_rapl_data(0, NULL);
        poll_rapl_data(1, NULL);
        sleep(10);
        poll_rapl_data(0, NULL);
        poll_rapl_data(1, NULL);

        printf("qqq2.4\n");
        // TODO: why???
		//fclose(writeFile);
	
        printf("qqq2.5\n");
        fprintf(writeFile, "Made 2\n");
		//Initial Dump
		char str[5]="Init";
		printDataUninterrupted(str);       
        fprintf(writeFile, "Made 3\n");
	
        printf("qqq2.6\n");
		setitimer(ITIMER_REAL, &tout_val, 0);
		signal(SIGALRM, printData);
	}

    printf("qqq3\n");
    rapl_finalize();

	PMPI_Barrier(MPI_COMM_WORLD);

    in_wrapper = 0;
    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Init =============== */
static void MPI_Init_fortran_wrapper(MPI_Fint *ierr) { 
    int argc = 0;
    char ** argv = NULL;
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Init(&argc, &argv);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INIT(MPI_Fint *ierr) { 
    fortran_init = 1;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init(MPI_Fint *ierr) { 
    fortran_init = 2;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init_(MPI_Fint *ierr) { 
    fortran_init = 3;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init__(MPI_Fint *ierr) { 
    fortran_init = 4;
    MPI_Init_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Init ================= */





/* ================== C Wrappers for MPI_Finalize ================== */
_EXTERN_C_ int PMPI_Finalize();
_EXTERN_C_ int MPI_Finalize() { 
    int _wrap_py_return_val = 0;
    if (in_wrapper) return PMPI_Finalize();
    in_wrapper = 1;


	PMPI_Barrier(MPI_COMM_WORLD);

	if(rank % procsPerPackage == 0)
	{
		tout_val.it_interval.tv_sec = 0;
        	tout_val.it_interval.tv_usec = 0;
        	tout_val.it_value.tv_sec = 0;
        	tout_val.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tout_val, 0);

		//Final Dump
		char str[5]="Fini";
		printDataUninterrupted(str);       

		finalize_msr();
	}
    printf("qqq4\n");
	_wrap_py_return_val = PMPI_Finalize();
    in_wrapper = 0;
    return _wrap_py_return_val;
}

/* =============== Fortran Wrappers for MPI_Finalize =============== */
static void MPI_Finalize_fortran_wrapper(MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Finalize();
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FINALIZE(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize_(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize__(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Finalize ================= */




int get_env_int(const char *name, int *val){
 	char *str=NULL;
        str = getenv(name);
        if(str ==NULL){
          *val = -1;
          return -1;
        }
        *val = (int) strtol(str, (char **)NULL, 0);
        return 0;
}

void get_cpuinfo_entry(int processor, char key[], char value[]){
        int i;
        FILE *fp;
        int keylen = strlen(key);
        char buf[1024];
        char *tok;
        fp = fopen("/proc/cpuinfo", "r");
        for( i = 0; i <= processor; i++){
                do{
                        fgets(buf, sizeof(buf), fp);
                }while(strncmp(key, buf, keylen) != 0);
        }
        tok = strtok(buf,":");
        tok = strtok(NULL,":"); // only works because the lines are in the format "description : value"
        strcpy(value,tok);
        fclose(fp);
}

void printData(int i)
{
	signal(SIGALRM, printData);
	struct timeval currentTime;
	struct itimerval tout_val;
	

	if(!init)
	{
		init = 1;
		gettimeofday(&startTime, NULL);
	}


	FILE *writeFile = getFileID();

	gettimeofday(&currentTime, NULL);
	double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;


	fprintf(writeFile, "Timestamp: %3.2lf\n", timeStamp);	

	fprintf(writeFile, "CoreTemp: ");
	dump_thermal_terse(writeFile);	
	fprintf(writeFile, "\n");

	fprintf(writeFile, "Power PKG0/DRAM0/PKG1/DRAM1: TODO");
    // TODO: fix this
	dump_rapl_terse(writeFile);
	fprintf(writeFile, "\n");

	fprintf(writeFile, "ThreadFreq APERF/MPERF/TSC (16 Threads): ");
	enable_fixed_counters();
	dump_clocks_terse(writeFile);
	fprintf(writeFile, "\n");

	fprintf(writeFile, "ThreadCounters UCC/IR/URC (16 Threads): ");
 	dump_fixed_terse(writeFile);
	fprintf(writeFile, "\n");

	tout_val.it_interval.tv_sec = 0;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 30;
	tout_val.it_value.tv_usec = 0;
	
	fclose(writeFile);

	setitimer(ITIMER_REAL, &tout_val, 0);
	
}

void printDataUninterrupted(char *foo)
{
	struct timeval currentTime;
	if(!init)
	{
		init = 1;
		gettimeofday(&startTime, NULL);
	}
	

	FILE *writeFile = getFileID();

	gettimeofday(&currentTime, NULL);
	double timeStamp = (double)(currentTime.tv_sec-startTime.tv_sec)+(currentTime.tv_usec-startTime.tv_usec)/1000000.0;
	fprintf(writeFile, "%s \n", foo);	
	fprintf(writeFile, "Timestamp: %3.2lf\n", timeStamp);	

	fprintf(writeFile, "CoreTemp: ");
	dump_thermal_terse(writeFile);	
	fprintf(writeFile, "\n");

	fprintf(writeFile, "Power PKG0/DRAM0/PKG1/DRAM1: TODO");
    // TODO: fix this
	dump_rapl_terse(writeFile);
	fprintf(writeFile, "\n");

	fprintf(writeFile, "ThreadFreq APERF/MPERF/TSC (16 Threads): ");
	enable_fixed_counters();
	dump_clocks_terse(writeFile);
	fprintf(writeFile, "\n");

	fprintf(writeFile, "ThreadCounters UCC/IR/URC (16 Threads): ");
 	dump_fixed_terse(writeFile);
	fprintf(writeFile, "\n");

	fclose(writeFile);
	
}

FILE* getFileID(){
	FILE* writeFile;
	char *filePath=NULL;

	gethostname(hname, 1024);

	filePath = getenv("LIBMSR_NODE_PATH");

  	if (filePath != NULL){
   	char fileName[4096];
      	sprintf(fileName, "%s%s%s%d%s", filePath, hname,"_",rank, "_msr.out");
	writeFile = fopen(fileName, "a");
 
      	if (writeFile == NULL){
        fprintf(stderr, "Failure in opening file for %s. Errno: %d. We are just going to use stdout as our output now\n", hname, errno);
       	if (errno == ENOENT){
        	fprintf(stderr, "It appears as though the directory path to the file does not exist (%s). Try ensuring it exists, then try again.\n", filePath);
            	}
           	writeFile = stdout;
         }
  	}

 	else {
        	fprintf(stderr, "It appears as though the directory to dump per-node power readings isn't set. Please set the environment variable LIBMSR_NODE_PATH to be the correct path. Now we are just going to use stdout as our output\n");
        	writeFile = stdout;
  	}

return writeFile;

}
