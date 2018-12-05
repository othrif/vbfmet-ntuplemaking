#ifndef VBFINVANALYSIS_VBFINV_DEFS_H
#define VBFINVANALYSIS_VBFINV_DEFS_H

namespace VBFInvCuts {

// enums
enum CutID {
   PreDerivation = 0,
   Processed     = 1,
   GRL           = 2,
   Vertex        = 3,
   //        Trigger = 4,
   DetError    = 4,
   JetBad      = 5,
   MET_skim    = 6,
   JetN_skim   = 7,
   JetpT1_skim = 8,
   JetpT2_skim = 9,
   Mjj_skim    = 10,
   DEta_skim   = 11
};

} // namespace VBFInvCuts

#endif