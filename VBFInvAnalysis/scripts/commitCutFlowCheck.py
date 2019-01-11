import ROOT
import sys

def Compare(raw_old,raw_new):


    if not raw_old or not raw_new:
        print 'Could not load histograms: ',raw_old,' new: ',raw_new
        sys.exit(0)
    
    for i in range(1,raw_old.GetNbinsX()):
        diff_ratio = raw_old.GetBinContent(i)- raw_new.GetBinContent(i)
        ratio = diff_ratio/raw_old.GetBinContent(i)
        if raw_old.GetXaxis().GetBinLabel(i)!=raw_new.GetXaxis().GetBinLabel(i):
            print 'Cut order changed. exiting...failure'
            sys.exit(0)
        print 'Cut: %s             %0.1f %0.1f            Diff:  %0.1f Ratio:  %0.3f' %(raw_old.GetXaxis().GetBinLabel(i), raw_old.GetBinContent(i), raw_new.GetBinContent(i), diff_ratio, ratio)
        if abs(ratio)>0.005:
        #if abs(ratio)>0.5:
            print 'Cutflow differs by more than 0.5\%. Exiting'
            sys.exit(0)


fold = ROOT.TFile.Open('root://fax.mwt2.org:1094//pnfs/uchicago.edu/atlaslocalgroupdisk/rucio/user/schae/b0/84/user.schae.15625195._000001.MiniNtuple.root')
fnew = ROOT.TFile.Open('root://fax.mwt2.org:1094//pnfs/uchicago.edu/atlaslocalgroupdisk/rucio/user/schae/4d/e4/user.schae.16569393._000001.MiniNtuple.root')

if not fold or not fnew:
    print 'Could not load files: ',fold,' new: ',fnew
    sys.exit(0)

#
# Load the raw histograms for cutflow
#
raw_old = fold.Get('cutflowhisto_raw_Nominal')
raw_new = fnew.Get('cutflowhisto_raw_Nominal')
print 'Raw counts!'
Compare(raw_old, raw_new)

#
# Load the weighted histograms for cutflow
#
w_old = fold.Get('cutflowhisto_Nominal')
w_new = fnew.Get('cutflowhisto_Nominal')
print 'Weighted counts!'
Compare(w_old, w_new)

fnew.Close()
fold.Close()
print 'Done! Congrats on your commit'

