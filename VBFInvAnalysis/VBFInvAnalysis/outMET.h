#ifndef __Analysis_outMET__
#define __Analysis_outMET__

#include <VBFInvAnalysis/outObject.h>
#include <xAODMissingET/MissingET.h>

namespace Analysis {
   class outMET : public outObject {
   public:
      Float_t et;
      Float_t etx;
      Float_t ety;
      Float_t sumet;
      Float_t phi;

   public:
      outMET(TString name = "", Bool_t doTrim = kFALSE, Bool_t doDetail = kFALSE);
      ~outMET();
      void reset();
      void attachToTree(TTree *tree);
      void add(const xAOD::MissingET &input);
   };
}

#endif

