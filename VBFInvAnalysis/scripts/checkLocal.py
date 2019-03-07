#!/usr/bin/env python

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

p = OptionParser()
p.add_option('-i', '--input', type='string', default="newv19Samps.txt", help='list of downloaded samples')
for option in p.option_list:
    if option.default != ("NO", "DEFAULT"):
        option.help += (" " if option.help else "") + "[default: %default]"
(options, args) = p.parse_args()
samples=options.input
print samples
dirDict=getDirNames(samples)
for ds in dirDict:
    processedEvents=getNumberEvents(dirDict[ds])
    print processedEvents
