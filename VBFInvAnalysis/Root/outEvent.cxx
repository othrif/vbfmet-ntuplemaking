#include "VBFInvAnalysis/outEvent.h"

#include <TTree.h>
#include <TError.h>

#include <string>

Analysis::outEvent::outEvent(TString name, Bool_t doTrim) : Analysis::outObject::outObject(name, doTrim)
{
   reset();
}

Analysis::outEvent::~outEvent() {}

void Analysis::outEvent::reset()
{

   // General
   year                 = 0;
   runNumber            = -9999;
   runPeriod            = -9999;
   randomRunNumber      = -9999;
   eventNumber          = 0;
   lumiBlock            = -9999;
   bcid                 = -9999;
   BCIDDistanceFromFront= -9999;
   BCIDDistanceFromTail = -9999;
   averageIntPerXing    = -9999;
   corAverageIntPerXing = -9999;
   vtx_sumpt2           = -9999;
   l1_met_trig_encoded  = 0;

   // Weights
   mcEventWeight     = 1.0;
   mcEventWeightXsec = 1.0;
   mcEventWeights.clear();
   puWeight       = 1.0;
   btagSFWeight   = 1.0;
   jvtSFWeight    = 1.0;
   fjvtSFWeight   = 1.0;
   fjvtSFTighterWeight   = 1.0;
   elSFWeight     = 1.0;
   muSFWeight     = 1.0;
   elSFTrigWeight = 1.0;
   muSFTrigWeight = 1.0;
   eleANTISF      = 1.0;

   // truth filters
   FlavourFilter=-9999;
   MGVTruthPt=-9999;
   SherpaVTruthPt=-9999;
   in_vy_overlap=false;
   in_vy_overlap_iso=false;

   // PDF
   /*
  pdf_id1 = -9999;
  pdf_id2 = -9999;
  pdf_x1 = -9999;
  pdf_x2 = -9999;
  pdf_pdf1 = -9999;
  pdf_pdf2 = -9999;
  pdf_scale = -9999;
  */

   // Truth
   n_jet_truth = -9999;
   truth_jet_pt.clear();
   truth_jet_eta.clear();
   truth_jet_phi.clear();
   truth_jet_m.clear();
   truth_jet_label.clear();
   truth_jetmu_pt.clear();
   truth_jetmu_eta.clear();
   truth_jetmu_phi.clear();
   truth_jetmu_m.clear();
   truth_jetmunu_pt.clear();
   truth_jetmunu_eta.clear();
   truth_jetmunu_phi.clear();
   truth_jetmunu_m.clear();
   n_fatjet_truth = -9999;
   truth_fatjet_pt.clear();
   truth_fatjet_eta.clear();
   truth_fatjet_phi.clear();
   truth_fatjet_m.clear();
   truth_V_bare_pt     = -9999;
   truth_V_bare_eta    = -9999;
   truth_V_bare_phi    = -9999;
   truth_V_bare_m      = -9999;
   truth_V_dressed_pt  = -9999;
   truth_V_dressed_eta = -9999;
   truth_V_dressed_phi = -9999;
   truth_V_dressed_m   = -9999;
   n_mu_truth          = -9999;
   truth_mu_pt.clear();
   truth_mu_eta.clear();
   truth_mu_phi.clear();
   truth_mu_m.clear();
   truth_mu_status.clear();
   n_el_truth = -9999;
   truth_el_pt.clear();
   truth_el_eta.clear();
   truth_el_phi.clear();
   truth_el_m.clear();
   truth_el_status.clear();
   n_tau_truth = -9999;
   truth_tau_pt.clear();
   truth_tau_eta.clear();
   truth_tau_phi.clear();
   truth_tau_m.clear();
   truth_tau_status.clear();

   for (auto &kv : trigger) {
      kv.second = -9999;
   }
   trigger_lep = -9999;
   trigger_met = -9999;

   passGRL           = -9999;
   passPV            = -9999;
   passJetCleanLoose = -9999;
   passJetCleanTight = -9999;
   passDetErr        = -9999;
   passBatman        = false;
   passVjetsFilter   = false;
   passVjetsPTV      = false;

   n_vx          = -9999;
   n_jet         = -9999;
   n_bjet        = -9999;
   n_el          = -9999;
   n_el_baseline = 0;
   n_mu          = -9999;
   n_mu_baseline_noOR = 0;
   n_mu_baseline = 0;
   n_ph          = 0;

   jj_mass               = -9999;
   jj_deta               = -9999;
   jj_dphi               = -9999;
   met_tst_j1_dphi       = -9999;
   met_tst_j2_dphi       = -9999;
   met_tst_nolep_j1_dphi = -9999;
   met_tst_nolep_j2_dphi = -9999;
   met_cst_jet           = -9999;
   metsig_tst            = -9999;
   metsig_tst_nolep      = -9999;

   truthF_jj_mass = -9999;
   truthF_jj_deta = -9999;
   truthF_jj_dphi = -9999;

   truth_vtx_z = -9999;
   reco_vtx_ntrk.clear();
   reco_vtx_x.clear();
   reco_vtx_y.clear();
   reco_vtx_z.clear();
   reco_vtx_sumPt2.clear();
   reco_vtx_chiSquared.clear();
   reco_vtx_vertexType.clear();

   // clear the map
   for (std::map<TString, Float_t>::iterator it = syst_var_map.begin(); it != syst_var_map.end(); ++it) {
      it->second = 1.0;
   }
   return;
}

