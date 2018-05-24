#include "VBFInvAnalysis/outJet.h"

#include <TTree.h>
#include <xAODBase/IParticleHelpers.h>

Analysis::outJet::outJet(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outJet::~outJet()
{
}

void Analysis::outJet::reset()
{

   pt.clear();
   eta.clear();
   phi.clear();
   m.clear();

   raw_pt.clear();
   raw_eta.clear();
   raw_phi.clear();
   raw_m.clear();

   timing.clear();
   btag_weight.clear();
   isbjet.clear();
   jvt.clear();
   passOR.clear();
   passJvt.clear();

   PartonTruthLabelID.clear();
   ConeTruthLabelID.clear();

   return;
}

void Analysis::outJet::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";


   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);

   tree->Branch(prefix + "raw_pt", &raw_pt);
   tree->Branch(prefix + "raw_eta", &raw_eta);
   tree->Branch(prefix + "raw_phi", &raw_phi);
   tree->Branch(prefix + "raw_m", &raw_m);

   tree->Branch(prefix + "timing", &timing);
   tree->Branch(prefix + "btag_weight", &btag_weight);
   tree->Branch(prefix + "isbjet", &isbjet);
   tree->Branch(prefix + "jvt", &jvt);
   tree->Branch(prefix + "passOR", &passOR);
   tree->Branch(prefix + "passJvt", &passJvt);

   tree->Branch(prefix + "PartonTruthLabelID", &PartonTruthLabelID);
   tree->Branch(prefix + "ConeTruthLabelID", &ConeTruthLabelID);



   return;
}

void Analysis::outJet::add(const xAOD::Jet &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
   m.push_back(input.m());

   const xAOD::Jet *thisRawJet = dynamic_cast< const xAOD::Jet* >(xAOD::getOriginalObject(input));
   raw_pt.push_back(thisRawJet->pt());
   raw_eta.push_back(thisRawJet->eta());
   raw_phi.push_back(thisRawJet->phi());
   raw_m.push_back(thisRawJet->m());

   Float_t tmp_timing(-9999);
   input.getAttribute(xAOD::JetAttribute::Timing, tmp_timing);
   timing.push_back(tmp_timing);

   double tmp_btag_weight(-9999.);
   input.btagging()->MVx_discriminant("MV2c10", tmp_btag_weight);
   btag_weight.push_back(tmp_btag_weight);

   // decorations from SUSYTools
   static SG::AuxElement::Accessor<char> acc_passOR("passOR");
   static SG::AuxElement::Accessor<char> acc_passJvt("passJvt");
   static SG::AuxElement::ConstAccessor<char> acc_bjet("bjet");
   static SG::AuxElement::ConstAccessor<float> acc_jvt("Jvt");

   // ?TODO? check if exists
   passOR.push_back(acc_passOR(input));
   passJvt.push_back(acc_passJvt(input));

   if (acc_bjet.isAvailable(input)) {
      isbjet.push_back(acc_bjet(input));
   } else {
      isbjet.push_back(9999);
   }
   if (acc_jvt.isAvailable(input)) {
      jvt.push_back(acc_jvt(input));
   } else {
      jvt.push_back(-9999);
   }

   static SG::AuxElement::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
   static SG::AuxElement::ConstAccessor<int> acc_ConeTruthLabelID("ConeTruthLabelID");
   Int_t tmp_PartonTruthLabelID(-9999);
   Int_t tmp_ConeTruthLabelID(-9999);
   if (acc_PartonTruthLabelID.isAvailable(input)) {
      tmp_PartonTruthLabelID = acc_PartonTruthLabelID(input);
      tmp_ConeTruthLabelID = acc_ConeTruthLabelID(input);
   }
   PartonTruthLabelID.push_back(tmp_PartonTruthLabelID);
   ConeTruthLabelID.push_back(tmp_ConeTruthLabelID);



   return;
}
