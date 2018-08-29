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
      Int_t year;
      Int_t runNumber;
      ULong64_t eventNumber;
      Int_t lumiBlock;
      Int_t bcid;
      Float_t averageIntPerXing;
      Float_t corAverageIntPerXing;

      Float_t mcEventWeight;
      std::vector<Float_t> mcEventWeights;
      Float_t puWeight;
      Float_t elSFWeight;
      Float_t muSFWeight;
      Float_t elSFTrigWeight;
      Float_t muSFTrigWeight;
      Float_t btagSFWeight;
      Float_t jvtSFWeight;

/*
      Int_t pdf_id1;
      Int_t pdf_id2;
      Float_t pdf_x1;
      Float_t pdf_x2;
      Float_t pdf_pdf1;
      Float_t pdf_pdf2;
      Float_t pdf_scale;
      */

      Int_t n_jet_truth;
      std::vector<Float_t> truth_jet_pt;
      std::vector<Float_t> truth_jet_eta;
      std::vector<Float_t> truth_jet_phi;
      std::vector<Float_t> truth_jet_m;

 /*     Float_t truth_V_bare_pt;
      Float_t truth_V_bare_eta;
      Float_t truth_V_bare_phi;
      Float_t truth_V_bare_m;*/

      Float_t truth_V_dressed_pt;
      Float_t truth_V_dressed_eta;
      Float_t truth_V_dressed_phi;
      Float_t truth_V_dressed_m;

      Int_t n_el_truth;
      std::vector<Float_t> truth_el_pt;
      std::vector<Float_t> truth_el_eta;
      std::vector<Float_t> truth_el_phi;
      std::vector<Float_t> truth_el_m;
      std::vector<Int_t>   truth_el_status;

      Int_t n_mu_truth;
      std::vector<Float_t> truth_mu_pt;
      std::vector<Float_t> truth_mu_eta;
      std::vector<Float_t> truth_mu_phi;
      std::vector<Float_t> truth_mu_m;
      std::vector<Int_t>   truth_mu_status;

/*
      Int_t n_tau_truth;
      std::vector<Float_t> truth_tau_pt;
      std::vector<Float_t> truth_tau_eta;
      std::vector<Float_t> truth_tau_phi;
      std::vector<Float_t> truth_tau_m;
      std::vector<Int_t>   truth_tau_status;
      */

      std::map<TString, Int_t> trigger;
      Int_t trigger_lep;

      Int_t passGRL;
//      Int_t passTrigger;
      Int_t passPV;
      Int_t passJetCleanLoose;
      Int_t passJetCleanTight;
      Int_t passDetErr;

      Int_t n_vx;
      Int_t n_jet;
      Double_t jj_mass;
      Double_t jj_deta;
      Double_t jj_dphi;
      Double_t met_tst_j1_dphi;
      Double_t met_tst_j2_dphi;
      Double_t met_tst_nolep_j1_dphi;
      Double_t met_tst_nolep_j2_dphi;
      Double_t met_cst_jet;
//      Double_t met_tst_nomuon_j1_dphi;
//      Double_t met_tst_nomuon_j2_dphi;
//      Double_t met_tst_noelectron_j1_dphi;
//      Double_t met_tst_noelectron_j2_dphi;
      Int_t n_bjet;
      Int_t n_mu;
      Int_t n_mu_baseline;
      Int_t n_el;
      Int_t n_el_baseline;


public:
      outEvent(TString name = "", Bool_t doTrim = kFALSE);
      ~outEvent();
      void reset();
      void attachToTree(TTree *tree);
      void setTriggers(const std::vector<std::string> &trigs);
};
}

#endif

