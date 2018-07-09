#!/bin/bash

# Analysis base release

if [ "$1" == "" ]; then
    echo "listMCSamples.sh vtag(version tag) ptag(for derivation) MC/data m(mkdir)"
    return
fi
if [ "$2" == "" ]; then
    echo "listMCSamples.sh vtag(version tag) ptag(for derivation) MC/data m(mkdir)"
    return
fi

if [ "$3" == "MC" ]; then
    echo 'Making list for MC'
elif [ "$3" == "data" ]; then
    echo 'Making list for data'
else 
    echo "listMCSamples.sh vtag(version tag) ptag(for derivation) MC/data m(mkdir)"
    return
fi

if [ "$4" != "" ]; then
    echo 'Making dir' $1 'in STAnalysisCode/VBFInvAnalysis/share/samples/'
    mkdir -p $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1
fi

if [ "$3" == "MC" ]; then
    echo "Making list for mc16a for " $2
    rucio ls --short mc16_13TeV.*DAOD_EXOT5.*r9364_r9315_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/mc16a_EXOT5.txt
    echo "Making list for mc16c for " $2
    rucio ls --short mc16_13TeV.*DAOD_EXOT5.*r9781_r9778_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/mc16c_EXOT5.txt
    echo "Making list for mc16d for " $2
    rucio ls --short mc16_13TeV.*DAOD_EXOT5.*r10201_r10210_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/mc16d_EXOT5.txt
else
    echo "Making list for data15 for " $2
    rucio ls --short data15_13TeV.*DAOD_EXOT5.*_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/data15_EXOT5.txt
    echo "Making list for data16 for " $2
    rucio ls --short data16_13TeV.*DAOD_EXOT5.*_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/data16_EXOT5.txt
    echo "Making list for data17 for " $2
    rucio ls --short data17_13TeV.*DAOD_EXOT5.*_$2 > $TestArea/../STAnalysisCode/VBFInvAnalysis/share/samples/$1/data17_EXOT5.txt
fi