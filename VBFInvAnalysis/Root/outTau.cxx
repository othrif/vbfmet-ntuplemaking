// Local include(s):
#include "VBFInvAnalysis/outTau.h"

// ROOT include(s):
#include <TTree.h>

Analysis::outTau::outTau(TString name, Bool_t doTrim) : Analysis::outObject(name, doTrim)
{
   reset();
}

void Analysis::outTau::reset()
{
   pt.clear();
   eta.clear();
   phi.clear();
}

void Analysis::outTau::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
}

void Analysis::outTau::add(const xAOD::TauJet &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
}
