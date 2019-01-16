#include "VBFInvAnalysis/outTrackJet.h"

#include <TTree.h>
#include <xAODBase/IParticleHelpers.h>

Analysis::outTrackJet::outTrackJet(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outTrackJet::~outTrackJet() {}

void Analysis::outTrackJet::reset()
{

   pt.clear();
   eta.clear();
   phi.clear();
   m.clear();
   btag_weight.clear();
   passDRcut.clear();

   return;
}

void Analysis::outTrackJet::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);
   tree->Branch(prefix + "btag_weight", &btag_weight);
   tree->Branch(prefix + "passDRcut", &passDRcut);

   return;
}

void Analysis::outTrackJet::add(const xAOD::Jet &input)
{

   if (!doTrim()) {
      pt.push_back(input.pt());
      eta.push_back(input.eta());
      phi.push_back(input.phi());
      m.push_back(input.m());

      double tmp_btag_weight(-9999.);
      input.btagging()->MVx_discriminant("MV2c10", tmp_btag_weight);
      btag_weight.push_back(tmp_btag_weight);

      static SG::AuxElement::Accessor<char> acc_passDR("passDRcut");
      passDRcut.push_back(acc_passDR(input));
   }

   return;
}
