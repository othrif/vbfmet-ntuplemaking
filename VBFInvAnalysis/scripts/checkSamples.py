#!/usr/bin/env python

import pyAMI.client
import pyAMI.atlas.api as AtlasAPI
import glob
import ROOT
from optparse import OptionParser
import re
import sys


def getNumberEvents(dirName, bini=4):
    nEvents=0
    rfiles=glob.glob(dirName+"/*.root*")
    for rf in rfiles:
        tmpRfile=ROOT.TFile(rf)
        NEhist=tmpRfile.Get("NumberEvents")
        try:
            nEvents+=NEhist.GetBinContent(bini)
        except:
            print "WARNING: NumberEvents not found in",rf
            continue
    return int(nEvents)


def getDirNames(dirNameListFile):
    dirNames={} # Dict of dsid:dirName
    iFile=open(dirNameListFile)
    lines=iFile.readlines()
    for l in lines:
        l=l.strip("\n")
        dsid=0
        try:
            dsid=(l.split("/")[-1]).split(".")[3]
        except:
            print l
        dirNames[dsid]=l
    return dirNames


def main(fileList, samples):
    client = pyAMI.client.Client('atlas')
    AtlasAPI.init()

    dirDict=getDirNames(samples)

    listInput=open(fileList)
    inputDatasets=listInput.readlines()
    listToCheck=[]
    for ds in inputDatasets:
        if "#" in ds:
            continue
        dsid=ds.split(".")[1]
        ds=ds.strip("\n")
        tmp=AtlasAPI.list_datasets(client, patterns = [ds], fields = ['events'])
        try:
            inputEvents=int(tmp[0]['events'])
        except:
            print "WARNING: skipping", ds
            continue
        try:
            processedEvents=getNumberEvents(dirDict[dsid])
        except KeyError:
            print "WARNING: tihs sample may be missing:",dsid, ",not found in input samples", samples
            processedEvents=0
        if inputEvents==0:
            print "inputEvents==0 for",ds
            continue
        frac=float(processedEvents)/float(inputEvents)
        diff=inputEvents-processedEvents
        if diff>0:
            listToCheck += [dsid]
            missing_sample_line = '  ----> Event numbers do NOT match (%s/%s). Please check your download for %s. Fraction of Events downloaded %0.4f. Difference in events: %s' %(processedEvents,inputEvents,dsid,frac,diff)
            print missing_sample_line
    print "\nThe following list of samples is incomplete/missing:", ' '.join(map(str, listToCheck))
    print "============================================================================================================"
    for sp in listToCheck:
        for line in inputDatasets:
            if re.search(sp, line):
                print line,
    print "============================================================================================================"
if __name__ == "__main__":
    p = OptionParser()
    p.add_option('-l', '--list', type='string', default="TEST_EXOT5.txt", help='list of DAOD files')
    p.add_option('-i', '--input', type='string', default="newv19Samps.txt", help='list of downloaded samples')

    for option in p.option_list:
        if option.default != ("NO", "DEFAULT"):
            option.help += (" " if option.help else "") + "[default: %default]"
    (options, args) = p.parse_args()

    main(options.list, options.input)



