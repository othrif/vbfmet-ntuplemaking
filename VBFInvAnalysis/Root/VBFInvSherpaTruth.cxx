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
#include <VBFInvAnalysis/DijetInfo.h>

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
        // A failure here should not be fatal-- we want to allow running over our own TRUTH1s.
        const xAOD::CutBookkeeperContainer* completeCBC = 0;
        if (m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()) {

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

    }

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvSherpaTruth::initialize()
{

    m_event = wk()->xaodEvent();

    std::string xSecFilePath = "dev/PMGTools/PMGxsecDB_mc15.txt";
    xSecFilePath = PathResolverFindCalibFile(xSecFilePath);
    my_XsecDB = new SUSY::CrossSectionDB(xSecFilePath);
    if (debug) ANA_MSG_INFO("xsec DB initialized using file:" <<  xSecFilePath);

    // I don't know if we still need most of this?
    ANA_MSG_INFO("Histogram output name is " << outputNameHist);
    TFile *outputFileHist = wk()->getOutputFile (outputNameHist);
    ANA_MSG_INFO("Tree output name is " << outputName);
    TFile *outputFile = wk()->getOutputFile (outputName);
    NumberEventsinNtuple = NumberEvents;
    NumberEvents->SetDirectory(outputFileHist);
    NumberEventsinNtuple->SetDirectory(outputFile);

    // Create the tree.
    truthTree = new TTree("MiniNtuple", "a truth Tree");
    truthTree->SetDirectory(outputFile);

    // General
    truthTree->Branch("EventNumber", &m_EventNumber);
    truthTree->Branch("RunNumber", &m_RunNumber);
    truthTree->Branch("crossSection", &m_crossSection);
    truthTree->Branch("EventWeight", &m_WeightEvents);
    truthTree->Branch("ChannelNumber", &m_ChannelNumber);

    // Status code used to do clustering.
    truthTree->Branch("clusterPartonCode", &m_clusterPartonCode);

    // MET
    truthTree->Branch("met_et", &m_met_et);
    truthTree->Branch("met_phi", &m_met_phi);

    // Create (and attach) dijet finders for everything.
    m_truthDijets = new Analysis::DijetFinder("truth", this->truthJetPtCut);
    m_status20Dijets = new Analysis::DijetFinder("status20", this->partonJetPtCut);
    m_status3Dijets = new Analysis::DijetFinder("status3", this->partonJetPtCut);
    m_partonDijets = new Analysis::DijetFinder("parton", this->partonJetPtCut);

    // Attach them to the tree.
    m_truthDijets->attachToTree(truthTree);
    m_status20Dijets->attachToTree(truthTree);
    m_status3Dijets->attachToTree(truthTree);
    m_partonDijets->attachToTree(truthTree);

    // Create three (!) parton clusterer objects.
    m_status20Partons = new Analysis::PartonClusterer("status20", this->partonJetPtCut, this->antiktDR, this->shouldNotCluster);
    m_status3Partons = new Analysis::PartonClusterer("status3", this->partonJetPtCut, this->antiktDR, this->shouldNotCluster);
    m_partons = new Analysis::PartonClusterer("parton", this->partonJetPtCut, this->antiktDR, this->shouldNotCluster);

    // Attach them to the tree.
    m_status20Partons->attachToTree(truthTree);
    m_status3Partons->attachToTree(truthTree);
    m_partons->attachToTree(truthTree);

    // This is an experiment-- create a clusterer for post-parton-shower particles.
    m_postShower = new Analysis::PartonClusterer("postShower", this->partonJetPtCut, this->antiktDR, this->shouldNotCluster);
    m_postShowerDijets = new Analysis::DijetFinder("postShower", this->partonJetPtCut);
    m_postShower->attachToTree(truthTree);
    m_postShowerDijets->attachToTree(truthTree);

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvSherpaTruth::execute()
{
    // Call the various reset() hooks.
    m_truthDijets->reset();
    m_status20Dijets->reset();
    m_status3Dijets->reset();
    m_partonDijets->reset();

    m_status20Partons->reset();
    m_status3Partons->reset();
    m_partons->reset();

    m_postShower->reset();
    m_postShowerDijets->reset();

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
            if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
            ANA_CHECK (evtStore()->retrieve( jets, "AntiKt4TruthDressedWZJets"));
            failedLookingFor = kTRUE;
        } else {
            if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthJets!");
        }
    } else {
        if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
        ANA_CHECK(evtStore()->retrieve(jets, "AntiKt4TruthDressedWZJets"));
    }

    // Post-process the truth jets into an array of TLorentzVectors.
    // TODO: make this a method.
    std::vector<TLorentzVector> truthJets;
    TLorentzVector truthJet;
    for (const auto* jet: *jets) {
        TLorentzVector truthJet;
        truthJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
        truthJets.push_back(truthJet);
    }

    // Pass the truth jets to the truth dijet finder.
    m_truthDijets->computeMjj(&truthJets);

    // Retrieve truth particles from the xAOD.
    const xAOD::TruthParticleContainer* truthParticles = nullptr;
    ANA_CHECK(evtStore()->retrieve(truthParticles, "TruthParticles"));
    if (debug) ANA_MSG_INFO("Retrieved truth particles container.");

    // Sort truth particles by status code.
    this->fillMapFromTruthParticles(truthParticles);
    if (debug) ANA_MSG_INFO("Mapped truth particles by status code.");

    // Determine if this is a S or H event by seeing if there are status 20 particles.
    std::vector<const xAOD::TruthParticle*> status20 = m_truthByStatus[20];
    std::vector<const xAOD::TruthParticle*> status3 = m_truthByStatus[3];

    // Extract the status 11 particles. Only keep those which have a production
    // vertex, and whose production vertex's id code == 4.
    // (4 : Parton Shower or QED radiation).
    std::vector<const xAOD::TruthParticle*> status11 = m_truthByStatus[11];
    std::vector<const xAOD::TruthParticle*> showerOuts;
    for (const auto* particle: showerOuts) {
        if (particle->hasProdVtx()) {
            if (particle->prodVtx()->id() == 4) {
                showerOuts.push_back(particle);
            }
        }
    }

    // This is a pointer to either the status 3 or status 20 particles.
    std::vector<const xAOD::TruthParticle*>* particles;

    // Now, cluster the status 3 and status 20 particles.
    std::vector<TLorentzVector>* status20Jets = m_status20Partons->clusterPartons(&status20);
    std::vector<TLorentzVector>* status3Jets = m_status3Partons->clusterPartons(&status3);

    std::vector<TLorentzVector>* showerJets = m_postShower->clusterPartons(&showerOuts);

    if (debug) ANA_MSG_INFO("Finished clustering particles into parton jets.");

    // If "clusterPartonCode" is 20, then we are a MC@NLO S Event.
    // Otherwise, we're presumably a MC@NLO H event, though this only holds for Sherpa 2.2.2+ MC@NLO.
    // Here, we assign the parton jet pointer appropriately.
    std::vector<TLorentzVector>* partonJets;
    if (status20.size() != 0) {
        m_clusterPartonCode = 20;
        particles = &status20;
        partonJets = status20Jets;
    } else {
        m_clusterPartonCode = 3;
        particles = &status3;
        partonJets = status3Jets;
    }
    if (debug) ANA_MSG_INFO("Selected status 20 vs status 3 particles.");

    // Now, write the status-code-neutral version of the truth particles.
    m_partons->clusterPartons(particles);

    // Now, using the parton jet collections, compute and store mjj in dijet finders.
    // This is a bit inefficient-- we do it once for status 3, once for status 20, and then again
    // for the status-code-neutral version. We could probably just do the computation once
    // and then store the right values... oh well.
    m_status20Dijets->computeMjj(status20Jets);
    m_status3Dijets->computeMjj(status3Jets);
    m_partonDijets->computeMjj(partonJets);

    // Now, we'd like to compute "best" mjjs. i.e. pick the pair of partons that's closest to
    // truth, or vice versa, only using this measure of "best".
    float truthMjj = m_truthDijets->getDijetInfo("lead")->getMass();
    float partonMjj = m_partonDijets->getDijetInfo("lead")->getMass();

    // This next bit is a tad ugly, and may mean I architected things incorrectly...
    // Reach in and set the "truth" values correctly for all the bestLead dijet infos.
    // Since getDijetInfo returns the base dijet info pointer, we need to explicitly cast to BestDijetInfo.
    ((Analysis::BestDijetInfo*)(m_truthDijets->getDijetInfo("bestLead")))->setTruthMjj(partonMjj);
    ((Analysis::BestDijetInfo*)(m_partonDijets->getDijetInfo("bestLead")))->setTruthMjj(truthMjj);
    ((Analysis::BestDijetInfo*)(m_status3Dijets->getDijetInfo("bestLead")))->setTruthMjj(truthMjj);
    ((Analysis::BestDijetInfo*)(m_status20Dijets->getDijetInfo("bestLead")))->setTruthMjj(truthMjj);

    // Now actually compute the "best" mjj in the truth dijet finder.
    m_truthDijets->getDijetInfo("bestLead")->compute(*(m_truthDijets->getPrunedJets()));

    // Do the same for all 3 parton dijet finders.
    m_partonDijets->getDijetInfo("bestLead")->compute(*(m_partonDijets->getPrunedJets()));
    m_status3Dijets->getDijetInfo("bestLead")->compute(*(m_status3Dijets->getPrunedJets()));
    m_status20Dijets->getDijetInfo("bestLead")->compute(*(m_status20Dijets->getPrunedJets()));

    // We now want to match the parton jets against the truth jets. We do that in the "dijet finder".
    // which isn't necessarily the best place, but it's where the jets get written out to the tree
    // so it's close enough.
    m_truthDijets->matchJets(partonJets, this->antiktDR);
    m_partonDijets->matchJets(&truthJets, this->antiktDR);
    m_status20Dijets->matchJets(&truthJets, this->antiktDR);
    m_status3Dijets->matchJets(&truthJets, this->antiktDR);

    // Experiment: handle the parton shower outputs too.
    m_postShowerDijets->computeMjj(showerJets);

    if (debug) ANA_MSG_INFO("Completed handling mjj computation.");

    // Fill the tree!
    truthTree->Fill();

    // Clear the map of status codes/truth particles.
    m_truthByStatus.clear();

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

// This helper method could be moved into its own class, but I don't know that it matters.

void VBFInvSherpaTruth::fillMapFromTruthParticles(const xAOD::TruthParticleContainer* truthParticles) {

    int status;

    // Create vectors for status 20 and status 3, always.
    this->m_truthByStatus[3] = std::vector<const xAOD::TruthParticle*>();
    this->m_truthByStatus[20] = std::vector<const xAOD::TruthParticle*>();

    for (const auto* particle: *truthParticles) {
        status = particle->status();

        // Check if we already created the vector. If not, create it!
        if (this->m_truthByStatus.count(status) == 0) {
            this->m_truthByStatus[status] = std::vector<const xAOD::TruthParticle*>();
        }

        // Add the particle to a status-code-indexed vector.
        m_truthByStatus[status].push_back(particle);
    }
}
