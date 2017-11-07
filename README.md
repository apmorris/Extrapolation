# Extrapolation

Extrapolating the final results between signal lifetimes. 

#### Steps to run the code:
1. Apply the selection to the signal samples using the `GenerateMCFiles/GenerateROOTFiles2017.py` script
2. Run the lifetime extrapolation on these slimmed files, with `ExtrapolateByBeta`
3. Plot the efficiency as a function of lifetime, using `plots/efficiencies.py`
4. Calculate the extrapolated limits using `ExtrapLimitFinder`
5. Make limit plots, with `plots/limits.py`

---

### To clone this repository

```bash
git clone https://github.com/apmorris/Extrapolation.git
```

### Set up (lxplus)

```bash
setupATLAS
lsetup "root 6.10.04-x86_64-slc6-gcc62-opt"
lsetup git
# Make sure local copy is up to date
git pull
```

---
## Generate the slimmed MC files

The script `GenerateMCFiles/GenerateROOTFiles2017.py` applies the selection to our 
signal samples, outputting a root file in the right format for the extrapolation 
code to use.

```bash
# First compile the selection macro (note: still need to make an overall Makefile)
cd GenerateMCFiles/
root -l CalRSelection2017.C+
# Then to run
python GenerateROOTFiles2017.py -s <SignalSampleFile> -o <OutputFile> -m <Selection> -n <nEvents>
```

`-s` Path to the sample file which is to be slimmed

`-o` Desired name of the slimmed file, usually slim_mH***_mS***_dv**.root

`-m` Selection to use, set to 1 for low-mass samples and 2 for high-mass samples

`-n` Number of events to process, for all events set to -1

---
## Run the lifetime extrapolation on the slimmed files

The `ExtrapolateByBeta` code takes the slimmed file generated above, and performs an 
extrapolation over lifetimes, resulting in another root file containing histograms.

```bash
# First compile the package
cd ../ExtrapolateByBeta/
make clean
make
# Then to run
./ExtrapolateByBeta -m <SlimmedSampleFile> -f <OutputFile> -c <GeneratedLifetime>
```

`-m` Name of the slimmed file, see previous step

`-f` Desired name of the file containing the extrapolation results, usually 
extrap_mH***_mS***_dv**.root

`-c` Generated ctau of the sample, look up in `GenerateMCFiles/Sample Meta Data.csv` 
or in the Note

This step takes ~3 hours to run for slimmed samples of ~390k events, so it's 
recommended to use a batch system, or a lot of patience.

---
## Make the efficiency plots

The pyROOT macro `efficiencies.py` reads all files containing the extrapolation results 
matching a certain pattern in a directory, gets the h_res_eff_A histograms and automatically 
plots them.

```bash
cd ../plots/
python efficiencies.py -p <PathToFiles> -m <ChainOfMasses> -o <PlotName>
```

`-p` Path to the directory containing the extrapolation results, see previous step

`-m` Full-stop separated list of mass points to consider, e.g. mH400_mS100.mH600_mS150.mH1000_mS150

`-o` File name for output efficiency plot 

---
## Calculate the extrapolated limits

The `ExtrapLimitFinder` code takes the extrapolated lifetime root file from above, and 
calculates the limits for the sample. The output is a collection of histograms which
are used to make the final limit plots.

```bash
cd ../ExtrapLimitFinder/
make clean
make
./ExtrapLimitFinder -e <FileFromExtrap> -A <nObsA> -B <nObsB> -C <nObsC> -D <nObsD> -f <OutputFile> -a [-E <ABCDerror> -L <Luminosity>]
```

`-e` Name of the file with the extrapolation info, see previous step

`-A` Number of data events observed in region A (set to 0 if blinded)

`-B -C -D` Number of data events in regions B, C and D

`-f` Desired name of the file containing the limit histograms, usually limit_mH***_mS***_dv**.root

`-a` Use asymptotic fit rather than toys (toys are slow!)


_NB:_ Make sure systematic errors are up to date in `main.cxx`

---
## Make the final limit plots

The pyROOT macro, `limits.py` makes a limit plot with the result of the ExtrapLimitFinder
code, in the correct ATLAS style.

```bash
cd ../plots/
python limits.py -e <LimitFile> -H <mH> -S <mS> -p <PlotName>
```

`-e` Name of the file with the limit histograms, see previous step

`-H` Mass of heavy boson (GeV)

`-S` Mass of scalar (GeV)

`-p` File name for output limit plot, usually limit_mH***_mS***_dv**.pdf


> Code inherited from Gordon Watts, modified to work on lxplus.