#############################################
# VBF Higgs to Invisible Full Run 2 Analysis
#############################################

[![pipeline status](https://gitlab.cern.ch/othrif/STAnalysisCode/badges/master/pipeline.svg)](https://gitlab.cern.ch/othrif/STAnalysisCode/commits/master)


## Developer ##
Othmane Rifki: othmane.rifki@cern.ch

## Setting up the code ##
``` bash
kinit $USER@CERN.CH
setupATLAS
lsetup git
git clone https://:@gitlab.cern.ch:8443/othrif/STAnalysisCode.git
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
# local with ASG test file (local file not needed) in debug mode
runVBF.py -f $ASG_TEST_FILE_MC -n100 --debug
# local with folder containing datasets (one per subdirectory)
runVBF.py -i <folder> -n100
# local running on TRUTH3, need to specifiy the algorithm (-a)
runVBF.py -i <folder> -n100 -a VBFInvTruth
# grid submission with file containing list of datasets (one per line)
runVBF.py -l <rucio list> -u $USER -v <Ntuple version> --driver grid
```
See more options in the runVBF.py help:
``` bash
runVBF.py -h
```

### test SUSYTools ###
``` bash
SUSYToolsTester <myAOD.pool.root> maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log
```
