#ifndef __Analysis_outTrackJet__
#define __Analysis_outTrackJet__

#include <VBFInvAnalysis/outObject.h>
#include <xAODJet/Jet.h>

namespace Analysis {
class outTrackJet : public outObject {
public:
   std::vector<Float_t> pt;
   std::vector<Float_t> eta;
   std::vector<Float_t> phi;
   std::vector<Float_t> m;
   std::vector<Float_t> btag_weight;
   std::vector<bool> passDRcut;

public:
   outTrackJet(TString name = "", Bool_t doTrim = kFALSE);
   ~outTrackJet();
   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::Jet &input);
};
} // namespace Analysis

#endif
