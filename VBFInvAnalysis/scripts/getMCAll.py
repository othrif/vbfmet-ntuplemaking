#!/usr/bin/env python
# Author: Othmane Rifki

"""
Use rucio to get the EVNT and AOD information based on a DSID
./getMCAll.py -d <DSID> -c <SubCampaigns>
Outputs:
- Default prints tid, containers, and total in each campaign
- "-s" prints summary of each campaign for AOD and the mc15 EVNT container
- "-o" only prints a row of AOD/DAOD(ratio) for each campaign, for mass running
- "-v" prints details for debugging
- Examples:
Produce just AOD's: python getMCAll.py -d 364114 -o -a 
Produce just EVNT's: python getMCAll.py -d 364114 -o -e
Produce just ratio: python getMCAll.py -d 364114 -o -r
"""

# Command line arguments
def commandline():
  import argparse
  # Take some arguments from the command line:
  parser = argparse.ArgumentParser( description = "Author: Othmane Rifki \nUse rucio to get the EVNT and AOD information based on a DSID", add_help=True )
  parser.add_argument("-d","--dsid",    dest="dsid",    action="store",      default="364114",                 help="input DSID (default: %default)")
  parser.add_argument("-c","--campaign",dest="campaign",action="store",      default="MC15c,MC16a,MC16c,MC16d,MC16e",help="input campaign (default: %default)")
  parser.add_argument("-s","--summary", dest='summary', action='store_true', default=False,                    help='prints summary of each campaign for AOD and the mc15 EVNT container (default: %default)')
  parser.add_argument("-o","--output",  dest='output',  action='store_true', default=False,                    help='sets up output mode (default: %default)')
  parser.add_argument("-r","--ratio",  dest='ratio',  action='store_true', default=False,                    help='only prints a row of AOD/DAOD(ratio) for each campaign, for mass running (default: %default)')
  parser.add_argument("-a","--aod",  dest='aod',  action='store_true', default=False,                    help='only prints AODs')
  parser.add_argument("-e","--evnt",  dest='evnt',  action='store_true', default=False,                    help='only prints EVNTs')
  parser.add_argument("-v","--verbose", dest='verbose', action='store_true', default=False,                    help='prints details for debugging (default: %default)')
  args, unknown = parser.parse_known_args()
  summary  = args.summary
  output   = args.output
  ratio    = args.ratio
  aod      = args.aod
  evnt     = args.evnt
  verbose  = args.verbose
  if not args.dsid:
    parser.error("Give 6-digit DISD number as argument")
  return args.dsid, args.campaign.split(','), summary, output, ratio, aod, evnt, verbose
def safe_div(x,y):
    if y == 0:
        return 0
    return x / y

