#!/bin/bash
module load intel/13.0 mvapich2-intel-psm/1.7
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/shoga1/lib
srun --nodes=1-1 --ntasks=2 --ntasks-per-node=2 -w venus1 -ppbatch  --auto-affinity=start=0,verbose,cpt=1 ./helloWorldMPI.o > data.txt
chown shoga1.shoga1 data.txt

