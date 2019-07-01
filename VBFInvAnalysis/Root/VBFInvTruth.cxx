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

// Local include(s):
#include <VBFInvAnalysis/VBFInvTruth.h>
#include <VBFInvAnalysis/HelperFunctions.h>

#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE

using namespace xAOD;

static SG::AuxElement::Decorator<bool> dec_passOR("passTruthOR");

// this is needed to distribute the algorithm to the workers
ClassImp(VBFInvTruth)

   VBFInvTruth ::VBFInvTruth()
   : my_XsecDB(0)
{
}

EL::StatusCode VBFInvTruth ::setupJob(EL::Job &job)
{
   job.useXAOD();

   xAOD::Init("VBFInvTruth").ignore(); // call before opening first file

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::histInitialize()
{

   NumberEvents = new TH1D("NumberEvents", "Number Events", 4, 0, 4);
   NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
   NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
   NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");
   NumberEvents->GetXaxis()->SetBinLabel(4, "RawTRUTH");
   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::fileExecute()
{

   m_event = wk()->xaodEvent();
   ANA_MSG_INFO("Number of events in this file = " << m_event->getEntries()); // print long long int

   if (skipCBK) {
      NumberEvents->Fill(0., m_event->getEntries());
      NumberEvents->Fill(3., m_event->getEntries());
   } else {
      // Read the CutBookkeeper container
      const xAOD::CutBookkeeperContainer *completeCBC = 0;
      if (!m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()) {
         ANA_MSG_ERROR("Failed to retrieve CutBookkeepers from MetaData! Exiting.");
      }

      const xAOD::CutBookkeeper *allEventsCBK = 0;
      int                        maxcycle     = -1;

      // let's find the right CBK (latest with StreamAOD input before derivations)
      for (auto cbk : *completeCBC) {
         if (cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD" && cbk->cycle() > maxcycle) {
            maxcycle     = cbk->cycle();
            allEventsCBK = cbk;
         }
      }

      uint64_t nEventsProcessed    = 0;
      double   sumOfWeights        = 0.;
      double   sumOfWeightsSquared = 0.;
      if (allEventsCBK) {
         nEventsProcessed    = allEventsCBK->nAcceptedEvents();
         sumOfWeights        = allEventsCBK->sumOfEventWeights();
         sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();
         ANA_MSG_INFO("CutBookkeepers Accepted:" << nEventsProcessed << ", SumWei:" << sumOfWeights
                                                 << ", sumWei2:" << sumOfWeightsSquared);
      } else {
         ANA_MSG_INFO("No relevent CutBookKeepers found");
         nEventsProcessed = m_event->getEntries();
      }

      NumberEvents->Fill(0., nEventsProcessed);
      NumberEvents->Fill(1., sumOfWeights);
      NumberEvents->Fill(2., sumOfWeightsSquared);
      NumberEvents->Fill(3., m_event->getEntries());
   }

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::initialize()
{

   m_event = wk()->xaodEvent();

   std::string xSecFilePath = "dev/PMGTools/PMGxsecDB_mc15.txt";
   xSecFilePath             = PathResolverFindCalibFile(xSecFilePath);
   my_XsecDB                = new SUSY::CrossSectionDB(xSecFilePath);
   if (debug) ANA_MSG_INFO("xsec DB initialized using file:" << xSecFilePath);

   m_jet_E     = new std::vector<float>();
   m_jet_pt    = new std::vector<float>();
   m_jet_eta   = new std::vector<float>();
   m_jet_phi   = new std::vector<float>();
   m_jet_m     = new std::vector<float>();
   m_jet_label = new std::vector<int>();

   m_el_m        = new std::vector<float>();
   m_el_pt       = new std::vector<float>();
   m_el_eta      = new std::vector<float>();
   m_el_phi      = new std::vector<float>();
   m_el_type     = new std::vector<uint>();
   m_el_origin   = new std::vector<uint>();
   m_el_ptcone30 = new std::vector<float>();
   m_el_etcone20 = new std::vector<float>();
   m_el_pdgid    = new std::vector<int>();

   m_ph_m        = new std::vector<float>();
   m_ph_pt       = new std::vector<float>();
   m_ph_eta      = new std::vector<float>();
   m_ph_phi      = new std::vector<float>();
   m_ph_type     = new std::vector<uint>();
   m_ph_origin   = new std::vector<uint>();
   m_ph_ptcone30 = new std::vector<float>();
   m_ph_etcone20 = new std::vector<float>();
   m_ph_pdgid    = new std::vector<int>();

   m_mu_m        = new std::vector<float>();
   m_mu_pt       = new std::vector<float>();
   m_mu_eta      = new std::vector<float>();
   m_mu_phi      = new std::vector<float>();
   m_mu_type     = new std::vector<uint>();
   m_mu_origin   = new std::vector<uint>();
   m_mu_ptcone30 = new std::vector<float>();
   m_mu_etcone20 = new std::vector<float>();
   m_mu_pdgid    = new std::vector<int>();

   m_tau_m              = new std::vector<float>();
   m_tau_pt             = new std::vector<float>();
   m_tau_eta            = new std::vector<float>();
   m_tau_phi            = new std::vector<float>();
   m_tau_type           = new std::vector<uint>();
   m_tau_origin         = new std::vector<uint>();
   m_tau_pdgid          = new std::vector<int>();
   m_tau_IsHadronicTau  = new std::vector<char>();
   m_tau_m_invis        = new std::vector<double>();
   m_tau_m_vis          = new std::vector<double>();
   m_tau_numCharged     = new std::vector<ULong_t>();
   m_tau_numChargedPion = new std::vector<ULong_t>();
   m_tau_numNeutral     = new std::vector<ULong_t>();
   m_tau_numNeutralPion = new std::vector<ULong_t>();
   m_tau_barcode        = new std::vector<int>();
   m_tau_status         = new std::vector<int>();

   m_boson_e     = new std::vector<float>();
   m_boson_m     = new std::vector<float>();
   m_boson_pt    = new std::vector<float>();
   m_boson_eta   = new std::vector<float>();
   m_boson_phi   = new std::vector<float>();
   m_boson_pdgid = new std::vector<int>();

   m_nu_e      = new std::vector<float>();
   m_nu_m      = new std::vector<float>();
   m_nu_pt     = new std::vector<float>();
   m_nu_eta    = new std::vector<float>();
   m_nu_phi    = new std::vector<float>();
   m_nu_type   = new std::vector<uint>();
   m_nu_origin = new std::vector<uint>();
   m_nu_pdgid  = new std::vector<int>();

   m_parton_x1      = new std::vector<float>();
   m_parton_x2      = new std::vector<float>();
   m_parton_xf1     = new std::vector<float>();
   m_parton_xf2     = new std::vector<float>();
   m_parton_Q       = new std::vector<float>();
   m_parton_pdgid1  = new std::vector<int>();
   m_parton_pdgid2  = new std::vector<int>();
   m_parton_pdfid1  = new std::vector<int>();
   m_parton_pdfid2  = new std::vector<int>();
   m_parton_pp      = new std::vector<int>();
   m_EventWeightSys = new std::vector<float>();

   ANA_MSG_INFO("Histogram output name is " << outputNameHist);
   TFile *outputFileHist = wk()->getOutputFile(outputNameHist);
   ANA_MSG_INFO("Tree output name is " << outputName);
   TFile *outputFile    = wk()->getOutputFile(outputName);
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
   truthTree->Branch("EventWeightSys", &m_EventWeightSys);

   // Jets
   truthTree->Branch("njets", &m_njets);
   truthTree->Branch("jet_E", &m_jet_E);
   truthTree->Branch("jet_pt", &m_jet_pt);
   truthTree->Branch("jet_eta", &m_jet_eta);
   truthTree->Branch("jet_phi", &m_jet_phi);
   truthTree->Branch("jet_m", &m_jet_m);
   truthTree->Branch("jet_label", &m_jet_label);

   // Electrons
   truthTree->Branch("nels", &m_nels);
   truthTree->Branch("el_m", &m_el_m);
   truthTree->Branch("el_pt", &m_el_pt);
   truthTree->Branch("el_eta", &m_el_eta);
   truthTree->Branch("el_phi", &m_el_phi);
   truthTree->Branch("el_type", &m_el_type);
   truthTree->Branch("el_origin", &m_el_origin);
   truthTree->Branch("el_ptcone30", &m_el_ptcone30);
   truthTree->Branch("el_etcone20", &m_el_etcone20);
   truthTree->Branch("el_pdgid", &m_el_pdgid);

   // Photons
   truthTree->Branch("nphs",        &m_nphs);
   truthTree->Branch("ph_m",        &m_ph_m);
   truthTree->Branch("ph_pt",       &m_ph_pt);
   truthTree->Branch("ph_eta",      &m_ph_eta);
   truthTree->Branch("ph_phi",      &m_ph_phi);
   truthTree->Branch("ph_type",     &m_ph_type);
   truthTree->Branch("ph_origin",   &m_ph_origin);
   truthTree->Branch("ph_ptcone30", &m_ph_ptcone30);
   truthTree->Branch("ph_etcone20", &m_ph_etcone20);
   truthTree->Branch("ph_pdgid",    &m_ph_pdgid);

   // Muons
   truthTree->Branch("nmus", &m_nmus);
   truthTree->Branch("mu_m", &m_mu_m);
   truthTree->Branch("mu_pt", &m_mu_pt);
   truthTree->Branch("mu_eta", &m_mu_eta);
   truthTree->Branch("mu_phi", &m_mu_phi);
   truthTree->Branch("mu_type", &m_mu_type);
   truthTree->Branch("mu_origin", &m_mu_origin);
   truthTree->Branch("mu_ptcone30", &m_mu_ptcone30);
   truthTree->Branch("mu_etcone20", &m_mu_etcone20);
   truthTree->Branch("mu_pdgid", &m_mu_pdgid);

   // Taus
   /*
   truthTree->Branch ("tau_m", &m_tau_m);
   truthTree->Branch ("tau_pt", &m_tau_pt);
   truthTree->Branch ("tau_eta", &m_tau_eta);
   truthTree->Branch ("tau_phi", &m_tau_phi);
   truthTree->Branch ("tau_type", &m_tau_type);
   truthTree->Branch ("tau_origin", &m_tau_origin);
   truthTree->Branch ("tau_pdgid", &m_tau_pdgid);
   truthTree->Branch ("tau_IsHadronicTau", &m_tau_IsHadronicTau);
   truthTree->Branch ("tau_m_invis", &m_tau_m_invis);
   truthTree->Branch ("tau_m_vis", &m_tau_m_vis);
   truthTree->Branch ("tau_numCharged", &m_tau_numCharged);
   truthTree->Branch ("tau_numChargedPion", &m_tau_numChargedPion);
   truthTree->Branch ("tau_numNeutral", &m_tau_numNeutral);
   truthTree->Branch ("tau_numNeutralPion", &m_tau_numNeutralPion);
   truthTree->Branch ("tau_barcode", &m_tau_barcode);
   truthTree->Branch ("tau_status", &m_tau_status);
*/
   // Bosons
   truthTree->Branch("nbosons", &m_nbosons);
   truthTree->Branch("boson_m", &m_boson_m);
   truthTree->Branch("boson_pt", &m_boson_pt);
   truthTree->Branch("boson_eta", &m_boson_eta);
   truthTree->Branch("boson_phi", &m_boson_phi);
   truthTree->Branch("boson_pdgid", &m_boson_pdgid);

   // Neutrinos
   truthTree->Branch("nnus", &m_nnus);
   truthTree->Branch("nu_e", &m_nu_e);
   truthTree->Branch("nu_m", &m_nu_m);
   truthTree->Branch("nu_pt", &m_nu_pt);
   truthTree->Branch("nu_eta", &m_nu_eta);
   truthTree->Branch("nu_phi", &m_nu_phi);
   truthTree->Branch("nu_type", &m_nu_type);
   truthTree->Branch("nu_origin", &m_nu_origin);
   truthTree->Branch("nu_pdgid", &m_nu_pdgid);

   // MET
   truthTree->Branch("met_et", &m_met_et);
   truthTree->Branch("met_phi", &m_met_phi);

   // Parton
   truthTree->Branch("parton_x1", &m_parton_x1);
   truthTree->Branch("parton_x2", &m_parton_x2);
   truthTree->Branch("parton_xf1", &m_parton_xf1);
   truthTree->Branch("parton_xf2", &m_parton_xf2);
   truthTree->Branch("parton_Q", &m_parton_Q);
   truthTree->Branch("parton_pdgid1", &m_parton_pdgid1);
   truthTree->Branch("parton_pdgid2", &m_parton_pdgid2);
   truthTree->Branch("parton_pdfid1", &m_parton_pdfid1);
   truthTree->Branch("parton_pdfid2", &m_parton_pdfid2);
   truthTree->Branch("parton_pp", &m_parton_pp);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::execute()
{

   m_jet_E->clear();
   m_jet_pt->clear();
   m_jet_eta->clear();
   m_jet_phi->clear();
   m_jet_m->clear();
   m_jet_label->clear();

   m_el_m->clear();
   m_el_pt->clear();
   m_el_eta->clear();
   m_el_phi->clear();
   m_el_type->clear();
   m_el_origin->clear();
   m_el_ptcone30->clear();
   m_el_etcone20->clear();
   m_el_pdgid->clear();

   m_ph_m->clear();
   m_ph_pt->clear();
   m_ph_eta->clear();
   m_ph_phi->clear();
   m_ph_type->clear();
   m_ph_origin->clear();
   m_ph_ptcone30->clear();
   m_ph_etcone20->clear();
   m_ph_pdgid->clear();

   m_mu_m->clear();
   m_mu_pt->clear();
   m_mu_eta->clear();
   m_mu_phi->clear();
   m_mu_type->clear();
   m_mu_origin->clear();
   m_mu_ptcone30->clear();
   m_mu_etcone20->clear();
   m_mu_pdgid->clear();

   m_boson_e->clear();
   m_boson_m->clear();
   m_boson_pt->clear();
   m_boson_eta->clear();
   m_boson_phi->clear();
   m_boson_pdgid->clear();

   m_nu_e->clear();
   m_nu_m->clear();
   m_nu_pt->clear();
   m_nu_eta->clear();
   m_nu_phi->clear();
   m_nu_type->clear();
   m_nu_origin->clear();
   m_nu_pdgid->clear();

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
   m_EventWeightSys->clear();

   if (debug) ANA_MSG_INFO("***New event***");

   //----------------------------
   // Event information
   //---------------------------
   if (debug) ANA_MSG_INFO("Retrieving event info collection.");

   const xAOD::EventInfo *eventInfo = 0;
   if (!m_event->retrieve(eventInfo, "EventInfo").isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve event info collection. Exiting.");
      return EL::StatusCode::FAILURE;
   }

   m_EventNumber                           = eventInfo->eventNumber();
   m_RunNumber                             = eventInfo->runNumber();
   m_WeightEvents                          = eventInfo->mcEventWeight(); //.at(0);
   m_ChannelNumber                         = eventInfo->mcChannelNumber();
   const std::vector<float> EventWeightSys = eventInfo->mcEventWeights();
   for (unsigned iE = 0; iE < EventWeightSys.size(); ++iE) m_EventWeightSys->push_back(EventWeightSys.at(iE));

   if (skipCBK) {
      NumberEvents->Fill(1, m_WeightEvents);
      NumberEvents->Fill(2, m_WeightEvents * m_WeightEvents);
   }

   m_crossSection = -1; // my_XsecDB->xsectTimesEff(eventInfo->mcChannelNumber());

   if (debug)
      ANA_MSG_INFO("EventNumber=" << m_EventNumber << ", RunNumber=" << m_WeightEvents
                                  << ", WeightEvents=" << m_WeightEvents << ", Cross section=" << m_crossSection);

   //-----------------------------------------------------------------------
   //  Retrieve containers
   //-----------------------------------------------------------------------

   xAOD::TEvent *event = wk()->xaodEvent();
   // Jets
   const xAOD::JetContainer *jets = nullptr;

   static Bool_t failedLookingFor(kFALSE); // trick to avoid infinite RuntimeWarning's for EXOT5
   if (!failedLookingFor) {
      if (!event->retrieve(jets, "AntiKt4TruthJets").isSuccess()) {
         if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
         ANA_CHECK(evtStore()->retrieve(jets, "AntiKt4TruthDressedWZJets"));
         failedLookingFor = kTRUE;
      } else {
         if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthJets!");
      }
   } else {
      if (debug) ANA_MSG_INFO("Retrieved truth jet container in AntiKt4TruthDressedWZJets!");
      ANA_CHECK(evtStore()->retrieve(jets, "AntiKt4TruthDressedWZJets"));
   }

   // Electrons
   const xAOD::TruthParticleContainer *els = nullptr;
   ANA_CHECK(evtStore()->retrieve(els, "TruthElectrons"));

   // Photons
   const xAOD::TruthParticleContainer *phs = nullptr;
   ANA_CHECK(evtStore()->retrieve(phs, "TruthPhotons"));

   // Muons
   const xAOD::TruthParticleContainer *mus = nullptr;
   ANA_CHECK(evtStore()->retrieve(mus, "TruthMuons"));

   // Taus
   // const xAOD::TruthParticleContainer* taus = nullptr;
   // ANA_CHECK (evtStore()->retrieve( taus, "TruthTaus"));

   // Neutrinos
   const xAOD::TruthParticleContainer *neus = nullptr;
   ANA_CHECK(evtStore()->retrieve(neus, "TruthNeutrinos"));

   // MET
   const xAOD::MissingETContainer *met = nullptr;
   ANA_CHECK(evtStore()->retrieve(met, "MET_Truth"));

   // Bosons
   const xAOD::TruthParticleContainer *bosons = nullptr;
   if (!noTruthBoson) ANA_CHECK(evtStore()->retrieve(bosons, "TruthBoson"));

   // Neutrinos
   const xAOD::TruthParticleContainer *neutrinos = nullptr;
   ANA_CHECK(evtStore()->retrieve(neutrinos, "TruthNeutrinos"));

   // TruthEvents
   const xAOD::TruthEventContainer *truthE = nullptr;
   ANA_CHECK(m_event->retrieve(truthE, "TruthEvents"));

   //-----------------------------------------------------------------------
   //  Overlap Removal
   //-----------------------------------------------------------------------

   // Jets - leptons
   for (const auto &truthJ_itr : *jets) {
      if (truthJ_itr->pt() > 5000.) {
         dec_passOR(*truthJ_itr) = true;
         // overlap with electrons
         for (const auto &elec_itr : *els) {
            if (elec_itr->pt() > 5000) {
               float dR = xAOD::P4Helpers::deltaR2(truthJ_itr, elec_itr, true);
               ANA_MSG_DEBUG("jet pt=" << truthJ_itr->pt() << ", el pt=" << elec_itr->pt() << ", DR=" << sqrt(dR));
               if (dR < (0.2 * 0.2)) {
                  dec_passOR(*truthJ_itr) = false;
                  ANA_MSG_DEBUG("jet-el OR: Rejecting jet!");
                  break;
               }
            }
         }
         // overlap with muons
         for (const auto &mu_itr : *mus) {
            if (mu_itr->pt() > 5000) {
               float dR = xAOD::P4Helpers::deltaR2(truthJ_itr, mu_itr, true);
               ANA_MSG_DEBUG("jet pt=" << truthJ_itr->pt() << ", mu pt=" << mu_itr->pt() << ", DR=" << sqrt(dR));
               if (dR < (0.2 * 0.2)) {
                  dec_passOR(*truthJ_itr) = false;
                  ANA_MSG_DEBUG("jet-mu OR: Rejecting jet!");
                  break;
               }
            }
         }
      } else {
         dec_passOR(*truthJ_itr) = false;
         ANA_MSG_DEBUG("jet-lep OR: Does not pass due to jet pt < 5 GeV!");
      }
   }
   // Electrons - Jets
   for (const auto &elec_itr : *els) {
      if (elec_itr->pt() > 5000) {
         dec_passOR(*elec_itr) = true;
         for (const auto &truthJ_itr : *jets)
            if (truthJ_itr->pt() > 5000. && truthJ_itr->auxdata<bool>("passTruthOR")) {
               float dR = xAOD::P4Helpers::deltaR2(elec_itr, truthJ_itr, true);
               if (dR < (0.4 * 0.4)) {
                  dec_passOR(*elec_itr) = false;
                  ANA_MSG_DEBUG("el-jet OR: Rejecting el!");
                  break;
               }
            }
      } else {
         dec_passOR(*elec_itr) = false;
         ANA_MSG_DEBUG("el-jet OR: Does not pass due to el pt < 7 GeV!");
      }
   }

   // Muons - Jets
   for (const auto &mu_itr : *mus) {
      if (mu_itr->pt() > 5000) {
         dec_passOR(*mu_itr) = true;
         for (const auto &truthJ_itr : *jets)
            if (truthJ_itr->pt() > 5000. && truthJ_itr->auxdata<bool>("passTruthOR")) {
               float dR = xAOD::P4Helpers::deltaR2(mu_itr, truthJ_itr, true);
               if (dR < (0.4 * 0.4)) {
                  dec_passOR(*mu_itr) = false;
                  ANA_MSG_DEBUG("mu-jet OR: Rejecting mu!");
                  break;
               }
            }
      } else {
         dec_passOR(*mu_itr) = false;
         ANA_MSG_DEBUG("mu-jet OR: Does not pass due to mu pt < 5 GeV!");
      }
   }

   //-----------------------------------------------------------------------
   //  Fill branches
   //-----------------------------------------------------------------------

   // Jets
   int njet5 = 0;
   for (const auto &truthJ_itr : *jets) {
      if (truthJ_itr->pt() > 5000. && truthJ_itr->auxdata<bool>("passTruthOR")) {
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

   // Electrons
   int nel5 = 0;
   for (const auto &elec_itr : *els)
      if (elec_itr->pt() > 5000. && elec_itr->auxdata<bool>("passTruthOR")) {
         m_el_m->push_back(elec_itr->m());
         m_el_pt->push_back(elec_itr->pt());
         m_el_eta->push_back(elec_itr->eta());
         m_el_phi->push_back(elec_itr->phi());
         m_el_type->push_back(elec_itr->auxdata<uint>("classifierParticleType"));
         m_el_origin->push_back(elec_itr->auxdata<uint>("classifierParticleOrigin"));
         m_el_ptcone30->push_back(elec_itr->auxdata<float>("ptcone30"));
         m_el_etcone20->push_back(elec_itr->auxdata<float>("etcone20"));
         m_el_pdgid->push_back(elec_itr->pdgId());
         nel5++;
      }
   m_nels = nel5;

   // Photons
   int nph5 = 0;
   for (const auto &ph_itr : *phs){
     //dec_passOR(*ph_itr) = true;
     if (ph_itr->pt() > 5000.){// && ph_itr->auxdata<bool>("passTruthOR")) {
         m_ph_m->push_back(ph_itr->m());
         m_ph_pt->push_back (ph_itr->pt());
         m_ph_eta->push_back(ph_itr->eta());
         m_ph_phi->push_back(ph_itr->phi());
         m_ph_type->push_back(ph_itr->auxdata<uint>("classifierParticleType"));
         m_ph_origin->push_back(ph_itr->auxdata<uint>("classifierParticleOrigin"));
         m_ph_ptcone30->push_back(ph_itr->auxdata<float>("ptcone30"));
         m_ph_etcone20->push_back(ph_itr->auxdata<float>("etcone20"));
         m_ph_pdgid->push_back(ph_itr->pdgId());
         nph5++;
      }
   }
   m_nphs = nph5;

   // Muons
   int nmu5 = 0;
   for (const auto &mu_itr : *mus) {
      if (mu_itr->pt() > 5000. && mu_itr->auxdata<bool>("passTruthOR")) {
         m_mu_m->push_back(mu_itr->m());
         m_mu_pt->push_back(mu_itr->pt());
         m_mu_eta->push_back(mu_itr->eta());
         m_mu_phi->push_back(mu_itr->phi());
         m_mu_type->push_back(mu_itr->auxdata<uint>("classifierParticleType"));
         m_mu_origin->push_back(mu_itr->auxdata<uint>("classifierParticleOrigin"));
         m_mu_ptcone30->push_back(mu_itr->auxdata<float>("ptcone30"));
         m_mu_etcone20->push_back(mu_itr->auxdata<float>("etcone20"));
         m_mu_pdgid->push_back(mu_itr->pdgId());
         nmu5++;
      }
   }
   m_nmus = nmu5;

   // taus
   /*
int ntau5=0;
for (const auto& tau_itr : *taus)
          if (tau_itr->pt() > 5000. && tau_itr->auxdata< bool >("passTruthOR")) {
   m_tau_m->push_back(tau_itr->m());
m_tau_pt->push_back(tau_itr->pt());
m_tau_eta->push_back(tau_itr->eta());
m_tau_phi->push_back(tau_itr->phi());
m_tau_type->push_back(tau_itr->auxdata<uint>("classifierParticleType"));
m_tau_origin->push_back(tau_itr->auxdata<uint>("classifierParticleOrigin"));
m_tau_pdgid->push_back(tau_itr->pdgId());
m_tau_IsHadronicTau->push_back(tau_itr->auxdata<char>("IsHadronicTau"));
m_tau_m_invis->push_back(-1.);//tau_itr->auxdata<double>("m_invis"));
m_tau_m_vis->push_back(-1.);//tau_itr->auxdata<double>("m_vis"));
m_tau_numCharged->push_back(tau_itr->auxdata<ULong_t>("numCharged"));
m_tau_numChargedPion->push_back(tau_itr->auxdata<ULong_t>("numChargedPion"));
m_tau_numNeutral->push_back(tau_itr->auxdata<ULong_t>("numNeutral"));
m_tau_numNeutralPion->push_back(tau_itr->auxdata<ULong_t>("numNeutralPion"));
m_tau_barcode->push_back(tau_itr->auxdata<int>("barcode"));
m_tau_status->push_back(tau_itr->auxdata<int>("status"));
ntau5++;
}
m_ntaus = ntau5;
*/
   // MET
   const xAOD::MissingET *met_nonint = (*met)["NonInt"];

   float EtmissTruth_Etx = met_nonint->mpx();
   float EtmissTruth_Ety = met_nonint->mpy();
   float EtmissTruth_Et  = sqrt(EtmissTruth_Etx * EtmissTruth_Etx + EtmissTruth_Ety * EtmissTruth_Ety);

   TLorentzVector MET_Truth(EtmissTruth_Etx, EtmissTruth_Ety, 0., EtmissTruth_Et);

   m_met_et  = MET_Truth.Perp();
   m_met_phi = MET_Truth.Phi();

   //  Bosons
   int nbos10 = 0;
   if (bosons) {
      for (const auto &bos_itr : *bosons) {
         if (bos_itr->pt() > 10000.) {
            m_boson_e->push_back(bos_itr->e());
            m_boson_m->push_back(bos_itr->m());
            m_boson_pt->push_back(bos_itr->pt());
            m_boson_eta->push_back(bos_itr->eta());
            m_boson_phi->push_back(bos_itr->phi());
            m_boson_pdgid->push_back(bos_itr->pdgId());
            nbos10++;
         }
      }
   }
   m_nbosons = nbos10;

   //  Neutrinos
   int nnu10 = 0;
   for (const auto &nu_itr : *neutrinos) {
      if (nu_itr->pt() > 10000.) {
         m_nu_e->push_back(nu_itr->e());
         m_nu_m->push_back(nu_itr->m());
         m_nu_pt->push_back(nu_itr->pt());
         m_nu_eta->push_back(nu_itr->eta());
         m_nu_phi->push_back(nu_itr->phi());
         m_nu_pdgid->push_back(nu_itr->pdgId());
         m_nu_type->push_back(nu_itr->auxdata<uint>("classifierParticleType"));
         m_nu_origin->push_back(nu_itr->auxdata<uint>("classifierParticleOrigin"));
         nnu10++;
      }
   }
   m_nnus = nnu10;

   // Partons
   for (const auto &truthE_itr : *truthE) {
      if (truthE_itr->isAvailable<float>("X1")) {
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
         if (truthE_itr->auxdata<int>("PDGID1") <= 6 && truthE_itr->auxdata<int>("PDGID2") <= 6)
            m_parton_pp->push_back(0);
         else if ((truthE_itr->auxdata<int>("PDGID1") == 21 && truthE_itr->auxdata<int>("PDGID2") <= 6) ||
                  (truthE_itr->auxdata<int>("PDGID2") == 21 && truthE_itr->auxdata<int>("PDGID1") <= 6))
            m_parton_pp->push_back(1);
         else if (truthE_itr->auxdata<int>("PDGID1") == 21 && truthE_itr->auxdata<int>("PDGID2") == 21)
            m_parton_pp->push_back(2);
         else {
            m_parton_pp->push_back(3);
            ANA_MSG_INFO("What the hell is this one " << truthE_itr->auxdata<int>("PDGID1") << " AND "
                                                      << truthE_itr->auxdata<int>("PDGID2"));
         }

      } else { // Somehow it does not exist
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
   //-----------------------------------------------------------------------
   //  Fill Tree
   //-----------------------------------------------------------------------
   truthTree->Fill();

   //-----------------------------------------------------------------------
   //  Tests
   //-----------------------------------------------------------------------

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::postExecute()
{
   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::finalize()
{

   ANA_MSG_INFO("All done. Now cleaning up...");

   if (my_XsecDB) {
      delete my_XsecDB;
      my_XsecDB = 0;
   }

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInvTruth ::histFinalize()
{

   return EL::StatusCode::SUCCESS;
}
