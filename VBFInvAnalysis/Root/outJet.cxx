#include "VBFInvAnalysis/outJet.h"

#include <TTree.h>
#include <xAODBase/IParticleHelpers.h>

Analysis::outJet::outJet(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim), m_savePV(false)
{
   reset();
}

Analysis::outJet::~outJet() {}

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
   isbjet.clear();
   btag_weight.clear();
   jvt.clear();
   fjvt.clear();
   jvfcorr.clear();
   jvtrpt.clear();
   passJvt.clear();
   passOR.clear();
   passJetLoose.clear();
   passJetTight.clear();

   //qg variables
   PartonTruthLabelID.clear();
   ConeTruthLabelID.clear();
   truthjet_pt.clear();
   truthjet_eta.clear();
   truthjet_nCharged.clear();
   
   for (auto it : NTracks) {
      it.clear();
   }
   NTracks.clear();

   for (auto it : SumPtTracks) {
      it.clear();
   }
   SumPtTracks.clear();

   TrackWidth.clear();
   TracksC1.clear(); //DFCommonJets_QGTagger_TracksWidth DFCommonJets_QGTagger_TracksC1 DFCommonJets_QGTagger_NTracks
   HighestJVFVtx.clear();
   FracSamplingMax.clear();
   HECFrac.clear();
   EMFrac.clear();
   fch.clear();
   Width.clear();

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
   tree->Branch(prefix + "jvfcorr", &jvfcorr);
   tree->Branch(prefix + "jvtrpt", &jvtrpt);
   tree->Branch(prefix + "fjvt", &fjvt);
   tree->Branch(prefix + "isbjet", &isbjet);

   if (!doTrim()) {
      tree->Branch(prefix + "raw_pt", &raw_pt);
      tree->Branch(prefix + "raw_eta", &raw_eta);
      tree->Branch(prefix + "raw_phi", &raw_phi);
      tree->Branch(prefix + "raw_m", &raw_m);
      tree->Branch(prefix + "passJvt", &passJvt);           // not needed with signal jets
      tree->Branch(prefix + "passOR", &passOR);             // not needed with signal jets
      tree->Branch(prefix + "passJetLoose", &passJetLoose); // not needed with signal jets
      tree->Branch(prefix + "passJetTight", &passJetTight); // not needed with signal jets
      tree->Branch(prefix + "btag_weight", &btag_weight);
      tree->Branch(prefix + "NTracks", &NTracks);
      tree->Branch(prefix + "SumPtTracks", &SumPtTracks);
      tree->Branch(prefix + "TrackWidth", &TrackWidth);
      tree->Branch(prefix + "TracksC1", &TracksC1);
      tree->Branch(prefix + "HighestJVFVtx", &HighestJVFVtx);
      tree->Branch(prefix + "HECFrac", &HECFrac);
      tree->Branch(prefix + "EMFrac", &EMFrac);
      tree->Branch(prefix + "FracSamplingMax", &FracSamplingMax);
      tree->Branch(prefix + "fch", &fch);
      tree->Branch(prefix + "Width", &Width);
      tree->Branch(prefix + "PartonTruthLabelID", &PartonTruthLabelID);
      tree->Branch(prefix + "ConeTruthLabelID", &ConeTruthLabelID);
      tree->Branch(prefix + "truthjet_pt", &truthjet_pt);
      tree->Branch(prefix + "truthjet_eta", &truthjet_eta);
      tree->Branch(prefix + "truthjet_nCharged", &truthjet_nCharged);
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
   std::vector<float> tmp_sumpttrk_vec;
   input.getAttribute("SumPtTrkPt500", tmp_sumpttrk_vec);

   Float_t tmp_fch = (tmp_sumpttrk_vec.size() > 0) ? tmp_sumpttrk_vec[0] / input.pt() : 0;
   fch.push_back(tmp_fch);

   double tmp_FracSamplingMax(-9999.);
   input.getAttribute("FracSamplingMax", tmp_FracSamplingMax);
   FracSamplingMax.push_back(tmp_FracSamplingMax);

   static SG::AuxElement::Accessor<char>       acc_passOR("passOR");
   static SG::AuxElement::Accessor<char>       acc_passJvt("passJvt");
   static SG::AuxElement::ConstAccessor<float> acc_jvt("Jvt");
   static SG::AuxElement::ConstAccessor<float> acc_jvfcorr("JVFCorr");
   static SG::AuxElement::ConstAccessor<float> acc_jvtrpt("JvtRpt");
   static SG::AuxElement::ConstAccessor<float> acc_fjvt("fJvt");
   // static SG::AuxElement::Accessor<char>       acc_bad("bad");
   static SG::AuxElement::Accessor<char> acc_jetCleanLoose("DFCommonJets_jetClean_LooseBad");
   static SG::AuxElement::Accessor<char> acc_jetCleanTight("DFCommonJets_jetClean_TightBad");

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
   if (acc_jvfcorr.isAvailable(input)) {
      jvfcorr.push_back(acc_jvfcorr(input));
   } else {
      jvfcorr.push_back(-9999);
   }
   if (acc_jvtrpt.isAvailable(input)) {
      jvtrpt.push_back(acc_jvtrpt(input));
   } else {
      jvtrpt.push_back(-9999);
   }
   if (acc_fjvt.isAvailable(input)) {
      fjvt.push_back(acc_fjvt(input));
   } else {
      fjvt.push_back(-9999);
   }
   if (acc_jetCleanTight.isAvailable(input)) {
      passJetTight.push_back(acc_jetCleanTight(input));
   } else {
      passJetTight.push_back(true);
   }
   if (acc_jetCleanLoose.isAvailable(input)) {
      passJetLoose.push_back(acc_jetCleanLoose(input));
   } else {
      passJetLoose.push_back(true);
   }

   if (!doTrim()) {

      const xAOD::Jet *thisRawJet = dynamic_cast<const xAOD::Jet *>(xAOD::getOriginalObject(input));
      raw_pt.push_back(thisRawJet->pt());
      raw_eta.push_back(thisRawJet->eta());
      raw_phi.push_back(thisRawJet->phi());
      raw_m.push_back(thisRawJet->m());

      double tmp_btag_weight(-9999.);
      input.btagging()->MVx_discriminant("MV2c10", tmp_btag_weight);
      btag_weight.push_back(tmp_btag_weight);

      // number of charged tracks within a jet
      std::vector<int> tmp_numtrk_vec;
      input.getAttribute("NumTrkPt500", tmp_numtrk_vec);

      // width charged tracks
      std::vector<float> tmp_trkwidth_vec;
      input.getAttribute("TrackWidthPt500", tmp_trkwidth_vec);

      std::vector<float>              tmp_sumpttrk;
      std::vector<short unsigned int> tmp_numtrk;

      // which vertex
      int   vtx      = -1;
      float maxSumPt = 0;
      for (unsigned int i = 0; i < tmp_sumpttrk_vec.size(); ++i) {
         tmp_sumpttrk.push_back(tmp_sumpttrk_vec.at(i));
         tmp_numtrk.push_back(tmp_numtrk_vec.at(i));
         if (tmp_sumpttrk_vec.at(i) > maxSumPt) {
            maxSumPt = tmp_sumpttrk_vec.at(i);
            vtx      = i;
         }
      }

      float tmp_trkwidth;
      if (tmp_sumpttrk_vec.size() > 0 /*&& susytools_handle->GetPrimVtx()*/) {
	tmp_trkwidth = tmp_trkwidth_vec[0]; // setting to primary vertex
      } else {
         tmp_trkwidth = 0.;
      }

      // reduce output to save only the PV
      if(m_savePV){
	tmp_sumpttrk.clear();
	tmp_numtrk.clear();
	if(tmp_sumpttrk_vec.size()>0){
	  tmp_sumpttrk.push_back(tmp_sumpttrk_vec.at(0));
	  tmp_numtrk.push_back(tmp_numtrk_vec.at(0));
	}
      }

      static SG::AuxElement::ConstAccessor<float> acc_DFCommonJets_QGTagger_TracksC1("DFCommonJets_QGTagger_TracksC1");
      float tmp_TracksC1=-9999.;
      if(acc_DFCommonJets_QGTagger_TracksC1.isAvailable(input)) tmp_TracksC1 = acc_DFCommonJets_QGTagger_TracksC1(input);

      HighestJVFVtx.push_back(vtx);
      SumPtTracks.push_back(tmp_sumpttrk);
      NTracks.push_back(tmp_numtrk);
      TrackWidth.push_back(tmp_trkwidth);
      TracksC1.push_back(tmp_TracksC1);

      double tmp_HECFrac(-9999.);
      input.getAttribute("HECFrac", tmp_HECFrac);
      HECFrac.push_back(tmp_HECFrac);

      double tmp_EMFrac(-9999.);
      input.getAttribute("EMFrac", tmp_EMFrac);
      EMFrac.push_back(tmp_EMFrac);

      double tmp_Width(-9999.);
      input.getAttribute("Width", tmp_Width);
      Width.push_back(tmp_Width);

      // decorations from SUSYTools
      // FwdJVT is applied in passJvt given that FwdJet.doJVT: true in ST config
      static SG::AuxElement::ConstAccessor<char> acc_bjet("bjet");

      if (acc_bjet.isAvailable(input)) {
         isbjet.push_back(acc_bjet(input));
      } else {
         isbjet.push_back(-9999);
      }

      static SG::AuxElement::ConstAccessor<int> acc_DFCommonJets_QGTagger_truthjet_nCharged("DFCommonJets_QGTagger_truthjet_nCharged");
      static SG::AuxElement::ConstAccessor<float> acc_DFCommonJets_QGTagger_truthjet_pt("DFCommonJets_QGTagger_truthjet_pt");
      static SG::AuxElement::ConstAccessor<float> acc_DFCommonJets_QGTagger_truthjet_eta("DFCommonJets_QGTagger_truthjet_eta");
      static SG::AuxElement::ConstAccessor<int> acc_PartonTruthLabelID("PartonTruthLabelID");
      static SG::AuxElement::ConstAccessor<int> acc_ConeTruthLabelID("ConeTruthLabelID");
      Int_t                                     tmp_truthjet_nCharged(-9999);
      Float_t                                   tmp_truthjet_pt(-9999);
      Float_t                                   tmp_truthjet_eta(-9999);
      Int_t                                     tmp_PartonTruthLabelID(-9999);
      Int_t                                     tmp_ConeTruthLabelID(-9999);
      if (acc_PartonTruthLabelID.isAvailable(input)) {
         tmp_PartonTruthLabelID = acc_PartonTruthLabelID(input);
         tmp_ConeTruthLabelID   = acc_ConeTruthLabelID(input);
      }
      PartonTruthLabelID.push_back(tmp_PartonTruthLabelID);
      ConeTruthLabelID.push_back(tmp_ConeTruthLabelID);
      if(acc_DFCommonJets_QGTagger_truthjet_nCharged.isAvailable(input)) {  
	tmp_truthjet_nCharged = acc_DFCommonJets_QGTagger_truthjet_nCharged(input);
	tmp_truthjet_pt       = acc_DFCommonJets_QGTagger_truthjet_pt(input);
	tmp_truthjet_eta      = acc_DFCommonJets_QGTagger_truthjet_eta(input);
      }
      truthjet_pt.push_back(tmp_truthjet_pt);
      truthjet_eta.push_back(tmp_truthjet_eta);
      truthjet_nCharged.push_back(tmp_truthjet_nCharged);
   }

   return;
}
