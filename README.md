#############################################
# VBF Higgs to Invisible Full Run 2 Analysis
#############################################

[![pipeline status](https://gitlab.cern.ch/othrif/STAnalysisCode/badges/master/pipeline.svg)](https://gitlab.cern.ch/othrif/STAnalysisCode/commits/master)

## Developer ##
Othmane Rifki (othmane.rifki@cern.ch) - DESY

### Notes ###
- Code runs on DAOD_EXOT5 using the `VBFInv` algorithm
- Code runs on DAOD_TRUTH3 using `VBFInvTruth` algorithm
- Output is a flat ntuple called `MiniNtuple`
- CutBookkeeper output for histogram normalization is in `NumberEvents` histogram

### Cutflow ###
- Cutflow comparisons here: https://docs.google.com/spreadsheets/d/1f3oW_cepPiYlOh_BJNviGTWO6lgxOzUfLOOB-cvS1B8/edit#gid=1239952533

## Setting up the code ##
``` bash
kinit $USER@CERN.CH
setupATLAS
lsetup git
git clone https://gitlab.cern.ch/VBFInv/STAnalysisCode.git
source STAnalysisCode/VBFInvAnalysis/scripts/setupRelease.sh
```

## Every login ##
``` bash
setupATLAS
source setup.sh
```
###  To compile for the first time or if you add files ###
``` bash
cbuild
# alias	for: cd	build && cmake ../STAnalysisCode && make && source */setup.sh && cd ../run
```

### To compile after modifying files (no new files)###
```
build
# alias for: cd build && make && source */setup.sh && cd ../run
```

### test run ###
``` bash
# local with exact path to file in debug mode
runVBF.py -f <path/file.root> -n100 --debug
# local with folder containing datasets (one per subdirectory)
runVBF.py -i <folder> -n100
# local running on TRUTH3, need to specifiy the algorithm (-a)
runVBF.py -i <folder> -n100 -a VBFInvTruth
# grid submission with file containing list of datasets (one per line)
runVBF.py -l <rucio list> -u $USER -v <Ntuple version> --driver prun
```
See more options in the runVBF.py help:
``` bash
runVBF.py -h
```

### test SUSYTools ###
``` bash
SUSYToolsTester <myAOD.pool.root> maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log
```

### make lists for all DAOD_EXOT5 samples ###
# To make for MC (change p-tag and version accordingly!)
``` bash
source setup.sh
source STAnalysisCode/VBFInvAnalysis/scripts/setupRelease.sh
source STAnalysisCode/VBFInvAnalysis/scripts/listMCSamples.sh v02 p3575 MC
```
# To make for data (change p-tag and version accordingly!)
``` bash
source STAnalysisCode/VBFInvAnalysis/scripts/listMCSamples.sh v02 p3576 data
```
# To create a new dir vNEW in STAnalysisCode/VBFInvAnalysis/share/samples/
``` bash
source STAnalysisCode/VBFInvAnalysis/scripts/listMCSamples.sh vNEW p3576 data m
```