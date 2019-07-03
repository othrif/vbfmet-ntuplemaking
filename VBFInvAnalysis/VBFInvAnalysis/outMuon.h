#ifndef __Analysis_outMuon__
#define __Analysis_outMuon__

#include <VBFInvAnalysis/outObject.h>
#include <xAODMuon/Muon.h>

namespace Analysis {
class outMuon : public outObject {
public:
   std::vector<Int_t>   charge;
   std::vector<Float_t> pt;
   std::vector<Float_t> eta;
   std::vector<Float_t> phi;
   std::vector<Float_t> m;
   std::vector<Int_t>   type;
   std::vector<Float_t> ptcone20;
   std::vector<Float_t> ptvarcone20;
   std::vector<Float_t> etcone20;
   std::vector<Float_t> topoetcone20;
   std::vector<Float_t> ptcone30;
   std::vector<Float_t> ptvarcone30;
   std::vector<Float_t> ptvarcone30_TightTTVA_pt1000;
   std::vector<Float_t> etcone30;
   std::vector<Float_t> topoetcone30;
   std::vector<Float_t> ptcone40;
   std::vector<Float_t> ptvarcone40;
   std::vector<Float_t> etcone40;
   std::vector<Float_t> topoetcone40;
   std::vector<Float_t> d0;
   std::vector<Float_t> d0sig;
   std::vector<Float_t> z0;
   std::vector<Float_t> z0sig;
   std::vector<Int_t>   truthType;
   std::vector<Int_t>   truthOrigin;
   std::vector<Bool_t>  passOR;

public:
   outMuon(TString name = "", Bool_t doTrim = kFALSE);
   ~outMuon();
   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::Muon &input);
};
} // namespace Analysis

#endif
