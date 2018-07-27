#!/usr/bin/env python

# Initial imports
import argparse
import ROOT
import re
import os
import shutil
import sys

# Take some arguments from the command line:
parser = argparse.ArgumentParser( description = "VBF invisible ntuple making job", add_help=True , fromfile_prefix_chars='@')

# Submit job
parser.add_argument( "-s", "--submitDir", type = str, dest = "submitDir", default = "submitDir", help = "Submission directory" )
parser.add_argument( "-v", "--version", type = str, dest = "version", default = "vXX", help = "Ntuple version in the grid" )
parser.add_argument( "-f", "--inputFile", type = str, dest = "input", default = "", help = "Input file to process" )
parser.add_argument( "-t", "--inputList", type = str, dest = "txt", default = "", help = "Comma-separated list of text files containing file paths" )
parser.add_argument( "-i", "--inputDirs", type = str, dest = "dir", default = "", help = "Comma-separated list of input directories (they will be scanned for samples, one per subdirectory)" )
parser.add_argument( "-g", "--inputRucio", type = str, dest="rucio", default = "", help = "Comma-separated list of input Rucio datasets" )
parser.add_argument( "-l", "--inputRucioLists", type = str, dest="ruciolist", default = "", help = "Comma-separated list of text files containing one Rucio sample per line (empty lines or lines starting with # are ignored)" )
parser.add_argument( "-n", "--nevents", type = int, dest = 'nmax', help="Maximum number of events to process for all the datasets")
parser.add_argument( "-k", "--nskip", type = int, dest = "nskip", help="Number of events to skip for all the datasets")
parser.add_argument( "-r", "--replicationSite", type = str, dest = "replicationSite", default = "DESY-HH_LOCALGROUPDISK", help="Name of disk where to replicate output of grid jobs" )
parser.add_argument( "-w", "--overwrite", dest = "overwrite", action = "store_false", default = True, help = "don't overwrite previous submitDir")
parser.add_argument( "-d", "--driver", type = str, dest = "driver", default = "local", choices = ["local", "prun"], help = "driver to be used (local, prun)", metavar="driver")
parser.add_argument( "-a", "--algo", type = str, dest = "algoName", default = "VBFInv", choices = ["VBFInv","VBFInvTruth"], help = "algorithm name (e.g. VBFInv, VBFInvTruth)")
parser.add_argument( "-u", "--user", type=str, dest="userName", default=os.environ["USER"], help="username for grid jobs", metavar="userName")

