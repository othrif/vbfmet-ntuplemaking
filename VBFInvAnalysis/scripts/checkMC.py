#!/usr/bin/env python  
import ROOT
import getMCAll
import sys
"""
Use rucio to get the EVNT and AOD information based on a DSID
./checkMC.py
Outputs:
- Default prints tid, containers, and total in each campaign
- "-i" inputs histogram
- Examples:
Produce just AOD's: python checkMC.py -i fout_v15.root
"""

# Command line argument
def commandline():
  import argparse
  # Take some arguments from the command line:
  parser = argparse.ArgumentParser( description = "Use rucio to check downloaded files", add_help=True )
  parser.add_argument("-c","--campaign",dest="campaign",action="store",      default="MC16a",help="input campaign MC15c,MC16a,MC16c,MC16d,MC16e (default: %default)")
  parser.add_argument("-i","--input",dest="input",action="store",      default="fout_v15.root",help="input root file for normalization")
  parser.add_argument("-a","--aod",  dest='aod',  action='store_true', default=False,                    help='only prints AODs')
  args, unknown = parser.parse_known_args()
  return args.campaign.split(','), args.input

# Main function
def main(argv=None):
    campaigns, inputFileName = commandline();
    fin = ROOT.TFile.Open(inputFileName,'READ')

    from rucio.client import DIDClient
    client = DIDClient()
    dataFileIn = open('STAnalysisCode/VBFInvAnalysis/data/samples/v19_data15_16_EXOT5.txt','r')
    dataFileInList=[]
    for d in dataFileIn:
	dataFileInList+=[d]
    if not fin:
        print 'Could not find file: ',inputFileName
        return;
    inputHist = fin.Get('h_total')
    if not inputHist:
        print 'Could not find the h_total histogram'
        return
    missing_samples=[]
    for i in range(1, inputHist.GetNbinsX()):
        print inputHist.GetXaxis().GetBinLabel(i),' ',inputHist.GetBinContent(i)

        raw_input_events = inputHist.GetBinContent(i)
        dsid = inputHist.GetXaxis().GetBinLabel(i)
        summary=True
	if len(dsid)>0 and dsid[0]=='0':
	    print dsid
	    #raw_input_events = inputHist.GetBinContent(4)
            for d in dataFileInList:
		if d.count(dsid):
		    nEvt=0
		    print 'Checking: ',d
                    dsnInfo  = DIDClient.list_files(client,d.split('.')[0],d.strip('\n'))
		    for data in dsnInfo:
                        if data['events'] is not None:
                             nEvt+= data['events']
		    print nEvt,' in AOD and local: ',raw_input_events
		    if raw_input_events!=nEvt:
	   	        print 'ERROR events do not match: ',d
		    break;
	    continue
        aod_mc_map,evnt_mc_map,deriv_mc_map = getMCAll.run(dsid, campaigns, summary, False, False, True, False, False)
        for c in campaigns:
          if c not in aod_mc_map:
            print 'Could not find the campaign: ',c,' for DSID: ',dsid,' skipping...'
            sys.stdout.flush()
            continue
          print 'Campaign: ',c,' DSID: ', dsid, ' AODs: ',aod_mc_map[c],' Local: ',raw_input_events

          if raw_input_events!=aod_mc_map[c]:
            diff = (aod_mc_map[c]-raw_input_events)
            frac=-1.0
            if aod_mc_map[c]>0.0:
              frac=raw_input_events/aod_mc_map[c]
            missing_sample_line = '  ----> Event numbers do NOT match. Please check your download for %s. Fraction of Events downloaded %0.4f. Difference in evetns: %s' %(dsid,frac,diff)
            if c in deriv_mc_map:
              if deriv_mc_map[c]>0:
                frac_deriv = raw_input_events/deriv_mc_map[c]
                missing_sample_line+=' Fraction of Derivation: %0.4f. Derivation: %s AOD: %s Local: %s' %(frac_deriv, deriv_mc_map[c], aod_mc_map[c], raw_input_events)
            print missing_sample_line
            missing_samples+=[missing_sample_line]
          sys.stdout.flush()
    print 'SUMMARY of missing samples!!!'
    for i in missing_samples:
      print i
    print 'DONE'
import sys
if __name__ == '__main__':
  sys.exit(main())
