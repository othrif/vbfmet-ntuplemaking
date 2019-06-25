#ifndef __Analysis_outFatJet__
#define __Analysis_outFatJet__

#include <VBFInvAnalysis/outObject.h>
#include <xAODJet/Jet.h>

namespace Analysis {
class outFatJet : public outObject {
public:
   std::vector<Float_t> pt;
   std::vector<Float_t> eta;
   std::vector<Float_t> phi;
   std::vector<Float_t> m;
   std::vector<bool>    tagStatus_top;
   std::vector<bool>    tagStatus_W;

public:
   outFatJet(TString name = "", Bool_t doTrim = kFALSE);
   ~outFatJet();
   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::Jet &input, const bool topTag, const bool WTag);
};
} // namespace Analysis

#endif
