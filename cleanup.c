#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include<msr_pebs.h>


int
main(int argc, char **argv){

        pebs_init();

        pebs_stop();

        pebs_finalize();
        
	return 0;
}


