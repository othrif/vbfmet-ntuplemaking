#ifndef __Analysis_outTrackMET__
#define __Analysis_outTrackMET__

#include <VBFInvAnalysis/outObject.h>
#include <xAODMissingET/MissingETContainer.h>

namespace Analysis {
class outTrackMET : public outObject {
public:
   std::vector<Float_t>  et;
   std::vector<Float_t>  sumet;
   std::vector<Float_t>  phi;
   std::vector<UShort_t> ivtx;

public:
   outTrackMET(TString name = "", Bool_t doTrim = kFALSE);
   ~outTrackMET();
   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::MissingETContainer *input);
};
} // namespace Analysis

#endif
