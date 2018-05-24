#ifndef VBFINVANALYSIS_VBFINV_H
#define VBFINVANALYSIS_VBFINV_H

// System include(s):
#include <string>

// Core include(s):
#include "EventLoop/Algorithm.h"
#include <AsgTools/AnaToolHandle.h>
#include <PathResolver/PathResolver.h>
#include <AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h>
#include <SUSYTools/ISUSYObjDef_xAODTool.h>
#include <VBFInvAnalysis/VBFInv_Defs.h>

// ROOT include(s):
#include <TROOT.h>
#include <TH1.h>
#include <TTree.h>
#include <TEnv.h>

// VBFInv classes
#include <VBFInvAnalysis/contentHolder.h>
#include <VBFInvAnalysis/outHolder.h>
#include <VBFInvAnalysis/CutFlowTool.h>

class VBFInv : public EL::Algorithm {

public:

 using EL::Algorithm::Algorithm;

  // this is a standard algorithm constructor
 VBFInv ();

  // these are the functions inherited from Algorithm
 virtual EL::StatusCode setupJob( EL::Job& job );
 virtual EL::StatusCode histInitialize();
 virtual EL::StatusCode fileExecute();
 virtual EL::StatusCode initialize();
 virtual EL::StatusCode execute();
 virtual EL::StatusCode finalize();
 virtual EL::StatusCode histFinalize();

   // analysis
 virtual EL::StatusCode readConfig();
 std::vector<std::string> getTokens(TString line, TString delim);
 virtual EL::StatusCode analyzeEvent(Analysis::ContentHolder &content, const ST::SystInfo &systInfo, Analysis::outHolder &cand);
 virtual EL::StatusCode fillTree(Analysis::ContentHolder &content, Analysis::outHolder &cand);
 virtual EL::StatusCode getMET(std::shared_ptr<xAOD::MissingETContainer> &met, std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer *jet, xAOD::ElectronContainer *el, xAOD::MuonContainer *mu, xAOD::PhotonContainer *ph, Bool_t doTST, Bool_t doJVT, xAOD::IParticleContainer *invis, TLorentzVector &myMET, double &myMETSig);
 virtual EL::StatusCode getTrackMET(std::shared_ptr<xAOD::MissingETContainer> &met, std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer *jet, xAOD::ElectronContainer *el, xAOD::MuonContainer *mu);
 void printObjects(xAOD::IParticleContainer obj, TString label);
 void printMET(TLorentzVector myMET, double myMETsig, TLorentzVector myTruthMET, TString label);
 void printMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label);
 void printTrackMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label);
 void print(TString label, float val);

   /// Name of the outputs
 std::string outputName = "HADAHA";

   // config file
  Bool_t debug;
  TString config_file;
  TString ST_config_file;
  TString prw_file;
  TString lumicalc_file;
  TString GRL_file;
  TString MC_campaign;
  TString skip_syst;
  TString trigger_list;
  Double_t pt1Skim;
  Double_t pt1SkimForSyst;
  Double_t pt2Skim;
  Double_t pt2SkimForSyst;
  Double_t metSkim;
  Double_t metSkimForSyst;
  Double_t mjjSkim;
  Double_t mjjSkimForSyst;
  Double_t detajjSkim;
  Double_t detajjSkimForSyst;
  Bool_t doPileup;
  Bool_t doSystematics;
  Bool_t doSkim;

private:
  // Configuration, and any other types of variables go here.

  TH1D *m_NumberEvents; //!
  Analysis::CutFlowTool m_CutFlow; //!

// Analysis
   Bool_t m_isMC; //!
   Bool_t m_isAFII; //!
   Int_t m_eventCounter; //!
   Bool_t m_determinedDerivation; //!
   Bool_t m_isEXOT5; //!

  asg::AnaToolHandle<IGoodRunsListSelectionTool> m_grl; //!
  asg::AnaToolHandle<ST::ISUSYObjDef_xAODTool> m_susytools_handle; //!

  xAOD::TEvent *m_event; //!
  xAOD::TStore *m_store; //!

   std::vector<ST::SystInfo> m_sysList; //!
   std::map<TString, TTree*> m_tree; //!
   std::map<TString, Analysis::outHolder> m_cand; //!

   Analysis::ContentHolder m_content_current; //!
   Analysis::ContentHolder m_content_nominal; //!

   /// Declare the class to ROOT
  ClassDef( VBFInv, 1 )

}; // class VBFInv

#endif