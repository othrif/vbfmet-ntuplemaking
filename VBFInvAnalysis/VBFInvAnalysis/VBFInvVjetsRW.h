#ifndef VBFInvAnalysis_VBFInvVjetsRW_H
#define VBFInvAnalysis_VBFInvVjetsRW_H

#include <EventLoop/Algorithm.h>
//#include "xAODEventInfo/EventInfo.h"
#include <AsgTools/AnaToolHandle.h>

#include <PathResolver/PathResolver.h>

#include <xAODCore/ShallowCopy.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODJet/JetContainer.h>

#include <VBFInvAnalysis/contentHolder.h>
#include <VBFInvAnalysis/VBFInv_Defs.h>
//#include <VBFInvAnalysis/OutputTruth.h>
#include <VBFInvAnalysis/CutFlowTool.h>

class VBFInvVjetsRW : public EL::Algorithm {
public:
   // put your configuration variables here as public variables.
   // that way they can be set directly from CINT and python.
public:
   TString outputName;
   bool        skipCBK;
   bool auto_skipCBK; //!

   // variables that don't get filled at submission time should be
   // protected from being send from the submission node to the worker
   // node (done by the //!)
public:
   // things used to access xAOD information
   Bool_t                                                                 m_determinedDerivation; //!
   Bool_t                                                                 m_isDAODTRUTH;          //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_particles;            //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_muons;                //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_photons;              //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_electrons;            //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_bosons;               //!
   std::pair<xAOD::TruthParticleContainer *, xAOD::ShallowAuxContainer *> m_neutrinos;            //!

   // things used for output
   TTree *m_tree; //!
   //   Analysis::OutputTruth m_cand; //!
   TH1F *m_histoEventCount; //!
   TH1D *NumberEvents; //!
   // things used in the analysis
   Bool_t                    m_iso_dyn;             //!
   Int_t                     m_run;                 //!
   Int_t                     m_event;               //!
   Int_t                     m_eventCounter;        //!
   Float_t                   m_mconly_weight;       //!
   Float_t                   m_n_partonLeptons;     //!
   Float_t                   m_n_partonNeutrinos;   //!
   Float_t                   m_n_partonPhotons;     //!
   Analysis::CutFlowTool     m_cutFlow;             //!
   std::map<TString, Bool_t> m_iso;                 //!
   std::vector<Float_t>      m_truth_mc_px;         //!
   std::vector<Float_t>      m_truth_mc_py;         //!
   std::vector<Float_t>      m_truth_mc_pz;         //!
   std::vector<Float_t>      m_truth_mc_e;          //!
   std::vector<Float_t>      m_truth_mc_px_dressed; //!
   std::vector<Float_t>      m_truth_mc_py_dressed; //!
   std::vector<Float_t>      m_truth_mc_pz_dressed; //!
   std::vector<Float_t>      m_truth_mc_e_dressed;  //!
   std::vector<Int_t>        m_truth_mc_barcode;    //!
   std::vector<Int_t>        m_truth_mc_status;     //!
   std::vector<Int_t>        m_truth_mc_type;       //!
   std::vector<Int_t>        m_truth_mc_origin;     //!
   std::vector<Int_t>        m_truth_mc_pdg;        //!
   std::vector<Int_t>        m_truth_mc_dyn_iso;    //!
   std::vector<Int_t>        m_truth_mc_fix_iso;    //!
   Float_t m_truth_V_simple_pt; //!
   Float_t m_jj_mass; //!
   Float_t m_jj_dphi; //!
   Float_t m_jj_deta; //!
   int                 m_njets;     //!
   int                 m_njets25;     //!
   std::vector<float> m_jet_E;     //!
   std::vector<float> m_jet_pt;    //!
   std::vector<float> m_jet_eta;   //!
   std::vector<float> m_jet_phi;   //!
   std::vector<float> m_jet_m;     //!
   std::vector<int>   m_jet_label; //!

   float m_met_et;  //!
   float m_met_phi; //!

   // this is a standard constructor
   VBFInvVjetsRW();

   // methods defined in the analysis
   virtual EL::StatusCode analyzeEvent();
   virtual EL::StatusCode resetBranches();
   virtual EL::StatusCode defineContainers();
   virtual EL::StatusCode keepEvent();
   virtual EL::StatusCode fillParticle(xAOD::TruthParticle *thisParticle);
   Bool_t                 selectMCTruthParticle(xAOD::TruthParticle *thisParticle);
   virtual EL::StatusCode checkPhotonIsolation(xAOD::TruthParticle *thisParticle);

   // these are the functions inherited from Algorithm
   virtual EL::StatusCode setupJob(EL::Job &job);
   virtual EL::StatusCode fileExecute();
   virtual EL::StatusCode histInitialize();
   virtual EL::StatusCode changeInput(bool firstFile);
   virtual EL::StatusCode initialize();
   virtual EL::StatusCode execute();
   virtual EL::StatusCode postExecute();
   virtual EL::StatusCode finalize();
   virtual EL::StatusCode histFinalize();

   // this is needed to distribute the algorithm to the workers
   ClassDef(VBFInvVjetsRW, 1);
};

#endif
