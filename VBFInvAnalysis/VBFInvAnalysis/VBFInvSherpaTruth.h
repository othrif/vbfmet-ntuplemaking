#ifndef VBFInvAnalysis_VBFInvSherpaTruth_H
#define VBFInvAnalysis_VBFInvSherpaTruth_H

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

class VBFInvSherpaTruth : public EL::Algorithm
{

private:
    SUSY::CrossSectionDB *my_XsecDB;  //!
    TTree* truthTree; //!

  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

    std::string outputName = "HADAHA";
    std::string outputNameHist = "hist";
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

    // Status 20 parton jets.
    std::vector<float>* m_parton20JetPt; //!
    std::vector<float>* m_parton20JetEta; //!
    std::vector<float>* m_parton20JetPhi; //!
    std::vector<float>* m_parton20JetE; //!

    // Status 3 parton jets.
    std::vector<float>* m_parton3JetPt; //!
    std::vector<float>* m_parton3JetEta; //!
    std::vector<float>* m_parton3JetPhi; //!
    std::vector<float>* m_parton3JetE; //!

    // Status 20 particles.
    std::vector<float>* m_particle20Pt; //!
    std::vector<float>* m_particle20Eta; //!
    std::vector<float>* m_particle20Phi; //!
    std::vector<float>* m_particle20E; //!
    std::vector<float>* m_particle20Mass; //!
    std::vector<int>* m_particle20PID; //!

    // Status 3 particles.
    std::vector<float>* m_particle3Pt; //!
    std::vector<float>* m_particle3Eta; //!
    std::vector<float>* m_particle3Phi; //!
    std::vector<float>* m_particle3E; //!
    std::vector<float>* m_particle3Mass; //!
    std::vector<int>* m_particle3PID; //!

    float m_met_et; //!
    float m_met_phi; //!

    ULong64_t m_EventNumber; //!
    UInt_t m_RunNumber; //!
    double m_crossSection; //!
    float m_WeightEvents; //!
    UInt_t m_ChannelNumber; //!

    TH1D *NumberEvents; //!
    TH1D *NumberEventsinNtuple; //!

    xAOD::TEvent *m_event;  //!

    // this is a standard constructor
    VBFInvSherpaTruth();

    // these are the functions inherited from Algorithm
    virtual EL::StatusCode setupJob(EL::Job& job);
    virtual EL::StatusCode histInitialize();
    virtual EL::StatusCode fileExecute();
    virtual EL::StatusCode initialize();
    virtual EL::StatusCode execute();
    virtual EL::StatusCode postExecute();
    virtual EL::StatusCode finalize();
    virtual EL::StatusCode histFinalize();

    // this is needed to distribute the algorithm to the workers
    ClassDef(VBFInvSherpaTruth, 1);
};

#endif
