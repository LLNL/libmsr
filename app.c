#include <stdio.h>
#include <stdint.h>


static uint64_t *stomp_buf;
static const uint64_t stomp_sz          = (uint64_t)1024 * 1024 * 64;
int iters = 1024;

static void
init_stomp(){
        // 512MB
        stomp_buf = calloc( (size_t)stomp_sz, sizeof(uint64_t) );
        assert(stomp_buf);
}

static void
stomp(){
        uint64_t i,j;

        for(j=0; j<iters; j++){
                for(i=stomp_sz-4097; i>0; i=i-4097){
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


