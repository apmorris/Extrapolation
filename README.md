# Extrapolation

Extrapolating the final results between signal lifetimes. 

#### Steps to run the code:
1. Apply the selection to the signal samples using the GenerateROOTFiles.py script
2. Run the lifetime extrapolation on these slimmed files, with ExtrapolateByBeta
3. Calculate the extrapolated limits using ExtrapLimitFinder
4. Make limit plots, scripts for this are in the plots directory

---

### To clone this repository

```bash
git clone https://github.com/apmorris/Extrapolation.git
```

### Set up (lxplus)

```bash
. setuplxplus.sh
# Make sure local copy is up to date
git pull origin master
```

---
## Generate the slimmed MC files

The script `GenerateMCFiles/GenerateROOTFiles.py` applies the selection to our 
signal samples, outputting a root file in the right format for the extrapolation 
code to use.

```bash
# First compile the selection header (note: still need to make an overall Makefile)
cd GenerateMCFiles/
root -l CalRSelection.h+
# Then to run
python GenerateROOTFiles.py -s <SignalSampleFile> -o <OutputFile> -n <nEvents>
```

---
## Run the lifetime extrapolation on the slimmed files

The ExtrapolateByBeta code takes the slimmed file generated above, and performs an 
extrapolation over lifetimes, resulting in another root file containing histograms.

```bash
# First compile the package
cd ../ExtrapolateByBeta/
make clean
make
# Then to run
./ExtrapolateByBeta -m <SlimmedSampleFile> -f <OutputFile> -c <GeneratedLifetime>
```

---
## Calculate the extrapolated limits

The ExtrapLimitFinder code takes the extrapolated lifetime root file from above, and 
calculates the limits for the sample. The output is a collection of histograms which
can then be turned into the classic limit plots.

```bash
cd ../ExtrapLimitFinder/
make clean
make
./ExtrapLimitFinder -e <FileFromExtrap> -A <nObsA> -B <nObsB> -C <nObsC> -D <nObsD> -f <OutputFile> -a
```
_NB:_ systematic errors are currently hardcoded into LimitCommonCode/run_ABCD.cxx

> Code inherited from Gordon Watts, modified to work on lxplus.