void Analysis::outEvent::attachToTree(TTree *tree)
{

   const TString prefix = (name() != "") ? name() + "_" : ""; // no prefix by default

   tree->Branch(prefix + "year", &year);
   tree->Branch(prefix + "runNumber", &runNumber);
   tree->Branch(prefix + "runPeriod", &runPeriod);
   tree->Branch(prefix + "randomRunNumber", &randomRunNumber);
   tree->Branch(prefix + "eventNumber", &eventNumber);
   tree->Branch(prefix + "lumiBlock", &lumiBlock);
   tree->Branch(prefix + "bcid", &bcid);
   tree->Branch(prefix + "BCIDDistanceFromFront", &BCIDDistanceFromFront);
   tree->Branch(prefix + "BCIDDistanceFromTail",  &BCIDDistanceFromTail);
   tree->Branch(prefix + "averageIntPerXing", &averageIntPerXing);
   tree->Branch(prefix + "corAverageIntPerXing", &corAverageIntPerXing);
   tree->Branch(prefix + "mcEventWeight", &mcEventWeight);
   if (!doTrim()) tree->Branch(prefix + "vtx_sumpt2", &vtx_sumpt2);
   if (!doTrim()) tree->Branch(prefix + "mcEventWeightXsec", &mcEventWeightXsec);
   if (!doTrim()) tree->Branch(prefix + "mcEventWeights", &mcEventWeights);
   if (!doTrim()) tree->Branch("l1_met_trig_encoded", &l1_met_trig_encoded);

   tree->Branch(prefix + "puWeight", &puWeight);
   tree->Branch(prefix + "btagSFWeight", &btagSFWeight);
   tree->Branch(prefix + "jvtSFWeight", &jvtSFWeight);
   tree->Branch(prefix + "fjvtSFWeight", &fjvtSFWeight);
   tree->Branch(prefix + "fjvtSFTighterWeight", &fjvtSFTighterWeight);
   tree->Branch(prefix + "elSFWeight", &elSFWeight);
   tree->Branch(prefix + "muSFWeight", &muSFWeight);
   tree->Branch(prefix + "elSFTrigWeight", &elSFTrigWeight);
   tree->Branch(prefix + "muSFTrigWeight", &muSFTrigWeight);
   tree->Branch(prefix + "eleANTISF", &eleANTISF);

   tree->Branch(prefix + "FlavourFilter", &FlavourFilter);
   tree->Branch(prefix + "MGVTruthPt", &MGVTruthPt);
   tree->Branch(prefix + "SherpaVTruthPt", &SherpaVTruthPt);
   tree->Branch(prefix + "in_vy_overlap", &in_vy_overlap);
   tree->Branch(prefix + "in_vy_overlap_iso", &in_vy_overlap_iso);

   for (auto &itrig : trigger) {
      const TString trigName = itrig.first;
      // std::cout << "Trigger: " << itrig.first << std::endl;
      if (
         // MET 2015-2016
         trigName == "HLT_xe70_mht" || trigName == "HLT_xe90_mht_L1XE50" || trigName == "HLT_xe100_mht_L1XE50" ||
         trigName == "HLT_xe110_mht_L1XE50" || trigName == "HLT_noalg_L1J400" ||
         trigName == "HLT_j70_j50_0eta490_invm1100j70_dphi20_deta40_L1MJJ-500-NFF" ||
         trigName == "HLT_j70_j50_0eta490_invm1000j50_dphi24_xe90_pufit_xe50_L1MJJ-500-NFF")
         tree->Branch(prefix + "trigger_" + trigName, &itrig.second);
      if (!doTrim() && (trigName == "HLT_2j35_btight_2j35_L13J25.0ETA23" ||
                        trigName == "HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25" ||
                        trigName == "HLT_2j15_gsc35_bmv2c1040_split_2j15_gsc35_boffperf_split_L14J15.0ETA25" ||
                        trigName == "HLT_2j35_bmv2c1060_split_2j35_L14J15.0ETA25"))
         tree->Branch(prefix + "trigger_" + trigName, &itrig.second);
   }
   tree->Branch(prefix + "trigger_lep", &trigger_lep);
   tree->Branch(prefix + "trigger_met", &trigger_met);

   /*
       tree->Branch(prefix + "pdf_id1", &pdf_id1);
       tree->Branch(prefix + "pdf_id2", &pdf_id2);
       tree->Branch(prefix + "pdf_x1", &pdf_x1);
       tree->Branch(prefix + "pdf_x2", &pdf_x2);
       tree->Branch(prefix + "pdf_pdf1", &pdf_pdf1);
       tree->Branch(prefix + "pdf_pdf2", &pdf_pdf2);
       tree->Branch(prefix + "pdf_scale", &pdf_scale);
   */
   if (!doTrim()) {
      tree->Branch(prefix + "n_jet_truth", &n_jet_truth);
      tree->Branch(prefix + "truth_jet_pt", &truth_jet_pt);
      tree->Branch(prefix + "truth_jet_eta", &truth_jet_eta);
      tree->Branch(prefix + "truth_jet_phi", &truth_jet_phi);
      tree->Branch(prefix + "truth_jet_m", &truth_jet_m);
      tree->Branch(prefix + "truth_jet_label", &truth_jet_label);
      if (!doExtraTrim()) {
         tree->Branch(prefix + "truth_jetmu_pt", &truth_jetmu_pt);
         tree->Branch(prefix + "truth_jetmu_eta", &truth_jetmu_eta);
         tree->Branch(prefix + "truth_jetmu_phi", &truth_jetmu_phi);
         tree->Branch(prefix + "truth_jetmu_m", &truth_jetmu_m);
         tree->Branch(prefix + "truth_jetmunu_pt", &truth_jetmunu_pt);
         tree->Branch(prefix + "truth_jetmunu_eta", &truth_jetmunu_eta);
         tree->Branch(prefix + "truth_jetmunu_phi", &truth_jetmunu_phi);
         tree->Branch(prefix + "truth_jetmunu_m", &truth_jetmunu_m);
         tree->Branch(prefix + "n_fatjet_truth", &n_fatjet_truth);
         tree->Branch(prefix + "truth_fatjet_pt", &truth_fatjet_pt);
         tree->Branch(prefix + "truth_fatjet_eta", &truth_fatjet_eta);
         tree->Branch(prefix + "truth_fatjet_phi", &truth_fatjet_phi);
         tree->Branch(prefix + "truth_fatjet_m", &truth_fatjet_m);
      }
      tree->Branch(prefix + "n_mu_truth", &n_mu_truth);
      tree->Branch(prefix + "truth_mu_pt", &truth_mu_pt);
      tree->Branch(prefix + "truth_mu_eta", &truth_mu_eta);
      tree->Branch(prefix + "truth_mu_phi", &truth_mu_phi);
      tree->Branch(prefix + "truth_mu_m", &truth_mu_m);
      tree->Branch(prefix + "truth_mu_status", &truth_mu_status);
      tree->Branch(prefix + "n_el_truth", &n_el_truth);
      tree->Branch(prefix + "truth_el_pt", &truth_el_pt);
      tree->Branch(prefix + "truth_el_eta", &truth_el_eta);
      tree->Branch(prefix + "truth_el_phi", &truth_el_phi);
      tree->Branch(prefix + "truth_el_m", &truth_el_m);
      tree->Branch(prefix + "truth_el_status", &truth_el_status);
      tree->Branch(prefix + "n_tau_truth", &n_tau_truth);
      tree->Branch(prefix + "truth_tau_pt", &truth_tau_pt);
      tree->Branch(prefix + "truth_tau_eta", &truth_tau_eta);
      tree->Branch(prefix + "truth_tau_phi", &truth_tau_phi);
      tree->Branch(prefix + "truth_tau_m", &truth_tau_m);
      tree->Branch(prefix + "truth_tau_status", &truth_tau_status);
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
   tree->Branch(prefix + "passPV", &passPV);
   tree->Branch(prefix + "passJetCleanLoose", &passJetCleanLoose);
   tree->Branch(prefix + "passJetCleanTight", &passJetCleanTight);
   tree->Branch(prefix + "passDetErr", &passDetErr);

   if (!doTrim()) tree->Branch(prefix + "passBatman", &passBatman);
   tree->Branch(prefix + "passVjetsFilter", &passVjetsFilter);
   tree->Branch(prefix + "passVjetsPTV", &passVjetsPTV);

   tree->Branch(prefix + "n_vx", &n_vx);
   tree->Branch(prefix + "n_jet", &n_jet);
   tree->Branch(prefix + "n_bjet", &n_bjet);
   tree->Branch(prefix + "n_el", &n_el);
   tree->Branch(prefix + "n_mu", &n_mu);
   tree->Branch(prefix + "n_el_baseline", &n_el_baseline);
   tree->Branch(prefix + "n_mu_baseline_noOR", &n_mu_baseline_noOR);
   tree->Branch(prefix + "n_mu_baseline", &n_mu_baseline);
   tree->Branch(prefix + "n_ph", &n_ph);

   tree->Branch(prefix + "jj_mass", &jj_mass);
   tree->Branch(prefix + "jj_deta", &jj_deta);
   tree->Branch(prefix + "jj_dphi", &jj_dphi);
   tree->Branch(prefix + "met_tst_j1_dphi", &met_tst_j1_dphi);
   tree->Branch(prefix + "met_tst_j2_dphi", &met_tst_j2_dphi);
   tree->Branch(prefix + "met_tst_nolep_j1_dphi", &met_tst_nolep_j1_dphi);
   tree->Branch(prefix + "met_tst_nolep_j2_dphi", &met_tst_nolep_j2_dphi);
   tree->Branch(prefix + "met_cst_jet", &met_cst_jet);
   tree->Branch(prefix + "metsig_tst", &metsig_tst);
   tree->Branch(prefix + "metsig_tst_nolep", &metsig_tst_nolep);

   tree->Branch(prefix + "truthF_jj_mass", &truthF_jj_mass);
   tree->Branch(prefix + "truthF_jj_deta", &truthF_jj_deta);
   tree->Branch(prefix + "truthF_jj_dphi", &truthF_jj_dphi);

   if (!doTrim() && doVertexDetail()) {
      tree->Branch(prefix + "truth_vtx_z", &truth_vtx_z);
      tree->Branch(prefix + "reco_vtx_ntrk", &reco_vtx_ntrk);
      tree->Branch(prefix + "reco_vtx_x", &reco_vtx_x);
      tree->Branch(prefix + "reco_vtx_y", &reco_vtx_y);
      tree->Branch(prefix + "reco_vtx_z", &reco_vtx_z);
      tree->Branch(prefix + "reco_vtx_sumPt2", &reco_vtx_sumPt2);
      tree->Branch(prefix + "reco_vtx_chiSquared", &reco_vtx_chiSquared);
      tree->Branch(prefix + "reco_vtx_vertexType", &reco_vtx_vertexType);
   }
   return;
}

void Analysis::outEvent::setTriggers(const std::vector<std::string> &trigs)
{
   for (auto &trigName : trigs) {
      trigger[trigName] = 0;
   }
}

float &Analysis::outEvent::GetSystVar(TString var, TString syst, TTree *tree)
{

   const TString var_name = ((name() != "") ? name() + "_" : "") + var + syst; // no prefix by default
   if (syst_var_map.find(var_name) == syst_var_map.end()) {
      syst_var_map[var_name] = 1.0;
      tree->Branch(var_name, &syst_var_map[var_name]);
   }

   return syst_var_map[var_name];
}
