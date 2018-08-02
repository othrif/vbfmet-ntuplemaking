#include "VBFInvAnalysis/outJet.h"

#include <TTree.h>
#include <xAODBase/IParticleHelpers.h>

Analysis::outJet::outJet(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outJet::~outJet()
{
}

void Analysis::outJet::reset()
{

   pt.clear();
   eta.clear();
   phi.clear();
   m.clear();

   raw_pt.clear();
   raw_eta.clear();
   raw_phi.clear();
   raw_m.clear();

   timing.clear();
   btag_weight.clear();
   isbjet.clear();
   jvt.clear();
   passOR.clear();
   passJvt.clear();

   PartonTruthLabelID.clear();
   ConeTruthLabelID.clear();

   NTracks.clear();
   SumPtTracks.clear();
   TrackWidth.clear();
   HighestJVFVtx.clear();
   FracSamplingMax.clear();
   HECFrac.clear();
   EMFrac.clear();
   fch.clear();



   return;
}

void Analysis::outJet::attachToTree(TTree *tree)
{
   const TString prefix = name() + "_";


   tree->Branch(prefix + "pt", &pt);
   tree->Branch(prefix + "eta", &eta);
   tree->Branch(prefix + "phi", &phi);
   tree->Branch(prefix + "m", &m);
   tree->Branch(prefix + "timing", &timing);
   tree->Branch(prefix + "jvt", &jvt);
   tree->Branch(prefix + "passJvt", &passJvt);
   tree->Branch(prefix + "isbjet", &isbjet);
   tree->Branch(prefix + "PartonTruthLabelID", &PartonTruthLabelID);
   tree->Branch(prefix + "ConeTruthLabelID", &ConeTruthLabelID);

   if (!doTrim()) {
     tree->Branch(prefix + "raw_pt", &raw_pt);// remove it
     tree->Branch(prefix + "raw_eta", &raw_eta);// remove it
     tree->Branch(prefix + "raw_phi", &raw_phi);// remove it
     tree->Branch(prefix + "raw_m", &raw_m);// remove it
     tree->Branch(prefix + "passOR", &passOR); // remove it
     tree->Branch(prefix + "btag_weight", &btag_weight);
     tree->Branch(prefix + "NTracks", &NTracks);
     tree->Branch(prefix + "SumPtTracks", &SumPtTracks);
     tree->Branch(prefix + "TrackWidth", &TrackWidth);
     tree->Branch(prefix + "HighestJVFVtx", &HighestJVFVtx);
     tree->Branch(prefix + "HECFrac", &HECFrac);
     tree->Branch(prefix + "EMFrac", &EMFrac);
     tree->Branch(prefix + "FracSamplingMax", &FracSamplingMax);
     tree->Branch(prefix + "fch", &fch);
   }
   return;
}

void Analysis::outJet::add(const xAOD::Jet &input)
{
   pt.push_back(input.pt());
   eta.push_back(input.eta());
   phi.push_back(input.phi());
   m.push_back(input.m());

   Float_t tmp_timing(-9999);
   input.getAttribute(xAOD::JetAttribute::Timing, tmp_timing);
   timing.push_back(tmp_timing);

   // momentum fraction carried by charged tracks
   std::vector<float>  tmp_sumpttrk_vec;
   input.getAttribute("SumPtTrkPt500", tmp_sumpttrk_vec);
   Float_t tmp_fch = (tmp_sumpttrk_vec.size() > 0) ? tmp_sumpttrk_vec[0] /   input.pt() : 0;
   fch.push_back(tmp_fch);

   double tmp_FracSamplingMax(-9999.);
   input.getAttribute("FracSamplingMax", tmp_FracSamplingMax);
   FracSamplingMax.push_back(tmp_FracSamplingMax);

   static SG::AuxElement::Accessor<char> acc_passOR("passOR");
   static SG::AuxElement::Accessor<char> acc_passJvt("passJvt");
   static SG::AuxElement::ConstAccessor<float> acc_jvt("Jvt");

   if (acc_passOR.isAvailable(input)) {
     passOR.push_back(acc_passOR(input));
   } else {
     passOR.push_back(-9999);
   }
   if (acc_passJvt.isAvailable(input)) {
     passJvt.push_back(acc_passJvt(input));
   } else {
     passJvt.push_back(-9999);
   }
   if (acc_jvt.isAvailable(input)) {
     jvt.push_back(acc_jvt(input));
   } else {
     jvt.push_back(-9999);
   }

   if (!doTrim()) {
     const xAOD::Jet *thisRawJet = dynamic_cast< const xAOD::Jet* >(xAOD::getOriginalObject(input));
     raw_pt.push_back(thisRawJet->pt());
     raw_eta.push_back(thisRawJet->eta());
     raw_phi.push_back(thisRawJet->phi());
     raw_m.push_back(thisRawJet->m());

     double tmp_btag_weight(-9999.);
     input.btagging()->MVx_discriminant("MV2c10", tmp_btag_weight);
     btag_weight.push_back(tmp_btag_weight);

     // number of charged tracks within a jet
     std::vector<int>  tmp_numtrk_vec;
     input.getAttribute("NumTrkPt500", tmp_numtrk_vec);

     // width charged tracks
     std::vector<float> tmp_trkwidth_vec;
     input.getAttribute("TrackWidthPt1000", tmp_trkwidth_vec);

     // which vertex
     int vtx = 0;
     float maxSumPt = -1;
     for (unsigned int i = 0; i < tmp_sumpttrk_vec.size(); ++i) {
       if (tmp_sumpttrk_vec.at(i) > maxSumPt) {
	 maxSumPt = tmp_sumpttrk_vec.at(i);
	 vtx = i;
       }}

     float tmp_sumpttrk;
     float tmp_trkwidth;
     int tmp_numtrk;
     if (tmp_sumpttrk_vec.size() > 0 /*&& susytools_handle->GetPrimVtx()*/) {
       tmp_sumpttrk = tmp_sumpttrk_vec[vtx];
       tmp_numtrk = tmp_numtrk_vec[vtx];
       tmp_trkwidth = tmp_trkwidth_vec[vtx];
     }
     else {
       tmp_sumpttrk = 0;
       tmp_numtrk = 0;
       tmp_trkwidth = 0.;}

     HighestJVFVtx.push_back(tmp_sumpttrk);
     SumPtTracks.push_back(tmp_sumpttrk);
     NTracks.push_back(tmp_numtrk);
     TrackWidth.push_back(tmp_trkwidth);

     double tmp_HECFrac(-9999.);
     input.getAttribute("HECFrac", tmp_HECFrac);
     HECFrac.push_back(tmp_HECFrac);

     double tmp_EMFrac(-9999.);
     input.getAttribute("EMFrac", tmp_EMFrac);
     EMFrac.push_back(tmp_EMFrac);

     // decorations from SUSYTools
     // FwdJVT is applied in passJvt given that FwdJet.doJVT: true in ST config
     static SG::AuxElement::ConstAccessor<char> acc_bjet("bjet");

     if (acc_bjet.isAvailable(input)) {
       isbjet.push_back(acc_bjet(input));
     } else {
       isbjet.push_back(-9999);
     }

     static SG::AuxElement::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
     static SG::AuxElement::ConstAccessor<int> acc_ConeTruthLabelID("ConeTruthLabelID");
     Int_t tmp_PartonTruthLabelID(-9999);
     Int_t tmp_ConeTruthLabelID(-9999);
     if (acc_PartonTruthLabelID.isAvailable(input)) {
       tmp_PartonTruthLabelID = acc_PartonTruthLabelID(input);
       tmp_ConeTruthLabelID = acc_ConeTruthLabelID(input);
     }
     PartonTruthLabelID.push_back(tmp_PartonTruthLabelID);
     ConeTruthLabelID.push_back(tmp_ConeTruthLabelID);
   }


   return;
}
