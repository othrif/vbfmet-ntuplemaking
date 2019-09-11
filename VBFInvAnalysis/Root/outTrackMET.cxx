#include "VBFInvAnalysis/outTrackMET.h"

#include <TTree.h>

Analysis::outTrackMET::outTrackMET(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outTrackMET::~outTrackMET() {}

void Analysis::outTrackMET::reset()
{
   et.clear();
   sumet.clear();
   phi.clear();
   ivtx.clear();

   return;
}

void Analysis::outTrackMET::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";

   tree->Branch(prefix + "et", &et);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "sumet", &sumet);
   tree->Branch(prefix + "ivtx", &ivtx);

   return;
}

void Analysis::outTrackMET::add(const xAOD::MissingETContainer *input)
{  
   UShort_t i = 0;
   for ( const auto& met_itr : *input ) {
      et.push_back(met_itr->met());
      phi.push_back(met_itr->phi());
      sumet.push_back(met_itr->sumet());
      ivtx.push_back(i);
      ++i;
   }
}
