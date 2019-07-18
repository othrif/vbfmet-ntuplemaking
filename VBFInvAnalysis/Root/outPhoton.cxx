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
   passOR.clear();
}

void Analysis::outPhoton::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   if (!doTrim()) {
     if(doORDetail()) tree->Branch(prefix + "passOR", &passOR);
   }
}

void Analysis::outPhoton::add(const xAOD::Photon &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());

   if (!doTrim()) {
      static SG::AuxElement::Accessor<char>       acc_passOR("passOR");
      if (acc_passOR.isAvailable(input)) {
         passOR.push_back(acc_passOR(input));
      } else {
         passOR.push_back(-9999);
      }
   }
   
}
