#ifdef ROOTCORE
#include <xAODRootAccess/tools/TReturnCode.h>
#include <AsgTools/StatusCode.h>
#endif
#include <PATInterfaces/CorrectionCode.h>
#include <PATInterfaces/SystematicCode.h>

// standard C++ headers
#include <iostream>
#include <algorithm>
#include <bitset>

// ROOT generic
#include <TLorentzVector.h>
#include <TSystem.h>
#include <TTree.h>
#include <TTreeFormula.h>
#include <TH1F.h>
#include <TFile.h>

// event loop
#include <AsgTools/MessageCheck.h>
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <EventLoop/OutputStream.h>

// xAODTruth
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthEvent.h>
#include "xAODTruth/xAODTruthHelpers.h"

// EDM
#include <xAODRootAccess/tools/TReturnCode.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODCutFlow/CutBookkeeper.h>
#include <xAODCutFlow/CutBookkeeperContainer.h>

// utils
#include <VBFInvAnalysis/GetTruthBosonP4.h>
#include <VBFInvAnalysis/VBFInvVjetsRW.h>
#include <VBFInvAnalysis/HelperFunctions.h>

#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include <xAODBase/IParticleHelpers.h>

// this is needed to distribute the algorithm to the workers
ClassImp(VBFInvVjetsRW)

   VBFInvVjetsRW ::VBFInvVjetsRW()
   : m_determinedDerivation(kFALSE), m_isDAODTRUTH(kFALSE), m_tree(nullptr),
     m_histoEventCount(nullptr)

        {
// Here you put any code for the base initialization of variables,
// e.g. initialize all pointers to 0.  Note that you should only put
// the most basic initialization here, since this method will be
// called on both the submission and the worker node.  Most of your
// initialization code will go into histInitialize() and
// initialize().

#ifdef ROOTCORE
// xAOD::TReturnCode::enableFailure();
// StatusCode::enableFailure();
#endif

           //   xAOD::TFileAccessTracer::enableDataSubmission(kFALSE);
        }

     EL::StatusCode VBFInvVjetsRW ::setupJob(EL::Job & job)
{
   // Here you put code that sets up the job on the submission object
   // so that it is ready to work with your algorithm, e.g. you can
   // request the D3PDReader service or add output files.  Any code you
   // put here could instead also go into the submission script.  The
   // sole advantage of putting it here is that it gets automatically
   // activated/deactivated when you add/remove the algorithm from your
   // job, which may or may not be of value to you.

   job.useXAOD();

   // add output stream (i.e. files in the data-XXX directory)
   // EL::OutputStream output("MiniNtuple");
   // job.outputAdd(output);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::histInitialize()
{
   // Here you do everything that needs to be done at the very
   // beginning on each worker node, e.g. create histograms and output
   // trees.  This method gets called before any input files are
   // connected.

   ANA_MSG_INFO("in histInitialize");

   TString cf_name("ProcessedEvents");

   ANA_MSG_INFO("histInitialize...Name will be " << cf_name.Data());
   m_cutFlow = Analysis::CutFlowTool(cf_name);
   m_cutFlow.addCut("processed");

   Info("histInitialize", "Initializing cut flow histogram");

   m_histoEventCount =
      new TH1F("histoEventCount", "event count (derivation-proof, only MC weight if any)", 100, 0, 100);
   m_histoEventCount->Fill("initial_weighted", 0);
   m_histoEventCount->Fill("initial_raw", 0);

   wk()->addOutput(m_histoEventCount);

  NumberEvents = new TH1D("NumberEvents", "Number Events", 4, 0, 4);
   NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
   NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
   NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");
   NumberEvents->GetXaxis()->SetBinLabel(4, "RawTRUTH");
   return EL::StatusCode::SUCCESS;

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::fileExecute()
{
   // Here you do everything that needs to be done exactly once for every
   // single file, e.g. collect a list of all lumi-blocks processed

   xAOD::TEvent *event = wk()->xaodEvent();

   TTree *MetaData = dynamic_cast<TTree *>(wk()->inputFile()->Get("MetaData"));
   if (!MetaData) {
      ANA_MSG_ERROR("MetaData not found!");
      return EL::StatusCode::FAILURE;
   }
   MetaData->LoadTree(0);

   const Bool_t isDerivation = !MetaData->GetBranch("StreamAOD");
   //   const Bool_t isTruthDerivation = (MetaData->GetBranch("StreamDAOD_TRUTH1") ||
   //   MetaData->GetBranch("StreamDAOD_TRUTH3"));

   // get sum of event weights in derivation skims
   if (isDerivation) {

      // check for corruption
      const xAOD::CutBookkeeperContainer *incompleteCBK = nullptr;
      if (!event->retrieveMetaInput(incompleteCBK, "IncompleteCutBookkeepers").isSuccess()) {
         ANA_MSG_ERROR("Failed to retrieve IncompleteCutBookkeepers from MetaData! Exiting.");
         return EL::StatusCode::FAILURE;
      }
      if (incompleteCBK->size() != 0) {
         Error("initializeEvent()", "Found incomplete Bookkeepers! Check file for corruption.");
         return EL::StatusCode::FAILURE;
      }

      // set pointers & retrieve the bookkeeper container
      const xAOD::CutBookkeeperContainer *bookkeepers = nullptr;
      if (!event->retrieveMetaInput(bookkeepers, "CutBookkeepers").isSuccess()) {
         ANA_MSG_ERROR("Failed to retrieve CutBookkeepers from MetaData");
         return EL::StatusCode::FAILURE;
      }
      const xAOD::CutBookkeeper *event_bookkeeper = nullptr;

      // find the max cycle where input stream is StreamAOD and the name is AllExecutedEvents
      int maxCycle = -1;
      for (auto cbk : *bookkeepers) {
         if (cbk->inputStream() == "StreamAOD" && cbk->name() == "AllExecutedEvents" && cbk->cycle() > maxCycle) {
            maxCycle         = cbk->cycle();
            event_bookkeeper = cbk;
         }
      }
      // if the right & proper bookkeeper is found, read info
            uint64_t nEventsProcessed    = 0;
      double   sumOfWeights        = 0.;
      double   sumOfWeightsSquared = 0.;
      if (event_bookkeeper) {
         m_histoEventCount->Fill("initial_weighted", event_bookkeeper->sumOfEventWeights());
         m_histoEventCount->Fill("initial_raw", event_bookkeeper->nAcceptedEvents());

         nEventsProcessed    = event_bookkeeper->nAcceptedEvents();
         sumOfWeights        = event_bookkeeper->sumOfEventWeights();
         sumOfWeightsSquared = event_bookkeeper->sumOfEventWeightsSquared();
         ANA_MSG_INFO("CutBookkeepers Accepted:" << nEventsProcessed << ", SumWei:" << sumOfWeights << ", sumWei2:" << sumOfWeightsSquared);
      }
          else {
         ANA_MSG_INFO("No relevent CutBookKeepers found");
         nEventsProcessed = event->getEntries();
      }
      NumberEvents->Fill(0., nEventsProcessed);
      NumberEvents->Fill(1., sumOfWeights);
      NumberEvents->Fill(2., sumOfWeightsSquared);
      NumberEvents->Fill(3., event->getEntries());


   } // derivation

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::changeInput(bool /*firstFile*/)
{
   // Here you do everything you need to do when we change input files,
   // e.g. resetting branch addresses on trees.  If you are using
   // D3PDReader or a similar service this method is not needed.
   //  (firstFile); // avoid compilation warnings

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::initialize()
{
   // Here you do everything that you need to do after the first input
   // file has been connected and before the first event is processed,
   // e.g. create additional histograms based on which variables are
   // available in the input files.  You can also create all of your
   // histograms and trees in here, but be aware that this method
   // doesn't get called if no events are processed.  So any objects
   // you create here won't be available in the output if you have no
   // input events.

   xAOD::TEvent *event = wk()->xaodEvent();

   ////////////////////////
   // Counters
   ////////////////////////
   m_eventCounter = 0;

   // as a check, let's see the number of events in our xAOD
   ANA_MSG_INFO("Number of events = " << event->getEntries());

   ////////////////////////
   // Output tree
   ////////////////////////
   //   m_cand.reset();

   TFile *thisFile = wk()->getOutputFile("MiniNtuple");
   NumberEvents->SetDirectory(thisFile);

   const TString treeName = "MiniNtuple";

   ANA_MSG_INFO("Creating TTree named " << treeName.Data());

   m_tree = new TTree(treeName, treeName);
   m_tree->SetDirectory(thisFile);
   m_tree->Branch("run", &m_run);
   m_tree->Branch("event", &m_event);
   m_tree->Branch("mconly_weight", &m_mconly_weight);
   m_tree->Branch("truth_mc_px", &m_truth_mc_px);
   m_tree->Branch("truth_mc_py", &m_truth_mc_py);
   m_tree->Branch("truth_mc_pz", &m_truth_mc_pz);
   m_tree->Branch("truth_mc_e", &m_truth_mc_e);
   m_tree->Branch("truth_mc_px_dressed", &m_truth_mc_px_dressed);
   m_tree->Branch("truth_mc_py_dressed", &m_truth_mc_py_dressed);
   m_tree->Branch("truth_mc_pz_dressed", &m_truth_mc_pz_dressed);
   m_tree->Branch("truth_mc_e_dressed", &m_truth_mc_e_dressed);
   m_tree->Branch("truth_mc_type", &m_truth_mc_type);
   m_tree->Branch("truth_mc_origin", &m_truth_mc_origin);
   m_tree->Branch("truth_mc_dyn_iso", &m_truth_mc_dyn_iso);
   m_tree->Branch("truth_mc_fix_iso", &m_truth_mc_fix_iso);
   m_tree->Branch("truth_mc_pdg", &m_truth_mc_pdg);
   m_tree->Branch("truth_mc_status", &m_truth_mc_status);
   m_tree->Branch("truth_mc_barcode", &m_truth_mc_barcode);
   m_tree->Branch("truth_V_simple_pt", &m_truth_V_simple_pt);
   m_tree->Branch("jj_mass", &m_jj_mass);
   m_tree->Branch("jj_dphi", &m_jj_dphi);
   m_tree->Branch("jj_deta", &m_jj_deta);

   m_tree->Branch("njets", &m_njets);
   m_tree->Branch("njets25", &m_njets25);
   m_tree->Branch("jet_E", &m_jet_E);
   m_tree->Branch("jet_pt", &m_jet_pt);
   m_tree->Branch("jet_eta", &m_jet_eta);
   m_tree->Branch("jet_phi", &m_jet_phi);
   m_tree->Branch("jet_m", &m_jet_m);
   m_tree->Branch("jet_label", &m_jet_label);

   //   m_cand.attachToTree(m_tree, ""); // we use empty prefix


   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::execute()
{
   // Here you do everything that needs to be done on every single
   // events, e.g. read input variables, apply cuts, and fill
   // histograms and trees.  This is where most of your actual analysis
   // code will go.
   //
   const char *APP_NAME = "VBFInvVjetsRW::execute";

   bool passExp = false;
   for (int i = 0; i < 9; i++) {
      int exponent = pow(10, i);
      passExp |= (m_eventCounter <= exponent && (m_eventCounter % exponent) == 0);
   }
   if (passExp) ANA_MSG_INFO("Event number = " << m_eventCounter);
   m_eventCounter++;

   m_n_partonLeptons   = 0;
   m_n_partonNeutrinos = 0;
   m_n_partonPhotons   = 0;

   analyzeEvent();

   // clear content
   //   m_cand.reset();
   SafeDelete(m_particles.first);
   SafeDelete(m_particles.second);
   SafeDelete(m_photons.first);
   SafeDelete(m_photons.second);
   SafeDelete(m_muons.first);
   SafeDelete(m_muons.second);
   SafeDelete(m_electrons.first);
   SafeDelete(m_electrons.second);
   SafeDelete(m_bosons.first);
   SafeDelete(m_bosons.second);
   SafeDelete(m_neutrinos.first);
   SafeDelete(m_neutrinos.second);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::analyzeEvent()
{

   //   xAOD::TEvent *event = wk()->xaodEvent();

   if (defineContainers() != EL::StatusCode::SUCCESS) {
      ANA_MSG_ERROR("Error in keepEvent");
      return EL::StatusCode::FAILURE;
   }

   const xAOD::EventInfo *evtInfo = 0;
   ANA_CHECK(evtStore()->retrieve(evtInfo, "EventInfo"));

   m_cutFlow.addCutCounter(0, evtInfo->mcEventWeight());

   if (keepEvent() != EL::StatusCode::SUCCESS) {
      ANA_MSG_ERROR("Error in keepEvent");
      return EL::StatusCode::FAILURE;
   }

   resetBranches();

   // int
   m_run   = evtInfo->mcChannelNumber();
   m_event = evtInfo->eventNumber();

   // float
   m_mconly_weight = evtInfo->mcEventWeight();
   //      mconly_weights = evtInfo->mcEventWeights();

   if (m_isDAODTRUTH) {
      for (auto thisMuon : *m_muons.first) {
         fillParticle(thisMuon);
      }
      for (auto thisElectron : *m_electrons.first) {
         fillParticle(thisElectron);
      }
      for (auto thisPhoton : *m_photons.first) {
         fillParticle(thisPhoton);
      }
      for (auto thisNeutrino : *m_neutrinos.first) {
         fillParticle(thisNeutrino);
      }
      for (auto thisBoson : *m_bosons.first) {
         fillParticle(thisBoson);
      }
   } else {
      for (auto thisParticle : *m_particles.first) {
         if (thisParticle->status() == 1 || thisParticle->status() == 3) {
            if (((abs(thisParticle->pdgId()) >= 11 && abs(thisParticle->pdgId()) <= 16) ||
                 (thisParticle->pdgId() >= 22 && thisParticle->pdgId() <= 24))) {
               fillParticle(thisParticle); // keep leptons and bosons
            }
         }
      }
   }

   xAOD::TEvent *event = wk()->xaodEvent();
   const xAOD::TruthParticleContainer *truthParticles(nullptr);
   ANA_CHECK(event->retrieve(truthParticles, "TruthParticles"));
   const TLorentzVector truth_V_simple = VBFInvAnalysis::getTruthBosonP4_simple(truthParticles);
   m_truth_V_simple_pt = truth_V_simple.Pt();

   // Add jet information
   const xAOD::JetContainer *jets(nullptr);
   if (!event->retrieve(jets,  "AntiKt4TruthDressedWZJets").isSuccess()) { // retrieve arguments: container type, container key
      ANA_MSG_ERROR("Failed to retrieve AntiKt4TruthDressedWZJets container");
      return EL::StatusCode::FAILURE;
   }

   std::vector<TLorentzVector> truthJets;
   TLorentzVector              truthJet;

   int njet = 0;
   int njet25 = 0;
   for (const auto *jet : *jets) {
      TLorentzVector truthJet;
      truthJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());
      truthJets.push_back(truthJet);
      m_jet_E.push_back(jet->e());
      m_jet_pt.push_back(jet->pt());
      m_jet_eta.push_back(jet->eta());
      m_jet_phi.push_back(jet->phi());
      m_jet_m.push_back(jet->m());
      m_jet_label.push_back(jet->auxdata<int>("PartonTruthLabelID"));
      njet++;
      if (jet->pt() > 25000. ) ++njet25;
   }
   m_njets = njet;
   m_njets25 = njet25;

   double jj_deta = -1., jj_mass = -1., jj_dphi = -1.;
   HelperFunctions::computejj(&truthJets, jj_mass, jj_deta, jj_dphi);
   m_jj_mass = jj_mass;
   m_jj_deta = jj_deta;
   m_jj_dphi = jj_dphi;

   m_tree->Fill();

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::postExecute()
{
   // Here you do everything that needs to be done after the main event
   // processing.  This is typically very rare, particularly in user
   // code.  It is mainly used in implementing the NTupleSvc.
   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::finalize()
{
   // This method is the mirror image of initialize(), meaning it gets
   // called after the last event has been processed on the worker node
   // and allows you to finish up any objects you created in
   // initialize() before they are written to disk.  This is actually
   // fairly rare, since this happens separately for each worker node.
   // Most of the time you want to do your post-processing on the
   // submission node after all your histogram outputs have been
   // merged.  This is different from histFinalize() in that it only
   // gets called on worker nodes that processed input events.

   ANA_MSG_INFO("Number of processed events = " << m_eventCounter);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW ::histFinalize()
{
   // This method is the mirror image of histInitialize(), meaning it
   // gets called after the last event has been processed on the worker
   // node and allows you to finish up any objects you created in
   // histInitialize() before they are written to disk.  This is
   // actually fairly rare, since this happens separately for each
   // worker node.  Most of the time you want to do your
   // post-processing on the submission node after all your histogram
   // outputs have been merged.  This is different from finalize() in
   // that it gets called on all worker nodes regardless of whether
   // they processed input events.
   //

   m_cutFlow.printUnw();
   m_cutFlow.print();

   TH1F *cflow_hist     = m_cutFlow.createTH1F();
   TH1F *cflow_hist_unw = m_cutFlow.createTH1Fraw();
   wk()->addOutput(cflow_hist);
   wk()->addOutput(cflow_hist_unw);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW::defineContainers()
{

   // THIS is the time-expensive part

   xAOD::TEvent *event = wk()->xaodEvent();

   const xAOD::TruthParticleContainer *particles = nullptr;
   const xAOD::TruthParticleContainer *muons     = nullptr;
   const xAOD::TruthParticleContainer *electrons = nullptr;
   const xAOD::TruthParticleContainer *neutrinos = nullptr;
   const xAOD::TruthParticleContainer *photons   = nullptr;
   const xAOD::TruthParticleContainer *bosons    = nullptr;

   if (!m_determinedDerivation) {
      ANA_MSG_INFO("Determining derivation type");
      m_isDAODTRUTH          = event->retrieve(particles, "TruthMuons").isSuccess();
      m_determinedDerivation = kTRUE;
      TString isTRUTH        = (m_isDAODTRUTH) ? "YES" : "NO";
      ANA_MSG_INFO(
         "Is it DAOD_TRUTH? (will trigger access to dedicated truth electron and muon containers): " << isTRUTH);
   }

   const TString particles_container = "TruthParticles";
   if (!event->retrieve(particles, particles_container.Data())
           .isSuccess()) { // retrieve arguments: container type, container key
      ANA_MSG_ERROR("Failed to retrieve TruthParticles container");
      return EL::StatusCode::FAILURE;
   }
   m_particles = xAOD::shallowCopyContainer(*particles);

   if (m_isDAODTRUTH) {
      const TString muons_container = "TruthMuons";
      if (!event->retrieve(muons, muons_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthMuons container");
         return EL::StatusCode::FAILURE;
      }

      const TString electrons_container = "TruthElectrons";
      if (!event->retrieve(electrons, electrons_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthElectrons container");
         return EL::StatusCode::FAILURE;
      }

      const TString neutrinos_container = "TruthNeutrinos";
      if (!event->retrieve(neutrinos, neutrinos_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthNeutrinos container");
         return EL::StatusCode::FAILURE;
      }

      const TString photons_container = "TruthPhotons";
      if (!event->retrieve(photons, photons_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthPhotons container");
         return EL::StatusCode::FAILURE;
      }

      const TString boson_container = "TruthBoson";
      if (!event->retrieve(bosons, boson_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthBoson container");
         return EL::StatusCode::FAILURE;
      }

      m_photons   = xAOD::shallowCopyContainer(*photons);
      m_neutrinos = xAOD::shallowCopyContainer(*neutrinos);
      m_muons     = xAOD::shallowCopyContainer(*muons);
      m_electrons = xAOD::shallowCopyContainer(*electrons);
      m_bosons    = xAOD::shallowCopyContainer(*bosons);
   }

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW::keepEvent()
{

   // keep events with parton leptons, photons and neutrinos

   for (auto thisParticle : *m_particles.first) {

      if (thisParticle->status() == 3) {
         if (abs(thisParticle->pdgId()) == 11 || abs(thisParticle->pdgId()) == 13 || abs(thisParticle->pdgId()) == 15)
            m_n_partonLeptons++;
         else if (abs(thisParticle->pdgId()) == 12 || abs(thisParticle->pdgId()) == 14 ||
                  abs(thisParticle->pdgId()) == 16)
            m_n_partonNeutrinos++;
         if (abs(thisParticle->pdgId()) == 22) m_n_partonPhotons++;
      }
   }

   if (m_n_partonLeptons + m_n_partonNeutrinos + m_n_partonPhotons > 0) {
      return EL::StatusCode::SUCCESS;
   } else {
      ANA_MSG_ERROR("Failed to retrieve parton leptons, neutrinos or photons");
      return EL::StatusCode::FAILURE;
   }
}

EL::StatusCode VBFInvVjetsRW::resetBranches()
{

   m_run           = 0;
   m_event         = 0;
   m_mconly_weight = 0;

   m_truth_mc_px.clear();
   m_truth_mc_py.clear();
   m_truth_mc_pz.clear();
   m_truth_mc_e.clear();

   m_truth_mc_px_dressed.clear();
   m_truth_mc_py_dressed.clear();
   m_truth_mc_pz_dressed.clear();
   m_truth_mc_e_dressed.clear();

   m_truth_mc_status.clear();
   m_truth_mc_barcode.clear();
   m_truth_mc_pdg.clear();
   m_truth_mc_fix_iso.clear();
   m_truth_mc_dyn_iso.clear();
   m_truth_mc_type.clear();
   m_truth_mc_origin.clear();

   m_jet_E.clear();
   m_jet_pt.clear();
   m_jet_eta.clear();
   m_jet_phi.clear();
   m_jet_m.clear();
   m_jet_label.clear();

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW::fillParticle(xAOD::TruthParticle *thisParticle)
{

   unsigned int paritcle_type   = thisParticle->auxdata<unsigned int>("classifierParticleType");
   unsigned int particle_origin = thisParticle->auxdata<unsigned int>("classifierParticleOrigin");

   if (thisParticle->pdgId() == 22 && m_n_partonPhotons > 0) {
      checkPhotonIsolation(thisParticle);
   }

   TLorentzVector m_truth_mc_p4 = thisParticle->p4();

   m_truth_mc_status.push_back((Int_t)thisParticle->status());
   m_truth_mc_pdg.push_back((Int_t)thisParticle->pdgId());
   m_truth_mc_barcode.push_back((Int_t)thisParticle->barcode());
   m_truth_mc_type.push_back((Int_t)paritcle_type);
   m_truth_mc_origin.push_back((Int_t)particle_origin);

   TLorentzVector m_truth_mc_p4_dressed;
   m_truth_mc_p4_dressed.SetPtEtaPhiE(
      thisParticle->auxdata<float>("pt_dressed"), thisParticle->auxdata<float>("eta_dressed"),
      thisParticle->auxdata<float>("phi_dressed"), thisParticle->auxdata<float>("e_dressed"));

   m_truth_mc_px_dressed.push_back((Float_t)m_truth_mc_p4_dressed.Px());
   m_truth_mc_py_dressed.push_back((Float_t)m_truth_mc_p4_dressed.Py());
   m_truth_mc_pz_dressed.push_back((Float_t)m_truth_mc_p4_dressed.Pz());
   m_truth_mc_e_dressed.push_back((Float_t)m_truth_mc_p4_dressed.E());

   m_truth_mc_dyn_iso.push_back((Int_t)m_iso["dyn"]);
   m_truth_mc_fix_iso.push_back((Int_t)m_iso["fix"]);

   m_truth_mc_px.push_back((Float_t)m_truth_mc_p4.Px());
   m_truth_mc_py.push_back((Float_t)m_truth_mc_p4.Py());
   m_truth_mc_pz.push_back((Float_t)m_truth_mc_p4.Pz());
   m_truth_mc_e.push_back((Float_t)m_truth_mc_p4.E());

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvVjetsRW::checkPhotonIsolation(xAOD::TruthParticle *thisParticle)
{

   Float_t Mz = 91.1876;

   Float_t ph_pt = thisParticle->pt();

   std::map<TString, Float_t> e0;
   e0["fix"] = 0.025;
   e0["dyn"] = 0.1;
   std::map<TString, Float_t> n;
   n["fix"] = 2;
   n["dyn"] = 1;
   std::map<TString, Float_t> R0;
   R0["fix"] = 0.4;
   R0["dyn"] = Mz * 1e3 / (ph_pt * TMath::Sqrt(e0["dyn"])); // min(1.0, Rdyn)
   if (R0["dyn"] > 1.0) R0["dyn"] = 1.0;

   std::map<TString, std::vector<Float_t>>        cones;
   std::map<TString, std::vector<TLorentzVector>> hInCone;

   for (auto kv : R0) {

      TString kv_iso = kv.first;
      Float_t kv_R0  = kv.second;

      m_iso[kv_iso] = kTRUE;

      for (auto hadron : *m_particles.first) {
         if (!hadron->isHadron()) continue;
         if (hadron->status() % 1000 != 1) continue;
         //  if (abs(hadron->pdgId()) > 1e8) // not suggested for Sherpa
         //         continue;
         if (hadron->barcode() > 200000) continue;

         Float_t dR = thisParticle->p4().DeltaR(hadron->p4());

         // fix isolation cones
         if (dR < kv_R0) {
            if (cones[kv_iso].size() == 0) {
               cones[kv_iso].push_back(dR);
               hInCone[kv_iso].push_back(hadron->p4());
            } else {
               Int_t i = 0;
               for (auto r : cones[kv_iso]) {
                  if (dR < r) break;
                  i += 1;
               }
               hInCone[kv_iso].insert(hInCone[kv_iso].begin() + i, hadron->p4());
               cones[kv_iso].insert(cones[kv_iso].begin() + i, dR);
            }
         }
      }

      Float_t sum_had_pt = 0;

      for (Int_t i = 0; i < (Int_t)hInCone[kv_iso].size(); i++) {
         Float_t R = cones[kv_iso].at(i);
         sum_had_pt += hInCone[kv_iso].at(i).Pt();
         Float_t chi = e0[kv_iso] * ph_pt * TMath::Power(((1 - TMath::Cos(R)) / (1 - TMath::Cos(kv_R0))), n[kv_iso]);
         if (sum_had_pt > chi) {
            m_iso[kv_iso] = kFALSE;
            break;
         }
      }
   }

   return EL::StatusCode::SUCCESS;
}
