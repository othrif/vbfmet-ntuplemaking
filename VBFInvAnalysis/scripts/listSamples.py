# Author: Othmane Rifki
# Date: August 2018
# Script that handles production of EVNT, AOD, DAOD_EXOT5, DAOD_TRUTH
# For now just DAOD_EXOT5, rest comes later...

#!/usr/bin/env python

from collections import OrderedDict

mainMCTest = OrderedDict([
  ('signal',    [308276,308567,308284])
  ])

mainMC = OrderedDict([
    ('MoreVjets', range(364216, 364229 + 1)),
    ('signal',    [308276,308567,308284]),
    ('signalAlt', [308275] + range(308277,308283 + 1)),
    ('VH',        [308071,308072]),
    ('VV',        range(363355, 363360+1) + [363489]),
    ('EWK',       range(308092, 308098+1)),
    ('Wenu',      range(364170,364183 + 1)),
    ('Wmunu',     range(364156,364169 + 1)),
    ('Wtaunu',    range(364184,364197 + 1)),
    ('Zee',       range(364114, 364127 + 1)),
    ('Zmumu',     range(364100, 364113 +1)),
    ('Ztautau',   range(364128, 364141 + 1)),
    ('Znunu',     range(364142, 364155 + 1)),
    ('ZnunuMG',   range(361515, 361519 + 1)),
    ('ZtautauMG', range(361510, 361514 + 1)),
    ('ZmumuMG',   range(363123, 363146 + 1)),
    ('ZeeMG',     range(363147, 363170 + 1)),
    ('WmunuMG',   range(363624, 363647 + 1)),
    ('WenuMG',    range(363600, 363623 + 1)),
    ('WtaunuMG',  range(363648, 363671 + 1)),
    ('QCDw',      range(361020,361032 + 1)),
    ('QCDpl',     range(310502,310502 + 1)),
    ('top',       [410470,410472,410011,410012,410013,410014,410025,410026])
    ])

altMC = OrderedDict([
    ('ZllVBF',    [345101,345102,345099,345100]),
    ('ZllDY',     range(364198, 364215+1)),
    ('topAlt',    [410471]),
    ('VVrare',    [363494,364250,364254,364255] ),
    ('VVV',       range(364242,364249 + 1)),
    ('QCDunw',    range(426001,426009 + 1))
    ])

truthMC = OrderedDict([
    ('signal',    [308276,308567,308284]),
    ('signalAlt', [308275] + range(308277,308283 + 1)),
    ('EWK',       range(308092, 308098+1)),
    ('Wenu',      range(364170,364183 + 1)),
    ('Wmunu',     range(364156,364169 + 1)),
    ('Wtaunu',    range(364184,364197 + 1)),
    ('Zee',       range(364114, 364127 + 1)),
    ('Zmumu',     range(364100, 364113 +1)),
    ('Ztautau',   range(364128,364141 + 1)),
    ('Znunu',     range(364142,364155 + 1)),
    ])

mcCampaign = {"mc16a":"r9364", "mc16c":"r9781", "mc16d":"r10201", "mc16e":"r10724"}

# Modified by user:
#####################
pTagDict = { "data15_13TeV"                  : "p3576",
             "data16_13TeV"                  : "p3576",
             "data17_13TeV"                  : "p3481",
             #"data18_13TeV"                  : "p3679", # misses jet variables
             "data18_13TeV"                  : "p3583",
             #"mc16_13TeV"                    : "p3596"} # skimmed or unskimmed here
             #"mc16_13TeV"                    : "p3627"} # skimmed or unskimmed here. for mc16e
             #"mc16_13TeV"                    : "p3500"}
             "mc16_13TeV"                    : "p3575"}
