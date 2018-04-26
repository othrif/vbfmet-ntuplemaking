
[![pipeline status](https://gitlab.cern.ch/VBFInv/STAnalysisCode/badges/master/pipeline.svg)](https://gitlab.cern.ch/VBFInv/STAnalysisCode/commits/master)

#############################################
# VBF Higgs to Invisible Full Run 2 Analysis
#############################################

This is the analysis code for the Full Run 2 VBF analysis based on SUSYTools.

## First time
```
setupATLAS
git clone --recursive https://:@gitlab.cern.ch:8443/VBFInv/STAnalysisCode.git
mkdir build run
cd build/
asetup 21.2.25,AnalysisBase
cmake ../STAnalysisCode
make
source */setup.sh
cd ../run
<run example>
```

## Every login
```
setupATLAS
asetup --restore
source */setup.sh
cd ../run
<run example>
```