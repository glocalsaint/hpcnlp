#!/bin/bash
#SBATCH -J hpc4nlp
#SBATCH --mail-user=glocalsaint@gmail.com
#SBATCH --mail-type=ALL
#SBATCH -e /home/vv52zasu/mpi/error.err
#SBATCH -o /home/vv52zasu/mpi/output.out
#SBATCH -n 256     # Number of tasks
#SBATCH -c 1  #Number of cores per process(task)
#SBATCH --mem-per-cpu=4000  # Main memory in MByte per MPI task
#SBATCH -t 240     # Hours, minutes and seconds, or '#SBATCH -t 10' - only minutes
cd /home/vv52zasu/mpi/
mpirun -np 256 hash
