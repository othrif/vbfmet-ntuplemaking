#ifndef __Analysis_outPhoton__
#define __Analysis_outPhoton__

// EDM's include(s):
#include <xAODEgamma/Photon.h>

// Local include(s):
#include <VBFInvAnalysis/outObject.h>

namespace Analysis {

class outPhoton : public outObject {

private:
   std::vector<Float_t> pt;
   std::vector<Float_t> eta;
   std::vector<Float_t> phi;
   std::vector<unsigned> isEM;
   std::vector<Bool_t> iso;
   std::vector<Float_t> topoetcone40;
   std::vector<Float_t> ptcone20;
   std::vector<Int_t>   truthOrigin;
   std::vector<Bool_t>  passOR;

public:
   outPhoton(TString name = "", Bool_t doTrim = false);
   ~outPhoton(){};

   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::Photon &input);
};
} // namespace Analysis

#endif // __Analysis_outPhoton__
