#ifndef __Analysis_outRnSmear__
#define __Analysis_outRnSmear__

#include <VBFInvAnalysis/outObject.h>
#include "SUSYTools/ISUSYObjDef_xAODTool.h"
#include <AsgTools/AnaToolHandle.h>
#include <TrigConfInterfaces/ITrigConfigTool.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <TTree.h>

namespace Analysis {
class outRnSmear : public outObject {
public:
   Float_t rnsPSweight;
   Float_t onlineJet1Pt;
   Float_t onlineJet1Eta;

public:
   outRnSmear(TString name = "", Bool_t doTrim = kFALSE);
   ~outRnSmear();
   void   reset();
   void   attachToTree(TTree *tree);
   double getPSweight(asg::AnaToolHandle<ST::ISUSYObjDef_xAODTool> susytools_handle, xAOD::TEvent *&event,
                      int RunNumber, bool debug = false);
};
} // namespace Analysis

#endif
