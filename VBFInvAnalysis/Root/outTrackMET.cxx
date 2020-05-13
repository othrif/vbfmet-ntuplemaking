#include "VBFInvAnalysis/outTrackMET.h"

#include <TTree.h>
#include <TString.h>

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

void Analysis::outTrackMET::add(const xAOD::MissingETContainer *input, const xAOD::VertexContainer *vertices)
{
   UShort_t i = 0;
   for (const auto &vx : *vertices) {
      if (vx->vertexType() != xAOD::VxType::PriVtx && vx->vertexType() != xAOD::VxType::PileUp) continue;
      TString vname = "PVTrack_vx";
      vname += vx->index();
      et.push_back((*input)[vname.Data()]->met());
      phi.push_back((*input)[vname.Data()]->phi());
      sumet.push_back((*input)[vname.Data()]->sumet());
      ivtx.push_back(vx->index());
      //std::cout << "i, i_vtx, MET: " << i << ", " << vx->index() << ", " << (*input)[vname.Data()]->met() << std::endl;
      ++i;
   }

   /*
   i = 0;
   for (const auto &met_itr : *input) {
      et.push_back(met_itr->met());
      phi.push_back(met_itr->phi());
      sumet.push_back(met_itr->sumet());
      ivtx.push_back(i);
      std::cout << "i, MET: " << i << ", " << met_itr->met() << std::endl;
      ++i;
   }
   */
      
}
