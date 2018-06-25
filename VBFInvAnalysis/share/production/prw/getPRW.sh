#!/bin/bash

# make sure to run:
# grid
# acmSetup AthAnalysis,21.2.16

dsids=$1
here=$PWD

# Make a list of the files
[ -e TEMP.txt ] && rm TEMP.txt
export prwdir="${here}/prwfiles"
if [ ! -d $prwdir ]; then
  mkdir -p $prwdir;
fi
ls /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/PRW_AUTOCONFIG/files > prw_exist.txt
ls $prwdir >> prw_exist.txt
for i in $(cat ${dsids});
do
    if grep -q $i prw_exist.txt;
        then
            echo "Found file with DSID $i in SUSYTools/PRW_AUTOCONFIG or LOCALLY!"
            echo "Moving to the next DSID..."
            continue
        else
            echo "File with DSID $i NOT FOUND, proceed to create it"
fi
    rucio list-dids mc16_13TeV:mc16_13TeV*${i}*merge*NTUP_PILEUP*/ --filter type=container | grep mc16 | awk -F "|" '{print $2}' | awk -F":" '{print $2}' >> TEMP.txt
done

# Get the files
while read line
do
    if echo $line | grep -Fq _r9364; then
    MCXXX=mc16a
    elif echo $line | grep -Fq _r9781; then
    MCXXX=mc16c
    elif echo $line | grep -Fq _r10201; then
    MCXXX=mc16d
    else
    MCXXX='mc16X'
    fi;
    DSID=`echo $line | awk -F"." '{print $2}'`
    printf "\n===============================================\n"
#    ls /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/SUSYTools/PRW_AUTOCONFIG/files > tmp_autoconf
    echo "Sample: $line"
    echo "MC campaign: $MCXXX"
    echo "DSID: $DSID"
    cd ${prwdir}
    rucio download $line
    export out_prw=pileup_${MCXXX}_dsid${DSID}.root
    rm -rf ${out_prw}
    hadd ${out_prw} $line/*.root*
    rm -rf $line
    echo $line > tmp_$line.txt
    echo "Now checking PRW..."
    checkPRW.py --inDsTxt=tmp_$line.txt ${out_prw}
    rm -f tmp_$line.txt
    cd ${here}
    printf "===============================================\n"
done < TEMP.txt
rm -f prw_exist.txt TEMP.txt