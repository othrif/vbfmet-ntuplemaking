#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <EventLoop/OutputStream.h>
#include "FourMomUtils/xAODP4Helpers.h"

// EDM includes:
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"

#include <AsgTools/MessageCheck.h>

#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

#include "PathResolver/PathResolver.h"

// Need (external) fastjet to do parton clustering.
#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

// Local include(s):
#include <VBFInvAnalysis/VBFInvSherpaTruth.h>
#include <VBFInvAnalysis/HelperFunctions.h>
#include <VBFInvAnalysis/SherpaPartonCluster.h>

#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE

using namespace xAOD;

static SG::AuxElement::Decorator<bool> dec_passOR("passTruthOR");

// this is needed to distribute the algorithm to the workers
ClassImp(VBFInvSherpaTruth)

VBFInvSherpaTruth::VBFInvSherpaTruth(): my_XsecDB(0)
{

}

EL::StatusCode VBFInvSherpaTruth::setupJob(EL::Job &job)
{
    job.useXAOD();

    // Call before opening first file.
    xAOD::Init("VBFInvSherpaTruth").ignore();

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvSherpaTruth::histInitialize()
{

    NumberEvents = new TH1D("NumberEvents", "Number Events", 3, 0, 3);
    NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
    NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
    NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvSherpaTruth::fileExecute()
{

    m_event = wk()->xaodEvent();
    ANA_MSG_INFO("Number of events in this file = " << m_event->getEntries() );

    if (skipCBK) {
        NumberEvents->Fill(0., m_event->getEntries());
    } else {

        // Read the CutBookkeeper container
        const xAOD::CutBookkeeperContainer* completeCBC = 0;
        if (!m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()) {
            ANA_MSG_ERROR( "Failed to retrieve CutBookkeepers from MetaData! Exiting.");
        }

        const xAOD::CutBookkeeper* allEventsCBK = 0;
        int maxcycle = -1;

        // Let's find the right CBK (latest with StreamAOD input before derivations)
        for ( auto cbk : *completeCBC ) {
            if ( cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD" && cbk->cycle() > maxcycle) {
                maxcycle = cbk->cycle();
                allEventsCBK = cbk;
            }
        }

        uint64_t nEventsProcessed  = 0;
        double sumOfWeights        = 0.;
        double sumOfWeightsSquared = 0.;
        if (allEventsCBK) {
            nEventsProcessed  = allEventsCBK->nAcceptedEvents();
            sumOfWeights        = allEventsCBK->sumOfEventWeights();
            sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();
            ANA_MSG_INFO("CutBookkeepers Accepted:" << nEventsProcessed << ", SumWei:" << sumOfWeights << ", sumWei2:" << sumOfWeightsSquared);
        } else {
            ANA_MSG_INFO("No relevent CutBookKeepers found" );
            nEventsProcessed = m_event->getEntries();
        }

        NumberEvents->Fill(0., nEventsProcessed);
        NumberEvents->Fill(1., sumOfWeights);
        NumberEvents->Fill(2., sumOfWeightsSquared);
    }

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvSherpaTruth::initialize ()
{

    m_event = wk()->xaodEvent();

    std::string xSecFilePath = "dev/PMGTools/PMGxsecDB_mc15.txt";
    xSecFilePath = PathResolverFindCalibFile(xSecFilePath);
    my_XsecDB = new SUSY::CrossSectionDB(xSecFilePath);
    if (debug) ANA_MSG_INFO("xsec DB initialized using file:" <<  xSecFilePath);

    m_jet_E = new std::vector<float>();
    m_jet_pt = new std::vector<float>();
    m_jet_eta = new std::vector<float>();
    m_jet_phi = new std::vector<float>();
    m_jet_m = new std::vector<float>();
    m_jet_label = new std::vector<int>();

    m_parton_x1 = new std::vector<float>();
    m_parton_x2 = new std::vector<float>();
    m_parton_xf1 = new std::vector<float>();
    m_parton_xf2 = new std::vector<float>();
    m_parton_Q = new std::vector<float>();
    m_parton_pdgid1 = new std::vector<int>();
    m_parton_pdgid2 = new std::vector<int>();
    m_parton_pdfid1 = new std::vector<int>();
    m_parton_pdfid2 = new std::vector<int>();
    m_parton_pp = new std::vector<int>();

    // Declare status 3, 20 particles and parton jet collections.

    m_parton20JetPt = new std::vector<float>();
    m_parton20JetEta = new std::vector<float>();
    m_parton20JetPhi = new std::vector<float>();
    m_parton20JetE = new std::vector<float>();

    m_parton3JetPt = new std::vector<float>();
    m_parton3JetEta = new std::vector<float>();
    m_parton3JetPhi = new std::vector<float>();
    m_parton3JetE = new std::vector<float>();

    m_particle20Pt = new std::vector<float>();
    m_particle20Eta = new std::vector<float>();
    m_particle20Phi = new std::vector<float>();
    m_particle20E = new std::vector<float>();
    m_particle20Mass = new std::vector<float>();
    m_particle20PID = new std::vector<int>();

    m_particle3Pt = new std::vector<float>();
    m_particle3Eta = new std::vector<float>();
    m_particle3Phi = new std::vector<float>();
    m_particle3E = new std::vector<float>();
    m_particle3Mass = new std::vector<float>();
    m_particle3PID = new std::vector<int>();

    ANA_MSG_INFO("Histogram output name is " << outputNameHist);
    TFile *outputFileHist = wk()->getOutputFile (outputNameHist);
    ANA_MSG_INFO("Tree output name is " << outputName);
    TFile *outputFile = wk()->getOutputFile (outputName);
    NumberEventsinNtuple = NumberEvents;
    NumberEvents->SetDirectory(outputFileHist);
    NumberEventsinNtuple->SetDirectory(outputFile);
    truthTree = new TTree("MiniNtuple", "a truth Tree");
    truthTree->SetDirectory(outputFile);

    // General
    truthTree->Branch("EventNumber", &m_EventNumber);
    truthTree->Branch("RunNumber", &m_RunNumber);
    truthTree->Branch("crossSection", &m_crossSection);
    truthTree->Branch("EventWeight", &m_WeightEvents);
    truthTree->Branch("ChannelNumber", &m_ChannelNumber);

    // Jets
    truthTree->Branch ("njets", &m_njets);
    truthTree->Branch ("jet_E", &m_jet_E);
    truthTree->Branch ("jet_pt", &m_jet_pt);
    truthTree->Branch ("jet_eta", &m_jet_eta);
    truthTree->Branch ("jet_phi", &m_jet_phi);
    truthTree->Branch ("jet_m", &m_jet_m);
    truthTree->Branch ("jet_label", &m_jet_label);

    // MET
    truthTree->Branch ("met_et", &m_met_et);
    truthTree->Branch ("met_phi", &m_met_phi);

    // Parton
    truthTree->Branch ("parton_x1", &m_parton_x1);
    truthTree->Branch ("parton_x2", &m_parton_x2);
    truthTree->Branch ("parton_xf1", &m_parton_xf1);
    truthTree->Branch ("parton_xf2", &m_parton_xf2);
    truthTree->Branch ("parton_Q", &m_parton_Q);
    truthTree->Branch ("parton_pdgid1", &m_parton_pdgid1);
    truthTree->Branch ("parton_pdgid2", &m_parton_pdgid2);
    truthTree->Branch ("parton_pdfid1", &m_parton_pdfid1);
    truthTree->Branch ("parton_pdfid2", &m_parton_pdfid2);
    truthTree->Branch ("parton_pp", &m_parton_pp);

    // Store number of these parton jets.
    truthTree->Branch("numStatus20Jets", &m_numStatus20Jets);
    truthTree->Branch("numStatus3Jets", &m_numStatus3Jets);

    // Add branches for Sherpa truth particles and parton jets.
    truthTree->Branch("parton20JetPt", &m_parton20JetPt);
    truthTree->Branch("parton20JetEta", &m_parton20JetEta);
    truthTree->Branch("parton20JetPhi", &m_parton20JetPhi);
    truthTree->Branch("parton20JetE", &m_parton20JetE);

    truthTree->Branch("parton3JetPt", &m_parton3JetPt);
    truthTree->Branch("parton3JetEta", &m_parton3JetEta);
    truthTree->Branch("parton3JetPhi", &m_parton3JetPhi);
    truthTree->Branch("parton3JetE", &m_parton3JetE);

    truthTree->Branch("particle20Pt", &m_particle20Pt);
    truthTree->Branch("particle20Eta", &m_particle20Eta);
    truthTree->Branch("particle20Phi", &m_particle20Phi);
    truthTree->Branch("particle20E", &m_particle20E);
    truthTree->Branch("particle20Mass", &m_particle20Mass);
    truthTree->Branch("particle20PID", &m_particle20PID);

    truthTree->Branch("particle3Pt", &m_particle3Pt);
    truthTree->Branch("particle3Eta", &m_particle3Eta);
    truthTree->Branch("particle3Phi", &m_particle3Phi);
    truthTree->Branch("particle3E", &m_particle3E);
    truthTree->Branch("particle3Mass", &m_particle20Mass);
    truthTree->Branch("particle3PID", &m_particle20PID);

    // Store number of these particles.
    truthTree->Branch("numStatus20", &m_numStatus20);
    truthTree->Branch("numStatus3", &m_numStatus3);

    return EL::StatusCode::SUCCESS;
}



EL::StatusCode VBFInvSherpaTruth::execute()
{

    m_jet_E->clear();
    m_jet_pt->clear();
    m_jet_eta->clear();
    m_jet_phi->clear();
    m_jet_m->clear();
    m_jet_label->clear();

    m_parton_x1->clear();
    m_parton_x2->clear();
    m_parton_xf1->clear();
    m_parton_xf2->clear();
    m_parton_Q->clear();
    m_parton_pdgid1->clear();
    m_parton_pdgid2->clear();
    m_parton_pdfid1->clear();
    m_parton_pdfid2->clear();
    m_parton_pp->clear();

    if (debug) ANA_MSG_INFO("***New event***" );

    //----------------------------
    // Event information
    //---------------------------
    if (debug) ANA_MSG_INFO("Retrieving event info collection." );

    const xAOD::EventInfo *eventInfo = 0;
    if ( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ) {
        ANA_MSG_ERROR("Failed to retrieve event info collection. Exiting." );
        return EL::StatusCode::FAILURE;
    }

    m_EventNumber = eventInfo->eventNumber();
    m_RunNumber = eventInfo->runNumber();
    m_WeightEvents = eventInfo->mcEventWeight(); //.at(0);
    m_ChannelNumber = eventInfo->mcChannelNumber();

    if (skipCBK) {
        NumberEvents->Fill(1, m_WeightEvents);
        NumberEvents->Fill(2, m_WeightEvents * m_WeightEvents);
    }

    m_crossSection = -1;// my_XsecDB->xsectTimesEff(eventInfo->mcChannelNumber());

    if(debug)
        ANA_MSG_INFO ( "EventNumber=" <<  m_EventNumber << ", RunNumber=" << m_WeightEvents << ", WeightEvents=" << m_WeightEvents << ", Cross section=" << m_crossSection);

    //-----------------------------------------------------------------------
    //  Retrieve containers
    //-----------------------------------------------------------------------

    xAOD::TEvent *event = wk()->xaodEvent();
    // Jets
    const xAOD::JetContainer* jets = nullptr;

    static Bool_t failedLookingFor(kFALSE); // trick to avoid infinite RuntimeWarning's for EXOT5
    if (!failedLookingFor) {
     if (!event->retrieve(jets, "AntiKt4TruthJets").isSuccess()) {
        if(debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
        ANA_CHECK (evtStore()->retrieve( jets, "AntiKt4TruthDressedWZJets"));
        failedLookingFor = kTRUE;
    }
    else {
        if(debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthJets!");
    }
    } else {
        if(debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
        ANA_CHECK (evtStore()->retrieve( jets, "AntiKt4TruthDressedWZJets"));
    }

    // TruthEvents
    const xAOD::TruthEventContainer* truthE = nullptr;
    ANA_CHECK(m_event->retrieve( truthE, "TruthEvents" ));

    const xAOD::TruthParticleContainer* truthParticles = nullptr;
    ANA_CHECK(evtStore()->retrieve(truthParticles, "TruthParticles"));

    // Create a jet definition, instantiate the Sherpa Parton clusterer object.
    fastjet::JetDefinition jetDef(fastjet::antikt_algorithm, this->antiktDR);
    SherpaPartonCluster clusterer(&jetDef, truthParticles);

    // Retrieve (as vectors) the status 20 and 3 particles and parton jets.
    std::vector<TLorentzVector> partonJets20 = clusterer.getRootPartonJets(20, this->partonJetPtCut, this->shouldNotCluster);
    std::vector<TLorentzVector> partonJets3 = clusterer.getRootPartonJets(3, this->partonJetPtCut, this->shouldNotCluster);

    std::vector<const xAOD::TruthParticle_v1*> particles20 = clusterer.getParticles(20);
    std::vector<const xAOD::TruthParticle_v1*> particles3 = clusterer.getParticles(3);

    //-----------------------------------------------------------------------
    //  Fill branches
    //-----------------------------------------------------------------------

    // Jets
        int njet5=0;
        for (const auto& truthJ_itr : *jets){
           if (truthJ_itr->pt() > 5000. && truthJ_itr->auxdata< bool >("passTruthOR")) {
            m_jet_E->push_back(truthJ_itr->e());
            m_jet_pt->push_back(truthJ_itr->pt());
            m_jet_eta->push_back(truthJ_itr->eta());
            m_jet_phi->push_back(truthJ_itr->phi());
            m_jet_m->push_back(truthJ_itr->m());
            m_jet_label->push_back(truthJ_itr->auxdata<int>("PartonTruthLabelID"));
            njet5++;
        }
    }
    m_njets = njet5;

    // Partons
    for (const auto& truthE_itr : *truthE){
    if(truthE_itr->isAvailable< float >( "X1" ) ){
        m_parton_x1->push_back(truthE_itr->auxdata<float>("X1"));
        m_parton_x2->push_back(truthE_itr->auxdata<float>("X2"));
        m_parton_xf1->push_back(truthE_itr->auxdata<float>("XF1"));
        m_parton_xf2->push_back(truthE_itr->auxdata<float>("XF2"));
        m_parton_Q->push_back(truthE_itr->auxdata<float>("Q"));
        m_parton_pdgid1->push_back(truthE_itr->auxdata<int>("PDGID1"));
        m_parton_pdgid2->push_back(truthE_itr->auxdata<int>("PDGID2"));
        m_parton_pdfid1->push_back(truthE_itr->auxdata<int>("PDFID1"));
        m_parton_pdfid2->push_back(truthE_itr->auxdata<int>("PDFID2"));

            // qq: 0
            // qg: 1
            // gg: 2
            // other: 3
        if(truthE_itr->auxdata<int>("PDGID1")<=6 && truthE_itr->auxdata<int>("PDGID2")<=6)
            m_parton_pp->push_back(0);
        else if ( (truthE_itr->auxdata<int>("PDGID1")==21 && truthE_itr->auxdata<int>("PDGID2")<=6) || (truthE_itr->auxdata<int>("PDGID2")==21 && truthE_itr->auxdata<int>("PDGID1")<=6) )
            m_parton_pp->push_back(1);
        else if (truthE_itr->auxdata<int>("PDGID1")==21 && truthE_itr->auxdata<int>("PDGID2")==21)
            m_parton_pp->push_back(2);
        else {
            m_parton_pp->push_back(3);
            ANA_MSG_INFO("What the hell is this one " << truthE_itr->auxdata<int>("PDGID1") << " AND " << truthE_itr->auxdata<int>("PDGID2"));
        }

        }else{ // Somehow it does not exist
            m_parton_x1->push_back(-1);
            m_parton_x2->push_back(-1);
            m_parton_xf1->push_back(-1);
            m_parton_xf2->push_back(-1);
            m_parton_Q->push_back(-1);
            m_parton_pdgid1->push_back(-1);
            m_parton_pdgid2->push_back(-1);
            m_parton_pdfid1->push_back(-1);
            m_parton_pdfid2->push_back(-1);
        }

    }

    // Fill the status 3 and 20 particle/parton jet collections.
    storeJets(partonJets20, m_parton20JetPt, m_parton20JetEta, m_parton20JetPhi, m_parton20JetE, &m_numStatus20Jets);
    storeJets(partonJets3, m_parton3JetPt, m_parton3JetEta, m_parton3JetPhi, m_parton3JetE, &m_numStatus3Jets);
    storeParticles(particles20, m_particle20Pt, m_particle20Eta, m_particle20Phi, m_particle20E, m_particle20Mass, m_particle20PID, &m_numStatus20);
    storeParticles(particles3, m_particle3Pt, m_particle3Eta, m_particle3Phi, m_particle3E, m_particle3Mass, m_particle3PID, &m_numStatus3);

    //-----------------------------------------------------------------------
    //  Fill Tree
    //-----------------------------------------------------------------------
    truthTree->Fill();

    //-----------------------------------------------------------------------
    //  Tests
    //-----------------------------------------------------------------------

    return EL::StatusCode::SUCCESS;
}



EL::StatusCode VBFInvSherpaTruth::postExecute()
{
    return EL::StatusCode::SUCCESS;
}



EL::StatusCode VBFInvSherpaTruth::finalize()
{

    ANA_MSG_INFO("All done. Now cleaning up...");

    if ( my_XsecDB ) {
        delete my_XsecDB;
        my_XsecDB = 0;
    }

    return EL::StatusCode::SUCCESS;
}



EL::StatusCode VBFInvSherpaTruth::histFinalize()
{
    return EL::StatusCode::SUCCESS;
}

// Helper functions to write things into (pointers to) tree branches.

void VBFInvSherpaTruth::storeJets(  std::vector<TLorentzVector> jets,
                                    std::vector<float>* ptvec,
                                    std::vector<float>* etavec,
                                    std::vector<float>* phivec,
                                    std::vector<float>* evec,
                                    unsigned int* size
                                ) {

    // Reset these vectors.
    ptvec->clear();
    etavec->clear();
    phivec->clear();
    evec->clear();

    // Loop over the jets, push their kinematics.
    for (unsigned int i = 0; i < jets.size(); i++) {
        TLorentzVector jet = jets.at(i);
        ptvec->push_back(jet.Perp());
        etavec->push_back(jet.Eta());
        phivec->push_back(jet.Phi());
        evec->push_back(jet.E());
    }

    // Dereference the pointer to size, assign the number of jets.
    (*size) = jets.size();

}

void VBFInvSherpaTruth::storeParticles( std::vector<const xAOD::TruthParticle_v1*> particles,
                                        std::vector<float>* ptvec,
                                        std::vector<float>* etavec,
                                        std::vector<float>* phivec,
                                        std::vector<float>* evec,
                                        std::vector<float>* massvec,
                                        std::vector<int>* pidvec,
                                        unsigned int* size
                                     ) {

    // Reset these vectors.
    ptvec->clear();
    etavec->clear();
    phivec->clear();
    evec->clear();
    massvec->clear();
    pidvec->clear();

    // Loop over the jets, push their kinematics.
    for (unsigned int i = 0; i < particles.size(); i++) {
        const xAOD::TruthParticle_v1* particle = particles.at(i);

        TLorentzVector rootParticle;
        rootParticle.SetPtEtaPhiE(particle->pt(), particle->eta(), particle->phi(), particle->e());
        //rootParticle.SetPxPyPzE(particle->px(), particle->py(), particle->pz(), particle->e());

        ptvec->push_back(rootParticle.Perp());
        etavec->push_back(rootParticle.Eta());
        phivec->push_back(rootParticle.Phi());
        evec->push_back(rootParticle.E());
        massvec->push_back(particle->m());

        pidvec->push_back(particle->pdgId());
    }

    // Dereference the pointer to size, assign the number of jets.
    (*size) = particles.size();

}
