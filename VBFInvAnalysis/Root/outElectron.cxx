#include "VBFInvAnalysis/outElectron.h"

#include <TTree.h>
#include <xAODEgamma/EgammaxAODHelpers.h>

Analysis::outElectron::outElectron(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outElectron::~outElectron()
{
}

void Analysis::outElectron::reset()
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
   ptcone30.clear();
   ptvarcone30.clear();
   etcone30.clear();
   topoetcone30.clear();
   ptcone40.clear();
   ptvarcone40.clear();
   etcone40.clear();
   topoetcone40.clear();
   d0.clear();
   d0sig.clear();
   z0.clear();
   z0sig.clear();
   truthType.clear();
   truthOrigin.clear();


   return;
}

void Analysis::outElectron::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "charge", &charge);
   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);
   tree->Branch(prefix + "ptcone20", &ptcone20);
   tree->Branch(prefix + "ptvarcone20", &ptvarcone20);
   tree->Branch(prefix + "etcone20", &etcone20);
   tree->Branch(prefix + "topoetcone20", &topoetcone20);
   tree->Branch(prefix + "ptcone30", &ptcone30);
   tree->Branch(prefix + "ptvarcone30", &ptvarcone30);
   tree->Branch(prefix + "etcone30", &etcone30);
   tree->Branch(prefix + "topoetcone30", &topoetcone30);
   tree->Branch(prefix + "ptcone40", &ptcone40);
   tree->Branch(prefix + "ptvarcone40", &ptvarcone40);
   tree->Branch(prefix + "etcone40", &etcone40);
   tree->Branch(prefix + "topoetcone40", &topoetcone40);
   tree->Branch(prefix + "d0", &d0);
   tree->Branch(prefix + "d0sig", &d0sig);
   tree->Branch(prefix + "z0", &z0);
   tree->Branch(prefix + "z0sig", &z0sig);
   tree->Branch(prefix + "truthType", &truthType);
   tree->Branch(prefix + "truthOrigin", &truthOrigin);

   return;
}

void Analysis::outElectron::add(const xAOD::Electron &input)
{

   charge.push_back(input.charge());
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
   m.push_back(input.m());

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

   // isolation variables
   Float_t tmp_ptcone20(-9999);
   Float_t tmp_ptvarcone20(-9999);
   Float_t tmp_etcone20(-9999);
   Float_t tmp_topoetcone20(-9999);
   Float_t tmp_ptcone30(-9999);
   Float_t tmp_ptvarcone30(-9999);
   Float_t tmp_etcone30(-9999);
   Float_t tmp_topoetcone30(-9999);
   Float_t tmp_ptcone40(-9999);
   Float_t tmp_ptvarcone40(-9999);
   Float_t tmp_etcone40(-9999);
   Float_t tmp_topoetcone40(-9999);
   input.isolationValue(tmp_ptcone20, xAOD::Iso::IsolationType::ptcone20);
   input.isolationValue(tmp_ptvarcone20, xAOD::Iso::IsolationType::ptvarcone20);
   input.isolationValue(tmp_etcone20, xAOD::Iso::IsolationType::etcone20);
   input.isolationValue(tmp_topoetcone20, xAOD::Iso::IsolationType::topoetcone20);
   input.isolationValue(tmp_ptcone30, xAOD::Iso::IsolationType::ptcone30);
   input.isolationValue(tmp_ptvarcone30, xAOD::Iso::IsolationType::ptvarcone30);
   input.isolationValue(tmp_etcone30, xAOD::Iso::IsolationType::etcone30);
   input.isolationValue(tmp_topoetcone30, xAOD::Iso::IsolationType::topoetcone30);
   input.isolationValue(tmp_ptcone40, xAOD::Iso::IsolationType::ptcone40);
   input.isolationValue(tmp_ptvarcone40, xAOD::Iso::IsolationType::ptvarcone40);
   input.isolationValue(tmp_etcone40, xAOD::Iso::IsolationType::etcone40);
   input.isolationValue(tmp_topoetcone40, xAOD::Iso::IsolationType::topoetcone40);
   ptcone20.push_back(tmp_ptcone20);
   ptvarcone20.push_back(tmp_ptvarcone20);
   etcone20.push_back(tmp_etcone20);
   topoetcone20.push_back(tmp_topoetcone20);
   ptcone30.push_back(tmp_ptcone30);
   ptvarcone30.push_back(tmp_ptvarcone30);
   etcone30.push_back(tmp_etcone30);
   topoetcone30.push_back(tmp_topoetcone30);
   ptcone40.push_back(tmp_ptcone40);
   ptvarcone40.push_back(tmp_ptvarcone40);
   etcone40.push_back(tmp_etcone40);
   topoetcone40.push_back(tmp_topoetcone40);

   static SG::AuxElement::ConstAccessor<int> acc_truthType("truthType");
   static SG::AuxElement::ConstAccessor<int> acc_truthOrigin("truthOrigin");

   try {
    truthType.push_back( acc_truthType(input) );
    truthOrigin.push_back( acc_truthOrigin(input) );
 } catch (SG::ExcBadAuxVar) {
    truthType.push_back( -9999 );
    truthOrigin.push_back( -9999 );
 }


 return;
}
