#include "VBFInvAnalysis/outEvent.h"

#include <TTree.h>
#include <TError.h>

#include <string>

Analysis::outEvent::outEvent(TString name, Bool_t doTrim, Bool_t doDetail) : Analysis::outObject::outObject(name, doTrim, doDetail)
{
 reset();
}

Analysis::outEvent::~outEvent()
{
}

void Analysis::outEvent::reset()
{

    // General
 year = 0;
 runNumber = -9999;
 eventNumber = 0;
 lumiBlock = -9999;
 bcid = -9999;
 averageIntPerXing = -9999;
 corAverageIntPerXing = -9999;

 flag_lar = 0;
 flag_tile = 0;
 flag_sct = 0;
 flag_core = 0;
 flag_bib = 0;
 flag_bib_raw = 0;

   // Weights
 mcEventWeight = 1.0;
 mcEventWeights.clear();
 puWeight = 1.0;
 btagSFWeight = 1.0;
 jvtSFWeight = 1.0;
 elSFWeight = 1.0;
 muSFWeight = 1.0;
 elSFTrigWeight = 1.0;
 muSFTrigWeight = 1.0;


  // PDF
 pdf_id1 = -9999;
 pdf_id2 = -9999;
 pdf_x1 = -9999;
 pdf_x2 = -9999;
 pdf_pdf1 = -9999;
 pdf_pdf2 = -9999;
 pdf_scale = -9999;

// Truth
 n_jet_truth = -9999;
 truth_jet1_pt = -9999;
 truth_jet1_eta = -9999;
 truth_jet1_phi = -9999;
 truth_jet1_m = -9999;
 truth_jet2_pt = -9999;
 truth_jet2_eta = -9999;
 truth_jet2_phi = -9999;
 truth_jet2_m = -9999;
 truth_V_bare_pt = -9999;
 truth_V_bare_eta = -9999;
 truth_V_bare_phi = -9999;
 truth_V_bare_m = -9999;
 truth_V_dressed_pt = -9999;
 truth_V_dressed_eta = -9999;
 truth_V_dressed_phi = -9999;
 truth_V_dressed_m = -9999;
 truth_mu_pt.clear();
 truth_mu_eta.clear();
 truth_mu_phi.clear();
 truth_mu_m.clear();
 truth_mu_status.clear();
 truth_el_pt.clear();
 truth_el_eta.clear();
 truth_el_phi.clear();
 truth_el_m.clear();
 truth_el_status.clear();


 for (auto &kv : trigger) {
  kv.second = -9999;
}

passGRL = -9999;
passTrigger = -9999;
passPV = -9999;
passJetClean = -9999;
passJetCleanTight = -9999;
passDetErr = -9999;

n_vx   = -9999;
n_jet  = -9999;
n_bjet = -9999;
n_el = -9999;
n_el_baseline = -9999;
n_mu = -9999;
n_mu_baseline = -9999;
jj_mass = -9999;
jj_deta = -9999; 
jj_dphi = -9999;
met_tst_j1_dphi = -9999;
met_tst_j2_dphi= -9999;
met_tst_nomuon_j1_dphi= -9999;
met_tst_nomuon_j2_dphi= -9999;
met_tst_noelectron_j1_dphi= -9999;
met_tst_noelectron_j2_dphi= -9999;

GenMET_pt  = -9999;
GenMET_phi  = -9999;
TrueMHT_pt  = -9999;
TrueMHT_phi  = -9999;

return;
}