# Main function
def main(argv=None):
  import pprint
  import re
  pp = pprint.PrettyPrinter(indent=1)
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
  if argv is None: argv = sys.argv
  dsid, campaigns, summary, output, ratio, aod, evnt, verbose = commandline();
  allflags = not ( output or ratio or aod or evnt or verbose) 
  if verbose:
    print (__doc__)

  if not ( output or summary):
    print "=================================== TIDs ==================================="
  ############################################################
  # EVNT
  ############################################################
  if not ( output or summary):
    print "---- EVNT ----"
  scope = "mc15_13TeV"
  count = 0
  evnt_mc_map = {"Name":"mySAMPLE","shortName":"mySAMPLE", "TOT":0, "MC15c":0, "MC16a":0, "MC16c":0, "MC16d":0, "MC16e":0}
  name = scope + "." + dsid + ".*.EVNT.*"
  dids = {'name': name}
  List = DIDClient.list_dids(client,scope,dids,'container')
  for dsn in List:
    short_name = dsn.split(".")[1]+"."+dsn.split(".")[2]
    count += 1
    container = dsn
    contevents=0
    contcampaigns=[]
    matchcampaign=False
    if ':' in container:
      scope = container.split(':')[0]
      container = container.split(':')[1]
    else:
      scope = container.split('.')[0]
    contents = DIDClient.list_content(client,scope,container)
    for task in contents:
      name = task['name']
      if verbose: print name
      scope = name.split('.')[0]
      meta = DIDClient.get_metadata(client,scope,name)
      if verbose: pp.pprint(meta)
      if ':' in meta['campaign']:
        campaign = meta['campaign'].split(':')[1]
      else:
        campaign = meta['campaign']
      if verbose: print campaign
      if campaign in campaigns:
          matchcampaign=True
      #campaign = meta['campaign'].split(':')[1]
      dsnInfo  = DIDClient.list_files(client,scope,name)
      events = 0
      for data in dsnInfo:
        if data['events'] is not None:
          events += int(data['events'])
      contevents+=events
      if verbose: print events
      # print campaign, events, tid
      if not ( output or summary):
        print '*EVNT %10s %10s %s' % (campaign, str(events), name)
      # Store events in each campaign
      evnt_mc_map["Name"] = dsn
      evnt_mc_map["shortName"] = short_name
      if campaign not in contcampaigns: contcampaigns.append(campaign)
      if ",".join(contcampaigns) not in evnt_mc_map:
        evnt_mc_map[",".join(contcampaigns)]=0
      evnt_mc_map[campaign] += events
    if matchcampaign:
        evnt_mc_map["TOT"] += contevents
  if count == 0: print "No evgen.EVNT containers for", dsid, "found [", name, "]"
  ############################################################
  # AOD
  ############################################################
  if not ( output or summary):
    print "---- AOD ----"
  count = 0
  aod_mc_map = {"Name":"mySAMPLE","shortName":"mySAMPLE", "TOT":0, "MC16a":0, "MC16c":0, "MC16d":0, "MC16e":0}
  aod_name_map = {"MC16a":"", "MC16c":"", "MC16d":"", "MC16e":""}
  scope = "mc16_13TeV"
  name = scope + "." + dsid + ".*.recon.AOD.*"
  dids = {'name': name}
  List = DIDClient.list_dids(client,scope,dids,'container')
  for dsn in List:
    pattern = re.compile(r'AOD.e\d\d\d\d_[s|a]\d{3,4}_r*\d')
    if verbose: print dsn, "\n > pattern AOD.eXXXX_a|sYYYY_rZZZZ is found: ", (pattern.search(dsn) is not None)
    if not pattern.search(dsn):
      continue
    short_name = dsn.split(".")[1]+"."+dsn.split(".")[2]
    count += 1
    container = dsn
    contevents=0
    contcampaigns=[]
    matchcampaign=False
    if ':' in container:
      scope = container.split(':')[0]
      container = container.split(':')[1]
    else:
      scope = container.split('.')[0]
    contents = DIDClient.list_content(client,scope,container)
    for task in contents:
      name = task['name']
      if verbose: print name
      scope = name.split('.')[0]
      meta = DIDClient.get_metadata(client,scope,name)
      if verbose: pp.pprint(meta)
      if ':' in meta['campaign']:
        campaign = meta['campaign'].split(':')[1]
      else:
        campaign = meta['campaign']
      if verbose: print campaign
      if campaign in campaigns:
          matchcampaign=True
      dsnInfo  = DIDClient.list_files(client,scope,name)
      events = 0
      if campaign not in contcampaigns: contcampaigns.append(campaign)
      if ( campaign == "MC16a" and "r9364" not in name ):
        if verbose: print "Found MC16a Sample but not r-tag!"
        continue
      if ( campaign == "MC16c" and "r9781" not in name ):
        if verbose: print "Found MC16c Sample but not r-tag!"
        continue
      if ( campaign == "MC16d" and "r10201" not in name ):
        if verbose: print "Found MC16d Sample but not r-tag!"
        continue
      for data in dsnInfo:
        events += int(data['events'])
      contevents+=events
      if verbose: print events
      # print campaign, events, tid
      if not ( output or summary):
        print '*AOD %10s %10s %s' % (campaign, str(events), name)
      # Store events in each campaign
      aod_mc_map["Name"] = dsn
      aod_mc_map["shortName"] = short_name
    if matchcampaign:
        aod_mc_map[",".join(contcampaigns)] += contevents
        aod_name_map[",".join(contcampaigns)] = container
        aod_mc_map["TOT"] += contevents
  if count == 0: print "No recon.AOD containers for", dsid, "found [", name, "]"

  ############################################################
  # Print Summary
  ############################################################
  if not ( output ):
    print "\n=================================== Summary ==================================="
  # EVNT
  if not ( output ):
    print "---- EVNT ----"
    print '#EVNT: %10s' % (str(evnt_mc_map["Name"]))
  if ( output and evnt and evnt_mc_map["Name"]):
    print '%10s' % (str(evnt_mc_map["Name"]))
  for key, value in evnt_mc_map.iteritems():
    if key in campaigns:
        if not ( output ):
            print '+EVNT %10s %10s ' % (key, str(value))
  if not ( output ): print '=EVNT %10s %10s ' % ("TOT", str(evnt_mc_map["TOT"]))
  # AOD
  if not ( output ):
    print "---- AOD ----"
    print '#AOD: %10s' % (str(aod_mc_map["shortName"]))
  for key, value in aod_mc_map.iteritems():
    if key in campaigns:
        if not ( output ):
            print '+AOD %10s %10s %10s' % (key, str(value), str(aod_name_map[key]))
        if  ( output and aod_name_map[key] and aod ):
            print '%s %s' % (key, str(aod_name_map[key]))
  if not ( output ):
    print '=AOD %10s %10s' % ("TOT", str(aod_mc_map["TOT"]))
  ############################################################
  # Print Output
  ############################################################
  if not ( output ):
    print "\n=================================== Final ==================================="
  if not ( output ):
    print "Short Name - Total EVNT  - AOD/EVNT(MC16a)  - AOD/EVNT(MC16c)   - AOD/EVNT(MC16d) -  AOD/EVNT(MC16e)"
  '''
  print '%10s %10s %10s/%s=%.2f %10s/%s=%.2f %10s/%s=%.2f %10s/%s=%.2f' % (evnt_mc_map["shortName"],
  '''

  if ( ratio or allflags):
    print '%s %s %s %s %.2f %s %s %.2f %s %s %.2f %s %s %.2f' % (evnt_mc_map["shortName"],
      str(evnt_mc_map["TOT"]/1.e6),
      str(aod_mc_map["MC16a"]/1e6), str((evnt_mc_map["MC16a"]+evnt_mc_map["MC15c"])/1e6), safe_div(float(aod_mc_map["MC16a"]),(evnt_mc_map["MC16a"]+evnt_mc_map["MC15c"])),
      str(aod_mc_map["MC16c"]/1e6), str(evnt_mc_map["MC16c"]/1e6), safe_div(float(aod_mc_map["MC16c"]),evnt_mc_map["MC16c"]),
      str(aod_mc_map["MC16d"]/1e6), str((evnt_mc_map["MC16d"]+evnt_mc_map["MC16c"])/1e6), safe_div(float(aod_mc_map["MC16d"]),(evnt_mc_map["MC16d"]+evnt_mc_map["MC16c"])),
      str(aod_mc_map["MC16e"]/1e6), str(evnt_mc_map["MC16e"]/1e6), safe_div(float(aod_mc_map["MC16e"]),evnt_mc_map["MC16e"])
      )


import sys
if __name__ == '__main__':
  sys.exit(main())
