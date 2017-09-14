#!/bin/bash
num_procs=()
num_procs+=("2")
num_procs+=("3")
num_procs+=("5")
num_procs+=("7")
num_procs+=("9")
# on a pbs/torque environment, run pro node
if [ -z "$PBS_O_WORKDIR" ]
then
    FLAGS=""
else 
    FLAGS="$--pernode"
fi

for i in `seq 0 4`;
do
  # workx=$((2**$i))
  N_PROCS=${num_procs[$i]}
  MPI_R="mpirun -np $N_PROCS ./perf_test"
  printf "\n%d processes:\n" $N_PROCS
  $MPI_R -w 1280 -h 720
  printf "\n"
  $MPI_R -w 1920 -h 1080
  printf "\n"
  $MPI_R -w 2560 -h 1440
  printf "\n"
  $MPI_R -w 3840 -h 2160
  printf "\n"
done