#include "VBFInvAnalysis/outFatJet.h"

#include <TTree.h>
#include <xAODBase/IParticleHelpers.h>

Analysis::outFatJet::outFatJet(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outFatJet::~outFatJet() {}

void Analysis::outFatJet::reset()
{

   pt.clear();
   eta.clear();
   phi.clear();
   m.clear();
   tagStatus_W.clear();
   tagStatus_top.clear();

   return;
}

void Analysis::outFatJet::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);
   tree->Branch(prefix + "tagStatus_W", &tagStatus_W);
   tree->Branch(prefix + "tagStatus_top", &tagStatus_top);
   return;
}

void Analysis::outFatJet::add(const xAOD::Jet &input, const bool topTag, const bool WTag)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
   m.push_back(input.m());
   tagStatus_top.push_back(topTag);
   tagStatus_W.push_back(WTag);

   return;
}
