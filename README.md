#############################################
# VBF Higgs to Invisible Full Run 2 Analysis
#############################################

[![pipeline status](https://gitlab.cern.ch/othrif/STAnalysisCode/badges/master/pipeline.svg)](https://gitlab.cern.ch/STAnalysisCode/commits/master)

## Developer ##
Othmane Rifki (othmane.rifki@cern.ch) - DESY

### Notes ###
- Code runs on DAOD_EXOT5 using the `VBFInv` algorithm
- Code runs on DAOD_TRUTH3 using `VBFInvTruth` algorithm
- Output is a flat ntuple called `MiniNtuple`
- CutBookkeeper output for histogram normalization is in `NumberEvents` histogram

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
To configure the script starting at Line 57 to:
- Select which DSIDs to list in the `OrderedDict` inside `listSamples.py`
- Determine the p-tags for data and MC you want to select
Then run:
``` bash
lsetup rucio
python STAnalysisCode/VBFInvAnalysis/scripts/listSamples.py -t mc16_13TeV,data15_13TeV,data16_13TeV -c mc16a,mc16d -d DAOD_EXOT5 -v v05 -s
```

### Tag master code git ###
``` bash
git tag -a vXX-AB21p2pYY -m "Tag version vXX"
git push origin vXX-AB21p2pYY
```

## Code Development ##

When developing new code, it is important to work on a seperate branch. Once the development is over you can make a merge request to
merged the changes into master. The CI tests  [.gitlab-ci.yml][.gitlab-ci.yml] will check that the code still compiles correctly before accepting the
changes. In the merge request, tag @othrif so that I get a notification to approve the merge.

### Check out latest version code ###
Make sure you have the latest version of STAnalysisCode checked out, otherwise you run into conflicts:
```bash
cd STAnalysisCode
git fetch origin
```

### Working with a branch ###
Create a topic branch with a meaningful name (here we used `master-my-topic` but consider using a meaningful name)
```bash
git checkout -b master-my-topic upstream/master --no-track
```

### How to commit ###

For each of the files you want to commit, do
```bash
git add <file>
```
double-check the status of the repository with
```bash
git status
```
Modifications can be listed with
``` bash
git diff STAnalysisCode
```
and commit locally
```bash
git commit -m "message"
```
Push the changes from your local repository to your fork with
```bash
git push --set-upstream origin master-my-topic
```
Git will now print an output message with a link to submit the merge request. The request can be submitted also by accessing on the browser your forked repository on GitLab and selecting the relevant branch. Make sure to tag Othmane Rifki (@othrif).