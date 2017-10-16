#!/bin/sh
jobid=$1
samplename=$2

source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/user/atlasLocalSetup.sh

export LD_LIBRARY_PATH=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/git/2.11.1-x86_64-slc6/lib64:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/png/1.6.17-x86_64_slc6_gcc62_opt/1.6.17-1a97c/x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/tbb/44_20160413-x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/davix/0.6.2-x86_64-slc6-gcc62-opt/x86_64-slc6-gcc62-opt/lib64:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/4.6.1-x86_64-slc6-gcc62-opt/4.6.1-5209d/x86_64-slc6-gcc62-opt/lib64:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/xrootd/4.6.1-x86_64-slc6-gcc62-opt/4.6.1-5209d/x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/6.10.04-x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/fftw/3.3.4-x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/gsl/2.1-x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/boost/boost-1.62.0-python2.7-x86_64-slc6-gcc62/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/python/2.7.13-x86_64-slc6-gcc62/2.7.13-597a5/x86_64-slc6-gcc62-opt/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc620_x86_64_slc6/6.2.0/x86_64-slc6/lib64:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc620_x86_64_slc6/6.2.0/x86_64-slc6/lib

#lsetup "root 6.10.04-x86_64-slc6-gcc62-opt"

which gcc

cd /unix/atlas3/apmorris/DisplacedJets/ExtrapolationCode/Extrapolation/ExtrapLimitFinder/

#make clean
#make

cd limits/2015_v13_Toys

mkdir -p outputs/stat
mkdir -p outputs/files

rm outputs/stat/${samplename}.job${jobid}.fail
rm outputs/stat/${samplename}.job${jobid}.done

touch outputs/stat/${samplename}.job${jobid}.run

if ( ../../ExtrapLimitFinder -e ../../../ExtrapolateByBeta/extrap_${samplename}.root -A 24 -B 16 -C 39 -D 34 -f limit_${samplename}_${jobid}.root -n 100 -s $jobid ) then
  touch outputs/stat/${samplename}.job${jobid}.done

else 
  touch outputs/stat/${samplename}.job${jobid}.fail

fi
rm outputs/stat/${samplename}.job${jobid}.run
