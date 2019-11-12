// ROOT include(s):
#include <TSystem.h>
#include <TFile.h>

// EDM include(s):
#include <xAODEventInfo/EventInfo.h>
#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthEvent.h>
#include "xAODTruth/xAODTruthHelpers.h"
#include <xAODBase/IParticleHelpers.h>
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODMetaData/FileMetaData.h"

// Core include(s):
#include "EventLoop/Job.h"
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"
#include "AsgTools/MessageCheck.h"
#include <PATInterfaces/CorrectionCode.h>
#include <EventLoop/Worker.h>
#include <EventLoop/StatusCode.h>
//#include <EventLoop/OutputStream.h>
#include "PathResolver/PathResolver.h"
#include "AsgTools/AsgMetadataTool.h"

// Local include(s):
#include <VBFInvAnalysis/VBFInv.h>
#include <VBFInvAnalysis/HelperFunctions.h>
#include <VBFInvAnalysis/GetTruthBosonP4.h>
#include <VBFInvAnalysis/TruthFilter.h>

// Utils
#include <boost/algorithm/string.hpp>

// Declare the class to ROOT:
ClassImp(VBFInv)

// Unchecked status codes become error:
#ifdef ROOTCORE
#include <xAODRootAccess/tools/TReturnCode.h>
#include <AsgTools/StatusCode.h>
#endif
#include <PATInterfaces/CorrectionCode.h>
#include <PATInterfaces/SystematicCode.h>

   VBFInv ::VBFInv()
   : debug(false), verbose(false), config_file(""), ST_config_file(""), prw_file(""), lumicalc_file(""), GRL_file(""),
     MC_campaign(""), skip_syst(""), trigger_list(""), pt1Skim(0), pt1SkimForSyst(0), pt2Skim(0), pt2SkimForSyst(0),
     metSkim(0), metSkimForSyst(0), mjjSkim(0), mjjSkimForSyst(0), detajjSkim(0), detajjSkimForSyst(0),
     rebalancedJetPt(20000.), doPileup(true), doSystematics(false), doSkim(false), doTrim(false), doTrimSyst(false),
     doRnS(false), doFatJetDetail(false), doTrackJetDetail(false), doElectronDetail(false), doMuonDetail(false),
     doJetDetail(false), doTauDetail(false), doPhotonDetail(false), doMETDetail(false), doEventDetail(false),
     doContLepDetail(false), doVertexDetail(false), doORDetail(false), doTTMet(false), savePVOnly(false), 
     copyEMTopoFJVT(false), JetEtaFilter(5.0), JetpTFilter(20.0e3), MjjFilter(800.0e3),
     PhijjFilter(2.5), getMCChannel(-1), computeXS(false), m_isMC(false), m_isAFII(false), m_eventCounter(0),
     m_determinedDerivation(false), m_isEXOT5(false), m_grl("GoodRunsListSelectionTool/grl", this),
     m_susytools_handle("ST::SUSYObjDef_xAOD/ST", this), m_susytools_Tight_handle("ST::SUSYObjDef_xAOD/STTight", this),
     m_susytools_Tighter_handle("ST::SUSYObjDef_xAOD/STTighter", this),
     m_susytools_Tenacious_handle("ST::SUSYObjDef_xAOD/STTenacious", this),
     m_jetFwdJvtTool("JetForwardJvtTool/JetForwardJvtTool_VBF", this),
     m_elecEfficiencySFTool_anti_id(
        "AsgElectronEfficiencyCorrectionTool/AsgElectronEfficiencyCorrectionTool_VBF_anti_id", this),
     m_elecEfficiencySFTool_anti_iso(
        "AsgElectronEfficiencyCorrectionTool/AsgElectronEfficiencyCorrectionTool_VBF_anti_iso", this)
{
}