# Algorithm configuration
parser.add_argument("--debug", dest='debug', action='store_true', default=False, help='run in debug mode in ST')
parser.add_argument("--verbose", dest='verbose', action='store_true', default=False, help='run in verbose mode with my output')
parser.add_argument("--configFile", type=str, dest="configFile", default="VBFInvAnalysis/vbfinv.conf", help="name of the config file to use", metavar="configFile")
parser.add_argument("--doSyst", dest="doSystematics", action="store_true", default=False, help="do systematic variations")
parser.add_argument("--noPileup", dest="noPileup", action="store_true", default=False, help="no pileup reweighting")
parser.add_argument("--doSkim", dest="doSkim", action="store_true", default=False, help="Apply skimming")
parser.add_argument("--pt1Skim", type=float, dest="pt1Skim", default=25000, help="leading jet pt skim (nominal tree), in MeV", metavar="cut")
parser.add_argument("--pt1SkimForSyst", type=float, dest="pt1SkimForSyst", default=25000, help="leading jet pt skim (systematics), in MeV", metavar="cut")
parser.add_argument("--pt2Skim", type=float, dest="pt2Skim", default=25000, help="subleading jet pt skim (nominal tree), in MeV", metavar="cut")
parser.add_argument("--pt2SkimForSyst", type=float, dest="pt2SkimForSyst", default=25000, help="subleading jet pt skim (systematics), in MeV", metavar="cut")
parser.add_argument("--metSkim", type=float, dest="metSkim", default=100000, help="MET skim (nominal tree), in MeV", metavar="cut")
parser.add_argument("--metSkimForSyst", type=float, dest="metSkimForSyst", default=100000, help="MET skim (systematics), in MeV", metavar="cut")
parser.add_argument("--mjjSkim", type=float, dest="mjjSkim", default=200000, help="jet invariant mass skim (nominal tree), in MeV", metavar="cut")
parser.add_argument("--mjjSkimForSyst", type=float, dest="mjjSkimForSyst", default=200000, help="jet invariant mass skim (systematics), in MeV", metavar="cut")
parser.add_argument("--detajjSkim", type=float, dest="detajjSkim", default=2.5, help="jet DEta skim (nominal tree)", metavar="cut")
parser.add_argument("--detajjSkimForSyst", type=float, dest="detajjSkimForSyst", default=2.5, help="jet DEta skim (systematics)V", metavar="cut")
parser.add_argument("--doElectronDetail", dest="doElectronDetail", action="store_true", default=False, help="add detailed branches for electrons")
parser.add_argument("--doMuonDetail", dest="doMuonDetail", action="store_true", default=False, help="add detailed branches for muons")
parser.add_argument("--doJetDetail", dest="doJetDetail", action="store_true", default=False, help="add detailed branches for jets")
parser.add_argument("--doMETDetail", dest="doMETDetail", action="store_true", default=False, help="add detailed branches for met")
parser.add_argument("--doEventDetail", dest="doEventDetail", action="store_true", default=False, help="add detailed branches for event level info")
parser.add_argument("--skipCutBookKeper", dest="skipCutBookKeper", action='store_true', default=False, help="skip CutBookKeper")
parser.add_argument("--isMultiWeight", dest="isMultiWeight",action='store_true', default=False, help="activate MultiWeight mode")
parser.add_argument("--doRnS", dest="doRnS", action="store_true", default=False, help="do Rebalance and Smear on SUSY11")
args, unknown = parser.parse_known_args()

print "\nArguments used:"
print "------------------------------------------------"
print "Command entered: " , " ".join(sys.argv[:])
print "Configuration:"
print args
print "------------------------------------------------"
print "\n"

# Overwrite submitDir
if args.overwrite:
    shutil.rmtree(args.submitDir, True)

#Set up the job for xAOD access:
ROOT.xAOD.Init().ignore()

# Set up the sample handler object:
sh = ROOT.SH.SampleHandler()

if args.input:
    chain = ROOT.TChain( "CollectionTree" )
    chain.Add( args.input )
    sampleName = '.'.join(args.input.split('/')[-1].split('.')[:-3])
    sh.add( ROOT.SH.makeFromTChain( sampleName, chain ) )
getlist = lambda x: filter(lambda y: y != '', x.replace(' ', '').split(','))
txts = getlist(args.txt)
for txt in txts:
  sampleName = ''.join(txt.split('/')[-1].split('.')[:-1]) # /a/b/c/d.txt -> d
  print 'adding sample path from text file ',sampleName
  ROOT.SH.readFileList(sh, sampleName, txt)
rucios = getlist(args.rucio)
for rucio in rucios:
  print 'adding rucio dataset ',rucio
  ROOT.SH.scanRucio(sh, rucio)
ruciolists = getlist(args.ruciolist)
for ruciolist in ruciolists:
  print 'adding rucio datasets from text file ',ruciolist
  for rucioraw in open(ruciolist).readlines():
    rucio = rucioraw.rstrip('\n').replace(' ', '') # remove spaces from line
    if rucio.startswith('#') == False and rucio != '': # ignore comments / separators
      ROOT.SH.scanRucio(sh, rucio)
dirs = getlist(args.dir)
for dir in dirs:
  print 'adding directory with sample ',dir
  ROOT.SH.scanDir(sh, dir)
#check if it is AFII or Data:
regexps = { 'isAFII' : re.compile('a\d\d\d'),
             'isData': re.compile('data'),
 }
for isample in xrange(sh.size()):
  sample = sh.at(isample)
  print sample.name()
  for opt,this_re in regexps.items():
    state = False
    if this_re.search(sample.name()):
      state = True
    sample.setMetaDouble(opt, state)
    print 'Sample {name} has {opt} set to {val}'.format(name=sample.name(), opt=opt, val=state)
sh.setMetaString( "nc_tree", "CollectionTree" )
# print SampleHandler object
getattr(sh, 'print')()