#myMC = mainMCTest
myMC = mainMC
myMC.update(altMC)
#####################
#
# Command line arguments
def get_args():
  import argparse
  parser = argparse.ArgumentParser( description = "For MC, Use DSIDs to get EVNT, AOD, and DAOD sample lists. For data, Use data year to get AOD and DAOD sample lists", add_help=True)
  parser.add_argument("-t", "--scope", dest="scope", type=str, default="mc16_13TeV,data15_13TeV,data16_13TeV,data17_13TeV,data18_13TeV", help="MC Project tag (default: %default)")
  parser.add_argument("-c", "--campaigns", dest="campaigns", type=str, default="mc16a,mc16d,mc16e", help="Project campaign tag (default: %default)")
  parser.add_argument("-d", "--derivation", dest="derivation", type=str, default="DAOD_EXOT5", help="Derivation (default: %default)")
  parser.add_argument("-v", "--version", dest="version", type=str, default="vXX", help="tag name for output file (default: %default)")
  parser.add_argument("-s", "--save", dest='save', action='store_true', default=False, help='Save to output file (default: %default)')
  parser.add_argument("--debug", dest='debug', action='store_true', default=False,                    help='prints details for debugging (default: %default)')
  return parser.parse_args()

def main(argv=None):
  import re

  options = get_args()
  scope = options.scope.split(',')
  campaigns = options.campaigns.split(',')
  daod = options.derivation #options.derivation.split(',')
  version = options.version
  debug = options.debug
  save = options.save

  # Output director
  if version is not "vXX" and save:
    print(os.environ['TestArea'])
    path = os.environ['TestArea']+"/../STAnalysisCode/VBFInvAnalysis/data/samples/"
    if not os.path.exists(path):
      os.makedirs(path)
  else: path = "."

  outFile = {}
  if '_' in daod: daod_name = daod.split('_')[1]
  else: daod_name = daod
  for proj in scope:
    if save:
      if not 'data' in proj:
        #for campaign in campaigns: outFile.append(campaign:open(campaign+"_"+daod+".txt", "w"))
        for campaign in campaigns:
          outFile[campaign] = open(path+"/"+version+"_"+campaign+"_"+daod_name+".txt", "w")
      else:
        if '_' in proj: proj_name = proj.split('_')[0]
        outFile[proj] = open(path+"/"+version+"_"+proj_name+"_"+daod_name+".txt", "w")
        outFile["pilot"] = open(path+"/"+version+"_"+"pilot.txt", "w")

  print '---------------------------------------'
  print '-- Script Configuration --'
  print '---------------------------------------'
  print 'Project tags: ', scope
  print 'MC Campaigns ',  campaigns
  print 'Derivation ',    daod
  print 'p-tags ',        pTagDict
  print 'Version ',       version
  print '---------------------------------------'

  try:
    from rucio.client.didclient import DIDClient
    try:
      client = DIDClient()
    except:
      print "Failed to get DIDClient"
      return
  except:
    print "Please first setup rucio"
    return
  for proj in scope:
    if not 'data' in proj: isData = False
    else: isData = True
    if not isData:
      for campaign in campaigns:
        print "\nGetting %s list..." % campaign
        for groupname, runs in myMC.items():
          if save:
            outFile[campaign].write("### %s ######\n" % groupname)
          print '#searching ',groupname, ' datasets'
          for dsid in runs:
            name = proj + "." + str(dsid) + ".*."+daod+".*"+mcCampaign[campaign]+"*"+pTagDict[proj]
            dids = {'name': name}
            List = list(DIDClient.list_dids(client,proj,dids,'container'))
            if not List:
              print '# %s not found' % (name)
              if save:
                outFile[campaign].write("# %s not found\n" % (name))
            for dsn in List:
              pattern = re.compile(r'e\d\d\d\d_s\d\d\d\d_r\d*_p\d*')
              if not pattern.search(dsn):
                pattern_AFII = re.compile(r'e\d\d\d\d_a\d*_r\d*_p\d*')
                if not pattern_AFII.search(dsn):
                  continue
              if debug or not save:
                print dsn
              if save: outFile[campaign].write("%s\n" %dsn)
        if save: outFile["pilot"].write("%s\n" % dsn)
    else:
      print "Getting %s list..." % (proj)
      name = proj + ".*.physics_Main.*"+daod+"*"+pTagDict[proj]
      dids = {'name': name}
      List = list(DIDClient.list_dids(client,proj,dids,'container'))
      if not List:
        print '# %s not found' % (name)
        outFile[campaign].write("# %s not found\n" % (name))
      for dsn in List:
        if debug or not save:
          print dsn
        if save: outFile[proj].write("%s\n" % dsn)
      if save: outFile["pilot"].write("%s\n" % dsn)

import sys, os
if __name__ == '__main__':
  sys.exit(main())
