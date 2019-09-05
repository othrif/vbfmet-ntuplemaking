// Container for all those variables which are event-related
// and are too complex to read automatically from xAOD
// (unlike e.g. jets)

#ifndef __Analysis_outEvent__
#define __Analysis_outEvent__

#include <VBFInvAnalysis/outObject.h>
#include <SUSYTools/SUSYObjDef_xAOD.h>
#include <map>

namespace Analysis {
class outEvent : public outObject {
public:
   // event variables
   Int_t     year;
   Int_t     runNumber;
   Int_t     runPeriod;
   Int_t     randomRunNumber;
   ULong64_t eventNumber;
   Int_t     lumiBlock;
   Int_t     bcid;
   Int_t     BCIDDistanceFromFront;
   Int_t     BCIDDistanceFromTail;
   Float_t   averageIntPerXing;
   Float_t   corAverageIntPerXing;
   Float_t   vtx_sumpt2;
   Int_t     l1_met_trig_encoded;

   Float_t              mcEventWeight;
   Float_t              mcEventWeightXsec;
   std::vector<Float_t> mcEventWeights;
   Float_t              puWeight;
   Float_t              elSFWeight;
   Float_t              muSFWeight;
   Float_t              phSFWeight;
   Float_t              elSFTrigWeight;
   Float_t              muSFTrigWeight;
   Float_t              btagSFWeight;
   Float_t              jvtSFWeight;
   Float_t              fjvtSFWeight;
   Float_t              fjvtSFTighterWeight;
   Float_t              eleANTISF;
   Float_t              dilepTrigSFWeight;

   Int_t FlavourFilter;
   Int_t   nParton;
   Float_t MGVTruthPt;
   Float_t SherpaVTruthPt;
   Bool_t in_vy_overlap;
   Bool_t in_vy_overlap_iso;
   /*
         Int_t pdf_id1;
         Int_t pdf_id2;
         Float_t pdf_x1;
         Float_t pdf_x2;
         Float_t pdf_pdf1;
         Float_t pdf_pdf2;
         Float_t pdf_scale;
         */

   Int_t                n_jet_truth;
   std::vector<Float_t> truth_jet_pt;
   std::vector<Float_t> truth_jet_eta;
   std::vector<Float_t> truth_jet_phi;
   std::vector<Float_t> truth_jet_m;
   std::vector<Int_t>   truth_jet_label;

   std::vector<Float_t> truth_jetmu_pt;
   std::vector<Float_t> truth_jetmu_eta;
   std::vector<Float_t> truth_jetmu_phi;
   std::vector<Float_t> truth_jetmu_m;

   std::vector<Float_t> truth_jetmunu_pt;
   std::vector<Float_t> truth_jetmunu_eta;
   std::vector<Float_t> truth_jetmunu_phi;
   std::vector<Float_t> truth_jetmunu_m;

   Int_t                n_fatjet_truth;
   std::vector<Float_t> truth_fatjet_pt;
   std::vector<Float_t> truth_fatjet_eta;
   std::vector<Float_t> truth_fatjet_phi;
   std::vector<Float_t> truth_fatjet_m;

   Float_t truth_V_bare_pt;
   Float_t truth_V_bare_eta;
   Float_t truth_V_bare_phi;
   Float_t truth_V_bare_m;

   Float_t truth_V_dressed_pt;
   Float_t truth_V_dressed_eta;
   Float_t truth_V_dressed_phi;
   Float_t truth_V_dressed_m;

   Int_t                n_el_truth;
   std::vector<Float_t> truth_el_pt;
   std::vector<Float_t> truth_el_eta;
   std::vector<Float_t> truth_el_phi;
   std::vector<Float_t> truth_el_m;
   std::vector<Int_t>   truth_el_status;

   Int_t                n_mu_truth;
   std::vector<Float_t> truth_mu_pt;
   std::vector<Float_t> truth_mu_eta;
   std::vector<Float_t> truth_mu_phi;
   std::vector<Float_t> truth_mu_m;
   std::vector<Int_t>   truth_mu_status;

   Int_t                n_tau_truth;
   std::vector<Float_t> truth_tau_pt;
   std::vector<Float_t> truth_tau_eta;
   std::vector<Float_t> truth_tau_phi;
   std::vector<Float_t> truth_tau_m;
   std::vector<Int_t>   truth_tau_status;

   std::map<TString, Float_t> syst_var_map;
   std::map<TString, Int_t>   trigger;
   Int_t                      lep_trig_match;
   Int_t                      trigger_lep;
   Int_t                      trigger_lep_OR;
   Int_t                      trigger_met;

   Int_t passGRL;
   //      Int_t passTrigger;
   Int_t passPV;
   Int_t passJetCleanLoose;
   Int_t passJetCleanTight;
   Int_t passJetCleanTightEM;
   Int_t passDetErr;

   Bool_t passBatman;
   Bool_t passVjetsFilter;
   Bool_t passVjetsFilterTauEl;
   Bool_t passVjetsPTV;

   Int_t    n_vx;
   Int_t    n_jet;
   Double_t jj_mass;
   Double_t jj_deta;
   Double_t jj_dphi;
   Double_t met_tst_j1_dphi;
   Double_t met_tst_j2_dphi;
   Double_t met_tst_nolep_j1_dphi;
   Double_t met_tst_nolep_j2_dphi;
   Double_t met_cst_jet;
   Double_t metsig_tst;
   Double_t metsig_tst_nolep;

   Double_t truthF_jj_mass;
   Double_t truthF_jj_deta;
   Double_t truthF_jj_dphi;

   Int_t n_bjet;
   Int_t n_mu;
   Int_t n_mu_w;
   Int_t n_mu_baseline_noOR;
   Int_t n_mu_baseline;
   Int_t n_mu_baseline_loose;
   Int_t n_mu_baseline_loose_noOR;
   Int_t n_mu_baseline_iso;
   Int_t n_el;
   Int_t n_el_w;
   Int_t n_el_baseline;
   Int_t n_el_baseline_iso;
   Int_t n_el_baseline_noOR;
   Int_t n_el_baseline_crackVetoCleaning;
   Int_t n_ph;
   Int_t n_ph_crackVetoCleaning;

   Float_t              truth_vtx_z;
   std::vector<Int_t>   reco_vtx_ntrk;
   std::vector<Float_t> reco_vtx_x;
   std::vector<Float_t> reco_vtx_y;
   std::vector<Float_t> reco_vtx_z;
   std::vector<Float_t> reco_vtx_chiSquared;
   std::vector<Float_t> reco_vtx_sumPt2;
   std::vector<short>   reco_vtx_vertexType;

public:
   outEvent(TString name = "", Bool_t doTrim = kFALSE);
   ~outEvent();
   void   reset();
   void   attachToTree(TTree *tree);
   void   setTriggers(const std::vector<std::string> &trigs);
   float &GetSystVar(TString var, TString syst, TTree *tree);
};
} // namespace Analysis

#endif
