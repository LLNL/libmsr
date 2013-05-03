#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include<msr_pebs.h>

static uint64_t *stomp_buf;
static const uint64_t stomp_sz          = (uint64_t)1024 * 1024 * 64;
int iters = 1024;
int pagesize = 0;

static void
init_stomp(){
        // 512MB
        stomp_buf = calloc( (size_t)stomp_sz, sizeof(uint64_t) );
        assert(stomp_buf);
        pagesize = sysconf(_SC_PAGE_SIZE);
        if (pagesize == -1)
           printf("Something is wrong -- pagesize is -1 in init_stomp");
}

static void
stomp(){
        uint64_t i,j;

        for(j=0; j<iters; j++){
                for(i=stomp_sz-(pagesize+1); i>0; i=i-(pagesize+1)){
                        stomp_buf[0]++;
                }
        }
}

int
main(int argc, char **argv){

        init_stomp();

        pebs_init();
       
        //start taking measurements 
        pebs_start();
        //function that stomps on memory
        stomp();
        //stop taking measurements 
        pebs_stop();

        pebs_finalize();
        
	return 0;
}