EL::StatusCode VBFInv::setupJob(EL::Job &job)
{

   ANA_CHECK_SET_TYPE(EL::StatusCode);
   ANA_MSG_INFO("in setupJob");

   job.useXAOD();

   // Return gracefully:
   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv ::histInitialize()
{

   ANA_MSG_INFO("in histInitialize");

   // Events processed before derivation
   m_NumberEvents = new TH1D("NumberEvents", "Number Events", 30, 0, 30);
   m_NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
   m_NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
   m_NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");
   m_NumberEvents->GetXaxis()->SetBinLabel(4, "RawEXOT5");
   m_NumberEvents->GetXaxis()->SetBinLabel(5, "90400");
   m_NumberEvents->GetXaxis()->SetBinLabel(6, "90401");
   m_NumberEvents->GetXaxis()->SetBinLabel(10, "All");
   m_NumberEvents->GetXaxis()->SetBinLabel(11, "passJetFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(12, "passBFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(13, "passCFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(14, "passLFFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(15, "sherpaptv90");
   m_NumberEvents->GetXaxis()->SetBinLabel(16, "sherpaptv100");
   m_NumberEvents->GetXaxis()->SetBinLabel(17, "sherpaptv500");
   m_NumberEvents->GetXaxis()->SetBinLabel(18, "passQCDFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(19, "passMGJetFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(20, "passMGJetFilterOREleTau");
   m_NumberEvents->GetXaxis()->SetBinLabel(21, "passMGJetFilterOREleTauPtV75");
   m_NumberEvents->GetXaxis()->SetBinLabel(22, "passMGJetFilterOREleTauPtV100");
   m_NumberEvents->GetXaxis()->SetBinLabel(23, "passMGJetFilterPTV100BFilter");
   m_NumberEvents->GetXaxis()->SetBinLabel(24, "passMGJetFilterPTV100");
   m_NumberEvents->GetXaxis()->SetBinLabel(25, "passPhotonOR");
   m_NumberEvents->GetXaxis()->SetBinLabel(26, "passPhotonORIso");

   // CutFlow
   TString NameCut("Nominal");
   m_CutFlow = Analysis::CutFlowTool(NameCut);
   m_CutFlow.addCut("Pre-Derivation");
   m_CutFlow.addCut("Processed");
   m_CutFlow.addCut("GRL");
   m_CutFlow.addCut("Vertex");
   m_CutFlow.addCut("Detector cleaning");
   m_CutFlow.addCut("Jet cleaning");
   m_CutFlow.addCut("MET skim");
   m_CutFlow.addCut("JetN skim");
   m_CutFlow.addCut("Jet pT1 skim");
   m_CutFlow.addCut("Jet pT2 skim");
   m_CutFlow.addCut("Mjj skim");
   m_CutFlow.addCut("DEta skim");

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv ::fileExecute()
{

   ANA_MSG_INFO("in fileExecute");

   m_event = wk()->xaodEvent();
   ANA_MSG_INFO("Number of events in this file = " << m_event->getEntries());

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
      unsigned nbin = 0;
      if (cbk->inputStream() == "StreamAOD") {
         if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_All") { // should always be the right cycle
            nbin = 10;
         } else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_VBFSherpa")
            nbin = 11;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_BFilter")
            nbin = 12;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_CFilter")
            nbin = 13;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_LFFilter")
            nbin = 14;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_SherpaPTV90")
            nbin = 15;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_SherpaPTV100")
            nbin = 16;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_SherpaPTV500")
            nbin = 17;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_JZVBFFilter")
            nbin = 18;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_MGVBFFilter")
            nbin = 19;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_MGVBFORFilter")
            nbin = 20;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_MGZnnNp01ORFilter")
            nbin = 21;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_ZOtherORFilter")
            nbin = 22;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_MGWFilter")
            nbin = 23;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_MGWNoFlavFilter")
            nbin = 24;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_PhotonFilter")
            nbin = 25;
         else if (cbk->name() == "EXOT5SumEvtWeightFilterAlg_PhotonwIsoFilter")
            nbin = 26;
         else if (cbk->name() == "LHE3Weight_PDFset=90400")
            nbin = 5;
         else if (cbk->name() == "LHE3Weight_PDFset=90401")
            nbin = 6;
         if (nbin > 0) {
	   double tmp_sumEVT    = m_NumberEvents->GetBinContent(nbin);
	   double tmp_sumEVTtwo = m_NumberEvents->GetBinError(nbin);
	   m_NumberEvents->SetBinContent(nbin, cbk->sumOfEventWeights()+tmp_sumEVT);// keep a running sum
	   m_NumberEvents->SetBinError(nbin, cbk->sumOfEventWeightsSquared()+tmp_sumEVTtwo); // keep a running sum
         }
      } // end AOD stream
   }

   uint64_t nEventsProcessed    = 0;
   double   sumOfWeights        = 0.;
   double   sumOfWeightsSquared = 0.;
   if (allEventsCBK) {
      nEventsProcessed    = allEventsCBK->nAcceptedEvents();
      sumOfWeights        = allEventsCBK->sumOfEventWeights();
      sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();
      ANA_MSG_INFO("CutBookkeepers Accepted " << nEventsProcessed << " SumWei " << sumOfWeights << " sumWei2 "
                                              << sumOfWeightsSquared);
      m_CutFlow.addContent(0, nEventsProcessed, sumOfWeights);
   } else {
      ANA_MSG_INFO("No relevant CutBookKeepers found");
   }

   if (m_event->getEntries()) {
      m_NumberEvents->Fill(0., nEventsProcessed);
      m_NumberEvents->Fill(1., sumOfWeights);
      m_NumberEvents->Fill(2., sumOfWeightsSquared);
   }
   // events processed by this VBFInv code
   m_NumberEvents->Fill(3., m_event->getEntries());

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::initialize()
{

   ANA_CHECK_SET_TYPE(EL::StatusCode);
   ANA_MSG_INFO(std::fixed << std::setprecision(2));
   ANA_MSG_INFO("in initialize");

   // Read configuration
   ANA_CHECK(readConfig());

   m_isMC   = !wk()->metaData()->castDouble("isData");
   m_isAFII = wk()->metaData()->castDouble("isAFII");

   Int_t showerType = -9999;
   if (m_isMC) showerType = ST::getMCShowerType(wk()->metaData()->castString("sample_name"));

   // print configuration
   ANA_MSG_INFO("called with:");
   ANA_MSG_INFO("  - isMC = " << m_isMC);
   ANA_MSG_INFO("  - isAFII = " << m_isAFII);
   ANA_MSG_INFO("  - doSystematics = " << doSystematics);
   ANA_MSG_INFO("  - VBFInvAnalysis config file = " << config_file);
   ANA_MSG_INFO("  - SUSYTools config file = " << ST_config_file);
   ANA_MSG_INFO("  - prw_file = " << prw_file);
   ANA_MSG_INFO("  - lumicalc_file = " << lumicalc_file);
   ANA_MSG_INFO("  - GRL_file = " << GRL_file);
   ANA_MSG_INFO("  - MC campaign = " << MC_campaign);
   ANA_MSG_INFO("  - doSkim = " << doSkim);
   ANA_MSG_INFO("  - doTrim = " << doTrim);
   ANA_MSG_INFO("  - doTrimSyst = " << doTrimSyst);
   ANA_MSG_INFO("  - pt1Skim = " << pt1Skim << " MeV ( " << pt1SkimForSyst << " MeV for systematics)");
   ANA_MSG_INFO("  - pt2Skim = " << pt1Skim << " MeV ( " << pt2SkimForSyst << " MeV for systematics)");
   ANA_MSG_INFO("  - metSkim = " << metSkim << " MeV ( " << metSkimForSyst << " MeV for systematics)");
   ANA_MSG_INFO("  - mjjSkim = " << mjjSkim << " MeV ( " << mjjSkimForSyst << " MeV for systematics)");
   ANA_MSG_INFO("  - detajjSkim = " << detajjSkim << " ( " << detajjSkimForSyst << " for systematics)");
   ANA_MSG_INFO("  - JetEtaFilter = " << JetEtaFilter);
   ANA_MSG_INFO("  - JetpTFilter = " << JetpTFilter << " MeV ");
   ANA_MSG_INFO("  - MjjFilter = " << MjjFilter << " MeV ");
   ANA_MSG_INFO("  - PhijjFilter = " << PhijjFilter);
   ANA_MSG_INFO("  - skip_syst = " << skip_syst);
   ANA_MSG_INFO("  - trigger_list = " << trigger_list);
   ANA_MSG_INFO("  - showerType = " << showerType);
   ANA_MSG_INFO("  - computeXS = " << computeXS);
   ANA_MSG_INFO("  - debug = " << debug);

   // Event counter
   m_eventCounter = 0;

   if (m_isMC && computeXS) {
      std::string xSecFilePath = "dev/PMGTools/PMGxsecDB_mc15.txt";
      // xSecFilePath             = "";//PathResolverFindCalibFile(xSecFilePath);
      my_XsecDB = new SUSY::CrossSectionDB(xSecFilePath);
   } else {
      my_XsecDB = NULL;
   }

   // GRL
   std::vector<std::string> vecStringGRL;
   if (!m_isMC) {
      if (doTTMet) {
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19_BjetHLT.xml")); // 2016 GRL
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_BjetHLT_Normal2017.xml")); // 2017 GRL
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.xml")); // 2018 GRL
      } else {
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data15_13TeV/20170619/physics_25ns_21.0.19.xml")); // 2015 GRL, R21 (3219.56 pb-1)
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19.xml")); // 2016 GRL, R21 (32988.1 pb-1)
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.xml")); // 2017 GRL, R21 (44307.4 pb-1)
         vecStringGRL.push_back(PathResolverFindCalibFile(
            "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.xml")); // 2018 GRL, R21 (58450.1 pb-1)
      }
      ANA_CHECK(m_grl.setProperty("GoodRunsListVec", vecStringGRL));
      ANA_CHECK(m_grl.setProperty("PassThrough",
                                  false)); // if true (default) will ignore result of GRL and will just pass all events
      ANA_CHECK(m_grl.initialize());
   }

   // configure forward JVT tool
   ANA_CHECK(m_jetFwdJvtTool.setProperty("OutputDec", "passFJvt")); // Output decoration
   ANA_CHECK(m_jetFwdJvtTool.setProperty("UseTightOP", true));      // Tight
   ANA_CHECK(m_jetFwdJvtTool.setProperty("EtaThresh", 2.5));
   ANA_CHECK(m_jetFwdJvtTool.setProperty("ForwardMaxPt", 120.0e3));
   ANA_CHECK(m_jetFwdJvtTool.retrieve());

   // anti-id sf's
   // if (m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH" || m_eleId == "Loose" )
   // std::vector<std::string>
   // inputFiles_ID{"dev/ElectronEfficiencyCorrection/2015_2017/rel21.2/Consolidation_September2018_v1/inefficiency/efficiencySF.offline.LooseAndBLayerLLH_d0z0_v13.root"}
   // ;
   std::vector<std::string> inputFiles_ID{
      "VBFInvAnalysis/electronSF/efficiencySF.offline.LooseAndBLayerLLH_d0z0_v13.root"};
   ANA_CHECK(m_elecEfficiencySFTool_anti_id.setProperty("CorrectionFileNameList", inputFiles_ID));
   ANA_CHECK(m_elecEfficiencySFTool_anti_id.setProperty("CorrelationModel", "TOTAL"));
   ANA_CHECK(m_elecEfficiencySFTool_anti_id.initialize());
   std::vector<std::string> inputFiles_ISO{
      "VBFInvAnalysis/electronSF/efficiencySF.Isolation.LooseAndBLayerLLH_d0z0_v13_FCLoose.root"};
   ANA_CHECK(m_elecEfficiencySFTool_anti_iso.setProperty("CorrectionFileNameList", inputFiles_ISO));
   ANA_CHECK(m_elecEfficiencySFTool_anti_iso.setProperty("CorrelationModel", "TOTAL"));
   ANA_CHECK(m_elecEfficiencySFTool_anti_iso.initialize());

   // SUSYTools
   const ST::ISUSYObjDef_xAODTool::DataSource datasource =
      (!m_isMC ? ST::ISUSYObjDef_xAODTool::Data
               : (m_isAFII ? ST::ISUSYObjDef_xAODTool::AtlfastII : ST::ISUSYObjDef_xAODTool::FullSim));
   ANA_CHECK(m_susytools_handle.setProperty("DataSource", datasource));
   ANA_CHECK(m_susytools_handle.setProperty("ConfigFile", ST_config_file.Data()));
   ANA_CHECK(m_susytools_Tight_handle.setProperty("DataSource", datasource));
   ANA_CHECK(m_susytools_Tight_handle.setProperty("ConfigFile", ST_config_file.Data()));
   ANA_CHECK(m_susytools_Tight_handle.setProperty("METJetSelection", "Tight"));
   // ANA_CHECK(m_susytools_Tenacious_handle.setProperty("FwdJetUseTightOP", true)); // for some reason this and
   // setBoolProperty do not work
   ANA_CHECK(m_susytools_Tenacious_handle.setProperty("DataSource", datasource));
   ANA_CHECK(m_susytools_Tenacious_handle.setProperty("ConfigFile", ST_config_file.Data()));
   ANA_CHECK(m_susytools_Tenacious_handle.setProperty("METJetSelection", "Tenacious"));
   ANA_CHECK(m_susytools_Tighter_handle.setProperty("DataSource", datasource));
   TString newST_config_file = ST_config_file + "tight";
   ANA_CHECK(m_susytools_Tighter_handle.setProperty("ConfigFile", newST_config_file.Data()));
   ANA_CHECK(m_susytools_Tighter_handle.setProperty("METJetSelection", "Tighter"));
   ANA_CHECK(m_susytools_Tighter_handle.setProperty("JetInputType", 1));//emtopo

   if (verbose) {
      ANA_CHECK(m_susytools_handle.setProperty("outLevel", MSG::VERBOSE));
      ANA_CHECK(m_susytools_handle.setProperty("DebugMode", true));
   }
   // Pile up configuration
   xAOD::TEvent *event = wk()->xaodEvent();
   if (m_isMC) {
      // get metadata
      const xAOD::FileMetaData *fmd    = nullptr;
      std::string               amiTag = "unknown";
      if (event->retrieveMetaInput(fmd, "FileMetaData").isSuccess()) {
         if (!fmd->value(xAOD::FileMetaData::amiTag, amiTag)) {
            ATH_MSG_ERROR("The object is available, the variable is not.");
         }
      } else {
         ATH_MSG_ERROR("The object is not available.");
      }
      ANA_MSG_INFO("\n AMI Tag: " << amiTag);

      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));
      std::vector<std::string> prw_conf;
      std::vector<std::string> prw_lumicalc;
      std::string              mc_campaign;
      std::string              simType = (m_isAFII ? "AFII" : "FS");
      uint32_t                 runNum  = eventInfo->runNumber();
      if (doTTMet) {
         switch (runNum) {
         case 284500:
            mc_campaign = "mc16a";
            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data16_13TeV/20180129/"
               "PHYS_StandardGRL_All_Good_25ns_BjetHLT_297730-311481_OflLumi-13TeV-009.root")); // 2016 LumiCalc
            break;
         case 300000:
            if (amiTag.find("r10201") != std::string::npos)
               mc_campaign = "mc16d";
            else if (amiTag.find("r9781") != std::string::npos)
               mc_campaign = "mc16c";
            else
               mc_campaign = "mc16d";
            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data17_13TeV/20180619/"
               "physics_25ns_BjetHLT_Normal2017.lumicalc.OflLumi-13TeV-010.root")); // 2017 LumiCalc
            prw_conf.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data17_13TeV/20180619/"
               "physics_25ns_BjetHLT_Normal2017.actualMu.OflLumi-13TeV-010.root")); // 2017 ActualMu
            break;
         case 310000:
            if (amiTag.find("r10724") != std::string::npos) // did not seem to find the right info in mc16e
               mc_campaign = "mc16e";
            else {
               mc_campaign = "mc16f";
            }

            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data18_13TeV/20190318/"
               "ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root")); // 2018 LumiCalc
            prw_conf.push_back(
               PathResolverFindCalibFile(
                  "GoodRunsLists/data18_13TeV/20190318/"
                  "physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root")); // 2018 ActualMu
            break;
         }
      } else {
         switch (runNum) {
         case 284500:
            mc_campaign = "mc16a";
            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data15_13TeV/20170619/"
               "PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root")); // 2015 LumiCalc
            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data16_13TeV/20180129/"
               "PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root")); // 2016 LumiCalc
            break;
         case 300000:
            if (amiTag.find("r10201") != std::string::npos)
               mc_campaign = "mc16d";
            else if (amiTag.find("r9781") != std::string::npos)
               mc_campaign = "mc16c";
            else
               mc_campaign = "mc16d";
            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data17_13TeV/20180619/"
               "physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root")); // 2017 LumiCalc
            prw_conf.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data17_13TeV/20180619/"
               "physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root")); // 2017 ActualMu
            break;
         case 310000:
            if (amiTag.find("r10724") != std::string::npos) // did not seem to find the right info in mc16e
               mc_campaign = "mc16e";
            else {
               mc_campaign = "mc16f";
            }

            prw_lumicalc.push_back(PathResolverFindCalibFile(
               "GoodRunsLists/data18_13TeV/20181111/"
               "ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-001.root")); // 2018 LumiCalc
            prw_conf.push_back(
               PathResolverFindCalibFile("GoodRunsLists/data18_13TeV/20181111/purw.actualMu.root")); // 2018 ActualMu
            break;
         }
      }
      MC_campaign = mc_campaign;
      unsigned mcchannel = runNum;
      if (m_isMC) mcchannel = eventInfo->mcChannelNumber();
      if (getMCChannel > 0) mcchannel = unsigned(getMCChannel);
      std::string prwConfigFile = "dev/SUSYTools/PRW_AUTOCONFIG_SIM/files/pileup_" + mc_campaign + "_dsid" +
                                  std::to_string(mcchannel) + "_" + simType + ".root";
      //"DSID" + std::to_string(DSID_INT/1000) + "xxx/pileup_" + mcCampaignMD + "_dsid" + std::to_string(DSID_INT) + "_"
      //+ simType + ".root"; prwConfigFile = "dev/PileupReweighting/mc16_13TeV/pileup_" + mc_campaign + "_dsid" +
      // std::to_string(eventInfo->mcChannelNumber()) + "_" + simType + ".root";
      prwConfigFile = "dev/PileupReweighting/share/DSID" + std::to_string(mcchannel / 1000) + "xxx/pileup_" +
                      mc_campaign + "_dsid" + std::to_string(mcchannel) + "_" + simType + ".root";
      if((mcchannel==312523 || mcchannel==312454) && mc_campaign=="mc16e")
	prwConfigFile = "data/VBFInvAnalysis/prwfiles/pileup_" + mc_campaign + "_dsid" + std::to_string(mcchannel) + "_" + simType + ".root";
	
      std::cout << "input:  " << prwConfigFile << " " << PathResolverFindCalibFile(prwConfigFile) << std::endl;
      prwConfigFile = PathResolverFindCalibFile(prwConfigFile);
      if (prwConfigFile.empty()) {
         std::vector<std::string> prw_conf_tmp = getTokens(prw_file, ",");
         std::transform(prw_conf_tmp.begin(), prw_conf_tmp.end(), prw_conf_tmp.begin(),
                        [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
         ANA_MSG_INFO("File " << prwConfigFile
                              << " not found in ST database. Set manual prw configuration with the following files:");
         for (uint i = 0; i < prw_conf_tmp.size(); i++) {
            ANA_MSG_INFO(prw_conf_tmp.at(i));
            if (prw_conf_tmp.at(i).find(mc_campaign) != std::string::npos) prw_conf.push_back(prw_conf_tmp.at(i));
         }
         std::transform(prw_conf.begin(), prw_conf.end(), prw_conf.begin(),
                        [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
      } else {
         prw_conf.push_back(prwConfigFile);
         std::transform(prw_conf.begin(), prw_conf.end(), prw_conf.begin(),
                        [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
      }

      ANA_CHECK(m_susytools_handle.setProperty("PRWConfigFiles", prw_conf));
      ANA_CHECK(m_susytools_handle.setProperty("PRWLumiCalcFiles", prw_lumicalc));
      ANA_CHECK(m_susytools_handle.setProperty("mcCampaign", mc_campaign));
      // ANA_CHECK(m_susytools_handle.setProperty("UsePeriodConfig", "MC16"));
   }

   // Guess shower type for btagging MC/MC SFs
   if (m_isMC) ANA_CHECK(m_susytools_handle.setProperty("ShowerType", (Int_t)showerType));
   // Init susytools
   ANA_CHECK(m_susytools_handle.initialize());
   ANA_CHECK(m_susytools_Tight_handle.initialize());
   ANA_CHECK(m_susytools_Tighter_handle.initialize());
   ANA_CHECK(m_susytools_Tenacious_handle.initialize());

   ASG_SET_ANA_TOOL_TYPE(m_DNNTop80, JSSWTopTaggerDNN);
   m_DNNTop80.setName("DNNTopTagger80");
   ANA_CHECK(m_DNNTop80.setProperty(
      "ConfigFile",
      "JSSWTopTaggerDNN/JSSDNNTagger_AntiKt10LCTopoTrimmed_TopQuarkContained_MC15c_20170824_BOOSTSetup80Eff.dat"));
   ANA_CHECK(m_DNNTop80.retrieve());

   ASG_SET_ANA_TOOL_TYPE(m_DNNW50, JSSWTopTaggerDNN);
   m_DNNW50.setName("DNNWTagger50");
   ANA_CHECK(m_DNNW50.setProperty(
      "ConfigFile",
      "JSSWTopTaggerDNN/JSSDNNTagger_AntiKt10LCTopoTrimmed_WBosonContained_MC15c_20170824_BOOSTSetup50Eff.dat"));
   ANA_CHECK(m_DNNW50.retrieve());

   //
   // list of systematics to process
   //
   m_sysList.clear();
   m_sysWeightList.clear();
   if (doSystematics && m_isMC) {
      auto fullSystList = m_susytools_handle->getSystInfoList();

      // list of uncertainties to be skipped
      std::vector<std::string> skip = getTokens(skip_syst, ",");

      for (const auto &syst : fullSystList) {
         const TString thisSyst = syst.systset.name();
         Bool_t        keepThis(kTRUE);
         for (const auto &toSkip : skip) {
            if (thisSyst.Contains(toSkip)) {
               ANA_MSG_INFO("Skipping the systematic variation \"" << thisSyst << "\"");
               keepThis = kFALSE;
               break;
            }
         } // loop over skip systematics

         if (keepThis) {
            ANA_MSG_INFO("********** Processing systematic variation: \"" << thisSyst << "\"");
            if (syst.affectsWeights) {
               ANA_MSG_INFO("running weightsyst: " << thisSyst);
               m_sysWeightList.push_back(syst);
            } else {
               m_sysList.push_back(syst);
            }
         }
      }
   } else {

      ANA_MSG_INFO("********** Processing only nominal: \"\"");
      ST::SystInfo nominal;
      nominal.affectsKinematics = false;
      nominal.affectsWeights    = false;
      nominal.affectsType       = ST::Unknown;
      m_sysList.push_back(nominal);
   }

   //
   // define the output TTree
   //
   m_tree.clear();

   //
   // TTree
   //
   ANA_MSG_INFO("Tree output name is " << outputName);
   ANA_MSG_INFO("Histogram output name is " << outputNameHist);
   TFile *outputFile     = wk()->getOutputFile(outputName);
   TFile *outputFileHist = wk()->getOutputFile(outputNameHist);

   //
   // Histograms
   //
   if (wk()->tree()->GetEntries()) {
      m_NumberEventsinNtuple = m_NumberEvents;
      m_NumberEvents->SetDirectory(outputFileHist);
      m_NumberEventsinNtuple->SetDirectory(outputFile);
   }

   // if RnS, then add extra detail
   if (doRnS) doORDetail = true;

   // setting up some the details to off for systematics
   bool tmp_doEventDetail    = doEventDetail;
   bool tmp_doTauDetail      = doTauDetail;
   bool tmp_doPhotonDetail   = doPhotonDetail;
   bool tmp_doElectronDetail = doElectronDetail;
   bool tmp_doMuonDetail     = doMuonDetail;
   bool tmp_doJetDetail      = doJetDetail;

   for (const auto &syst : m_sysList) {
      const TString thisSyst  = syst.systset.name();
      const TString treeName  = (thisSyst == "") ? "MiniNtuple" : ("MiniNtuple_" + thisSyst).ReplaceAll(" ", "_");
      const TString treeTitle = thisSyst;
      ANA_MSG_INFO("Will consider the systematic variation \"" << thisSyst << "\"");

      // skipping the weight syst
      // if(syst.affectsWeights){
      //	ANA_MSG_INFO("Weight systematic variation...saving only the weight \"" << thisSyst << "\"");
      //	continue;
      //}
      const Bool_t isNominal = (thisSyst == "");
      const Bool_t trim = (!isNominal || doTrim || doElectronDetail || doMuonDetail || doJetDetail || doMETDetail ||
                           doEventDetail || doRnS || doContLepDetail);

      // turn off detail for the systematics
      if (doTrimSyst) {
         if (isNominal) { // no change
            doEventDetail    = tmp_doEventDetail;
            doTauDetail      = tmp_doTauDetail;
            doPhotonDetail   = tmp_doPhotonDetail;
            doElectronDetail = tmp_doElectronDetail;
            doMuonDetail     = tmp_doMuonDetail;
            doJetDetail      = tmp_doJetDetail;
         } else {
            doEventDetail    = false;
            doTauDetail      = false;
            doPhotonDetail   = false;
            doElectronDetail = false;
            doMuonDetail     = false;
            doJetDetail      = false;
         }
      }

      ANA_MSG_INFO("Creating TTree named " << treeName.Data() << " for systematic named \"" << thisSyst.Data() << "\"");

      m_tree[thisSyst] = new TTree(treeName, treeTitle);
      m_tree[thisSyst]->SetDirectory(outputFile);
      m_tree[thisSyst]->SetAutoFlush(-250000); // lower to 0.5MB before writing

      // save, in all trees, the trigger decision
      std::vector<std::string> triggersToSave = getTokens(trigger_list, ",");
      for (uint i = 0; i < triggersToSave.size(); i++) boost::algorithm::trim(triggersToSave.at(i));
      if (debug)
         for (uint i = 0; i < triggersToSave.size(); i++)
            ANA_MSG_INFO("Save trigger decision of: " << triggersToSave.at(i));
      m_cand[thisSyst].evt.setTriggers(triggersToSave);

      // define all elements of the output tree
      m_cand[thisSyst].met["met_tst"]       = Analysis::outMET("met_tst", (trim && !doMETDetail));
      m_cand[thisSyst].met["met_tst_nolep"] = Analysis::outMET("met_tst_nolep", (trim && !doMETDetail));
      m_cand[thisSyst].met["met_soft_tst"]  = Analysis::outMET("met_soft_tst", (trim && !doMETDetail));

      if (doMETDetail) {
         m_cand[thisSyst].met["met_tight_tst"]       = Analysis::outMET("met_tight_tst", (trim && !doMETDetail));
         m_cand[thisSyst].met["met_tight_tst_nolep"] = Analysis::outMET("met_tight_tst_nolep", (trim && !doMETDetail));
         m_cand[thisSyst].met["met_tenacious_tst"]   = Analysis::outMET("met_tenacious_tst", (trim && !doMETDetail));
         m_cand[thisSyst].met["met_tenacious_tst_nolep"] =
            Analysis::outMET("met_tenacious_tst_nolep", (trim && !doMETDetail));

         // Only add detail if needed
         if (doORDetail) {
            m_cand[thisSyst].met["met_jet_tst"]        = Analysis::outMET("met_jet_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_ele_tst"]        = Analysis::outMET("met_ele_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_mu_tst"]         = Analysis::outMET("met_mu_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_jet_tight_tst"]  = Analysis::outMET("met_jet_tight_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_ele_tight_tst"]  = Analysis::outMET("met_ele_tight_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_mu_tight_tst"]   = Analysis::outMET("met_mu_tight_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_soft_tight_tst"] = Analysis::outMET("met_soft_tight_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_jet_tenacious_tst"] =
               Analysis::outMET("met_jet_tenacious_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_ele_tenacious_tst"] =
               Analysis::outMET("met_ele_tenacious_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_mu_tenacious_tst"] =
               Analysis::outMET("met_mu_tenacious_tst", (trim && !doMETDetail));
            m_cand[thisSyst].met["met_soft_tenacious_tst"] =
               Analysis::outMET("met_soft_tenacious_tst", (trim && !doMETDetail));
         }
      }

      //m_cand[thisSyst].met["met_track"] = Analysis::outMET("met_track", (trim && !doMETDetail));
      m_cand[thisSyst].met["met_truth"] = Analysis::outMET("met_truth", (trim && !doMETDetail));
      m_cand[thisSyst].mu["mu"]         = Analysis::outMuon("mu", (trim && !doMuonDetail));
      if (doMuonDetail) m_cand[thisSyst].mu["basemu"] = Analysis::outMuon("basemu", (trim && !doMuonDetail));
      if (doContLepDetail) m_cand[thisSyst].mu["contmu"] = Analysis::outMuon("contmu", (trim && !doContLepDetail));
      m_cand[thisSyst].el["el"] = Analysis::outElectron("el", (trim && !doElectronDetail));
      if (doElectronDetail)
         m_cand[thisSyst].el["baseel"] = Analysis::outElectron("baseel", (trim && !doElectronDetail));
      if (doContLepDetail) m_cand[thisSyst].el["contel"] = Analysis::outElectron("contel", (trim && !doContLepDetail));
      m_cand[thisSyst].jet["jet"] = Analysis::outJet("jet", (trim && !doJetDetail && !doRnS));
      m_cand[thisSyst].jet["jet"].setOutPV(savePVOnly);
      if (doFatJetDetail) m_cand[thisSyst].fatjet["fatjet"] = Analysis::outFatJet("fatjet", (trim && !doFatJetDetail));
      if (doTrackJetDetail)
         m_cand[thisSyst].trackjet["trackjet"] = Analysis::outTrackJet("trackjet", (trim && !doTrackJetDetail));
      if (doTauDetail) m_cand[thisSyst].tau["tau"] = Analysis::outTau("tau", trim && !doTauDetail);
      m_cand[thisSyst].ph["ph"] = Analysis::outPhoton("ph", trim && !doPhotonDetail);

      // Set trimming option for remaning outHolder objects
      m_cand[thisSyst].evt.setDoTrim((trim && !doEventDetail && !doRnS));
      m_cand[thisSyst].evt.setDoExtraTrim(doTrimSyst);
      m_cand[thisSyst].evt.setDoVertexDetail(doVertexDetail);
      m_cand[thisSyst].evt.setComputeXS(computeXS);
      m_cand[thisSyst].evt.setIsMC(m_isMC);
      m_cand[thisSyst].evt.setDoORDetail(doORDetail);
      m_cand[thisSyst].rns.setDoTrim((trim && !doRnS));
      m_cand[thisSyst].attachToTree(m_tree[thisSyst]);
   }
   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::execute()
{
   ANA_CHECK_SET_TYPE(EL::StatusCode);

   // Fail if unchecked status code
   // StatusCode::enableFailure();
   // CP::SystematicCode::enableFailure();
   // CP::CorrectionCode::enableFailure();

   if (debug) {
      ANA_MSG_INFO("====================================================================");
      ANA_MSG_INFO("in execute, on entry " << wk()->treeEntry());
   }

   bool passExp = false;
   for (int i = 0; i < 9; i++) {
      int exponent = pow(10, i);
      passExp |= (m_eventCounter <= exponent && (m_eventCounter % exponent) == 0);
   }
   if (passExp) ANA_MSG_INFO("Event number = " << m_eventCounter);
   m_eventCounter++;

   // loop on systematic variations
   Bool_t isFirstIteration = kTRUE; // first iteration is nominal (strictly needed!)
   // setting up some the details to off for systematics
   bool tmp_doEventDetail    = doEventDetail;
   bool tmp_doTauDetail      = doTauDetail;
   bool tmp_doPhotonDetail   = doPhotonDetail;
   bool tmp_doElectronDetail = doElectronDetail;
   bool tmp_doMuonDetail     = doMuonDetail;
   bool tmp_doJetDetail      = doJetDetail;

   for (const auto &systInfo : m_sysList) {
      const TString systName = systInfo.systset.name();

      // turn off detail for the systematics
      if (doTrimSyst) {
         if (systName == "") { // no change for nominal
            doEventDetail    = tmp_doEventDetail;
            doTauDetail      = tmp_doTauDetail;
            doPhotonDetail   = tmp_doPhotonDetail;
            doElectronDetail = tmp_doElectronDetail;
            doMuonDetail     = tmp_doMuonDetail;
            doJetDetail      = tmp_doJetDetail;
         } else {
            doEventDetail    = false;
            doTauDetail      = false;
            doPhotonDetail   = false;
            doElectronDetail = false;
            doMuonDetail     = false;
            doJetDetail      = false;
         }
      }

      if (isFirstIteration) { // || sysInfo.affectsWeights) {
         m_content_nominal.isNominal = kTRUE;
      } else {
         m_content_current           = m_content_nominal; // copy from nominal
         m_content_current.isNominal = kFALSE;
      }
      Analysis::ContentHolder &content = (isFirstIteration) ? m_content_nominal : m_content_current;
      analyzeEvent(content, systInfo, m_cand[systName]);
      isFirstIteration = kFALSE;
   }

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::finalize()
{

   ANA_CHECK_SET_TYPE(EL::StatusCode);
   ANA_MSG_INFO("in finalize");

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv ::histFinalize()
{

   m_CutFlow.printUnw();
   m_CutFlow.print();

   // Cutflow
   m_cflow_hist     = m_CutFlow.createTH1F();
   m_cflow_hist_unw = m_CutFlow.createTH1Fraw();

   TFile *outputFile = wk()->getOutputFile(outputName);
   m_cflow_hist->SetDirectory(outputFile);
   m_cflow_hist_unw->SetDirectory(outputFile);

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv ::readConfig()
{
   const std::string readFrom = PathResolverFindCalibFile(config_file.Data());
   ANA_MSG_INFO("Reading from file \"" << readFrom << "\"");
   TEnv env;
   if (env.ReadFile(readFrom.c_str(), kEnvAll) != 0) {
      ANA_MSG_ERROR("Unable to read configuration file from PathResolverFindCalibFile of input \"" << config_file
                                                                                                   << "\"");
      return EL::StatusCode::FAILURE;
   }

   ST_config_file    = env.GetValue("VBF.ST_config_file", "EMPTY");
   prw_file          = env.GetValue("VBF.prw_file", "EMPTY");
   lumicalc_file     = env.GetValue("VBF.lumicalc_file", "EMPTY");
   GRL_file          = env.GetValue("VBF.GRL_file", "EMPTY");
   MC_campaign       = env.GetValue("VBF.MC_campaign", "EMPTY");
   skip_syst         = env.GetValue("VBF.skip_syst", "");
   trigger_list      = env.GetValue("VBF.trigger_list", "");
   pt1Skim           = env.GetValue("VBF.pt1Skim", 0);
   pt2Skim           = env.GetValue("VBF.pt2Skim", 0);
   metSkim           = env.GetValue("VBF.metSkim", 0);
   mjjSkim           = env.GetValue("VBF.mjjSkim", 0);
   detajjSkim        = env.GetValue("VBF.detajjSkim", 0);
   pt1SkimForSyst    = env.GetValue("VBF.pt1SkimForSyst", 0);
   pt2SkimForSyst    = env.GetValue("VBF.pt2SkimForSyst", 0);
   metSkimForSyst    = env.GetValue("VBF.metSkimForSyst", 0);
   mjjSkimForSyst    = env.GetValue("VBF.mjjSkimForSyst", 0);
   detajjSkimForSyst = env.GetValue("VBF.detajjSkimForSyst", 0);

   return EL::StatusCode::SUCCESS;
}

std::vector<std::string> VBFInv ::getTokens(TString line, TString delim)
{
   std::vector<std::string> vtokens;
   TObjArray *              tokens = TString(line).Tokenize(delim); // delimiters
   if (tokens->GetEntriesFast()) {
      TIter       iString(tokens);
      TObjString *os = 0;
      while ((os = (TObjString *)iString())) {
         vtokens.push_back(os->GetString().Data());
      }
   }
   delete tokens;

   return vtokens;
}

EL::StatusCode VBFInv ::analyzeEvent(Analysis::ContentHolder &content, const ST::SystInfo &systInfo,
                                     Analysis::outHolder &cand)
{
   xAOD::TEvent *event = wk()->xaodEvent();

   // Initialize SUSYTools
   ANA_CHECK(m_susytools_handle->ApplyPRWTool());

   // event selection using a given systematic variation
   const Bool_t syst_affectsElectrons = ST::testAffectsObject(xAOD::Type::Electron, systInfo.affectsType);
   const Bool_t syst_affectsMuons     = ST::testAffectsObject(xAOD::Type::Muon, systInfo.affectsType);
   const Bool_t syst_affectsJets      = ST::testAffectsObject(xAOD::Type::Jet, systInfo.affectsType);
   const Bool_t syst_affectsFatJets   = ST::testAffectsObject(xAOD::Type::Jet, systInfo.affectsType);
   const Bool_t syst_affectsTrackJets = ST::testAffectsObject(xAOD::Type::Jet, systInfo.affectsType);
   const Bool_t syst_affectsPhotons   = ST::testAffectsObject(xAOD::Type::Photon, systInfo.affectsType);
   const Bool_t syst_affectsTaus      = ST::testAffectsObject(xAOD::Type::Tau, systInfo.affectsType);

   // apply systematic uncertainty
   const CP::SystematicSet &sys = systInfo.systset;
   if (m_susytools_handle->applySystematicVariation(sys) != CP::SystematicCode::Ok) {
      ANA_MSG_ERROR("Cannot configure SUSYTools for systematic var. " << sys.name().c_str());
      return EL::StatusCode::FAILURE;
   }
   if (m_susytools_Tighter_handle->applySystematicVariation(sys) != CP::SystematicCode::Ok) {
      ANA_MSG_ERROR("Cannot configure SUSYTools alternate for systematic var. " << sys.name().c_str());
      return EL::StatusCode::FAILURE;
   }

   // determine which objects should be retrieved
   content.doElectrons      = (syst_affectsElectrons || content.isNominal);
   content.doMuons          = (syst_affectsMuons || content.isNominal);
   content.doJets           = (syst_affectsJets || content.isNominal);
   content.doFatJets        = (syst_affectsFatJets || content.isNominal) && doFatJetDetail;
   content.doTrackJets      = (syst_affectsTrackJets || content.isNominal) && doTrackJetDetail;
   content.doPhotons        = (syst_affectsPhotons || content.isNominal); // needed for overlap removal
   content.doTaus           = (syst_affectsTaus || content.isNominal);
   content.doMET            = kTRUE;
   content.doOverlapRemoval = (content.doElectrons || content.doMuons || content.doJets || content.doPhotons ||
                               content.doMET || content.doTaus);

   // define new decorators
   static SG::AuxElement::Decorator<float> dec_new_d0("new_d0");
   static SG::AuxElement::Decorator<float> dec_new_d0sig("new_d0sig");
   static SG::AuxElement::Decorator<float> dec_new_z0("new_z0");
   static SG::AuxElement::Decorator<float> dec_new_z0sig("new_z0sig");
   static SG::AuxElement::Decorator<float> dec_lep_tot_SF("lep_tot_SF");

   // retrieve the eventInfo object from the event store
   content.eventInfo = nullptr;
   ANA_CHECK(evtStore()->retrieve(content.eventInfo, "EventInfo"));

   // sanity check
   if (m_isMC != content.eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
      ANA_MSG_FATAL("Mismatch between isMC (" << m_isMC << ") and xAOD::EventInfo::IS_SIMULATION ("
                                              << xAOD::EventInfo::IS_SIMULATION << ")");
      return EL::StatusCode::FAILURE;
   }
   int runNbr = content.eventInfo->runNumber();
   int evtNbr = content.eventInfo->eventNumber();
   if (debug) ANA_MSG_INFO("in execute, runNumber = " << runNbr << ", eventNumber = " << evtNbr);

   float event_weight = 1.;
   if (m_isMC) {
      event_weight *= content.eventInfo->mcEventWeight();
      if (doPileup) event_weight *= m_susytools_handle->GetPileupWeight();
   }

   //-----------------------------------------------------------------------
   // Event cleaning
   //-----------------------------------------------------------------------

   // no cuts
   m_CutFlow.hasPassed(VBFInvCuts::Processed, event_weight);

   // grl
   Bool_t passesGRL = (m_isMC || m_grl->passRunLB(*content.eventInfo));
   if (!m_isMC && !passesGRL && doSkim) {
      if (debug) {
         ANA_MSG_INFO("drop event: GRL");
      }
      return EL::StatusCode::SUCCESS; // go to next event
   }
   m_CutFlow.hasPassed(VBFInvCuts::GRL, event_weight);
   content.passGRL = passesGRL;

   // primary vertex
   Bool_t       passesVertex(kFALSE);
   const UInt_t nMinTracks(2);
   content.vertices = nullptr;
   if (!event->retrieve(content.vertices, "PrimaryVertices").isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve PrimaryVertices container");
      return EL::StatusCode::FAILURE;
   }
   for (auto thisVertex : *content.vertices) {
      if ((UInt_t)(thisVertex->nTrackParticles()) >= nMinTracks) {
         passesVertex = kTRUE;
         break;
      }
   }
   if (!passesVertex && doSkim) return EL::StatusCode::SUCCESS;
   m_CutFlow.hasPassed(VBFInvCuts::Vertex, event_weight);
   content.passPV = passesVertex;

   // detector cleaning
   // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataPreparationCheckListForPhysicsAnalysis
   Bool_t passesDetErr =
      (m_isMC || !((content.eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error) ||
                   (content.eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error) ||
                   (content.eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error) ||
                   (content.eventInfo->eventFlags(xAOD::EventInfo::Core) & 0x40000)));
   if (!m_isMC && !passesDetErr && doSkim) {
      return EL::StatusCode::SUCCESS; // go to next event
   }
   m_CutFlow.hasPassed(VBFInvCuts::DetError, event_weight);
   content.passDetErr = passesDetErr;

   //-----------------------------------------------------------------------
   // Retrieve baseline objects
   //-----------------------------------------------------------------------

   // Accessors needed for the object selection
   static SG::AuxElement::Accessor<char> acc_baseline("baseline");
   static SG::AuxElement::Accessor<char> acc_signal("signal");
   static SG::AuxElement::Accessor<char> acc_signal_less_JVT("signal_less_JVT");
   static SG::AuxElement::Accessor<char> acc_passOR("passOR");
   static SG::AuxElement::Accessor<char> acc_passJvt("passJvt");
   static SG::AuxElement::Accessor<char> acc_passFJvt("passFJvt");
   static SG::AuxElement::Accessor<char> acc_bad("bad");
   static SG::AuxElement::Accessor<char> acc_cosmic("cosmic");
   const static SG::AuxElement::ConstAccessor<char> acc_isol("isol");
   const static SG::AuxElement::ConstAccessor<char> acc_isolHighPt("isolHighPt"); // use different WPs for low-pt and high-pt. split at 200 GeV.
   const static SG::AuxElement::ConstAccessor<float> acc_d0sig("d0sig");
   const static SG::AuxElement::ConstAccessor<float> acc_z0sinTheta("z0sinTheta");

   //-- JETS --
   if (content.doJets) {
      content.jets    = nullptr;
      content.jetsAux = nullptr;
      content.allJets.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetJets(content.jets, content.jetsAux, kTRUE));
      m_jetFwdJvtTool->modify(*content.jets); // add fjvt
      for (auto jet : *content.jets) {
         if (acc_baseline(*jet) == 1) {
            content.allJets.push_back(jet);
         }
      }
      content.allJets.sort(&HelperFunctions::comparePt); // sort by Pt

      // read in emtopo jets to get the fjvt
      if(copyEMTopoFJVT){
	static SG::AuxElement::ConstAccessor<float> acc_fjvt("fJvt");
	static SG::AuxElement::Decorator<float> dec_fjvt("fJvt");
	static SG::AuxElement::Accessor<char> acc_jetCleanTight("DFCommonJets_jetClean_TightBad");
	static SG::AuxElement::Decorator<char> dec_jetCleanTight("DFCommonJets_jetClean_TightBad");
	content.jetsEM    = nullptr;
	content.jetsEMAux = nullptr;
	content.allEMJets.clear(SG::VIEW_ELEMENTS);
	ANA_CHECK(m_susytools_Tighter_handle->GetJets(content.jetsEM, content.jetsEMAux, true, "AntiKt4EMTopoJets"));
	m_jetFwdJvtTool->modify(*content.jetsEM); // add fjvt
	for (auto jet : content.allJets) {
	  // set the defaults
	  dec_fjvt(*jet) = 0.0;
	  dec_jetCleanTight(*jet) = 1;
	  float minDR=999.0;
	  dec_fjvt(*jet)=0.0;
	  for (auto jetEM : *content.jetsEM) {
	    // if close then copy the fjvt score & tight jet cleaning
	    float dr=jet->p4().DeltaR(jetEM->p4());
	    if(dr<minDR && dr<0.6){ minDR=dr;  dec_fjvt(*jet) = acc_fjvt(*jetEM); dec_jetCleanTight(*jet) = acc_jetCleanTight(*jetEM);  }
	  }
	}
      }// end copy fjvt score
   }                                                     // done with jets

   //-- FatJETS --
   if (content.doFatJets) {
      content.fatjets    = nullptr;
      content.fatjetsAux = nullptr;
      content.allFatJets.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetFatJets(content.fatjets, content.fatjetsAux, kTRUE));
      for (auto fatjet : *content.fatjets) {
         if (fatjet->pt() > 200e3 && fabs(fatjet->eta()) < 2.0 && fatjet->m() > 40e3 &&
             (fatjet->m() / fatjet->pt()) < 1.)
            content.allFatJets.push_back(fatjet);
      }
      content.allFatJets.sort(&HelperFunctions::comparePt); // sort by Pt
   }                                                        // done with fatjets

   //-- TrackJETS --
   if (content.doTrackJets) {
      content.trackjets    = nullptr;
      content.trackjetsAux = nullptr;
      content.allTrackJets.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetTrackJets(content.trackjets, content.trackjetsAux, kTRUE));
      for (auto trackjet : *content.trackjets) {
         if (acc_baseline(*trackjet) == 1) {
            content.allTrackJets.push_back(trackjet);
         }
      }
      content.allTrackJets.sort(&HelperFunctions::comparePt); // sort by Pt
   }                                                          // done with trackjets

   //-- MUONS --
   if (content.doMuons) {
      content.muons    = nullptr;
      content.muonsAux = nullptr;
      content.allMuons.clear(SG::VIEW_ELEMENTS);
      content.contMuons.clear(SG::VIEW_ELEMENTS);

      ANA_CHECK(m_susytools_handle->GetMuons(content.muons, content.muonsAux, kTRUE));
      for (auto mu : *content.muons) {
         if (acc_baseline(*mu) == 1 && acc_cosmic(*mu) == 0 && acc_bad(*mu) == 0) {
            // calculate d0 and z0
            const xAOD::Vertex *pv               = m_susytools_handle->GetPrimVtx();
            const Float_t       primary_vertex_z = pv ? pv->z() : 0;
            dec_new_d0(*mu)                      = HelperFunctions::getD0(mu);
            dec_new_d0sig(*mu)                   = HelperFunctions::getD0sig(mu, content.eventInfo);
            dec_new_z0(*mu)                      = HelperFunctions::getZ0(mu, primary_vertex_z);
            dec_new_z0sig(*mu)                   = HelperFunctions::getZ0sig(mu);
            if (debug)
               ANA_MSG_INFO("d0  >>  " << HelperFunctions::getD0sig(mu, content.eventInfo) << ", z0 >> "
                                       << HelperFunctions::getZ0(mu, primary_vertex_z));
            content.allMuons.push_back(mu);
         }
         if (acc_cosmic(*mu) == 0 && acc_bad(*mu) == 0) {
            content.contMuons.push_back(mu);
         }
      }
      content.allMuons.sort(&HelperFunctions::comparePt);
      content.contMuons.sort(&HelperFunctions::comparePt);
   } // done with muons

   //-- ELECTRONS --
   if (content.doElectrons) {
      content.electrons    = nullptr;
      content.electronsAux = nullptr;
      content.allElectrons.clear(SG::VIEW_ELEMENTS);
      content.contElectrons.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetElectrons(content.electrons, content.electronsAux, kTRUE));

      for (auto el : *content.electrons) {
         if (acc_baseline(*el) == 1) {
            // impact parameters
            const xAOD::Vertex *pv               = m_susytools_handle->GetPrimVtx();
            const Float_t       primary_vertex_z = pv ? pv->z() : 0;
            dec_new_d0(*el)                      = HelperFunctions::getD0(el);
            dec_new_d0sig(*el)                   = HelperFunctions::getD0sig(el, content.eventInfo);
            dec_new_z0(*el)                      = HelperFunctions::getZ0(el, primary_vertex_z);
            dec_new_z0sig(*el)                   = HelperFunctions::getZ0sig(el);
            content.allElectrons.push_back(el);
         }
         content.contElectrons.push_back(el); // Container electrons
      }
      content.allElectrons.sort(&HelperFunctions::comparePt);
      content.contElectrons.sort(&HelperFunctions::comparePt);
   } // done with electrons

   //-- PHOTONS --
   if (content.doPhotons) {
      content.photons    = nullptr;
      content.photonsAux = nullptr;
      content.allPhotons.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetPhotons(content.photons, content.photonsAux, kTRUE));
      for (auto ph : *content.photons) {
         if (acc_baseline(*ph) == 1) {
            content.allPhotons.push_back(ph);
         }
      }
      content.allPhotons.sort(&HelperFunctions::comparePt);
   }

   //-- TAUS --
   if (content.doTaus) {
      content.taus    = nullptr;
      content.tausAux = nullptr;
      content.allTaus.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetTaus(content.taus, content.tausAux, kTRUE));
      for (auto tau : *content.taus) {
         static SG::AuxElement::Accessor<char> acc_baseline("baseline");
         if (acc_baseline(*tau) == 1) {
            content.allTaus.push_back(tau);
         }
      }
      content.allTaus.sort(&HelperFunctions::comparePt);
   }

   //-----------------------------------------------------------------------
   // Select good objects after overlap removal
   //-----------------------------------------------------------------------

   // Good objects containers clear
   content.goodMuons.clear(SG::VIEW_ELEMENTS);
   content.baselineMuons.clear(SG::VIEW_ELEMENTS);
   content.wMuons.clear(SG::VIEW_ELEMENTS);
   content.goodElectrons.clear(SG::VIEW_ELEMENTS);
   content.baselineElectrons.clear(SG::VIEW_ELEMENTS);
   content.wElectrons.clear(SG::VIEW_ELEMENTS);
   content.goodJets.clear(SG::VIEW_ELEMENTS);
   content.goodPhotons.clear(SG::VIEW_ELEMENTS);
   content.baselinePhotons.clear(SG::VIEW_ELEMENTS);
   content.goodTaus.clear(SG::VIEW_ELEMENTS);
   content.baselineTaus.clear(SG::VIEW_ELEMENTS);

   //-- Overlap removal -- applies baseline selection before overlap removal using the acc_selected labelling!
   ANA_CHECK(m_susytools_handle->OverlapRemoval(content.electrons, content.muons, content.jets, content.photons));

   //-- JETS -- done after the overlap removal
   if (debug) {
      for (auto jet : *content.jets) {
         ANA_MSG_INFO("jet pt=" << jet->pt() * 0.001 << ", eta=" << jet->eta() << ", phi=" << jet->phi()
                                << ", bad=" << (acc_bad(*jet) == 1) << ", passOR=" << (acc_passOR(*jet) == 1)
                                << ", passJVT=" << (acc_passJvt(*jet) == 1) << ", passFJVT="
                                << (acc_passFJvt(*jet) == 1) << " => BASELINE=" << (acc_baseline(*jet) == 1)
                                << ", SIGNAL=" << (acc_signal(*jet) == 1)
                                << ", No JVT SIGNAL=" << (acc_signal_less_JVT(*jet) == 1));
      }
   }

   Float_t mhtx = 0;
   Float_t mhty = 0;
   for (auto jet : content.allJets) {
      if ((acc_signal(*jet) == 1 && acc_passOR(*jet) == 1 && acc_bad(*jet) == 0) || doRnS)
         content.goodJets.push_back(jet);
      if (acc_passOR(*jet) == 1 && acc_bad(*jet) == 0) {
         mhtx += jet->pt() * TMath::Cos(jet->phi());
         mhty += jet->pt() * TMath::Sin(jet->phi());
      }
   }
   // Calculated MHT by hand
   double mht = sqrt(mhtx * mhtx + mhty * mhty);

   //-- MUONS --
   for (auto muon : content.allMuons) {
      if (acc_baseline(*muon) == 1 && acc_passOR(*muon) == 1) { // cosmic, baseline, bad muon already applied
         content.baselineMuons.push_back(muon);
         if(muon->quality()<3 && (muon->pt()<200.0e3 ? acc_isol(*muon) : acc_isolHighPt(*muon)) && 
	    fabs(acc_z0sinTheta(*muon))<0.5 && 
	    fabs(acc_d0sig(*muon))<3.0) content.goodMuons.push_back(muon); // add iso, impact parameter cuts and Loose PID
	 else if(acc_signal(*muon) == 1) content.goodMuons.push_back(muon); // make sure we don't miss a signal muon. shouldn't happen ever.
         if (acc_signal(*muon) == 1) {
	   content.wMuons.push_back(muon); // CR muons
         }
      }
   }

   //-- ELECTRONS --
   for (auto electron : content.allElectrons) {
      if (acc_baseline(*electron) == 1 && acc_passOR(*electron) == 1) { // baseline is already applied
         content.baselineElectrons.push_back(electron);
         if((electron->pt()<200.0e3 ? acc_isol(*electron) : acc_isolHighPt(*electron)) && 
	    fabs(acc_z0sinTheta(*electron))<0.5 && 
	    fabs(acc_d0sig(*electron))<5.0) content.goodElectrons.push_back(electron); // add iso, impact parameter cuts
	 else if(acc_signal(*electron) == 1) content.goodElectrons.push_back(electron); // make sure we don't miss a signal electron. shouldn't happen ever. 
         if (acc_signal(*electron) == 1) content.wElectrons.push_back(electron); // CR electrons
      }
   }

   // need to label these as signal leptons for the later SF treatment
   const static SG::AuxElement::Decorator<char> dec_signal("signal");
   for (auto electron : content.goodElectrons) { dec_signal(*electron)=1; }
   for (auto muon : content.goodMuons) { dec_signal(*muon)=1; }

   static const SG::AuxElement::Accessor<uint8_t> acc_ambiguityType("ambiguityType");  
   static const SG::AuxElement::Accessor<ElementLink<xAOD::EgammaContainer> > acc_ambiguityLink("ambiguityLink");
   //-- PHOTONS --
   for (auto photon : content.allPhotons) {
     if (acc_baseline(*photon) == 1 && acc_passOR(*photon) == 1) { // baseline already applied
	content.baselinePhotons.push_back(photon);
	if (acc_signal(*photon) == 1 && acc_passOR(*photon) == 1) content.goodPhotons.push_back(photon);
	//if  ( fabs( photon->caloCluster()->etaBE(2) ) >1.37 &&  fabs( photon->caloCluster()->etaBE(2) ) <1.52) continue;
	//if (acc_isol(*photon) && acc_passOR(*photon) == 1){ content.goodPhotons.push_back(photon); dec_signal(*photon)=1; }
      }
   }

   //-- TAUS --
   for (auto tau : content.allTaus) {
      // apply baseline
      if (acc_baseline(*tau) == 1) {
         content.baselineTaus.push_back(tau);
         if (acc_signal(*tau) == 1) content.goodTaus.push_back(tau);
      }
   }
   //-- MET --

   // real MET, with el mu ph soft
   TLorentzVector myMET_tst;
   double         myMETsig_tst;
   getMET(content.met_tst, content.met_tstAux,
          content.jets, // use all jets (before OR and after corrections) for MET utility
          &(content.baselineElectrons), &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE,                  // do TST
          kTRUE,                  // do JVT
          nullptr,                // invisible particles
          myMET_tst, myMETsig_tst, 0);
   content.metsig_tst = myMETsig_tst;

   TLorentzVector myMET_Tight_tst;
   double         myMETsig_Tight_tst;
   getMET(content.met_tight_tst, content.met_tight_tstAux, content.jets, &(content.baselineElectrons),
          &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE, kTRUE, nullptr, myMET_Tight_tst, myMETsig_Tight_tst, 1);

   TLorentzVector myMET_Tenacious_tst;
   double         myMETsig_Tenacious_tst;
   getMET(content.met_tenacious_tst, content.met_tenacious_tstAux, content.jets, &(content.baselineElectrons),
          &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE, kTRUE, nullptr, myMET_Tenacious_tst, myMETsig_Tenacious_tst, 3);

   double met_tst_j1_dphi = -1., met_tst_j2_dphi = -1.;
   HelperFunctions::computeMETj(myMET_tst, content.goodJets, met_tst_j1_dphi, met_tst_j2_dphi);
   if (debug) {
      print("met_tst_j1_dphi: ", met_tst_j1_dphi);
      print("met_tst_j2_dphi: ", met_tst_j2_dphi);
   }
   content.met_tst_j1_dphi = met_tst_j1_dphi;
   content.met_tst_j2_dphi = met_tst_j2_dphi;

   // invisible particles - just removing all of the baseline objects for study of looser lepton definition
   xAOD::IParticleContainer invis(SG::VIEW_ELEMENTS);
   for (const auto &muon : content.baselineMuons) {
      invis.push_back(muon);
   }
   for (const auto &electron : content.baselineElectrons) {
      invis.push_back(electron);
   }
   // MET, with invisble leptons
   TLorentzVector myMET_tst_nolep;
   double         myMETsig_tst_nolep;
   getMET(content.met_tst_nolep, content.met_tst_nolepAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          &(content.baselineElectrons), &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE,                  // do TST
          kTRUE,                  // do JVT
          nullptr,                // invisible particles - using the old style of object removal here
          myMET_tst_nolep, myMETsig_tst_nolep, 0);
   content.metsig_tst_nolep = myMETsig_tst_nolep;

   TLorentzVector myMET_Tight_tst_nolep;
   double         myMETsig_Tight_tst_nolep;
   getMET(content.met_tight_tst_nolep, content.met_tight_tst_nolepAux, content.jets, &(content.baselineElectrons),
          &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE, kTRUE, &invis, myMET_Tight_tst_nolep, myMETsig_Tight_tst_nolep, 1);
   TLorentzVector myMET_Tenacious_tst_nolep;
   double         myMETsig_Tenacious_tst_nolep;
   getMET(content.met_tenacious_tst_nolep, content.met_tenacious_tst_nolepAux, content.jets,
          &(content.baselineElectrons), &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kTRUE, kTRUE, &invis, myMET_Tenacious_tst_nolep, myMETsig_Tenacious_tst_nolep, 3);
   // create sum of muon and electron pts
   {
      Float_t px = 0;
      Float_t py = 0;
      for (auto muon : content.baselineMuons) {
         px += muon->pt() * TMath::Cos(muon->phi());
         py += muon->pt() * TMath::Sin(muon->phi());
      }
      for (auto electron : content.baselineElectrons) {
         px += electron->pt() * TMath::Cos(electron->phi());
         py += electron->pt() * TMath::Sin(electron->phi());
      }
      const Float_t mpx = (*content.met_tst_nolep)["Final"]->mpx();
      const Float_t mpy = (*content.met_tst_nolep)["Final"]->mpy();
      (*content.met_tst_nolep)["Final"]->setMpx(mpx + px);
      (*content.met_tst_nolep)["Final"]->setMpy(mpy + py);
   }
   // set the vector
   myMET_tst_nolep.SetPxPyPzE((*content.met_tst_nolep)["Final"]->mpx(), (*content.met_tst_nolep)["Final"]->mpy(), 0.0,
                              (*content.met_tst_nolep)["Final"]->met());
   double met_tst_nolep_j1_dphi = -1., met_tst_nolep_j2_dphi = -1.;
   HelperFunctions::computeMETj(myMET_tst_nolep, content.goodJets, met_tst_nolep_j1_dphi, met_tst_nolep_j2_dphi);
   if (debug) {
      print("met_tst_nolep_j1_dphi: ", met_tst_nolep_j1_dphi);
      print("met_tst_nolep_j2_dphi: ", met_tst_nolep_j2_dphi);
   }
   content.met_tst_nolep_j1_dphi = met_tst_nolep_j1_dphi;
   content.met_tst_nolep_j2_dphi = met_tst_nolep_j2_dphi;

   // MET CST, for HT
   TLorentzVector myMET_cst;
   double         myMETsig_cst;
   getMET(content.met_cst, content.met_cstAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          &(content.baselineElectrons), &(content.baselineMuons),
          &(content.goodPhotons), // note baseline is applied inside SUSYTools. Electrons and photons have OR applied.
                                  // Muons do not, but they do have cleaning
          kFALSE,                 // do TST
          kFALSE,                 // do JVT
          nullptr,                // invisible particles
          myMET_cst, myMETsig_cst, 0);

   double met_cst_jet  = -1.;
   met_cst_jet         = (*content.met_cst)["RefJet"]->met();
   content.met_cst_jet = met_cst_jet;

   if (debug) {
      print("MET CST", met_cst_jet * 1e-3);
      print("MHT without jVT", mht * 1e-3);
   }

   // track MET
   //getTrackMET(content.met_track, content.met_trackAux,
   //            content.jets, // use all objects (before OR and after corrections) for MET utility
   //            content.electrons, content.muons);

   // truth MET
   TLorentzVector myTruthMET;
   if (m_isMC) {
      content.met_truth = nullptr;
      if (!event->retrieve(content.met_truth, "MET_Truth")
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("VBFInv::getObjects : Failed to retrieve MET_Truth container");
         return EL::StatusCode::FAILURE;
      }

      xAOD::MissingETContainer::const_iterator met_truth_it = content.met_truth->find("NonInt");

      if (met_truth_it == content.met_truth->end()) {
         ANA_MSG_ERROR("VBFInv::getObjects : No NonInt inside truth MET container");
         return EL::StatusCode::FAILURE;
      } else {
         float Etmiss_Etx = (*met_truth_it)->mpx();
         float Etmiss_Ety = (*met_truth_it)->mpx();
         float Etmiss_Et  = sqrt(Etmiss_Etx * Etmiss_Etx + Etmiss_Ety * Etmiss_Ety);
         myTruthMET.SetPxPyPzE(Etmiss_Etx, Etmiss_Ety, 0., Etmiss_Et);
      }
   }

   //-----------------------------------------------------------------------
   // Debugs and sanity checks
   //-----------------------------------------------------------------------
   if (debug) {
      //-- JETS --
      printObjects(content.allJets, "allJets");
      printObjects(content.goodJets, "goodJets");
      //-- MUONS --
      printObjects(content.allMuons, "allMuons");
      printObjects(content.baselineMuons, "baselineMuons");
      printObjects(content.wMuons, "wMuons");
      printObjects(content.contMuons, "contMuons");
      printObjects(content.goodMuons, "goodMuons");
      //-- ELECTRONS --
      printObjects(content.allElectrons, "allElectrons");
      printObjects(content.baselineElectrons, "baselineElectrons");
      printObjects(content.wElectrons, "wElectrons");
      printObjects(content.contElectrons, "contElectrons");
      printObjects(content.goodElectrons, "goodElectrons");
      //-- MET --
      printMET(myMET_tst, myMETsig_tst, myTruthMET, "MET");
      printMET(content.met_tst_nolep, "MET no leptons");
      //    printMET(content.met_tst_noelectron, "MET no electrons");
      //    printMET(content.met_tst_nomuon, "MET no muons");
      //printTrackMET(content.met_track, "track MET");
      //-- PHOTONS --
      printObjects(content.allPhotons, "photons");
   }

   //-----------------------------------------------------------------------
   // Continue cleaning...
   //-----------------------------------------------------------------------

   // asg::AnaToolHandle< ST::ISUSYObjDef_xAODTool >&  handle{"ST::SUSYObjDef_xAOD/SUSYTools"};
   const asg::AsgTool *toolPtr = dynamic_cast<const asg::AsgTool *>(m_susytools_handle.get());
   if (!toolPtr) {
      // Handle the error...
      std::cout << "Error with tool!" << std::endl;
   }
   // auto testProp = toolPtr->getProperty< int >( "Jet.InputType" );
   // std::cout << testProp << std::endl;

   // Jet cleaning
   // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJets2017

   Bool_t                                passesJetCleanLoose  = true;
   Bool_t                                passesJetCleanTight  = true;
   Bool_t                                passesBadBatmanClean = true;
   static SG::AuxElement::Accessor<char> acc_eventClean("DFCommonJets_eventClean_LooseBad");
   if (acc_eventClean.isAvailable(*content.eventInfo)) {
      if (debug) print("eventClean_LooseBad", (bool)acc_eventClean(*content.eventInfo));
      passesJetCleanLoose = !(acc_eventClean(*content.eventInfo) == 0);
      if (!passesJetCleanLoose && doSkim) {
         return EL::StatusCode::SUCCESS;
      }
   }

   // batman cleaning flag
   static SG::AuxElement::Accessor<char> acc_isBadBatman("DFCommonJets_isBadBatman");
   if (acc_isBadBatman.isAvailable(*content.eventInfo)) {
      if (debug) print("eventClean_BadBatman", (bool)acc_isBadBatman(*content.eventInfo));
      passesBadBatmanClean = !(acc_isBadBatman(*content.eventInfo) == 0);
   }

   // Tight cleaning for EMTopo
   static SG::AuxElement::Accessor<char> acc_jetCleanTight("DFCommonJets_jetClean_TightBad");
   for (auto jet : content.goodJets) {
      if (acc_jetCleanTight.isAvailable(*jet)) {
         if (debug) print("jetClean_TightBad", (bool)acc_jetCleanTight(*jet));
         if (acc_jetCleanTight(*jet) == 0) {
            passesJetCleanTight = false;
	    break;
         }
      }
   }

   // Tight cleaning for PFlow
   Bool_t passesJetCleanTightCustom = true;
   for (auto jet : content.goodJets) {
      std::vector<float> SumTrkPt500vec;
      SumTrkPt500vec    = jet->auxdata<std::vector<float>>("SumPtTrkPt500");
      float SumTrkPt500 = -1;
      if (SumTrkPt500vec.size() > 0) SumTrkPt500 = SumTrkPt500vec.at(0);
      float chf  = SumTrkPt500 / jet->pt();
      float fmax = jet->auxdata<float>("FracSamplingMax");
      if (std::fabs(jet->eta()) < 2.4 && chf / fmax < 0.1) {
         passesJetCleanTightCustom = false;
	 break;
      }
   }

   m_CutFlow.hasPassed(VBFInvCuts::JetBad, event_weight);
   content.passJetCleanLoose = passesJetCleanLoose;
   content.passJetCleanTight = passesJetCleanTight;
   content.passJetCleanTightEM = passesJetCleanTightCustom;
   if(copyEMTopoFJVT){
     content.passJetCleanTight   = passesJetCleanTightCustom;
     content.passJetCleanTightEM = passesJetCleanTight;
   }
   content.passBatman        = passesBadBatmanClean;

   //-----------------------------------------------------------------------
   // Fill tree
   //-----------------------------------------------------------------------

   // skimming
   const Double_t pt1SkimToUse    = (content.isNominal) ? pt1Skim : pt1SkimForSyst;
   const Double_t pt2SkimToUse    = (content.isNominal) ? pt2Skim : pt2SkimForSyst;
   const Double_t metSkimToUse    = (content.isNominal) ? metSkim : metSkimForSyst;
   const Double_t mjjSkimToUse    = (content.isNominal) ? mjjSkim : mjjSkimForSyst;
   const Double_t detajjSkimToUse = (content.isNominal) ? detajjSkim : detajjSkimForSyst;
   // TVector2 met_nomuon_to_use = TVector2((*content.met_tst_nomuon)["Final"]->mpx(),
   // (*content.met_tst_nomuon)["Final"]->mpy()); TVector2 met_noelectron_to_use =
   // TVector2((*content.met_tst_noelectron)["Final"]->mpx(), (*content.met_tst_noelectron)["Final"]->mpy());
   TVector2 met_nolep_to_use =
      TVector2((*content.met_tst_nolep)["Final"]->mpx(), (*content.met_tst_nolep)["Final"]->mpy());
   double detajjMax = -1., mjjMax = -1., dphijjMax = -1.; // not really max here, ?TO DO?
   HelperFunctions::computejj(content.goodJets, mjjMax, detajjMax, dphijjMax);
   if (debug) {
      print("Max Mjj", mjjMax);
      print("Max DEta", detajjMax);
      print("Max DPhi", dphijjMax);
   }

   // Skimming
   // Bool_t saveMe = ( met_nomuon_to_use.Mod() > metSkimToUse || met_noelectron_to_use.Mod() > metSkimToUse );
   Bool_t saveMe = (met_nolep_to_use.Mod() > metSkimToUse);
   if (doMETDetail)
      saveMe = saveMe || ((*content.met_tight_tst_nolep)["Final"]->met() > metSkimToUse) ||
               ((*content.met_tight_tst)["Final"]->met() > metSkimToUse) ||
               //((*content.met_tighter_tst_nolep)["Final"]->met() > metSkimToUse) ||
               //((*content.met_tighter_tst)["Final"]->met() > metSkimToUse) ||
               ((*content.met_tenacious_tst_nolep)["Final"]->met() > metSkimToUse) ||
               ((*content.met_tenacious_tst)["Final"]->met() > metSkimToUse);
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::MET_skim, event_weight);
   if (content.goodJets.size() < 2) // At least two good jet
      return EL::StatusCode::SUCCESS;
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetN_skim, event_weight);
   saveMe &= (content.goodJets[0]->pt() > pt1SkimToUse);
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetpT1_skim, event_weight);
   saveMe &= (content.goodJets[1]->pt() > pt2SkimToUse);
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetpT2_skim, event_weight);
   saveMe &= (mjjMax > mjjSkimToUse);
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::Mjj_skim, event_weight);
   saveMe &= (detajjMax > detajjSkimToUse);
   if (saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::DEta_skim, event_weight);
   saveMe |= !doSkim; // user flag to skim or not

   if (saveMe) fillTree(content, cand, systInfo);

   if (debug) ANA_MSG_INFO("====================================================================");

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::fillTree(Analysis::ContentHolder &content, Analysis::outHolder &cand,
                                const ST::SystInfo &systInfo)
{

   xAOD::TEvent *event = wk()->xaodEvent();

   cand.reset();

   // Deal with Rebalance and Smear first:
   if (doRnS && !m_isMC) {
      cand.rns.rnsPSweight = cand.rns.getPSweight(m_susytools_handle, event, content.eventInfo->runNumber(), debug);
   }

   // add vertex info
   static SG::AuxElement::ConstAccessor<float> acc_sumPt2("sumPt2");
   static SG::AuxElement::ConstAccessor<float> acc_chiSquared("chiSquared");
   const xAOD::Vertex *                        pvD = m_susytools_handle->GetPrimVtx();
   if (pvD) {
      if (acc_sumPt2.isAvailable(*pvD)) cand.evt.vtx_sumpt2 = acc_sumPt2(*pvD);
   } else
      cand.evt.vtx_sumpt2 = -999;

   // raw event info
   unsigned mcchannel = m_isMC ? content.eventInfo->mcChannelNumber() : content.eventInfo->runNumber();
   if (getMCChannel > 0) mcchannel = unsigned(getMCChannel);
   cand.evt.runNumber   = (m_isMC) ? mcchannel : content.eventInfo->runNumber();
   cand.evt.runPeriod   = content.eventInfo->runNumber();
   cand.evt.eventNumber = (ULong64_t)content.eventInfo->eventNumber();
   cand.evt.lumiBlock   = content.eventInfo->lumiBlock();
   cand.evt.bcid        = content.eventInfo->bcid();
   static SG::AuxElement::Accessor<Int_t> acc_BCIDDistanceFromFront("BCIDDistanceFromFront");
   static SG::AuxElement::Accessor<Int_t> acc_BCIDDistanceFromTail("BCIDDistanceFromTail");
   if (acc_BCIDDistanceFromFront.isAvailable(*content.eventInfo))
      cand.evt.BCIDDistanceFromFront = acc_BCIDDistanceFromFront(*content.eventInfo);
   if (acc_BCIDDistanceFromTail.isAvailable(*content.eventInfo))
      cand.evt.BCIDDistanceFromTail = acc_BCIDDistanceFromTail(*content.eventInfo);
   cand.evt.averageIntPerXing    = content.eventInfo->averageInteractionsPerCrossing();
   cand.evt.corAverageIntPerXing = content.eventInfo->actualInteractionsPerCrossing();
   if(!m_isMC) cand.evt.corAverageIntPerXing = m_susytools_handle->GetCorrectedAverageInteractionsPerCrossing();

   // Which year are we in?
   cand.evt.year = (m_isMC) ? m_susytools_handle->treatAsYear() : 0; // RandomRunNumber from the PRWTool

   Bool_t is2015(kFALSE);
   Bool_t is2016(kFALSE);
   Bool_t is2017(kFALSE);
   Bool_t is2018(kFALSE);

   if ((cand.evt.year == 0 && cand.evt.runNumber >= 276262 && cand.evt.runNumber <= 284484) || cand.evt.year == 2015)
      is2015 = kTRUE; // data2015
   if ((cand.evt.year == 0 && cand.evt.runNumber >= 296939 && cand.evt.runNumber <= 311481) || cand.evt.year == 2016)
      is2016 = kTRUE; // data2016
   if ((cand.evt.year == 0 && cand.evt.runNumber >= 324320 && cand.evt.runNumber <= 341649) || cand.evt.year == 2017)
      is2017 = kTRUE; // data2017
   if ((cand.evt.year == 0 && cand.evt.runNumber >= 348197 && cand.evt.runNumber <= 364485) || cand.evt.year == 2018)
      is2018 = kTRUE; // data2018
   
   //
   // trigger. Loading the trigger decisions
   //
   for (auto &kv : cand.evt.trigger) {
      kv.second = m_susytools_handle->IsTrigPassed(kv.first.Data());
   }
   // encoding the L1 trigger items for future checks
   if (m_susytools_handle->IsTrigPassed("L1_XE50")) cand.evt.l1_met_trig_encoded += 0x1;
   if (m_susytools_handle->IsTrigPassed("L1_XE55")) cand.evt.l1_met_trig_encoded += 0x2;
   if (m_susytools_handle->IsTrigPassed("L1_XE60")) cand.evt.l1_met_trig_encoded += 0x4;

   // Trigger matching for single lepton variables
   const static SG::AuxElement::ConstAccessor<char> acc_trigmatched("trigmatched");

   // Single lepton triggers by year
   cand.evt.randomRunNumber = (m_isMC) ? m_susytools_handle->GetRandomRunNumber() : cand.evt.runNumber;
   std::vector<std::string> single_lep_2015 = {"HLT_e24_lhmedium_L1EM20VH","HLT_e60_lhmedium", "HLT_e120_lhloose","HLT_mu20_iloose_L1MU15","HLT_mu40","HLT_mu60_0eta105_msonly"};
   std::vector<std::string> single_muo_2016_perA = {"HLT_mu24_iloose","HLT_mu24_ivarloose","HLT_mu40","HLT_mu50","HLT_mu24_iloose_L1MU15","HLT_mu24_ivarloose_L1MU15"};
   std::vector<std::string> single_muo_2016_perBD3 = {"HLT_mu24_ivarmedium","HLT_mu24_imedium","HLT_mu50"};
   std::vector<std::string> single_muo_2016_perD4L = {"HLT_mu26_ivarmedium","HLT_mu50"};
   std::vector<std::string> single_ele_2016_perAD3 = {"HLT_e24_lhtight_nod0_ivarloose","HLT_e60_lhmedium_nod0","HLT_e60_medium","HLT_e140_lhloose_nod0","HLT_e300_etcut"};
   std::vector<std::string> single_ele_2016_run298687 = {"HLT_e24_lhmedium_nod0_L1EM20VH","HLT_e24_lhtight_nod0_ivarloose","HLT_e60_lhmedium_nod0","HLT_e60_medium","HLT_e140_lhloose_nod0","HLT_e300_etcut"};
   std::vector<std::string> single_ele_2016_perD4L = {"HLT_e26_lhtight_nod0_ivarloose","HLT_e60_lhmedium_nod0","HLT_e60_medium","HLT_e140_lhloose_nod0","HLT_e300_etcut"};
   std::vector<std::string> single_lep_2017 = {"HLT_e26_lhtight_nod0_ivarloose","HLT_e60_lhmedium_nod0","HLT_e140_lhloose_nod0","HLT_e300_etcut","HLT_mu26_ivarmedium","HLT_mu50","HLT_mu60_0eta105_msonly"};
   std::vector<std::string> single_lep_2018 = {"HLT_e26_lhtight_nod0_ivarloose","HLT_e26_lhtight_nod0","HLT_e60_lhmedium_nod0","HLT_e140_lhloose_nod0","HLT_e300_etcut", // HLT_e26_lhtight_nod0 - Unprescaled since run 349169 
					       "HLT_mu26_ivarmedium","HLT_mu50","HLT_mu60_0eta105_msonly"};

   std::string str_muon_trigger_OR="";
   //
   // Fill the single lepton triggers with periods
   //
   if(is2015){ 
     for(unsigned i=0; i<single_lep_2015.size(); ++i) if(cand.evt.trigger[single_lep_2015.at(i)]) cand.evt.trigger_lep = 1; 
     m_susytools_handle->TrigMatch(&(content.goodElectrons), single_lep_2015); 
     m_susytools_handle->TrigMatch(&(content.goodMuons), single_lep_2015);
     str_muon_trigger_OR="HLT_mu20_iloose_L1MU15_OR_HLT_mu50";
   }
   if(is2016){ 
     str_muon_trigger_OR="HLT_mu26_ivarmedium_OR_HLT_mu50";
     if     (cand.evt.randomRunNumber<=300287){ for(unsigned i=0; i<single_muo_2016_perA.size(); ++i)      if(cand.evt.trigger[single_muo_2016_perA.at(i)]) cand.evt.trigger_lep = 1; m_susytools_handle->TrigMatch(&(content.goodMuons), single_muo_2016_perA);      }
     else if(cand.evt.randomRunNumber<=302872){ for(unsigned i=0; i<single_muo_2016_perBD3.size(); ++i)    if(cand.evt.trigger[single_muo_2016_perBD3.at(i)]) cand.evt.trigger_lep = 1; m_susytools_handle->TrigMatch(&(content.goodMuons), single_muo_2016_perBD3); }
     if     (cand.evt.randomRunNumber==298687){ for(unsigned i=0; i<single_ele_2016_run298687.size(); ++i) if(cand.evt.trigger[single_ele_2016_run298687.at(i)]) cand.evt.trigger_lep = 1;  m_susytools_handle->TrigMatch(&(content.goodElectrons), single_ele_2016_run298687); }
     else if(cand.evt.randomRunNumber<=302872){ for(unsigned i=0; i<single_ele_2016_perAD3.size(); ++i)    if(cand.evt.trigger[single_ele_2016_perAD3.at(i)])    cand.evt.trigger_lep = 1; m_susytools_handle->TrigMatch(&(content.goodElectrons), single_ele_2016_perAD3); }
     else                                     { 
       for(unsigned i=0; i<single_ele_2016_perD4L.size(); ++i)    if(cand.evt.trigger[single_ele_2016_perD4L.at(i)])    cand.evt.trigger_lep = 1; 
       for(unsigned i=0; i<single_muo_2016_perD4L.size(); ++i)    if(cand.evt.trigger[single_muo_2016_perD4L.at(i)])    cand.evt.trigger_lep = 1; 
       m_susytools_handle->TrigMatch(&(content.goodElectrons), single_ele_2016_perD4L); 
       m_susytools_handle->TrigMatch(&(content.goodMuons),     single_muo_2016_perD4L);
     }
   }
   if(is2017){ 
     str_muon_trigger_OR="HLT_mu26_ivarmedium_OR_HLT_mu50";
     for(unsigned i=0; i<single_lep_2017.size(); ++i) if(cand.evt.trigger[single_lep_2017.at(i)]) cand.evt.trigger_lep = 1; 
       m_susytools_handle->TrigMatch(&(content.goodElectrons), single_lep_2017); 
       m_susytools_handle->TrigMatch(&(content.goodMuons),     single_lep_2017);     
   }
   if(is2018){ 
     str_muon_trigger_OR="HLT_mu26_ivarmedium_OR_HLT_mu50";
     for(unsigned i=0; i<single_lep_2018.size(); ++i) if(cand.evt.trigger[single_lep_2018.at(i)]) cand.evt.trigger_lep = 1; 
       m_susytools_handle->TrigMatch(&(content.goodElectrons), single_lep_2018); 
       m_susytools_handle->TrigMatch(&(content.goodMuons),     single_lep_2018);

   }

   // fill the trigger matching
   for (auto electron : content.goodElectrons) { if(acc_trigmatched.isAvailable(*electron) && acc_trigmatched(*electron)) cand.evt.lep_trig_match=1; }
   for (auto muon : content.goodMuons) {         if(acc_trigmatched.isAvailable(*muon) && acc_trigmatched(*muon))         cand.evt.lep_trig_match=1; }
   cand.evt.trigger_lep_OR =
      // el 2015
      cand.evt.trigger["HLT_e24_lhmedium_L1EM20VH"] || cand.evt.trigger["HLT_e60_lhmedium"] ||
      cand.evt.trigger["HLT_e120_lhloose"] ||
      // el 2016+2017+2018
      cand.evt.trigger["HLT_e24_lhtight_nod0_ivarloose"] || cand.evt.trigger["HLT_e26_lhtight_nod0_ivarloose"] ||
      cand.evt.trigger["HLT_e60_lhmedium_nod0"] || cand.evt.trigger["HLT_e60_medium"] ||
      cand.evt.trigger["HLT_e120_lhloose_nod0"] || cand.evt.trigger["HLT_e140_lhloose_nod0"] ||
      cand.evt.trigger["HLT_e300_etcut"] || cand.evt.trigger["HLT_e24_lhmedium_nod0_L1EM20VH"] ||
      // el added in 2018
      cand.evt.trigger["HLT_e26_lhtight_nod0"] ||
      // mu 2015
      cand.evt.trigger["HLT_mu20_iloose_L1MU15"] || cand.evt.trigger["HLT_mu40"] ||
      cand.evt.trigger["HLT_mu60_0eta105_msonly"] ||
      // mu 2016
      cand.evt.trigger["HLT_mu24_iloose"] || cand.evt.trigger["HLT_mu24_ivarloose"] || cand.evt.trigger["HLT_mu40"] ||
      cand.evt.trigger["HLT_mu24_iloose_L1MU15"] ||
      cand.evt.trigger["HLT_mu24_ivarloose_L1MU15"] || // adding the MC triggers, which were named differently
      cand.evt.trigger["HLT_mu50"] || cand.evt.trigger["HLT_mu24_ivarmedium"] || cand.evt.trigger["HLT_mu24_imedium"] ||
      cand.evt.trigger["HLT_mu26_ivarmedium"] || cand.evt.trigger["HLT_mu26_imedium"];

   //
   // trigger logic implemented by year
   //
   // https://twiki.cern.ch/twiki/bin/view/Atlas/MuonTriggerPhysicsRecommendationsRel212017
   bool diMuon = cand.evt.trigger["HLT_mu18_mu8noL1"] || cand.evt.trigger["HLT_mu20_mu8noL1"] ||
                 cand.evt.trigger["HLT_2mu10"] ||                                       // 2015+2016
                 cand.evt.trigger["HLT_mu22_mu8noL1"] || cand.evt.trigger["HLT_2mu14"]; // 2017+2018
   bool diMuonYearlyOpt1L1 = is2015 ? cand.evt.trigger["HLT_mu18_mu8noL1"] : cand.evt.trigger["HLT_mu22_mu8noL1"];
   bool diMuonYearlyOpt2L1 = is2015 ? cand.evt.trigger["HLT_2mu10"] : cand.evt.trigger["HLT_2mu14"];
   bool muonTrig           = is2015 ? (cand.evt.trigger["HLT_mu20_iloose_L1MU15"] || cand.evt.trigger["HLT_mu40"] ||
                             cand.evt.trigger["HLT_mu60_0eta105_msonly"])
                          : (cand.evt.trigger["HLT_mu26_ivarmedium"] || cand.evt.trigger["HLT_mu50"] ||
                             cand.evt.trigger["HLT_mu60_0eta105_msonly"]);
   bool elecTrig =
      is2015 ? (cand.evt.trigger["HLT_e24_lhmedium_L1EM20VH"] || cand.evt.trigger["HLT_e60_lhmedium"] ||
                cand.evt.trigger["HLT_e120_lhloose"])
             : (cand.evt.trigger["HLT_e26_lhtight_nod0_ivarloose"] || cand.evt.trigger["HLT_e60_lhmedium_nod0"] ||
                cand.evt.trigger["HLT_e140_lhloose_nod0"] || cand.evt.trigger["HLT_e300_etcut"]);
   // need to decide on the years of usage
   // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/LatestRecommendationsElectronIDRun2
   bool diEle = cand.evt.trigger["HLT_2e12_lhloose_L12EM10VH"] || // 2015
                cand.evt.trigger["HLT_2e17_lhvloose_nod0"] ||     // 2016
                cand.evt.trigger["HLT_2e17_lhvloose_nod0_L12EM15VHI"] ||
                cand.evt.trigger["HLT_2e24_lhvloose_nod0"]; // 2017+2018
   bool diEleYearlyOpt1 =
      is2015 ? (cand.evt.trigger["HLT_2e12_lhloose_L12EM10VH"])
     : (is2016 ? cand.evt.trigger["HLT_2e17_lhvloose_nod0"] : cand.evt.trigger["HLT_2e24_lhvloose_nod0"]); // recovers 0.9%, but is partially prescaled in 1 run
   bool diEleYearlyOpt2 = is2015 ? (cand.evt.trigger["HLT_2e12_lhloose_L12EM10VH"])
     : (is2016 ? cand.evt.trigger["HLT_2e17_lhvloose_nod0"]
	: (is2017 ? cand.evt.trigger["HLT_2e24_lhvloose_nod0"] // prefer this one because it isn't accidently prescaled. also recovers 1.2% 
	   : cand.evt.trigger["HLT_2e17_lhvloose_nod0_L12EM15VHI"]));
   if (diMuon) cand.evt.trigger_lep += 0x10;
   if (diMuonYearlyOpt1L1) cand.evt.trigger_lep += 0x20; // this one is preferred. 5% unique rate
   if (diMuonYearlyOpt2L1) cand.evt.trigger_lep += 0x40; // only 1% unique rate
   if (diEle) cand.evt.trigger_lep += 0x100;
   if (diEleYearlyOpt1) cand.evt.trigger_lep += 0x200;
   if (diEleYearlyOpt2) cand.evt.trigger_lep += 0x400;// prefered
   if (muonTrig) cand.evt.trigger_lep += 0x2;
   if (elecTrig) cand.evt.trigger_lep += 0x4;

   Bool_t customMETtrig(kFALSE);
   if (is2015 && cand.evt.trigger["HLT_xe70_mht"])
      customMETtrig = kTRUE;
   else if (is2016 && cand.evt.randomRunNumber <= 302872 && cand.evt.trigger["HLT_xe90_mht_L1XE50"])
      customMETtrig = kTRUE;
   else if (is2016 && cand.evt.randomRunNumber > 302872 && cand.evt.trigger["HLT_xe110_mht_L1XE50"])
      customMETtrig = kTRUE;
   else if ((is2015 || is2016) && cand.evt.trigger["HLT_noalg_L1J400"])
      customMETtrig = kTRUE;
   // 2017
   if (is2017 && cand.evt.trigger["HLT_xe90_pufit_L1XE50"] && cand.evt.randomRunNumber <= 328393)
      customMETtrig = kTRUE;
   else if (is2017 && cand.evt.trigger["HLT_xe100_pufit_L1XE55"] && 329385 <= cand.evt.randomRunNumber &&
            cand.evt.randomRunNumber <= 330470)
      customMETtrig = kTRUE;
   else if (is2017 && cand.evt.trigger["HLT_xe110_pufit_L1XE55"] && 330857 <= cand.evt.randomRunNumber &&
            cand.evt.randomRunNumber <= 331975)
      customMETtrig = kTRUE;
   else if (is2017 && cand.evt.trigger["HLT_xe110_pufit_L1XE50"] && 341649 >= cand.evt.randomRunNumber &&
            cand.evt.randomRunNumber > 331975)
      customMETtrig = kTRUE;
   // 2018
   if (is2018 && cand.evt.trigger["HLT_xe110_pufit_xe70_L1XE50"] && cand.evt.randomRunNumber < 350067)
      customMETtrig = kTRUE;
   else if (is2018 && cand.evt.trigger["HLT_xe110_pufit_xe65_L1XE50"] && cand.evt.randomRunNumber >= 350067)
      customMETtrig = kTRUE;
   cand.evt.trigger_met = customMETtrig;
   // extra trigger info
   if (is2017 && cand.evt.trigger["HLT_xe110_pufit_L1XE55"]) cand.evt.trigger_met += 0x2; // unprescaled
   if (is2017 && cand.evt.trigger["HLT_xe90_pufit_L1XE50"])
      cand.evt.trigger_met += 0x4; // best combination. loss of 270/pb or 900/pb
   if (is2018 && cand.evt.trigger["HLT_xe110_pufit_xe70_L1XE50"]) cand.evt.trigger_met += 0x8; // unprescaled
   if (cand.evt.trigger["HLT_xe90_pufit_L1XE50"]) cand.evt.trigger_met += 0x10;
   if (cand.evt.trigger["HLT_xe100_pufit_L1XE50"]) cand.evt.trigger_met += 0x20;
   if (cand.evt.trigger["HLT_xe100_pufit_L1XE55"]) cand.evt.trigger_met += 0x40;
   if (cand.evt.trigger["HLT_xe110_pufit_L1XE50"]) cand.evt.trigger_met += 0x80;
   if (cand.evt.trigger["HLT_xe110_pufit_L1XE55"]) cand.evt.trigger_met += 0x100;
   if (cand.evt.trigger["HLT_xe110_pufit_xe70_L1XE50"]) cand.evt.trigger_met += 0x200;
   if (cand.evt.trigger["HLT_xe120_pufit_L1XE50"]) cand.evt.trigger_met += 0x400;
   if (cand.evt.trigger["HLT_xe110_pufit_xe65_L1XE50"]) cand.evt.trigger_met += 0x800;
   if (cand.evt.trigger["HLT_j420"]) cand.evt.trigger_met += 0x1000;
   if (cand.evt.trigger["HLT_g25_medium_L1EM22VHI_2j35_0eta490_bmv2c1077_split_2j35_0eta490"])
      cand.evt.trigger_met += 0x2000;
   if (cand.evt.trigger["HLT_j70_j50_0eta490_invm1000j50_dphi24_xe90_pufit_xe50_L1MJJ-500-NFF"])
      cand.evt.trigger_met += 0x4000;
   if (cand.evt.trigger["HLT_j70_j50_0eta490_invm1100j70_dphi20_deta40_L1MJJ-500-NFF"]) cand.evt.trigger_met += 0x8000;
   if (cand.evt.trigger["HLT_g35_medium_j70_j50_0eta490_invm900j50_L1MJJ-500-NFF"]) cand.evt.trigger_met += 0x10000;

   // pass event flags
   cand.evt.passGRL = content.passGRL;
   cand.evt.passPV  = content.passPV;
   cand.evt.passDetErr          = content.passDetErr;
   cand.evt.passJetCleanLoose   = content.passJetCleanLoose;
   cand.evt.passJetCleanTight   = content.passJetCleanTight;
   cand.evt.passJetCleanTightEM = content.passJetCleanTightEM;
   cand.evt.passBatman          = content.passBatman;

   // vertex information
   cand.evt.n_vx = content.vertices->size(); // absolute number of PV's (i.e. no track cut)
   if (doVertexDetail) {
      if (savePVOnly) {
         cand.evt.reco_vtx_ntrk.push_back(pvD->nTrackParticles());
         cand.evt.reco_vtx_x.push_back(pvD->x());
         cand.evt.reco_vtx_y.push_back(pvD->y());
         cand.evt.reco_vtx_z.push_back(pvD->z());
         cand.evt.reco_vtx_vertexType.push_back(pvD->vertexType());
         if (acc_sumPt2.isAvailable(*pvD)) cand.evt.reco_vtx_sumPt2.push_back(acc_sumPt2(*pvD));
         if (acc_chiSquared.isAvailable(*pvD)) cand.evt.reco_vtx_chiSquared.push_back(pvD->chiSquared());
      } else {
         for (auto thisVertex : *content.vertices) {
            cand.evt.reco_vtx_ntrk.push_back(thisVertex->nTrackParticles());
            cand.evt.reco_vtx_x.push_back(thisVertex->x());
            cand.evt.reco_vtx_y.push_back(thisVertex->y());
            cand.evt.reco_vtx_z.push_back(thisVertex->z());
            cand.evt.reco_vtx_vertexType.push_back(thisVertex->vertexType());
            if (acc_sumPt2.isAvailable(*thisVertex)) cand.evt.reco_vtx_sumPt2.push_back(acc_sumPt2(*thisVertex));
            if (acc_chiSquared.isAvailable(*thisVertex))
               cand.evt.reco_vtx_chiSquared.push_back(thisVertex->chiSquared());
         }
      }
   }

   // jj and met_j
   double jj_deta = -1., jj_mass = -1., jj_dphi = -1.;
   HelperFunctions::computejj(content.goodJets, jj_mass, jj_deta, jj_dphi);
   if (debug) {
      print(" Mjj", jj_mass);
      print(" DEta", jj_deta);
      print(" DPhi", jj_dphi);
   }

   cand.evt.jj_mass               = jj_mass;
   cand.evt.jj_deta               = jj_deta;
   cand.evt.jj_dphi               = jj_dphi;
   cand.evt.met_tst_j1_dphi       = content.met_tst_j1_dphi;
   cand.evt.met_tst_j2_dphi       = content.met_tst_j2_dphi;
   cand.evt.met_tst_nolep_j1_dphi = content.met_tst_nolep_j1_dphi;
   cand.evt.met_tst_nolep_j2_dphi = content.met_tst_nolep_j2_dphi;
   cand.evt.met_cst_jet           = content.met_cst_jet;
   cand.evt.metsig_tst            = content.metsig_tst;
   cand.evt.metsig_tst_nolep      = content.metsig_tst_nolep;

   // MC-only information

   if (m_isMC) {
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_prodMode("HTXS_prodMode");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_errorCode("HTXS_errorCode");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Stage0_Category("HTXS_Stage0_Category");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Stage1_1_Category_pTjet30("HTXS_Stage1_1_Category_pTjet30");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Stage1_1_Category_pTjet25("HTXS_Stage1_1_Category_pTjet25");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Stage1_1_Fine_Category_pTjet30("HTXS_Stage1_1_Fine_Category_pTjet30");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Stage1_1_Fine_Category_pTjet25("HTXS_Stage1_1_Fine_Category_pTjet25");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Njets_pTjet25("HTXS_Njets_pTjet25");
     static SG::AuxElement::ConstAccessor<Int_t> acc_HTXS_Njets_pTjet30("HTXS_Njets_pTjet30");
     static SG::AuxElement::ConstAccessor<Float_t> acc_HTXS_V_pt("HTXS_V_pt");
     static SG::AuxElement::ConstAccessor<Float_t> acc_HTXS_Higgs_pt("HTXS_Higgs_pt");
      // Record all weights
      cand.evt.mcEventWeight = content.eventInfo->mcEventWeight();
      if(acc_HTXS_prodMode.isAvailable(*content.eventInfo)){
	cand.evt.HTXS_prodMode                       = acc_HTXS_prodMode(*content.eventInfo);
      }
      if(acc_HTXS_Stage0_Category.isAvailable(*content.eventInfo)){
        cand.evt.HTXS_errorCode                      = acc_HTXS_errorCode(*content.eventInfo);
	cand.evt.HTXS_Stage0_Category                = acc_HTXS_Stage0_Category(*content.eventInfo);
	cand.evt.HTXS_Stage1_1_Category_pTjet30      = acc_HTXS_Stage1_1_Category_pTjet30(*content.eventInfo);
	cand.evt.HTXS_Stage1_1_Category_pTjet25      = acc_HTXS_Stage1_1_Category_pTjet25(*content.eventInfo);
	cand.evt.HTXS_Stage1_1_Fine_Category_pTjet30 = acc_HTXS_Stage1_1_Fine_Category_pTjet30(*content.eventInfo);
	cand.evt.HTXS_Stage1_1_Fine_Category_pTjet25 = acc_HTXS_Stage1_1_Fine_Category_pTjet25(*content.eventInfo);
	cand.evt.HTXS_Njets_pTjet25                  = acc_HTXS_Njets_pTjet25(*content.eventInfo);
	cand.evt.HTXS_Njets_pTjet30                  = acc_HTXS_Njets_pTjet30(*content.eventInfo);
	cand.evt.HTXS_V_pt                           = acc_HTXS_V_pt(*content.eventInfo);
	cand.evt.HTXS_Higgs_pt                       = acc_HTXS_Higgs_pt(*content.eventInfo);
      }
      if (my_XsecDB)
         cand.evt.mcEventWeightXsec = content.eventInfo->mcEventWeight() * my_XsecDB->xsectTimesEff(cand.evt.runNumber);
      cand.evt.mcEventWeights = content.eventInfo->mcEventWeights();
      // Fixes a bug in the PDF weights from production
      if(cand.evt.runNumber==346588) cand.evt.mcEventWeight = cand.evt.mcEventWeights.at(111);
      if(cand.evt.runNumber==346600) cand.evt.mcEventWeight = cand.evt.mcEventWeights.at(109);
      cand.evt.puWeight       = m_susytools_handle->GetPileupWeight();
      cand.evt.btagSFWeight   = m_susytools_handle->BtagSF(&content.goodJets);

      // GetTotalJetSF(jets, bool btagSF, bool jvtSF)
      // load the leading two jets only, so we turn off the signal less jvt temporily
      static SG::AuxElement::Accessor<char> acc_signal_less_JVT("signal_less_JVT");
      unsigned                              ij = 0;
      for (auto jet : content.goodJets) {
         if (ij < 2) acc_signal_less_JVT(*jet) = 0;
         ++ij;
      }
      cand.evt.jvtSFWeight         = m_susytools_handle->GetTotalJetSF(content.jets, false, true);
      cand.evt.fjvtSFWeight        = m_susytools_handle->FJVT_SF(&content.goodJets);
      cand.evt.fjvtSFTighterWeight = m_susytools_Tighter_handle->FJVT_SF(&content.goodJets);
      ij                           = 0; // reset the signal value
      for (auto jet : content.goodJets) {
         if (ij < 2) acc_signal_less_JVT(*jet) = 1;
         ++ij;
      }

      // add the truth filters
      static SG::AuxElement::Accessor<int>   acc_FlavourFilter("FlavourFilter");
      static SG::AuxElement::Accessor<float> acc_MGVTruthPt("MGVTruthPt");
      static SG::AuxElement::Accessor<float> acc_SherpaVTruthPt("SherpaVTruthPt");
      static SG::AuxElement::Accessor<bool>  acc_in_vy_overlap("in_vy_overlap");
      static SG::AuxElement::Accessor<bool>  acc_in_vy_overlap_iso("in_vy_overlap_iso");
      if (acc_FlavourFilter.isAvailable(*content.eventInfo))
         cand.evt.FlavourFilter = acc_FlavourFilter(*content.eventInfo);
      if (acc_MGVTruthPt.isAvailable(*content.eventInfo)) cand.evt.MGVTruthPt = acc_MGVTruthPt(*content.eventInfo);
      if (acc_SherpaVTruthPt.isAvailable(*content.eventInfo))
         cand.evt.SherpaVTruthPt = acc_SherpaVTruthPt(*content.eventInfo);
      if (acc_in_vy_overlap.isAvailable(*content.eventInfo))
         cand.evt.in_vy_overlap = acc_in_vy_overlap(*content.eventInfo);
      if (acc_in_vy_overlap_iso.isAvailable(*content.eventInfo))
         cand.evt.in_vy_overlap_iso = acc_in_vy_overlap_iso(*content.eventInfo);

      // electron anti-id SF
      // implementing setup
      // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ElectronEfficiencyAntiID#Correlation%20of%20uncertainties
      static SG::AuxElement::Accessor<char> acc_baseline("baseline");
      //-- ELECTRONS --
      const xAOD::TruthParticleContainer *truthElectrons = nullptr;
      if (!m_determinedDerivation) {
         ANA_MSG_INFO("Determining derivation type");
         m_isEXOT5              = event->retrieve(truthElectrons, "EXOT5TruthElectrons").isSuccess();
         m_determinedDerivation = kTRUE;
         TString isEXOT5        = (m_isEXOT5) ? "YES" : "NO";
         ANA_MSG_INFO(
            "Is it EXOT5? (will trigger access to dedicated truth electron and muon containers): " << isEXOT5);
      }

      const TString el_container = (m_isEXOT5) ? "EXOT5TruthElectrons" : "TruthElectrons";
      if (!event->retrieve(truthElectrons, el_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthElectrons container");
         return EL::StatusCode::FAILURE;
      }
      //-- PHOTONS --
      //const xAOD::TruthParticleContainer *truthPhotons = nullptr;
      //const TString ph_container = (m_isEXOT5) ? "EXOT5TruthPhotons" : "TruthPhotons";
      //if (!event->retrieve(truthPhotons, ph_container.Data())
      //        .isSuccess()) { // retrieve arguments: container type, container key
      //   ANA_MSG_ERROR("Failed to retrieve TruthPhotons container");
      //   return EL::StatusCode::FAILURE;
      //}
      //-- TAUS --
      const xAOD::TruthParticleContainer *truthTaus = nullptr;
      if (!event->retrieve(truthTaus, "TruthTaus").isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthTaus container");
         return EL::StatusCode::FAILURE;
      }

      // apply antiID SF
      cand.evt.eleANTISF = 1.0;
      GetAntiIDSF(content, truthElectrons, cand.evt.eleANTISF);

      if (content.isNominal && doSystematics) { // only run for the nominal
         ANA_CHECK(m_elecEfficiencySFTool_anti_id->applySystematicVariation(systInfo.systset));
         const CP::SystematicSet &syst_anti_id_set = m_elecEfficiencySFTool_anti_id->recommendedSystematics();
         CP::SystematicSet        systset;
         for (const auto &systE : syst_anti_id_set) {
            systset.clear();
            systset.insert(systE);
            const TString      thisSyst = systE.name();
            CP::SystematicCode ret      = m_elecEfficiencySFTool_anti_id->applySystematicVariation(systset);
            if (ret != CP::SystematicCode::Ok) ATH_MSG_ERROR("cannot configure anti ID tool");
            float &sysEleAntiSF = cand.evt.GetSystVar("eleANTISF", thisSyst, m_tree[""]);
            GetAntiIDSF(content, truthElectrons, sysEleAntiSF);
         }
         ANA_CHECK(m_elecEfficiencySFTool_anti_id->applySystematicVariation(systInfo.systset));
         const CP::SystematicSet &syst_anti_iso_set = m_elecEfficiencySFTool_anti_iso->recommendedSystematics();
         for (const auto &systE : syst_anti_iso_set) {
            systset.clear();
            systset.insert(systE);
            const TString      thisSyst = systE.name();
            CP::SystematicCode ret      = m_elecEfficiencySFTool_anti_iso->applySystematicVariation(systset);
            if (ret != CP::SystematicCode::Ok) ATH_MSG_ERROR("cannot configure anti iso tool");
            float &sysEleAntiSF = cand.evt.GetSystVar("eleANTISF", thisSyst, m_tree[""]);
            GetAntiIDSF(content, truthElectrons, sysEleAntiSF);
         }
         ANA_CHECK(m_elecEfficiencySFTool_anti_iso->applySystematicVariation(systInfo.systset));
      }

      // Lepton Scale Factors
      // See definition of Trig.Singlelep20XX in SUSYObjDef_xAOD.cxx
      // You can modify it in the ST config under Trigger SFs configuration
      // Total Electron SF: GetTotalElectronSF(const xAOD::ElectronContainer& electrons, const bool recoSF, const bool
      // idSF, const bool triggerSF, const bool isoSF, const std::string& trigExpr, const bool chfSF)
      if((cand.evt.n_el==2 || cand.evt.n_el_baseline>1 ) && cand.evt.n_mu==0){
	cand.evt.elSFWeight     = m_susytools_Tighter_handle->GetTotalElectronSF(content.goodElectrons, true, true, false, true, "");
	cand.evt.elSFTrigWeight = m_susytools_Tighter_handle->GetTotalElectronSF(content.goodElectrons, false, false, true, false, "singleLepton");
      }else{ // for the tigher lepton selection  
	cand.evt.elSFWeight = m_susytools_handle->GetTotalElectronSF(content.wElectrons, true, true, false, true, "");
	cand.evt.elSFTrigWeight = m_susytools_handle->GetTotalElectronSF(content.wElectrons, false, false, true, false, "singleLepton");
      }
      // Total Muon SF: GetTotalMuonTriggerSF(const xAOD::MuonContainer& sfmuons, const std::string& trigExpr)
      if((cand.evt.n_el==0) && (cand.evt.n_mu==2 || cand.evt.n_mu_baseline>1)){
	cand.evt.muSFWeight     = m_susytools_Tighter_handle->GetTotalMuonSF(content.goodMuons, true, true, "");
	cand.evt.muSFTrigWeight = m_susytools_Tighter_handle->GetTotalMuonSF(content.goodMuons, false, false,str_muon_trigger_OR);
      }else{ // for the tigher lepton selection
	cand.evt.muSFWeight     = m_susytools_handle->GetTotalMuonSF(content.wMuons, true, true, "");
	cand.evt.muSFTrigWeight = m_susytools_handle->GetTotalMuonSF(content.wMuons, false, false,str_muon_trigger_OR);
      }
      // dilepton trigger SFs
      if(content.goodElectrons.size()>1 || content.goodMuons.size()>1){
	cand.evt.dilepTrigSFWeight = m_susytools_handle->GetTriggerGlobalEfficiencySF(content.goodElectrons,content.goodMuons,"diLepton");
      }
      // photon efficiency SF
      cand.evt.phSFWeight     = m_susytools_handle->GetTotalPhotonSF(content.goodPhotons,true,true,false);

      if (debug) {
         print("Electron SF", cand.evt.elSFWeight);
         print("Electron Trig SF", cand.evt.elSFTrigWeight);
         print("Muon SF", cand.evt.muSFWeight);
         print("Muon Trig SF", cand.evt.muSFTrigWeight);
      }


      // add the weight systematics
      if (content.isNominal) { // only run for the nominal
         for (const auto &weightSysInfo : m_sysWeightList) {
            const CP::SystematicSet &sysWeight = weightSysInfo.systset;
            const TString            thisSyst  = sysWeight.name();
            if (m_susytools_handle->applySystematicVariation(sysWeight) != CP::SystematicCode::Ok) {
               ANA_MSG_ERROR("Cannot configure SUSYTools for weight systematic var. " << sysWeight.name().c_str());
               return EL::StatusCode::FAILURE;
            }
            if (thisSyst.Contains("PRW_")) {
               float &sysSF = cand.evt.GetSystVar("puWeight", thisSyst, m_tree[""]);
               sysSF        = m_susytools_handle->GetPileupWeight();
            } else if (thisSyst.Contains("FT_EFF_")) {
               float &sysSF = cand.evt.GetSystVar("btagSFWeight", thisSyst, m_tree[""]);
               sysSF        = m_susytools_handle->BtagSF(&content.goodJets);
            } else if (thisSyst.Contains("JET_JvtEfficiency")) {
               float &sysSF = cand.evt.GetSystVar("jvtSFWeight", thisSyst, m_tree[""]);
               sysSF        = m_susytools_handle->GetTotalJetSF(&content.goodJets, false, true);
            } else if (thisSyst.Contains("JET_fJvtEfficiency")) {
               float &sysSF = cand.evt.GetSystVar("fjvtSFWeight", thisSyst, m_tree[""]);
               sysSF        = m_susytools_handle->FJVT_SF(&content.goodJets);
               float &sysSF2 = cand.evt.GetSystVar("fjvtSFTighterWeight", thisSyst, m_tree[""]);
               sysSF2        = m_susytools_Tighter_handle->FJVT_SF(&content.goodJets);
            } else if (thisSyst.Contains("EL_EFF_Trigger")) {
               float &sysSF2 = cand.evt.GetSystVar("elSFTrigWeight", thisSyst, m_tree[""]);
	       if((cand.evt.n_el==2 || cand.evt.n_el_baseline>1 ) && cand.evt.n_mu==0)
		 sysSF2        = m_susytools_Tighter_handle->GetTotalElectronSF(content.goodElectrons, false, false, true, false,"singleLepton");
	       else
		 sysSF2        = m_susytools_handle->GetTotalElectronSF(content.wElectrons, false, false, true, false,"singleLepton");
               float &sysSF3 = cand.evt.GetSystVar("dilepTrigSFWeight", thisSyst, m_tree[""]);
	       if(content.goodElectrons.size()>1 || content.goodMuons.size()>1){
		 sysSF3 = m_susytools_handle->GetTriggerGlobalEfficiencySFsys(content.goodElectrons,content.goodMuons, sysWeight,"diLepton");		 
	       }
            } else if (thisSyst.Contains("EL_EFF")) {
               float &sysSF = cand.evt.GetSystVar("elSFWeight", thisSyst, m_tree[""]);
	       if((cand.evt.n_el==2 || cand.evt.n_el_baseline>1 ) && cand.evt.n_mu==0)
		 sysSF = m_susytools_Tighter_handle->GetTotalElectronSF(content.goodElectrons, true, true, false, true, "");
	       else
		 sysSF = m_susytools_handle->GetTotalElectronSF(content.wElectrons, true, true, false, true, "");
            } else if (thisSyst.Contains("MUON_EFF_Trig")) {
               float &sysSF2 = cand.evt.GetSystVar("muSFTrigWeight", thisSyst, m_tree[""]);
	       if((cand.evt.n_el==0) && (cand.evt.n_mu==2 || cand.evt.n_mu_baseline>1))
		 sysSF2        = m_susytools_Tighter_handle->GetTotalMuonSF(content.goodMuons, false, false,str_muon_trigger_OR);
	       else sysSF2        = m_susytools_handle->GetTotalMuonSF(content.wMuons, false, false,str_muon_trigger_OR);
               float &sysSF3 = cand.evt.GetSystVar("dilepTrigSFWeight", thisSyst, m_tree[""]);
	       if(content.goodElectrons.size()>1 || content.goodMuons.size()>1){
		 sysSF3 = m_susytools_handle->GetTriggerGlobalEfficiencySFsys(content.goodElectrons,content.goodMuons,sysWeight,"diLepton");		 
	       }
            } else if (thisSyst.Contains("MUON_EFF")) {
               float &sysSF = cand.evt.GetSystVar("muSFWeight", thisSyst, m_tree[""]);
	       if((cand.evt.n_el==0) && (cand.evt.n_mu==2 || cand.evt.n_mu_baseline>1))
		 sysSF        = m_susytools_Tighter_handle->GetTotalMuonSF(content.goodMuons, true, true, "");
	       else sysSF        = m_susytools_handle->GetTotalMuonSF(content.wMuons, true, true, "");
            } else if (thisSyst.Contains("TAUS_EFF")) { // not implemented!! we do not use taus
            } else if (thisSyst.Contains("PH_EFF")) {   // photon efficiency SF 
	      float &sysSF = cand.evt.GetSystVar("phSFWeight", thisSyst, m_tree[""]);
	      sysSF = m_susytools_handle->GetTotalPhotonSF(content.goodPhotons,true,true,false);
            } else if (thisSyst.Contains("EL_CHARGEID_")) {   // electron charge flip working point. Skipping because we don't use this
            } else {
               ANA_MSG_INFO("Not configured to save this weight systematic var. " << sysWeight.name().c_str());
            }
         }
         // set back to nominal
         if (m_susytools_handle->applySystematicVariation(systInfo.systset) != CP::SystematicCode::Ok) {
            ANA_MSG_ERROR("Cannot configure SUSYTools for weight systematic var. " << systInfo.systset.name().c_str());
            return EL::StatusCode::FAILURE;
         }
      }

      // PDF
      /*
        const xAOD::TruthEventContainer *truthE(nullptr);
        if (!event->retrieve(truthE, "TruthEvents").isSuccess()) {
         ANA_MSG_ERROR("Failed to retrieve Truth container");
       } else {
         try {
          xAOD::TruthEventContainer::const_iterator truthE_itr = truthE->begin();
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_id1, xAOD::TruthEvent::PDGID1);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_id2, xAOD::TruthEvent::PDGID2);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_x1, xAOD::TruthEvent::X1);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_x2, xAOD::TruthEvent::X2);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_pdf1, xAOD::TruthEvent::PDF1);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_pdf2, xAOD::TruthEvent::PDF2);
          (*truthE_itr)->pdfInfoParameter(cand.evt.pdf_scale, xAOD::TruthEvent::Q);
        } catch (SG::ExcBadAuxVar) {
                // ignore this variable, when unavailable
                // see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MC15aKnownIssues
        }
      }
      */

      // Rebalance and Smear
      if (doRnS && !m_isMC) {
         cand.rns.rnsPSweight = cand.rns.getPSweight(m_susytools_handle, event, content.eventInfo->runNumber(), debug);
      }

      // Truth
      //-- JETS --
      const xAOD::JetContainer *truthJets(nullptr);
      static Bool_t             failedLookingFor(kFALSE); // trick to avoid infinite RuntimeWarning's for EXOT5
      if (!failedLookingFor) {
         if (!event->retrieve(truthJets, "AntiKt4TruthJets").isSuccess()) {
            ANA_MSG_ERROR("VBFInv::analyzeEvent : Failed to access Truth Jets container; not attempting again, "
                          "truth_jet* variables will be empty");
            failedLookingFor = kTRUE;
         } else {
            // number of truth jets with pt > 20 GeV and OR
            // loop over all truth jets passing basic cuts
            Int_t                               nTruthJets(0);
            const xAOD::TruthParticleContainer *truthP(0);
            ANA_CHECK(event->retrieve(truthP, "TruthParticles"));
	    // count the number partons. many for madgraph
	    cand.evt.nParton=0;
	    for (const auto &truthP_itr : *truthP) {
	      if(truthP_itr->status()==23 && (truthP_itr->pdgId()==21 || fabs(truthP_itr->pdgId())<7)) cand.evt.nParton+=1;
	      //std::cout << "   part: " << truthP_itr->status() << " pt: " << truthP_itr->pt() << " eta " << truthP_itr->eta() << " id: " << truthP_itr->pdgId() << std::endl;
	    }
	    if(false && cand.evt.nParton!=4){
	      std::cout << "Event with nparton: " << cand.evt.nParton <<std::endl;
	      for (const auto &truthP_itr : *truthP) {
		std::cout << "   part: " << truthP_itr->status() << " pt: " << truthP_itr->pt() << " eta " << truthP_itr->eta() << " id: " << truthP_itr->pdgId() << std::endl;
	      }	      
	    }
            for (const auto &truthJ_itr : *truthJets) {
               if (truthJ_itr->pt() > 20000.) { // no eta cut
                  float minDR2 = 9999999.;
                  // for each jet, loop over all electrons and muons passing basic cuts to check if the jets should be
                  // counted or not
                  for (const auto &truthP_itr : *truthP) {
                     if (truthP_itr->pt() > 25000. && truthP_itr->status() == 1 &&
                         (abs(truthP_itr->pdgId()) == 11 || abs(truthP_itr->pdgId()) == 13)) { // no eta cut
                        if (xAOD::P4Helpers::isInDeltaR(*truthJ_itr, *truthP_itr, 0.2, true)) {
                           float dR2 = xAOD::P4Helpers::deltaR2(truthJ_itr, truthP_itr, true);
                           if (dR2 < minDR2) {
                              minDR2 = dR2;
                              // stop if we already know the jet shouldn't be counted
                              if (minDR2 < 0.2 * 0.2) break;
                           }
                        }
                     }
                  }
                  if (minDR2 > 0.2 * 0.2) nTruthJets++;
               }
            }
            cand.evt.n_jet_truth = nTruthJets;
            std::vector<int> usedBC;
            for (const auto &part : *truthJets) {
               if (part->pt() < 7.0e3) continue;
               //// Now loop over all truth muons and neutrinos and add them vectorically to truth jet if dR < 0.4
               TLorentzVector nuActivity(0., 0., 0., 0.);
               TLorentzVector muActivity(0., 0., 0., 0.);
               for (const auto &truthP_itr : *truthP) {
                  if (truthP_itr->status() == 1 && (abs(truthP_itr->pdgId()) == 12 || abs(truthP_itr->pdgId()) == 14 ||
                                                    abs(truthP_itr->pdgId()) == 16)) {
                     double dR = truthP_itr->p4().DeltaR(part->p4());
                     if (dR < 0.4) {
                        if (std::find(usedBC.begin(), usedBC.end(), truthP_itr->barcode()) == usedBC.end()) {
                           usedBC.push_back(truthP_itr->barcode());
                           nuActivity += truthP_itr->p4();
                        }
                     }
                  }
                  if (truthP_itr->status() == 1 && abs(truthP_itr->pdgId()) == 13) {
                     double dR = truthP_itr->p4().DeltaR(part->p4());
                     if (dR < 0.4) {
                        if (std::find(usedBC.begin(), usedBC.end(), truthP_itr->barcode()) == usedBC.end()) {
                           usedBC.push_back(truthP_itr->barcode());
                           muActivity += truthP_itr->p4();
                        }
                     }
                  }
               }
               cand.evt.truth_jet_pt.push_back(part->pt());
               cand.evt.truth_jet_eta.push_back(part->eta());
               cand.evt.truth_jet_phi.push_back(part->phi());
               cand.evt.truth_jet_m.push_back(part->m());
               cand.evt.truth_jet_label.push_back(part->getAttribute<int>("PartonTruthLabelID"));
               cand.evt.truth_jetmu_pt.push_back((part->p4() + muActivity).Pt());
               cand.evt.truth_jetmu_eta.push_back((part->p4() + muActivity).Eta());
               cand.evt.truth_jetmu_phi.push_back((part->p4() + muActivity).Phi());
               cand.evt.truth_jetmu_m.push_back((part->p4() + muActivity).M());
               cand.evt.truth_jetmunu_pt.push_back((part->p4() + muActivity + nuActivity).Pt());
               cand.evt.truth_jetmunu_eta.push_back((part->p4() + muActivity + nuActivity).Eta());
               cand.evt.truth_jetmunu_phi.push_back((part->p4() + muActivity + nuActivity).Phi());
               cand.evt.truth_jetmunu_m.push_back((part->p4() + muActivity + nuActivity).M());
            }
            // Truth filter for V+jets Extension
            double tmp_mjj, tmp_detajj, tmp_dphijj;
            cand.evt.passVjetsFilter = VBFInvAnalysis::passTruthFilter(truthJets, JetEtaFilter, JetpTFilter, MjjFilter,
                                                                       PhijjFilter, tmp_mjj, tmp_detajj, tmp_dphijj);
            cand.evt.passVjetsFilterTauEl = VBFInvAnalysis::passTruthFilter(truthJets, JetEtaFilter, JetpTFilter, MjjFilter,
									    PhijjFilter, tmp_mjj, tmp_detajj, tmp_dphijj, truthElectrons, truthTaus);
            cand.evt.truthF_jj_mass  = tmp_mjj;
            cand.evt.truthF_jj_deta  = tmp_detajj;
            cand.evt.truthF_jj_dphi  = tmp_dphijj;
         }
      }

      //-- FATJETS --
      const xAOD::JetContainer *truthFatJets(nullptr);
      if (content.doFatJets) {
         if (!failedLookingFor) {
            if (!event->retrieve(truthFatJets, "AntiKt10TruthTrimmedPtFrac5SmallR20Jets").isSuccess()) {
               ANA_MSG_ERROR("VBFInv::analyzeEvent : Failed to access Truth FatJets container; not attempting again, "
                             "truth_fatjet* variables will be empty");
               failedLookingFor = kTRUE;
            } else {
               // number of truth fatjets
               Int_t nTruthFatJets(0);
               for (const auto &part : *truthFatJets) {
                  nTruthFatJets++;
                  cand.evt.truth_fatjet_pt.push_back(part->pt());
                  cand.evt.truth_fatjet_eta.push_back(part->eta());
                  cand.evt.truth_fatjet_phi.push_back(part->phi());
                  cand.evt.truth_fatjet_m.push_back(part->m());
               }
               cand.evt.n_fatjet_truth = nTruthFatJets;
            }
         }
      }

      //-- MUONS --
      const xAOD::TruthParticleContainer *truthMuons   = nullptr;
      const TString                       mu_container = (m_isEXOT5) ? "EXOT5TruthMuons" : "TruthMuons";
      if (!event->retrieve(truthMuons, mu_container.Data())
              .isSuccess()) { // retrieve arguments: container type, container key
         ANA_MSG_ERROR("Failed to retrieve TruthMuons container");
         return EL::StatusCode::FAILURE;
      }
      cand.evt.n_mu_truth = truthMuons->size();
      for (const auto &part : *truthMuons) {
         if (part->pt() < 4.0e3) continue;
         if (part->status() != 1) continue;
         cand.evt.truth_mu_pt.push_back(part->pt());
         cand.evt.truth_mu_eta.push_back(part->eta());
         cand.evt.truth_mu_phi.push_back(part->phi());
         cand.evt.truth_mu_m.push_back(part->m());
         cand.evt.truth_mu_status.push_back(part->status());
      }

      static SG::AuxElement::Accessor<unsigned int> acc_classifierParticleOrigin("classifierParticleOrigin");
      //-- ELECTRONS --
      cand.evt.n_el_truth = truthElectrons->size();
      for (const auto &part : *truthElectrons) {
         if (part->pt() < 4.0e3) continue;
         if (part->status() != 1) continue;
	 //std::cout << "origin ele: " << acc_classifierParticleOrigin(*part) <<std::endl;
         cand.evt.truth_el_pt.push_back(part->pt());
         cand.evt.truth_el_eta.push_back(part->eta());
         cand.evt.truth_el_phi.push_back(part->phi());
         cand.evt.truth_el_m.push_back(part->m());
         cand.evt.truth_el_status.push_back(part->status());
      }
      //-- PHOTONS --
      const xAOD::TruthParticleContainer *truthParticles(nullptr);
      ANA_CHECK(event->retrieve(truthParticles, "TruthParticles"));
      cand.evt.n_ph_truth = 0; //truthPhotons->size();
      for (const auto &part : *truthParticles) {
         if (part->pdgId() != 22) continue;
         if (part->pt() < 10.0e3) continue;
         if (part->status() != 1) continue;
	 //if( part->pt() >20.0e3) std::cout << "origin ph: " << acc_classifierParticleOrigin(*part) << " pt: " << part->pt() << " eta: " << part->eta() <<std::endl;
         cand.evt.truth_ph_pt.push_back(part->pt());
         cand.evt.truth_ph_eta.push_back(part->eta());
         cand.evt.truth_ph_phi.push_back(part->phi());
	 ++cand.evt.n_ph_truth;
      }

      //-- TAUS --
      cand.evt.n_tau_truth = truthTaus->size();
      for (const auto &part : *truthTaus) {
         if (part->pt() < 4.0e3) continue;
         // if(part->status()!=1) continue;
	 //std::cout << "origin tau: " << acc_classifierParticleOrigin(*part) <<std::endl;
         cand.evt.truth_tau_pt.push_back(part->pt());
         cand.evt.truth_tau_eta.push_back(part->eta());
         cand.evt.truth_tau_phi.push_back(part->phi());
         cand.evt.truth_tau_m.push_back(part->m());
         cand.evt.truth_tau_status.push_back(part->status());
      }

      //-- BOSONS --
      // Dressed
      const TLorentzVector truth_V_dressed =
         VBFInvAnalysis::getTruthBosonP4(truthParticles, truthElectrons, truthMuons, truthParticles);
      cand.evt.truth_V_dressed_pt  = truth_V_dressed.Pt();
      cand.evt.truth_V_dressed_eta = truth_V_dressed.Eta();
      cand.evt.truth_V_dressed_phi = truth_V_dressed.Phi();
      cand.evt.truth_V_dressed_m   = truth_V_dressed.M();
      // Bare
      const TLorentzVector truth_V_bare =
         VBFInvAnalysis::getTruthBosonP4(truthParticles, truthElectrons, truthMuons, truthParticles, kFALSE);
      cand.evt.truth_V_bare_pt  = truth_V_bare.Pt();
      cand.evt.truth_V_bare_eta = truth_V_bare.Eta();
      cand.evt.truth_V_bare_phi = truth_V_bare.Phi();
      cand.evt.truth_V_bare_m   = truth_V_bare.M();

      // Used for PTV slicing PTV500_1000 and PTV1000_E_CMS samples ( 364216-364229 )
      bool checkPTV = false;
      if (cand.evt.truth_V_dressed_pt > 500.0e3) checkPTV = true;
      cand.evt.passVjetsPTV = checkPTV;

      // -- vertices --
      const xAOD::TruthVertexContainer *truthVertices(nullptr);
      if (event->retrieve(truthVertices, "TruthVertices").isSuccess()) {
         xAOD::TruthVertexContainer::const_iterator vtx_itr = truthVertices->begin();
         cand.evt.truth_vtx_z                               = (*vtx_itr)->z();
         /* for (const auto&  truthVtx_itr : *truthVertices) {
          cand.evt.truth_vtx_ntrk = truthVtx_itr->nOutgoingParticles();//.push_back(truthVtx_itr->nOutgoingParticles());
          cand.evt.truth_vtx_z = truthVtx_itr->z();//.push_back(truthVtx_itr->z());
        }*/
      }

   } // done with MC only

   //-----------------------------------------------------------------------
   // Selected jets
   //-----------------------------------------------------------------------
   cand.evt.n_jet = content.goodJets.size();
   static SG::AuxElement::Accessor<char> acc_bjet("bjet");
   UInt_t                                tmpNBJet(0);
   for (auto jet : content.goodJets) {
      cand.jet["jet"].add(*jet);
      if (acc_bjet(*jet)) tmpNBJet++;
   }
   cand.evt.n_bjet = tmpNBJet;

   //-----------------------------------------------------------------------
   // Fat jets
   //-----------------------------------------------------------------------
   for (auto fatjet : content.allFatJets) {
      bool dnntop80 = false;
      bool dnnw50   = false;
      if (m_DNNTop80->tag(*fatjet)) {
         dnntop80 = true;
      }
      if (m_DNNW50->tag(*fatjet)) {
         dnnw50 = true;
      }

      cand.fatjet["fatjet"].add(*fatjet, dnntop80, dnnw50);
   }

   //-----------------------------------------------------------------------
   // Track jets
   //-----------------------------------------------------------------------
   for (auto trackjet : content.allTrackJets) {
      cand.trackjet["trackjet"].add(*trackjet);
   }

   //-----------------------------------------------------------------------
   // Selected muons
   //-----------------------------------------------------------------------
   cand.evt.n_mu = content.goodMuons.size();
   cand.evt.n_mu_w = content.wMuons.size();
   // if( cand.evt.n_mu_baseline != 0)
   // std::cout << "Number of muons in event=" << cand.evt.n_mu << ", baseline=" << cand.evt.n_mu_baseline << std::endl;
   static SG::AuxElement::Accessor<char> acc_signal("signal");
   static SG::AuxElement::Accessor<char> acc_bad("bad");
  const static SG::AuxElement::ConstAccessor<char> acc_isol("isol");
  const static SG::AuxElement::ConstAccessor<char> acc_isolHighPt("isolHighPt"); // use different WPs for low-pt and high-pt. split at 200 GeV.

   for (auto muon : content.goodMuons) {
      cand.mu["mu"].add(*muon);
      /* Isolation investigations
          Float_t tmp_topoetcone20(-9999);
          tmp_topoetcone20 = muon->auxdata<float>("topoetcone20");
          Float_t tmp_ptvarcone30(-9999);
          tmp_ptvarcone30 = muon->auxdata<float>("ptvarcone30");
          Float_t tmp_ptvarcone30_TightTTVA_pt1000(-9999);
          tmp_ptvarcone30_TightTTVA_pt1000 = muon->auxdata<float>("tmp_ptvarcone30_TightTTVA_pt1000");
      if( tmp_ptvarcone30_TightTTVA_pt1000/muon->pt() > 0.04 || tmp_topoetcone20/muon->pt() > 0.15)
           std::cout << "Muon pT=" << muon->pt()*1e-3 << ", ptvarcone30_TightTTVA_pt1000/pt=" <<
      tmp_ptvarcone30_TightTTVA_pt1000/muon->pt() << ", topoetcone20/pt=" << tmp_topoetcone20/muon->pt() << std::endl;
      */
   }
   static SG::AuxElement::Accessor<char> acc_baseline("baseline");
   for (auto muon : content.allMuons) {
     if (acc_baseline(*muon) == 1) ++cand.evt.n_mu_baseline_noOR;
     if(muon->quality()<3 && (acc_baseline(*muon) == 1))  ++cand.evt.n_mu_baseline_loose_noOR;
   }
   for (auto muon : content.baselineMuons) { // saving leptons failing the signal selection, but still baseline
      if (cand.mu.find("basemu") != cand.mu.end() && !(acc_signal(*muon) == 1)) {
         cand.mu["basemu"].add(*muon);
      }
      if(muon->pt()<200.0e3 ? acc_isol(*muon) : acc_isolHighPt(*muon))  ++cand.evt.n_mu_baseline_iso;
      if(muon->quality()<3)  ++cand.evt.n_mu_baseline_loose;
      ++cand.evt.n_mu_baseline;
   }

   //-----------------------------------------------------------------------
   // Selected electrons
   //-----------------------------------------------------------------------
   for (auto electron : content.allElectrons) {
      if (acc_baseline(*electron) == 1) ++cand.evt.n_el_baseline_noOR;
   }
   cand.evt.n_el = content.goodElectrons.size();
   cand.evt.n_el_w = content.wElectrons.size();
   for (auto electron : content.goodElectrons) {
      cand.el["el"].add(*electron);
   }
   static SG::AuxElement::Accessor<char> acc_DFCommonCrackVetoCleaning("DFCommonCrackVetoCleaning");
   for (auto electron : content.baselineElectrons) { // saving leptons failing the signal selection, but still baseline
      if (acc_DFCommonCrackVetoCleaning.isAvailable(*electron) && acc_DFCommonCrackVetoCleaning(*electron) == 0)
         ++cand.evt.n_el_baseline_crackVetoCleaning;
      if (cand.el.find("baseel") != cand.el.end() && !(acc_signal(*electron) == 1)) cand.el["baseel"].add(*electron);
      ++cand.evt.n_el_baseline;
      if(electron->pt()<200.0e3 ? acc_isol(*electron) : acc_isolHighPt(*electron))  ++cand.evt.n_el_baseline_iso;
   }
   // add the container leptons for lepton veto studies
   if (doContLepDetail) {
      for (auto electron : content.contElectrons) {
         cand.el["contel"].add(*electron);
      }
      for (auto muon : content.contMuons) {
         cand.mu["contmu"].add(*muon);
      }
   }

   /////////////////////////////
   // Selected photons
   ////////////////////////////
   for (auto thisPh : content.goodPhotons) {
      if (acc_DFCommonCrackVetoCleaning.isAvailable(*thisPh) && acc_DFCommonCrackVetoCleaning(*thisPh) == 0)
         ++cand.evt.n_ph_crackVetoCleaning;
      if (cand.ph.find("ph") != cand.ph.end()) {
         cand.ph["ph"].add(*thisPh);
      }
      ++cand.evt.n_ph;
   }

   /////////////////////////////
   // Selected taus
   ////////////////////////////
   if (cand.tau.find("tau") != cand.tau.end()) {
      for (auto thisTau : content.goodTaus) {
         cand.tau["tau"].add(*thisTau);
      }
   }

   //-----------------------------------------------------------------------
   // MET
   //-----------------------------------------------------------------------
   cand.met["met_tst"].add(*((*content.met_tst)["Final"]));
   cand.met["met_jet_tst"].add(*((*content.met_tst)["RefJet"]));
   cand.met["met_ele_tst"].add(*((*content.met_tst)["RefEle"]));
   cand.met["met_mu_tst"].add(*((*content.met_tst)["Muons"]));
   cand.met["met_soft_tst"].add(*((*content.met_tst)["PVSoftTrk"]));

   cand.met["met_tight_tst"].add(*((*content.met_tight_tst)["Final"]));
   cand.met["met_jet_tight_tst"].add(*((*content.met_tight_tst)["RefJet"]));
   cand.met["met_ele_tight_tst"].add(*((*content.met_tight_tst)["RefEle"]));
   cand.met["met_mu_tight_tst"].add(*((*content.met_tight_tst)["Muons"]));
   cand.met["met_soft_tight_tst"].add(*((*content.met_tight_tst)["PVSoftTrk"]));

   // cand.met["met_tighter_tst"].add(*((*content.met_tighter_tst)["Final"]));

   cand.met["met_tenacious_tst"].add(*((*content.met_tenacious_tst)["Final"]));
   cand.met["met_jet_tenacious_tst"].add(*((*content.met_tenacious_tst)["RefJet"]));
   cand.met["met_ele_tenacious_tst"].add(*((*content.met_tenacious_tst)["RefEle"]));
   cand.met["met_mu_tenacious_tst"].add(*((*content.met_tenacious_tst)["Muons"]));
   cand.met["met_soft_tenacious_tst"].add(*((*content.met_tenacious_tst)["PVSoftTrk"]));

   cand.met["met_tight_tst_nolep"].add(*((*content.met_tight_tst_nolep)["Final"]));
   // cand.met["met_tighter_tst_nolep"].add(*((*content.met_tighter_tst_nolep)["Final"]));
   cand.met["met_tenacious_tst_nolep"].add(*((*content.met_tenacious_tst_nolep)["Final"]));

   //  cand.met["met_tst_nomuon"].add(*((*content.met_tst_nomuon)["Final"]));
   //  cand.met["met_tst_noelectron"].add(*((*content.met_tst_noelectron)["Final"]));
   cand.met["met_tst_nolep"].add(*((*content.met_tst_nolep)["Final"]));

   //cand.met["met_track"].add(*((*content.met_track)["Track"]));
   if (m_isMC) cand.met["met_truth"].add(*((*content.met_truth)["NonInt"]));
   //-----------------------------------------------------------------------
   // write tree
   //-----------------------------------------------------------------------
   cand.save();

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::getMET(std::shared_ptr<xAOD::MissingETContainer> &   met,
                              std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer *jet,
                              xAOD::ElectronContainer *el, xAOD::MuonContainer *mu, xAOD::PhotonContainer * /*ph*/,
                              Bool_t doTST, Bool_t doJVT, xAOD::IParticleContainer *invis, TLorentzVector &myMET,
                              double &myMETsig, int METType)
{
   myMETsig = 0;
   met      = std::make_shared<xAOD::MissingETContainer>();
   metAux   = std::make_shared<xAOD::MissingETAuxContainer>();
   met->setStore(metAux.get());

   if (METType == 0) {
      ANA_CHECK(m_susytools_handle->GetMET(*met,
                                           jet, // use all objects (before OR and after corrections) for MET utility
                                           el, mu,
                                           0, // ph, // ph term
                                           0, // tau term
                                           doTST, doJVT,
                                           invis // invisible particles
                                           ));
      ANA_CHECK(m_susytools_handle->GetMETSig(*met, myMETsig, doTST, doJVT));
   } else if (METType == 1) {
      ANA_CHECK(
         m_susytools_Tight_handle->GetMET(*met,
                                          jet, // use all objects (before OR and after corrections) for MET utility
                                          el, mu,
                                          0, // ph, // ph term
                                          0, // tau term
                                          doTST, doJVT,
                                          invis // invisible particles
                                          ));
      ANA_CHECK(m_susytools_Tight_handle->GetMETSig(*met, myMETsig, doTST, doJVT));
   } else if (METType == 2) {
      ANA_CHECK(
         m_susytools_Tighter_handle->GetMET(*met,
                                            jet, // use all objects (before OR and after corrections) for MET utility
                                            el, mu,
                                            0, // ph, // ph term
                                            0, // tau term
                                            doTST, doJVT,
                                            invis // invisible particles
                                            ));
      ANA_CHECK(m_susytools_Tighter_handle->GetMETSig(*met, myMETsig, doTST, doJVT));
   } else if (METType == 3) {
      ANA_CHECK(
         m_susytools_Tenacious_handle->GetMET(*met,
                                              jet, // use all objects (before OR and after corrections) for MET utility
                                              el, mu,
                                              0, // ph, // ph term
                                              0, // tau term
                                              doTST, doJVT,
                                              invis // invisible particles
                                              ));
      ANA_CHECK(m_susytools_Tenacious_handle->GetMETSig(*met, myMETsig, doTST, doJVT));
   }

   xAOD::MissingETContainer::const_iterator met_it = met->find("Final");

   if (met_it == met->end()) {
      ANA_MSG_ERROR("VBFInv::getMET : No RefFinal inside MET container");
      return EL::StatusCode::FAILURE;
   } else {
      float Etmiss_Etx = (*met_it)->mpx();
      float Etmiss_Ety = (*met_it)->mpy();
      float Etmiss_Et  = sqrt(Etmiss_Etx * Etmiss_Etx + Etmiss_Ety * Etmiss_Ety);
      myMET.SetPxPyPzE(Etmiss_Etx, Etmiss_Ety, 0., Etmiss_Et);
   }

   return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::getTrackMET(std::shared_ptr<xAOD::MissingETContainer> &   met,
                                   std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer *jet,
                                   xAOD::ElectronContainer *el, xAOD::MuonContainer *mu)
{
   met    = std::make_shared<xAOD::MissingETContainer>();
   metAux = std::make_shared<xAOD::MissingETAuxContainer>();
   met->setStore(metAux.get());

   ANA_CHECK(m_susytools_handle->GetTrackMET(*met, // note that we don't need to pass the Aux container here
                                             jet,  // use all objects (before OR and after corrections) for MET utility
                                             el, mu));
   xAOD::MissingETContainer::const_iterator met_it = met->find("Track");

   if (met_it == met->end()) {
      ANA_MSG_ERROR("VBFInv::getMET : No Track inside MET container");
      return EL::StatusCode::FAILURE;
   }

   return EL::StatusCode::SUCCESS;
}

void VBFInv::GetAntiIDSF(Analysis::ContentHolder &content, const xAOD::TruthParticleContainer *truthElectrons,
                         float &antiIdSF)
{
   static SG::AuxElement::Accessor<char> acc_baseline("baseline");
   for (auto el : *content.electrons) {
      if (fabs(el->caloCluster()->eta()) >= 2.47) continue;
      if (el->pt() < 4.5e3) continue;

      // match truth
      bool matchEleTruth = false;
      for (const auto &part : *truthElectrons) {
         if (part->pt() < 2.0e3) continue;
         if (part->status() != 1) continue;
         if (part->p4().DeltaR(el->p4()) < 0.3) {
            matchEleTruth = true;
            break;
         }
      }
      if (!matchEleTruth) continue;

      // compute SF's
      if (acc_baseline(*el) == 1) {
         /* NOT applying isolation, so removing this code!
               Float_t tmp_ptvarcone20;
               el->isolationValue(tmp_ptvarcone20, xAOD::Iso::IsolationType::ptvarcone20);
               if (tmp_ptvarcone20 / el->pt() > 0.30) {
                  double             el_iso_sf = 1.0;
                  CP::CorrectionCode result    = m_elecEfficiencySFTool_anti_iso->getEfficiencyScaleFactor(*el,
            el_iso_sf); switch (result) { case CP::CorrectionCode::Ok: antiIdSF *= el_iso_sf; break; case
            CP::CorrectionCode::Error: ATH_MSG_ERROR("Failed to retrieve signal electron reco SF"); break; case
            CP::CorrectionCode::OutOfValidityRange: ATH_MSG_VERBOSE("OutOfValidityRange found for signal electron reco
            SF"); break; default: ATH_MSG_WARNING("Don't know what to do for signal electron reco SF");
                  }
               }
         */
      } else {
         double             el_pid_sf = 1.0;
         CP::CorrectionCode result    = m_elecEfficiencySFTool_anti_id->getEfficiencyScaleFactor(*el, el_pid_sf);
         switch (result) {
         case CP::CorrectionCode::Ok: antiIdSF *= el_pid_sf; break;
         case CP::CorrectionCode::Error: ATH_MSG_ERROR("Failed to retrieve signal electron reco SF"); break;
         case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE("OutOfValidityRange found for signal electron reco SF");
            break;
         default: ATH_MSG_WARNING("Don't know what to do for signal electron reco SF");
         }
      }
   } // end loop over electrons
}

void VBFInv::printObjects(xAOD::IParticleContainer obj, TString label)
{
   if (obj.size() != 0) {
      ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
      int iobj = 0;
      ANA_MSG_INFO(label << " size is " << obj.size());
      for (auto thisObj : obj) {
         ANA_MSG_INFO(label << " " << iobj << ": pt=" << thisObj->pt() * 0.001 << ", eta=" << thisObj->eta()
                            << ", phi=" << thisObj->phi());
         iobj++;
      }
   }
}

void VBFInv::printMET(TLorentzVector myMET, double myMETsig, TLorentzVector myTruthMET, TString label)
{
   ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
   ANA_MSG_INFO(label << ": met=" << myMET.Perp() * 0.001 << ", phi=" << myMET.Phi());
   ANA_MSG_INFO(label << ": metSig=" << myMETsig);
   if (m_isMC) ANA_MSG_INFO(label << ": truthMET=" << myTruthMET.Perp() * 0.001 << ", truthPhi=" << myTruthMET.Phi());
}

void VBFInv::printMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label)
{
   ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
   const Float_t mpx       = (*met)["Final"]->mpx();
   const Float_t mpy       = (*met)["Final"]->mpy();
   float         Etmiss_Et = sqrt(mpx * mpx + mpy * mpy);
   ANA_MSG_INFO(label << ": met=" << Etmiss_Et * 0.001);
}

void VBFInv::printTrackMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label)
{
   ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
   const Float_t mpx       = (*met)["Track"]->mpx();
   const Float_t mpy       = (*met)["Track"]->mpy();
   float         Etmiss_Et = sqrt(mpx * mpx + mpy * mpy);
   ANA_MSG_INFO(label << ": met=" << Etmiss_Et * 0.001);
}

void VBFInv::print(TString label, float val)
{
   ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
   ANA_MSG_INFO(label << "=" << val);
}
