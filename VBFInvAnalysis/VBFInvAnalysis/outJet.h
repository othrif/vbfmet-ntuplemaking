#ifndef __Analysis_outJet__
#define __Analysis_outJet__

#include <VBFInvAnalysis/outObject.h>
#include <xAODJet/Jet.h>

namespace Analysis {
class outJet : public outObject {
public:
   std::vector<Float_t> pt;
   std::vector<Float_t> eta;
   std::vector<Float_t> phi;
   std::vector<Float_t> m;

   std::vector<Float_t> raw_pt;
   std::vector<Float_t> raw_eta;
   std::vector<Float_t> raw_phi;
   std::vector<Float_t> raw_m;

   std::vector<Float_t> timing;
   std::vector<Int_t>   isbjet;
   std::vector<Float_t> btag_weight;
   std::vector<Float_t> jvt;
   std::vector<Float_t> fjvt;
   std::vector<Bool_t>  passJvt;
   std::vector<Bool_t>  passOR;
   std::vector<Bool_t>  passJetLoose;
   std::vector<Bool_t>  passJetTight;
   std::vector<Int_t>   PartonTruthLabelID;
   std::vector<Int_t>   ConeTruthLabelID;

   std::vector<std::vector<UShort_t>> NTracks;
   std::vector<std::vector<Float_t>> SumPtTracks;
   std::vector<Float_t>  TrackWidth;
   std::vector<Int_t>    HighestJVFVtx;
   std::vector<Float_t>  FracSamplingMax;
   std::vector<Float_t>  HECFrac;
   std::vector<Float_t>  EMFrac;
   std::vector<Float_t>  fch;
   std::vector<Float_t>  Width;

public:
   outJet(TString name = "", Bool_t doTrim = kFALSE);
   ~outJet();
   void reset();
   void attachToTree(TTree *tree);
   void add(const xAOD::Jet &input);
};
} // namespace Analysis

#endif
