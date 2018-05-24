#include "VBFInvAnalysis/outMuon.h"
#include "xAODTruth/TruthParticleContainer.h"

#include <TTree.h>

Analysis::outMuon::outMuon(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outMuon::~outMuon()
{
}

void Analysis::outMuon::reset()
{

   charge.clear();
   pt.clear();
   eta.clear();
   phi.clear();
   m.clear();
   ptcone20.clear();
   ptvarcone20.clear();
   etcone20.clear();
   topoetcone20.clear();
   d0.clear();
   d0sig.clear();
   z0.clear();
   z0sig.clear();
   SF.clear();

   return;
}

void Analysis::outMuon::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "charge", &charge);
   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);
   tree->Branch(prefix + "d0", &d0);
   tree->Branch(prefix + "d0sig", &d0sig);
   tree->Branch(prefix + "z0", &z0);
   tree->Branch(prefix + "z0sig", &z0sig);
   tree->Branch(prefix + "ptcone20", &ptcone20);
   tree->Branch(prefix + "ptvarcone20", &ptvarcone20);
   tree->Branch(prefix + "etcone20", &etcone20);
   tree->Branch(prefix + "topoetcone20", &topoetcone20);
   tree->Branch(prefix + "SF", &SF);

   return;
}

void Analysis::outMuon::add(const xAOD::Muon &input)
{

   charge.push_back(input.charge());
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
   m.push_back(input.m());

   Float_t tmp_ptcone20(-9999);
   Float_t tmp_etcone20(-9999);
   Float_t tmp_topoetcone20(-9999);

   input.isolation(tmp_ptcone20, xAOD::Iso::IsolationType::ptcone20);
   input.isolation(tmp_etcone20, xAOD::Iso::IsolationType::etcone20);
   input.isolation(tmp_topoetcone20, xAOD::Iso::IsolationType::topoetcone20);
   ptcone20.push_back(tmp_ptcone20);
   etcone20.push_back(tmp_etcone20);
   topoetcone20.push_back(tmp_topoetcone20);


   static SG::AuxElement::ConstAccessor<float> acc_ptvarcone20("ptvarcone20");
   if (acc_ptvarcone20.isAvailable(input)) {
      ptvarcone20.push_back(acc_ptvarcone20(input));
   }

   static SG::AuxElement::ConstAccessor<float> acc_new_d0("new_d0");
   static SG::AuxElement::ConstAccessor<float> acc_new_d0sig("new_d0sig");
   static SG::AuxElement::ConstAccessor<float> acc_new_z0("new_z0");
   static SG::AuxElement::ConstAccessor<float> acc_new_z0sig("new_z0sig");
   try {
      d0.push_back(acc_new_d0(input));
      d0sig.push_back(acc_new_d0sig(input));
      z0.push_back(acc_new_z0(input));
      z0sig.push_back(acc_new_z0sig(input));
   } catch (SG::ExcBadAuxVar) {
      d0.push_back(-9999);
      d0sig.push_back(-9999);
      z0.push_back(-9999);
      z0sig.push_back(-9999);
   }

  static SG::AuxElement::ConstAccessor<float> acc_lep_tot_SF("lep_tot_SF");
   Float_t tmp_lep_tot_SF(-9999);
   try {
      tmp_lep_tot_SF =  acc_lep_tot_SF(input);
   } catch (SG::ExcBadAuxVar) {
      tmp_lep_tot_SF = 1.0;
   }
   SF.push_back(tmp_lep_tot_SF);

   return;
}
