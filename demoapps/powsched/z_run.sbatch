#! /bin/bash
###### SBATCH
#SBATCH -N 2
#SBATCH -p pdebug
#SBATCH -O
#SBATCH -t 0:01:30
#SBATCH -o z_run.txt
#SBATCH --msr-safe

export POWSCHED_BOUND=$((115*4))
export POWSCHED_THRESHOLD=5
export POWSCHED_RECLAIM=.75
export POWERACTUATOR_BOUND=$((115*4))

echo "Starting..."
srun sh $HOME/wlfix.sh
date
export PATH=$PATH:$HOME/linux_local/bin:$HOME/catalyst_local/bin
export EXPOSE_MASTER_PORT=8001
export EXPOSE_MASTER_HOST=`scontrol show hostname $SLURM_NODELIST | head -n 1`

echo "$EXPOSE_MASTER_HOST:$EXPOSE_MASTER_PORT"

srun -n1 -N1 -r0 sh start.sh &
sleep 1
scontrol show hostname $SLURM_NODELIST | uniq | while read a; do echo "insert into cluster values ('$a','1')"; done | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT


srun --ntasks-per-node=1 powmon haltwait &

echo "starting actuators"
srun --ntasks-per-node=1 power_actuator &
srun --ntasks-per-node=1 power_monitor &
sleep 1
echo "starting powsched"
srun -n1 -N1 -r0 powsched &

sleep 30


echo "shutting down"
echo "insert into kickactuator values ('shutdown')" | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
sleep 1
echo "insert into kickactuator values ('poweractuator')" | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
srun --ntasks-per-node=1 $HOME/linux_local/bin/shutdown
sleep 2

date
echo "Done"

