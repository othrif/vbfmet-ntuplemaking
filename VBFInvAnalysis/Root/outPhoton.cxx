// Local include(s):
#include "VBFInvAnalysis/outPhoton.h"

// ROOT include(s):
#include <TTree.h>
#include <xAODEgamma/EgammaxAODHelpers.h>

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
   isEM.clear();
   iso.clear();
   topoetcone40.clear();
   ptcone20.clear();
   truthOrigin.clear();
   truthType.clear();
   vtxpos.clear();
   vtxerr.clear();
   isConv.clear();
}

void Analysis::outPhoton::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "vtxpos", &vtxpos);

   if(isMC()) tree->Branch(prefix + "truthOrigin", &truthOrigin);
   if(isMC()) tree->Branch(prefix + "truthType", &truthType);
   if (!doTrim()) {
     if(doORDetail()) tree->Branch(prefix + "passOR", &passOR);
     tree->Branch(prefix + "isEM", &isEM);
     tree->Branch(prefix + "isConv", &isConv);
     tree->Branch(prefix + "iso", &iso);
     tree->Branch(prefix + "topoetcone40", &topoetcone40);
     tree->Branch(prefix + "ptcone20",     &ptcone20);
     tree->Branch(prefix + "vtxerr", &vtxerr);
   }
}

void Analysis::outPhoton::add(const xAOD::Photon &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());

   static SG::AuxElement::ConstAccessor<float> acc_ph_zvx("ph_zvx");  
   static SG::AuxElement::ConstAccessor<float> acc_ph_zvx_err("ph_zvx_err");
   Float_t tmp_ph_zvx(-9999);
   Float_t tmp_ph_zvx_err(-9999);   
   try {
     tmp_ph_zvx = acc_ph_zvx(input);
     tmp_ph_zvx_err = acc_ph_zvx_err(input);
   } catch (SG::ExcBadAuxVar) {
     tmp_ph_zvx = -9999;
     tmp_ph_zvx_err = -9999;
   }
   vtxpos.push_back(tmp_ph_zvx);
   vtxerr.push_back(tmp_ph_zvx_err);
   
   if (!doTrim()) {

     isConv.push_back(xAOD::EgammaHelpers::conversionType(&input));

     static SG::AuxElement::ConstAccessor<int> acc_truthOrigin("truthOrigin");
     static SG::AuxElement::ConstAccessor<int> acc_truthType("truthType");
      try {
	truthType.push_back(acc_truthType(input));
	truthOrigin.push_back(acc_truthOrigin(input));
      } catch (SG::ExcBadAuxVar) {
	truthType.push_back(-9999);
	truthOrigin.push_back(-9999);
      }     
 
     const static SG::AuxElement::ConstAccessor<char> acc_isol("isol");
     if(acc_isol.isAvailable(input)){
       iso.push_back(acc_isol(input));
     }
     const static SG::AuxElement::ConstAccessor<unsigned> acc_isEM("isEM");
     if(acc_isEM.isAvailable(input)){
       isEM.push_back(acc_isEM(input));
     }
     Float_t tmp_ptcone20(-9999);
     Float_t tmp_topoetcone40(-9999);
     input.isolationValue(tmp_ptcone20, xAOD::Iso::IsolationType::ptcone20);
     input.isolationValue(tmp_topoetcone40, xAOD::Iso::IsolationType::topoetcone40);
     ptcone20.push_back(tmp_ptcone20);
     topoetcone40.push_back(tmp_topoetcone40);
     
      static SG::AuxElement::Accessor<char>       acc_passOR("passOR");
      if (acc_passOR.isAvailable(input)) {
         passOR.push_back(acc_passOR(input));
      } else {
         passOR.push_back(-9999);
      }
   }
   
}
