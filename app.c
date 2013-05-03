#include <stdio.h>
#include <stdint.h>

int
main(int argc, char **argv){
	uint64_t i, j, k;
	for(i=0; i<10000; i++){
		for(j=0; j<10000; j++){
			k+= i*j+(i-j)*(j-i);
		}
	}
	return 0;
}
