#include "VBFInvAnalysis/outMET.h"

#include <TTree.h>

Analysis::outMET::outMET(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outMET::~outMET() {}

void Analysis::outMET::reset()
{
   et    = -9999;
   sumet = -9999;
   phi   = -9999;

   return;
}

void Analysis::outMET::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "et", &et);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "sumet", &sumet);

   return;
}

void Analysis::outMET::add(const xAOD::MissingET &input)
{
   et    = input.met();
   phi   = input.phi();
   sumet = input.sumet();
}
