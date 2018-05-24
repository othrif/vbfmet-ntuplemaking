#ifndef __Analysis_outMuon__
#define __Analysis_outMuon__

#include <VBFInvAnalysis/outObject.h>
#include <xAODMuon/Muon.h>

namespace Analysis {
   class outMuon : public outObject {
   public:

      std::vector<Float_t> charge;
      std::vector<Float_t> pt;
      std::vector<Float_t> eta;
      std::vector<Float_t> phi;
      std::vector<Float_t> m;
      std::vector<Float_t> ptcone20;
      std::vector<Float_t> ptvarcone20;
      std::vector<Float_t> etcone20;
      std::vector<Float_t> topoetcone20;
      std::vector<Float_t> d0;
      std::vector<Float_t> d0sig;
      std::vector<Float_t> z0;
      std::vector<Float_t> z0sig;
      std::vector<Float_t> SF;

   public:
      outMuon(TString name = "", Bool_t doTrim = kFALSE);
      ~outMuon();
      void reset();
      void attachToTree(TTree *tree);
      void add(const xAOD::Muon &input);
   };
}

#endif

