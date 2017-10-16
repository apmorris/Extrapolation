#!/bin/bash

mkdir  -p outputs/logs
mkdir  -p outputs/stat

N=0
samplename=mH400_mS100_lt5m_mc15_304811

while (($N < 100));
do 
echo job $N
echo "$PWD/runjob.sh $N $samplename" | qsub -q short -o $PWD/outputs/logs/job${N}.log -e $PWD/outputs/logs/job${N}.err 
echo "--> logfiles  $PWD/outputs/logs/${N}.log -e $PWD/outputs/logs/${N}.err "
(( N=$N+1 ))
done 
