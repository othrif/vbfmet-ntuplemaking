// Local include(s):
#include "VBFInvAnalysis/outPhoton.h"

// ROOT include(s):
#include <TTree.h>

Analysis::outPhoton::outPhoton(TString name, Bool_t doTrim) : Analysis::outObject(name, doTrim)
{
   reset();
}

void Analysis::outPhoton::reset()
{
   pt.clear();
   eta.clear();
   phi.clear();

}

void Analysis::outPhoton::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

      tree->Branch(prefix + "pt", &pt);
      tree->Branch(prefix + "eta", &eta);
      tree->Branch(prefix + "phi", &phi);

}

void Analysis::outPhoton::add(const xAOD::Photon &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());

}
