#ifndef __Analysis_outTau__
#define __Analysis_outTau__

// EDM's include(s):
#include <xAODTau/TauJet.h>

// Local include(s):
#include <VBFInvAnalysis/outObject.h>

namespace Analysis {

   class outTau : public outObject {

   private:

      std::vector<Float_t> pt;
      std::vector<Float_t> eta;
      std::vector<Float_t> phi;

   public:

      outTau(TString name = "", Bool_t doTrim = false, Bool_t doDetail = kFALSE);
      ~outTau() {};

      void reset();
      void attachToTree(TTree *tree);
      void add(const xAOD::TauJet &input);
   };
}

#endif // __Analysis_outTau__