void Analysis::outEvent::attachToTree(TTree *tree)
{
   const TString prefix = (name() != "") ? name() + "_" : ""; // no prefix by default

 tree->Branch(prefix + "year", &year);
   tree->Branch(prefix + "runNumber", &runNumber);
   tree->Branch(prefix + "eventNumber", &eventNumber);
   tree->Branch(prefix + "lumiBlock", &lumiBlock);
   tree->Branch(prefix + "bcid", &bcid);
   tree->Branch(prefix + "averageIntPerXing", &averageIntPerXing);
   tree->Branch(prefix + "corAverageIntPerXing", &corAverageIntPerXing);

   tree->Branch(prefix + "mcEventWeight", &mcEventWeight);
   if (!doTrim()) {
    tree->Branch(prefix + "mcEventWeights", &mcEventWeights);
  }
  tree->Branch(prefix + "puWeight", &puWeight);
  tree->Branch(prefix + "btagSFWeight", &btagSFWeight);
  tree->Branch(prefix + "jvtSFWeight", &jvtSFWeight);
  tree->Branch(prefix + "elSFWeight", &elSFWeight);
  tree->Branch(prefix + "muSFWeight", &muSFWeight);
  tree->Branch(prefix + "elSFTrigWeight", &elSFWeight);
  tree->Branch(prefix + "muSFTrigWeight", &muSFWeight);

  if (!doTrim()) {
      for (auto &itrig : trigger) {
    const TString trigName = itrig.first;
    /*if (trigName  == "HLT_xe70"
      || trigName == "HLT_xe80_tc_lcw_L1XE50"
      || trigName == "HLT_xe90_mht_L1XE50"
      || trigName == "HLT_xe100_mht_L1XE50"
      || trigName == "HLT_xe110_mht_L1XE50"
      || trigName == "HLT_xe130_mht_L1XE50"
      || trigName == "HLT_xe120_L1XE50"
      || trigName == "HLT_xe110_L1XE50"
      )*/
    tree->Branch(prefix + "trigger_" + trigName, &itrig.second);
  }

    tree->Branch(prefix + "flag_lar", &flag_lar);
    tree->Branch(prefix + "flag_tile", &flag_tile);
    tree->Branch(prefix + "flag_sct", &flag_sct);
    tree->Branch(prefix + "flag_core", &flag_core);
    tree->Branch(prefix + "flag_bib", &flag_bib);
    tree->Branch(prefix + "flag_bib_raw", &flag_bib_raw);

    tree->Branch(prefix + "pdf_id1", &pdf_id1);
    tree->Branch(prefix + "pdf_id2", &pdf_id2);
    tree->Branch(prefix + "pdf_x1", &pdf_x1);
    tree->Branch(prefix + "pdf_x2", &pdf_x2);
    tree->Branch(prefix + "pdf_pdf1", &pdf_pdf1);
    tree->Branch(prefix + "pdf_pdf2", &pdf_pdf2);
    tree->Branch(prefix + "pdf_scale", &pdf_scale);

    tree->Branch(prefix + "n_jet_truth", &n_jet_truth);
    tree->Branch(prefix + "truth_jet1_pt", &truth_jet1_pt);
    tree->Branch(prefix + "truth_jet1_eta", &truth_jet1_eta);
    tree->Branch(prefix + "truth_jet1_phi", &truth_jet1_phi);
    tree->Branch(prefix + "truth_jet1_m", &truth_jet1_m);
    tree->Branch(prefix + "truth_jet2_pt", &truth_jet2_pt);
    tree->Branch(prefix + "truth_jet2_eta", &truth_jet2_eta);
    tree->Branch(prefix + "truth_jet2_phi", &truth_jet2_phi);
    tree->Branch(prefix + "truth_jet2_m", &truth_jet2_m);
    tree->Branch(prefix + "truth_mu_pt", &truth_mu_pt);
    tree->Branch(prefix + "truth_mu_eta", &truth_mu_eta);
    tree->Branch(prefix + "truth_mu_phi", &truth_mu_phi);
    tree->Branch(prefix + "truth_mu_m", &truth_mu_m);
    tree->Branch(prefix + "truth_mu_status", &truth_mu_status);
    tree->Branch(prefix + "truth_el_pt", &truth_el_pt);
    tree->Branch(prefix + "truth_el_eta", &truth_el_eta);
    tree->Branch(prefix + "truth_el_phi", &truth_el_phi);
    tree->Branch(prefix + "truth_el_m", &truth_el_m);
    tree->Branch(prefix + "truth_el_status", &truth_el_status);
    tree->Branch(prefix + "truth_V_bare_pt", &truth_V_bare_pt);
    tree->Branch(prefix + "truth_V_bare_eta", &truth_V_bare_eta);
    tree->Branch(prefix + "truth_V_bare_phi", &truth_V_bare_phi);
    tree->Branch(prefix + "truth_V_bare_m", &truth_V_bare_m);
    tree->Branch(prefix + "truth_V_dressed_pt", &truth_V_dressed_pt);
    tree->Branch(prefix + "truth_V_dressed_eta", &truth_V_dressed_eta);
    tree->Branch(prefix + "truth_V_dressed_phi", &truth_V_dressed_phi);
    tree->Branch(prefix + "truth_V_dressed_m", &truth_V_dressed_m);

  }

  tree->Branch(prefix + "passGRL", &passGRL);
  tree->Branch(prefix + "passTrigger", &passTrigger);
  tree->Branch(prefix + "passPV", &passPV);
  tree->Branch(prefix + "passJetClean", &passJetClean);
  tree->Branch(prefix + "passJetCleanTight", &passJetCleanTight);
  tree->Branch(prefix + "passDetErr", &passDetErr);

  tree->Branch(prefix + "n_vx", &n_vx);
  tree->Branch(prefix + "n_jet", &n_jet);
  tree->Branch(prefix + "n_bjet", &n_bjet);
  tree->Branch(prefix + "n_el", &n_el);
  tree->Branch(prefix + "n_mu", &n_mu);
  tree->Branch(prefix + "n_el_baseline", &n_el_baseline);
  tree->Branch(prefix + "n_mu_baseline", &n_mu_baseline);

  tree->Branch(prefix + "jj_mass", &jj_mass);
  tree->Branch(prefix + "jj_deta", &jj_deta);
  tree->Branch(prefix + "jj_dphi", &jj_dphi);
  tree->Branch(prefix + "met_tst_j1_dphi", &met_tst_j1_dphi);
  tree->Branch(prefix + "met_tst_j2_dphi", &met_tst_j2_dphi);
  tree->Branch(prefix + "met_tst_nomuon_j1_dphi", &met_tst_nomuon_j1_dphi);
  tree->Branch(prefix + "met_tst_nomuon_j2_dphi", &met_tst_nomuon_j2_dphi);
  tree->Branch(prefix + "met_tst_noelectron_j1_dphi", &met_tst_noelectron_j1_dphi);
  tree->Branch(prefix + "met_tst_noelectron_j2_dphi", &met_tst_noelectron_j2_dphi);

  tree->Branch(prefix + "GenMET_pt", &GenMET_pt);
  tree->Branch(prefix + "GenMET_phi", &GenMET_phi);
  tree->Branch(prefix + "TrueMHT_pt", &TrueMHT_pt);
  tree->Branch(prefix + "TrueMHT_phi", &TrueMHT_phi);

  return;
}

void Analysis::outEvent::setTriggers(const std::vector<std::string> &trigs)
{
 for (auto &trigName : trigs) {
  trigger[trigName] = 0;
}

}