# Create an EventLoop job:
job = ROOT.EL.Job()
job.sampleHandler( sh )

# Add the analysis algorithm:
alg = getattr( ROOT, args.algoName )()
alg.SetName( args.algoName )
# Algorithm args:
alg.outputName = "MiniNtuple"
alg.outputNameHist = "hist"
alg.debug = args.debug
alg.verbose = args.verbose
if( args.algoName == "VBFInv" ):
  alg.config_file = args.configFile
  alg.doPileup = not args.noPileup
  alg.doSystematics = args.doSystematics
  alg.doSkim = args.doSkim
  alg.pt1Skim = args.pt1Skim
  alg.pt1SkimForSyst = args.pt1SkimForSyst
  alg.pt2Skim = args.pt2Skim
  alg.pt2SkimForSyst = args.pt2SkimForSyst
  alg.metSkim = args.metSkim
  alg.metSkimForSyst = args.metSkimForSyst
  alg.mjjSkim = args.mjjSkim
  alg.mjjSkimForSyst = args.mjjSkimForSyst
  alg.detajjSkim = args.detajjSkim
  alg.detajjSkimForSyst = args.detajjSkimForSyst
  alg.doElectronDetail = args.doElectronDetail
  alg.doMuonDetail = args.doMuonDetail
  alg.doJetDetail = args.doJetDetail
  alg.doMETDetail = args.doMETDetail
  alg.doEventDetail = args.doEventDetail
  alg.doRnS = args.doRnS
elif ( args.algoName == "VBFInvTruth"):
  alg.skipCBK = args.skipCutBookKeper
  alg.MultiWeight = args.isMultiWeight
else:
  print("ERROR: You need to enter a valid algorithm name: \"VBFInv\" or \"VBFInvTruth\"")
  sys.exit()
job.algsAdd( alg )

# make sure we can read trigger decision
job.options().setString(ROOT.EL.Job.optXaodAccessMode, ROOT.EL.Job.optXaodAccessMode_class)
# print to screen information about xAOD variables used
# job.options().setDouble (ROOT.EL.Job.optXAODPerfStats, 1);
# job.options().setDouble (ROOT.EL.Job.optPrintPerFileStats, 1);

# limit number of events
if args.nmax > 0:
  job.options().setDouble(ROOT.EL.Job.optMaxEvents, args.nmax);
  print 'Jobs will process a maximum of {n} events'.format(n=args.nmax)
# limit number of events
if args.nskip > 0:
  job.options().setDouble(ROOT.EL.Job.optSkipEvents, args.nskip);
  print 'Jobs will skip {n} events'.format(n=args.nskip)

# define an output
output = ROOT.EL.OutputStream  ("MiniNtuple")
outputHist = ROOT.EL.OutputStream ("hist")
job.outputAdd (output);
job.outputAdd (outputHist)

# Run the job:
if (args.driver == 'local'):
    driver = ROOT.EL.DirectDriver()
    driver.submit( job, args.submitDir )
elif (args.driver == 'prun'):
    dset_name_mask = 'user.{user}.{tag}.%in:name[2]%.%in:name[3]%.%in:name[6]%'.format(user=args.userName, tag=args.version)
    print dset_name_mask, len(dset_name_mask)
    driver = ROOT.EL.PrunDriver()
    if args.doSystematics:
      driver.options().setString(ROOT.EL.Job.optGridNGBPerJob, '1')
      driver.options().setString('nc_optGridNfilesPerJob', '1')
      #--nGBPerJob=4
    else:
      driver.options().setString(ROOT.EL.Job.optGridNGBPerJob, '5')
      driver.options().setString('nc_optGridNfilesPerJob', '5')
      #--nGBPerJob=10
    driver.options().setString('nc_outputSampleName', dset_name_mask)
    driver.options().setString("nc_optGridDestSE","DESY-HH_LOCALGROUPDISK")
    if args.replicationSite != None:
        driver.options().setString('nc_destSE', args.replicationSite)
    driver.submitOnly(job, args.submitDir )
else:
    raise RuntimeError('Unrecognized driver option {opt}'.format(opt=args.driver))

