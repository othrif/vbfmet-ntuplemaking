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

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticle.h"

#include <VBFInvAnalysis/DijetFinder.h>
#include <VBFInvAnalysis/PartonClusterer.h>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

namespace ORUtils {
    class IOverlapRemovalTool;
}

class VBFInvSherpaTruth : public EL::Algorithm
{

private:
    SUSY::CrossSectionDB *my_XsecDB;  //!
    TTree* truthTree; //!

    // Dijet finder objects.
    Analysis::DijetFinder* m_truthDijets; //!
    Analysis::DijetFinder* m_status3Dijets; //!
    Analysis::DijetFinder* m_status20Dijets; //!
    Analysis::DijetFinder* m_partonDijets; //!

    // Parton clusterer objects.
    Analysis::PartonClusterer* m_status20Partons; //!
    Analysis::PartonClusterer* m_status3Partons; //!
    Analysis::PartonClusterer* m_partons; //!

    // Truth particles, sorted by status code.
    std::map<int, std::vector<const xAOD::TruthParticle*>> m_truthByStatus; //!

  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

    std::string outputName = "HADAHA";
    std::string outputNameHist = "hist";
    bool debug;
    bool skipCBK;
    bool MultiWeight;

    // The value of DR to use in anti-KT.
    float antiktDR = 0.4;

    // The parton jet pT to cut on, in GeV.
    float partonJetPtCut = 20;

    // Same for truth jets. We might want separation?
    float truthJetPtCut = 20;

    // Should we cluster the partons? Defaults to 'yes'.
    bool shouldNotCluster = true;

    // variables that don't get filled at submission time should be
    // protected from being send from the submission node to the worker
    // node (done by the //!)

public:

    float m_met_et; //!
    float m_met_phi; //!

    ULong64_t m_EventNumber; //!
    UInt_t m_RunNumber; //!
    double m_crossSection; //!
    float m_WeightEvents; //!
    UInt_t m_ChannelNumber; //!

    // Which status code was used to do clustering for this event?
    int m_clusterPartonCode = -1;

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

    // Helper method to sort truth particles by status code.
    void fillMapFromTruthParticles(const xAOD::TruthParticleContainer* truthParticles);

    // this is needed to distribute the algorithm to the workers
    ClassDef(VBFInvSherpaTruth, 1);
};

#endif
