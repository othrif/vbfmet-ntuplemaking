# VBF Higgs to Invisible Full Run 2 Analysis #

  * [Developer](#developer)
  * [Notes](#notes)
  * [Setting up the code](#setting-up-the-code)
      * [Every login](#every-login)
      * [To compile for the first time or if you add files](#to-compile-for-the-first-time-or-if-you-add-files)
      * [To compile after modifying files (no new files)](#to-compile-after-modifying-files-(no-new-files))
      * [Test run](#test-run)
      * [Check downloaded samples](#check-downloaded-samples)
      * [Test SUSYTools](#test-susytools)
      * [Make lists for all DAOD_EXOT5 samples](#make-lists-for-all-daod_exot5-samples)
  * [Code Development](#code-development)
      * [Check out latest version code](#check-out-latest-version-code)
      * [Working with a branch](#working-with-a-branch)
      * [How to commit](#how-to-commit)
      * [Format code](#format-code)
      * [Tag master code git](#tag-master-code-git)

# Developer #
Othmane Rifki (othmane.rifki@cern.ch) - DESY

## Contributors ##
Doug Schaefer, Ben Rosser, Christian Sander, Rui Zou

# Notes #
- Code runs on DAOD_EXOT5 using the `VBFInv` algorithm
- Code runs on DAOD_TRUTH3 using `VBFInvTruth` algorithm
- Code runs on DAOD_HIGG2D1 using the `VBFInv` algorithm (use the `ZHdarkPhoton` branch)
- Output is a flat ntuple called `MiniNtuple`
- CutBookkeeper output for histogram normalization is in `NumberEvents` histogram

# Setting up the code (for running on `DAOD_HIGG2D1`) #
``` bash
kinit $USER@CERN.CH
setupATLAS
lsetup git
git clone https://gitlab.cern.ch/VBFInv/STAnalysisCode.git
cd STAnalysisCode 
git checkout ZHdarkPhoton && cd -
source STAnalysisCode/VBFInvAnalysis/scripts/setupRelease.sh
```

## Every login ##
``` bash
setupATLAS
source setup.sh
```
##  To compile for the first time or if you add files ##
``` bash
cbuild
# alias	for: cd	build && cmake ../STAnalysisCode && make && source */setup.sh && cd ../run
```

## To compile after modifying files (no new files) ##
```
build
# alias for: cd build && make && source */setup.sh && cd ../run
```

## Test run ##
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

## Check downloaded samples ##
To check that you have downloaded all of the samples. Run
``` bash
checkSample.py -l data_MC_EXOT5.txt -i local_List.txt
```
`data_MC_EXOT5.txt` is the list of DAOD's submitted for processing with STAnalysisCode.
`local_List.txt` is the list of directories containing the output of STAnalysisCode (`ls -d path/*`)

## Test SUSYTools ##
``` bash
SUSYToolsTester <myAOD.pool.root> maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log
```

## Make lists for all DAOD_EXOT5 samples ##
To configure the script starting at Line 57 to:
- Select which DSIDs to list in the `OrderedDict` inside `listSamples.py`
- Determine the p-tags for data and MC you want to select
Then run:
``` bash
lsetup rucio
python STAnalysisCode/VBFInvAnalysis/scripts/listSamples.py -t mc16_13TeV,data15_13TeV,data16_13TeV -c mc16a,mc16d -d DAOD_EXOT5 -v v05 -s
```

# Code Development #

When developing new code, it is important to work on a seperate branch. Once the development is over you can make a merge request to
merged the changes into master. The CI tests  [.gitlab-ci.yml][.gitlab-ci.yml] will check that the code still compiles correctly before accepting the
changes. In the merge request, tag @othrif so that I get a notification to approve the merge.

## Check out latest version code ##
Make sure you have the latest version of STAnalysisCode checked out, otherwise you run into conflicts:
```bash
cd STAnalysisCode
git fetch origin
```

## Working with a branch ##
Create a topic branch with a meaningful name (here we used `master-my-topic` but consider using a meaningful name)
```bash
git checkout -b master-my-topic
```

## How to commit ##

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
``` bash
git push --set-upstream origin master-my-topic
```
Git will now print an output message with a link to submit the merge request. The request can be submitted also by accessing on the browser your forked repository on GitLab and selecting the relevant branch. Make sure to tag Othmane Rifki (@othrif).

## Format code ##

 To format code please use clang-format, which is based on the `.clang-format` file in this folder:
``` bash
clang-format -i *.h *.cxx
```

## Table of contents ##
If you edit the README, you can update the Table of Contents by running the following script:
``` bash
ruby tableOfcontent.rb
```

## Tag master code git ##
``` bash
git tag -a vXX-AB21p2pYY -m "Tag version vXX"
git push origin vXX-AB21p2pYY
```

## Cutflow check
This is run from my service account, and the password is set under settings+CI. A maintainer of the code can updated it.
If you intentially changed the cutflow, then you can update the reference file. This is done by
``` bash
runVBF.py -f root://eosuser.cern.ch//eos/user/s/schae/HInv/mc16_13TeV.308276.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4nu_MET125.deriv.DAOD_EXOT5.e6126_s3126_r9364_p3575/DAOD_EXOT5.14490426._000001.pool.root.1 --doMuonDetail --doElectronDetail  --doEventDetail --doSkim -n 5000
cp submitDir/data-MiniNtuple/mc16_13TeV.308276.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4nu_MET125.root /eos/user/s/schae/HInv/mc16_13TeV.308276.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4nu_MET125.deriv.DAOD_EXOT5.e6126_s3126_r9364_p3575/mc16_13TeV.308276.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125_ZZ4nu_MET125.root
```