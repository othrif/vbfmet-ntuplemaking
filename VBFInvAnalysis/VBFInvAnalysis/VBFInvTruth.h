#ifndef VBFInvAnalysis_VBFInvTruth_H
#define VBFInvAnalysis_VBFInvTruth_H

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "AsgTools/ToolHandle.h"

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <SUSYTools/SUSYCrossSection.h>

namespace ORUtils {
  class IOverlapRemovalTool;
}

class VBFInvTruth : public EL::Algorithm
{

private:
  SUSY::CrossSectionDB *my_XsecDB;  //!
  TTree* truthTree; //!

  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

 std::string outputName = "HADAHA";
 bool debug;
 bool skipCBK;
 bool MultiWeight;

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:

  int m_njets; //!
  std::vector<float> *m_jet_E; //!
  std::vector<float> *m_jet_pt; //!
  std::vector<float> *m_jet_eta; //!
  std::vector<float> *m_jet_phi; //!
  std::vector<float> *m_jet_m; //!
  std::vector<int> *m_jet_label; //!

  int m_nels; //!
  std::vector<float> *m_el_m; //!
  std::vector<float> *m_el_pt; //!
  std::vector<float> *m_el_eta; //!
  std::vector<float> *m_el_phi; //!
  std::vector<uint> *m_el_type; //!
  std::vector<uint> *m_el_origin; //!
  std::vector<float> *m_el_ptcone30; //!
  std::vector<float> *m_el_etcone20; //!
  std::vector<int> *m_el_pdgid; //!

  int m_nmus; //!
  std::vector<float> *m_mu_m; //!
  std::vector<float> *m_mu_pt; //!
  std::vector<float> *m_mu_eta; //!
  std::vector<float> *m_mu_phi; //!
  std::vector<uint> *m_mu_type; //!
  std::vector<uint> *m_mu_origin; //!
  std::vector<float> *m_mu_ptcone30; //!
  std::vector<float> *m_mu_etcone20; //!
  std::vector<int> *m_mu_pdgid; //!

  int m_ntaus; //!
  std::vector<float> *m_tau_m; //!
  std::vector<float> *m_tau_pt; //!
  std::vector<float> *m_tau_eta; //!
  std::vector<float> *m_tau_phi; //!
  std::vector<uint> *m_tau_type; //!
  std::vector<uint> *m_tau_origin; //!
  std::vector<int> *m_tau_pdgid; //!
  std::vector<char> *m_tau_IsHadronicTau; //!
  std::vector<double> *m_tau_m_invis; //!
  std::vector<double> *m_tau_m_vis; //!
  std::vector<ULong_t> *m_tau_numCharged; //!
  std::vector<ULong_t> *m_tau_numChargedPion; //!
  std::vector<ULong_t> *m_tau_numNeutral; //!
  std::vector<ULong_t> *m_tau_numNeutralPion; //!
  std::vector<int> *m_tau_barcode; //!
  std::vector<int> *m_tau_status; //!

  int m_nbosons; //!
  std::vector<float> *m_boson_e; //!
  std::vector<float> *m_boson_m; //!
  std::vector<float> *m_boson_pt; //!
  std::vector<float> *m_boson_eta; //!
  std::vector<float> *m_boson_phi; //!
  std::vector<int> *m_boson_pdgid; //!

  int m_nnus; //!
  std::vector<float> *m_nu_e; //!
  std::vector<float> *m_nu_m; //!
  std::vector<float> *m_nu_pt; //!
  std::vector<float> *m_nu_eta; //!
  std::vector<float> *m_nu_phi; //!
  std::vector<uint> *m_nu_type; //!
  std::vector<uint> *m_nu_origin; //!
  std::vector<int> *m_nu_pdgid; //!

  std::vector<float> *m_parton_x1; //!
  std::vector<float> *m_parton_x2; //!
  std::vector<float> *m_parton_xf1; //!
  std::vector<float> *m_parton_xf2; //!
  std::vector<float> *m_parton_Q; //!
  std::vector<int> *m_parton_pdgid1; //!
  std::vector<int> *m_parton_pdgid2; //!
  std::vector<int> *m_parton_pdfid1; //!
  std::vector<int> *m_parton_pdfid2; //!
  std::vector<int> *m_parton_pp; //!

  float m_met_et; //!
  float m_met_phi; //!

  ULong64_t m_EventNumber; //!
  UInt_t m_RunNumber; //!
  double m_crossSection; //!
  float m_WeightEvents; //!
  UInt_t m_ChannelNumber; //!

  TH1D *NumberEvents; //!

  xAOD::TEvent *m_event;  //!

  // this is a standard constructor
  VBFInvTruth ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode fileExecute();
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(VBFInvTruth, 1);
};

#endif
