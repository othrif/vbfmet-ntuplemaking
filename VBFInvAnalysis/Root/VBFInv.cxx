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
#include <EventLoop/OutputStream.h>
#include "PathResolver/PathResolver.h"
#include "AsgTools/AsgMetadataTool.h"

// Local include(s):
#include <VBFInvAnalysis/VBFInv.h>
#include <VBFInvAnalysis/HelperFunctions.h>
#include <VBFInvAnalysis/GetTruthBosonP4.h>

// Utils
#include <boost/algorithm/string.hpp>

// Declare the class to ROOT:
ClassImp( VBFInv )

// Unchecked status codes become error:
#ifdef ROOTCORE
#include <xAODRootAccess/tools/TReturnCode.h>
#include <AsgTools/StatusCode.h>
#endif
#include <PATInterfaces/CorrectionCode.h>
#include <PATInterfaces/SystematicCode.h>

VBFInv :: VBFInv ()
:
debug(false),
verbose(false),
config_file(""),
ST_config_file(""),
ST_config_Tight_file(""),
ST_config_Tighter_file(""),
ST_config_Tenacious_file(""),
prw_file(""),
lumicalc_file(""),
GRL_file(""),
MC_campaign(""),
skip_syst(""),
trigger_list(""),
pt1Skim(0),
pt1SkimForSyst(0),
pt2Skim(0),
pt2SkimForSyst(0),
metSkim(0),
metSkimForSyst(0),
mjjSkim(0),
mjjSkimForSyst(0),
detajjSkim(0),
detajjSkimForSyst(0),
rebalancedJetPt(20000.),
doPileup(true),
doSystematics(false),
doSkim(false),
doTrim(false),
doRnS(false),
doElectronDetail(false),
doMuonDetail(false),
doJetDetail(false),
doMETDetail(false),
doEventDetail(false),
m_isMC(false),
m_isAFII(false),
m_eventCounter(0),
m_determinedDerivation(false),
m_isEXOT5(false),
m_grl("GoodRunsListSelectionTool/grl", this),
m_susytools_handle("ST::SUSYObjDef_xAOD/ST", this),
m_susytools_Tight_handle("ST::SUSYObjDef_xAOD/STTight", this),
m_susytools_Tighter_handle("ST::SUSYObjDef_xAOD/STTighter", this),
m_susytools_Tenacious_handle("ST::SUSYObjDef_xAOD/STTenacious", this),
m_jetFwdJvtTool("JetForwardJvtTool/JetForwardJvtTool_VBF", this)
{
}

EL::StatusCode VBFInv::setupJob( EL::Job& job ) {

  ANA_CHECK_SET_TYPE (EL::StatusCode);
  ANA_MSG_INFO ("in setupJob");

  job.useXAOD();

   // Return gracefully:
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv :: histInitialize()
{

 ANA_MSG_INFO("in histInitialize");

   // Events processed before derivation
 m_NumberEvents = new TH1D("NumberEvents", "Number Events", 3, 0, 3);
 m_NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
 m_NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
 m_NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");

  // CutFlow
 TString NameCut("Nominal");
 m_CutFlow = Analysis::CutFlowTool(NameCut);
 m_CutFlow.addCut("Pre-Derivation");
 m_CutFlow.addCut("Processed");
 m_CutFlow.addCut("GRL");
 m_CutFlow.addCut("Vertex");
// m_CutFlow.addCut("Trigger");
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

EL::StatusCode VBFInv :: fileExecute()
{

 ANA_MSG_INFO("in fileExecute");


 m_event = wk()->xaodEvent();
 ANA_MSG_INFO( "Number of events in this file = " << m_event->getEntries() );

    //Read the CutBookkeeper container
 const xAOD::CutBookkeeperContainer* completeCBC = 0;
 if (!m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()) {
  ANA_MSG_ERROR( "Failed to retrieve CutBookkeepers from MetaData! Exiting.");
}

const xAOD::CutBookkeeper* allEventsCBK = 0;
int maxcycle = -1;
    //let's find the right CBK (latest with StreamAOD input before derivations)
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
  ANA_MSG_INFO( "CutBookkeepers Accepted " << nEventsProcessed << " SumWei " << sumOfWeights << " sumWei2 " << sumOfWeightsSquared);
  m_CutFlow.addContent(0, nEventsProcessed, sumOfWeights);
} else {
  ANA_MSG_INFO( "No relevant CutBookKeepers found" );
}


if(m_event->getEntries() && wk()->metaData()->castDouble("isData") != 1 ){

  const xAOD::EventInfo *eventInfo = nullptr;
  ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));
  m_NumberEvents->Fill(0., nEventsProcessed);
  m_NumberEvents->Fill(1., sumOfWeights);
  m_NumberEvents->Fill(2., sumOfWeightsSquared);
}


return EL::StatusCode::SUCCESS;
}


EL::StatusCode VBFInv::initialize() {

 ANA_CHECK_SET_TYPE (EL::StatusCode);
 ANA_MSG_INFO( std::fixed << std::setprecision(2) );
 ANA_MSG_INFO ("in initialize");

  // Read configuration
 ANA_CHECK(readConfig());

 m_isMC = !wk()->metaData()->castDouble("isData");
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
 ANA_MSG_INFO("  - pt1Skim = "    << pt1Skim    << " MeV ( " << pt1SkimForSyst    << " MeV for systematics)");
 ANA_MSG_INFO("  - pt2Skim = "    << pt1Skim    << " MeV ( " << pt2SkimForSyst    << " MeV for systematics)");
 ANA_MSG_INFO("  - metSkim = "    << metSkim    << " MeV ( " << metSkimForSyst    << " MeV for systematics)");
 ANA_MSG_INFO("  - mjjSkim = "    << mjjSkim    << " MeV ( " << mjjSkimForSyst    << " MeV for systematics)");
 ANA_MSG_INFO("  - detajjSkim = " << detajjSkim << " ( " << detajjSkimForSyst << " for systematics)");
 ANA_MSG_INFO("  - skip_syst = " << skip_syst);
 ANA_MSG_INFO("  - trigger_list = " << trigger_list);
 ANA_MSG_INFO("  - showerType = " << showerType);
 ANA_MSG_INFO("  - debug = " << debug);

 // Event counter
 m_eventCounter = 0;

 // GRL
 std::vector<std::string> vecStringGRL;
 vecStringGRL.push_back(PathResolverFindCalibFile("GoodRunsLists/data15_13TeV/20170619/physics_25ns_21.0.19.xml"));// 2015 GRL, R21 (3219.56 pb-1)
 vecStringGRL.push_back(PathResolverFindCalibFile("GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19.xml"));// 2016 GRL, R21 (32988.1 pb-1)
 vecStringGRL.push_back(PathResolverFindCalibFile("GoodRunsLists/data17_13TeV/20180309/physics_25ns_Triggerno17e33prim.xml")); // 2017 GRL, R21 (43593.8 pb-1)
 ANA_CHECK(m_grl.setProperty( "GoodRunsListVec", vecStringGRL));
 ANA_CHECK(m_grl.setProperty("PassThrough", false)); // if true (default) will ignore result of GRL and will just pass all events
 ANA_CHECK(m_grl.initialize());

 // configure forward JVT tool
 //m_jetFwdJvtTool.setTypeAndName("JetForwardJvtTool/JetForwardJvtTool_VBF");
 ANA_CHECK( m_jetFwdJvtTool.setProperty("OutputDec", "passFJvt") ); //Output decoration
 ANA_CHECK( m_jetFwdJvtTool.setProperty("UseTightOP", true) ); // Tight
 ANA_CHECK( m_jetFwdJvtTool.setProperty("EtaThresh", 2.5) );
 ANA_CHECK( m_jetFwdJvtTool.setProperty("ForwardMaxPt", 120.0e3) );
 ANA_CHECK( m_jetFwdJvtTool.retrieve() );

  // SUSYTools
 const ST::ISUSYObjDef_xAODTool::DataSource datasource = (!m_isMC ? ST::ISUSYObjDef_xAODTool::Data : (m_isAFII ? ST::ISUSYObjDef_xAODTool::AtlfastII : ST::ISUSYObjDef_xAODTool::FullSim));
 ANA_CHECK(m_susytools_handle.setProperty("DataSource", datasource));
 ANA_CHECK(m_susytools_handle.setProperty("ConfigFile", ST_config_file.Data()));
 ANA_CHECK(m_susytools_Tight_handle.setProperty("DataSource", datasource));
 ANA_CHECK(m_susytools_Tight_handle.setProperty("ConfigFile", ST_config_Tight_file.Data()));
 ANA_CHECK(m_susytools_Tighter_handle.setProperty("DataSource", datasource));
 ANA_CHECK(m_susytools_Tighter_handle.setProperty("ConfigFile", ST_config_Tighter_file.Data()));
 ANA_CHECK(m_susytools_Tenacious_handle.setProperty("DataSource", datasource));
 ANA_CHECK(m_susytools_Tenacious_handle.setProperty("ConfigFile", ST_config_Tenacious_file.Data()));

 if(verbose){
   ANA_CHECK( m_susytools_handle.setProperty("outLevel", MSG::VERBOSE));
   ANA_CHECK( m_susytools_handle.setProperty("DebugMode", true));
 }
  // Pile up configuration
 xAOD::TEvent *event = wk()->xaodEvent();
 if(doPileup && m_isMC){
      // get metadata
  const xAOD::FileMetaData* fmd = nullptr;
  std::string amiTag = "unknown";
  if( event->retrieveMetaInput( fmd, "FileMetaData" ).isSuccess() ) {
   if( !fmd->value( xAOD::FileMetaData::amiTag, amiTag ) ) {
     ATH_MSG_ERROR( "The object is available, the variable is not." );
   }
 } else {
   ATH_MSG_ERROR( "The object is not available." );
 }
 ANA_MSG_INFO("\n AMI Tag: " << amiTag);

 const xAOD::EventInfo *eventInfo = nullptr;
 ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));
 std::vector<std::string> prw_conf;
 std::vector<std::string> prw_lumicalc;
 std::string mc_campaign;
 std::string simType = (m_isAFII ? "AFII" : "FS");
 uint32_t runNum = eventInfo->runNumber();
 switch(runNum) {
  case 284500 :
  mc_campaign="mc16a";
        prw_lumicalc.push_back(PathResolverFindCalibFile("GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root")); // 2015 LumiCalc
        prw_lumicalc.push_back(PathResolverFindCalibFile("GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root")); // 2016 LumiCalc
        break;
        case 300000 :
        if( amiTag.find("r10201")!=std::string::npos)
          mc_campaign = "mc16d";
        else
          mc_campaign = "mc16c";
        prw_lumicalc.push_back(PathResolverFindCalibFile("GoodRunsLists/data17_13TeV/20180309/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root")); // 2017 LumiCalc
        prw_conf.push_back(PathResolverFindCalibFile("GoodRunsLists/data17_13TeV/20180309/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root")); // 2017 ActualMu
        break;
      }
      std::string prwConfigFile = "dev/SUSYTools/PRW_AUTOCONFIG_SIM/files/pileup_" + mc_campaign + "_dsid" + std::to_string(eventInfo->mcChannelNumber()) + "_" + simType + ".root";
      prwConfigFile = PathResolverFindCalibFile(prwConfigFile);
      if(prwConfigFile.empty()) {
        std::vector<std::string> prw_conf_tmp = getTokens(prw_file, ",");
        std::transform(prw_conf_tmp.begin(), prw_conf_tmp.end(), prw_conf_tmp.begin(), [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
        ANA_MSG_INFO( "File " << prwConfigFile << " not found in ST database. Set manual prw configuration with the following files:" );
        for (uint i = 0; i< prw_conf_tmp.size(); i++) {
          ANA_MSG_INFO( prw_conf_tmp.at(i) );
          if (prw_conf_tmp.at(i).find(mc_campaign) != std::string::npos) prw_conf.push_back(prw_conf_tmp.at(i));}
          std::transform(prw_conf.begin(), prw_conf.end(), prw_conf.begin(), [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
        }
        else {
          prw_conf.push_back(prwConfigFile);
          std::transform(prw_conf.begin(), prw_conf.end(), prw_conf.begin(), [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
        }

        ANA_CHECK(m_susytools_handle.setProperty("PRWConfigFiles", prw_conf) );
        ANA_CHECK(m_susytools_handle.setProperty("PRWLumiCalcFiles", prw_lumicalc) );
        ANA_CHECK(m_susytools_handle.setProperty("mcCampaign", mc_campaign));
      }

      //Guess shower type for btagging MC/MC SFs
      if (m_isMC)
       ANA_CHECK(m_susytools_handle.setProperty("ShowerType", (Int_t)showerType));

   //
   // list of systematics to process
   //

     m_sysList.clear();
     if (doSystematics && m_isMC) {
      auto fullSystList = m_susytools_handle->getSystInfoList();

      // list of uncertainties to be skipped
      std::vector<std::string> skip = getTokens(skip_syst, ",");

      for (const auto& syst : fullSystList) {
       const TString thisSyst = syst.systset.name();
       Bool_t keepThis(kTRUE);
       for (const auto& toSkip : skip) {
        if (thisSyst.Contains(toSkip)) {
         ANA_MSG_INFO("Skipping the systematic variation \"" << thisSyst << "\"");
         keepThis = kFALSE;
         break;
       }
         } // loop over skip systematics

         if (keepThis) {
          ANA_MSG_INFO("********** Processing systematic variation: \"" << thisSyst << "\"");
          m_sysList.push_back(syst);
        }
      }
    } else {

      ANA_MSG_INFO("********** Processing only nominal: \"\"" );
      ST::SystInfo nominal;
      nominal.affectsKinematics = false;
      nominal.affectsWeights = false;
      nominal.affectsType = ST::Unknown;
      m_sysList.push_back(nominal);
    }

   //
   // define the output TTree
   //
    m_tree.clear();
   //m_cand.clear();

  //
  // TTree
  //
    ANA_MSG_INFO("Tree output name is " << outputName);
    ANA_MSG_INFO("Histogram output name is " << outputNameHist);
    TFile *outputFile = wk()->getOutputFile (outputName);
    TFile *outputFileHist = wk()->getOutputFile(outputNameHist);

  //
  // Histograms
  //
    if(m_event->getEntries() && wk()->metaData()->castDouble("isData") != 1){
      m_NumberEventsinNtuple = m_NumberEvents;
      m_NumberEvents->SetDirectory(outputFileHist);
      m_NumberEventsinNtuple->SetDirectory(outputFile);
    }


    for (const auto& syst : m_sysList) {
      const TString thisSyst = syst.systset.name();
      const TString treeName = (thisSyst == "") ? "MiniNtuple" : ("MiniNtuple_" + thisSyst).ReplaceAll(" ", "_");
      const TString treeTitle = thisSyst;
      ANA_MSG_INFO("Will consider the systematic variation \"" << thisSyst << "\"");

      const Bool_t isNominal = (thisSyst == "");
      const Bool_t trim = (!isNominal || doTrim || doElectronDetail || doMuonDetail || doJetDetail || doMETDetail || doEventDetail || doRnS);

      ANA_MSG_INFO("Creating TTree named " <<  treeName.Data() << " for systematic named \"" << thisSyst.Data() << "\"");

      m_tree[thisSyst] = new TTree(treeName, treeTitle);
      m_tree[thisSyst]->SetDirectory(outputFile);

      // save, in all trees, the trigger decision
      std::vector<std::string> triggersToSave = getTokens(trigger_list, ",");
      for (uint i = 0; i< triggersToSave.size();i++)    boost::algorithm::trim(triggersToSave.at(i));
        if(debug) for (uint i = 0; i< triggersToSave.size();i++) ANA_MSG_INFO("Save trigger decision of: " << triggersToSave.at(i));
      m_cand[thisSyst].evt.setTriggers(triggersToSave);

      // define all elements of the output tree
      m_cand[thisSyst].met["met_tst"] = Analysis::outMET("met_tst", (trim && !doMETDetail) );
      m_cand[thisSyst].met["met_tst_nolep"] = Analysis::outMET("met_tst_nolep", (trim && !doMETDetail));
      m_cand[thisSyst].met["met_soft_tst"] = Analysis::outMET("met_soft_tst", (trim && !doMETDetail));
      
      if(doMETDetail){
	m_cand[thisSyst].met["met_tight_tst"] = Analysis::outMET("met_tight_tst", (trim && !doMETDetail) );
	m_cand[thisSyst].met["met_tight_tst_nolep"] = Analysis::outMET("met_tight_tst_nolep", (trim && !doMETDetail));
	m_cand[thisSyst].met["met_tighter_tst"] = Analysis::outMET("met_tighter_tst", (trim && !doMETDetail) );
	m_cand[thisSyst].met["met_tighter_tst_nolep"] = Analysis::outMET("met_tighter_tst_nolep", (trim && !doMETDetail));
	m_cand[thisSyst].met["met_tenacious_tst"] = Analysis::outMET("met_tenacious_tst", (trim && !doMETDetail) );
	m_cand[thisSyst].met["met_tenacious_tst_nolep"] = Analysis::outMET("met_tenacious_tst_nolep", (trim && !doMETDetail));
      }

      //      m_cand[thisSyst].met["met_cst"] = Analysis::outMET("met_cst", (trim && !doMETDetail) );
      m_cand[thisSyst].met["met_track"] = Analysis::outMET("met_track", (trim && !doMETDetail));
      m_cand[thisSyst].met["met_truth"] = Analysis::outMET("met_truth", (trim && !doMETDetail));
      m_cand[thisSyst].mu["mu"] = Analysis::outMuon("mu", (trim && !doMuonDetail));
      if(doMuonDetail) m_cand[thisSyst].mu["basemu"] = Analysis::outMuon("basemu", (trim && !doMuonDetail));
      m_cand[thisSyst].el["el"] = Analysis::outElectron("el", (trim && !doElectronDetail) );
      if(doElectronDetail) m_cand[thisSyst].el["baseel"] = Analysis::outElectron("baseel", (trim && !doElectronDetail) );
      m_cand[thisSyst].jet["jet"] = Analysis::outJet("jet", (trim && !doJetDetail));
      m_cand[thisSyst].tau["tau"] = Analysis::outTau("tau", trim);

      // Set trimming option for remaning outHolder objects
      m_cand[thisSyst].evt.setDoTrim((trim && !doEventDetail));
      m_cand[thisSyst].rns.setDoTrim((trim && !doRnS));
      //m_cand[thisSyst].setDoTrim(trim); // this forces trimming for all objects
      m_cand[thisSyst].attachToTree(m_tree[thisSyst]);
    }

    return EL::StatusCode::SUCCESS;
  }

  EL::StatusCode VBFInv::execute() {

   ANA_CHECK_SET_TYPE (EL::StatusCode);

// Fail if unchecked status code
// StatusCode::enableFailure();
// CP::SystematicCode::enableFailure();
// CP::CorrectionCode::enableFailure();

   if(debug){
    ANA_MSG_INFO ("====================================================================");
    ANA_MSG_INFO ("in execute, on entry " << wk()->treeEntry());
  }

  bool passExp = false;
  for (int i=0;i<9;i++){
    int exponent=pow(10,i);
    passExp |= (m_eventCounter <= exponent && (m_eventCounter % exponent) == 0);
  }
  if ( passExp )
    ANA_MSG_INFO("Event number = " << m_eventCounter);
  m_eventCounter++;

   // loop on systematic variations
   Bool_t isFirstIteration = kTRUE; // first iteration is nominal (strictly needed!)

   for (const auto& systInfo : m_sysList) {
    const TString systName = systInfo.systset.name();

    if (isFirstIteration) {
     m_content_nominal.isNominal = kTRUE;
   } else {
         m_content_current = m_content_nominal; // copy from nominal
         m_content_current.isNominal = kFALSE;
       }
       Analysis::ContentHolder &content = (isFirstIteration) ? m_content_nominal : m_content_current;
       analyzeEvent(content, systInfo, m_cand[systName]);
       isFirstIteration = kFALSE;
     }

     return EL::StatusCode::SUCCESS;
   }

   EL::StatusCode VBFInv::finalize() {

     ANA_CHECK_SET_TYPE (EL::StatusCode);
     ANA_MSG_INFO ("in finalize");

     return EL::StatusCode::SUCCESS;
   }

   EL::StatusCode VBFInv :: histFinalize()
   {

     m_CutFlow.printUnw();
     m_CutFlow.print();

  // Cutflow
     m_cflow_hist = m_CutFlow.createTH1F();
     m_cflow_hist_unw = m_CutFlow.createTH1Fraw();

     TFile *outputFile = wk()->getOutputFile (outputName);
     m_cflow_hist->SetDirectory(outputFile);
     m_cflow_hist_unw->SetDirectory(outputFile);

     return EL::StatusCode::SUCCESS;
   }

   EL::StatusCode VBFInv :: readConfig()
   {
     const std::string readFrom = PathResolverFindCalibFile(config_file.Data());
     ANA_MSG_INFO("Reading from file \"" << readFrom << "\"");
     TEnv env;
     if (env.ReadFile(readFrom.c_str(), kEnvAll) != 0) {
      ANA_MSG_ERROR("Unable to read configuration file from PathResolverFindCalibFile of input \"" << config_file << "\"");
      return EL::StatusCode::FAILURE;
    }

    ST_config_file = env.GetValue("VBF.ST_config_file", "EMPTY");
    ST_config_Tight_file = env.GetValue("VBF.ST_config_Tight_file", "EMPTY");
    ST_config_Tighter_file = env.GetValue("VBF.ST_config_Tighter_file", "EMPTY");
    ST_config_Tenacious_file = env.GetValue("VBF.ST_config_Tenacious_file", "EMPTY");
    prw_file = env.GetValue("VBF.prw_file", "EMPTY");
    lumicalc_file = env.GetValue("VBF.lumicalc_file", "EMPTY");
    GRL_file = env.GetValue("VBF.GRL_file", "EMPTY");
    MC_campaign = env.GetValue("VBF.MC_campaign", "EMPTY");
    skip_syst = env.GetValue("VBF.skip_syst", "");
    trigger_list = env.GetValue("VBF.trigger_list", "");
    pt1Skim = env.GetValue("VBF.pt1Skim", 0);
    pt2Skim = env.GetValue("VBF.pt2Skim", 0);
    metSkim = env.GetValue("VBF.metSkim", 0);
    mjjSkim = env.GetValue("VBF.mjjSkim", 0);
    detajjSkim = env.GetValue("VBF.detajjSkim", 0);
    pt1SkimForSyst = env.GetValue("VBF.ptSkimForSyst", 0);
    pt2SkimForSyst = env.GetValue("VBF.ptSkimForSyst", 0);
    metSkimForSyst = env.GetValue("VBF.metSkimForSyst", 0);
    mjjSkimForSyst = env.GetValue("VBF.mjjSkimForSyst", 0);
    detajjSkimForSyst = env.GetValue("VBF.detajjSkimForSyst", 0);

    return EL::StatusCode::SUCCESS;
  }

  std::vector<std::string> VBFInv :: getTokens(TString line, TString delim)
  {
   std::vector<std::string> vtokens;
   TObjArray* tokens = TString(line).Tokenize(delim); //delimiters
   if (tokens->GetEntriesFast()) {
    TIter iString(tokens);
    TObjString* os = 0;
    while ((os = (TObjString*)iString())) {
     vtokens.push_back(os->GetString().Data());
   }
 }
 delete tokens;

 return vtokens;
}

EL::StatusCode VBFInv :: analyzeEvent(Analysis::ContentHolder &content, const ST::SystInfo &systInfo, Analysis::outHolder &cand)
{
  xAOD::TEvent *event = wk()->xaodEvent();

  // Initialize SUSYTools
  ANA_CHECK(m_susytools_handle->ApplyPRWTool());

  // event selection using a given systematic variation
  const Bool_t syst_affectsElectrons = ST::testAffectsObject(xAOD::Type::Electron, systInfo.affectsType);
  const Bool_t syst_affectsMuons     = ST::testAffectsObject(xAOD::Type::Muon,     systInfo.affectsType);
  const Bool_t syst_affectsJets      = ST::testAffectsObject(xAOD::Type::Jet,      systInfo.affectsType);
  const Bool_t syst_affectsPhotons   = ST::testAffectsObject(xAOD::Type::Photon,   systInfo.affectsType);
  const Bool_t syst_affectsTaus      = ST::testAffectsObject(xAOD::Type::Tau, systInfo.affectsType);

   // apply systematic uncertainty
  const CP::SystematicSet& sys = systInfo.systset;
  if (m_susytools_handle->applySystematicVariation(sys) != CP::SystematicCode::Ok) {
    ANA_MSG_ERROR("Cannot configure SUSYTools for systematic var. " << sys.name().c_str());
    return EL::StatusCode::FAILURE;}

   // determine which objects should be retrieved
    content.doElectrons = (syst_affectsElectrons || content.isNominal);
    content.doMuons = (syst_affectsMuons || content.isNominal);
    content.doJets = (syst_affectsJets || content.isNominal);
    content.doPhotons = (syst_affectsPhotons || content.isNominal); // needed for overlap removal
    content.doTaus = (syst_affectsTaus || content.isNominal);
    content.doMET = kTRUE;
    content.doOverlapRemoval = (content.doElectrons || content.doMuons || content.doJets  || content.doPhotons || content.doMET || content.doTaus);

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
      ANA_MSG_FATAL("Mismatch between isMC (" << m_isMC << ") and xAOD::EventInfo::IS_SIMULATION (" << xAOD::EventInfo::IS_SIMULATION << ")");
      return EL::StatusCode::FAILURE;
    }
    int runNbr = content.eventInfo->runNumber();
    int evtNbr = content.eventInfo->eventNumber();
    if(debug)
      ANA_MSG_INFO ("in execute, runNumber = " << runNbr << ", eventNumber = " << evtNbr);

    float event_weight = 1.;
    if(m_isMC){
      event_weight *=  content.eventInfo->mcEventWeight();
      event_weight *=  m_susytools_handle->GetPileupWeight();
    }

  //-----------------------------------------------------------------------
  // Event cleaning
  //-----------------------------------------------------------------------

    // no cuts
    m_CutFlow.hasPassed(VBFInvCuts::Processed, event_weight);

    // grl
    Bool_t passesGRL = (m_isMC || m_grl->passRunLB(*content.eventInfo));
    if (!m_isMC && !passesGRL && doSkim) {
     if(debug) {
      ANA_MSG_INFO("drop event: GRL");
    }
         return EL::StatusCode::SUCCESS; // go to next event
       }
       m_CutFlow.hasPassed(VBFInvCuts::GRL, event_weight);
       content.passGRL = passesGRL;

  // primary vertex
       Bool_t passesVertex(kFALSE);
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
     if (!passesVertex && doSkim)
      return EL::StatusCode::SUCCESS;
    m_CutFlow.hasPassed(VBFInvCuts::Vertex, event_weight);
    content.passPV = passesVertex;

    // Trigger
    /*
    Bool_t passesTrigger(kTRUE);
    if (!passesTrigger && doSkim)
      return EL::StatusCode::SUCCESS;
    m_CutFlow.hasPassed(VBFInvCuts::Trigger, event_weight);
    content.passTrigger = passesTrigger;
    */

    // detector cleaning
    // https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataPreparationCheckListForPhysicsAnalysis
    Bool_t passesDetErr = (m_isMC || !((content.eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error ) ||
     (content.eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error ) ||
     (content.eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error) ||
     (content.eventInfo->eventFlags(xAOD::EventInfo::Core) & 0x40000)
     ));
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

   //-- JETS --
      if (content.doJets) {
        content.jets = nullptr;
        content.jetsAux = nullptr;
        content.allJets.clear(SG::VIEW_ELEMENTS);
        ANA_CHECK(m_susytools_handle->GetJets(content.jets, content.jetsAux, kTRUE));
	m_jetFwdJvtTool->modify(*content.jets); // add fjvt
        for (auto jet : *content.jets) {
         if (acc_baseline(*jet) == 1) {
          content.allJets.push_back(jet);
        }
      }
      content.allJets.sort(&HelperFunctions::comparePt);  //sort by Pt
   } // done with jets

   //-- MUONS --
   if (content.doMuons) {
    content.muons = nullptr;
    content.muonsAux = nullptr;
    content.allMuons.clear(SG::VIEW_ELEMENTS);
    ANA_CHECK(m_susytools_handle->GetMuons(content.muons, content.muonsAux, kTRUE));
    for (auto mu : *content.muons) {
     //static SG::AuxElement::Accessor<char> acc_baseline("baseline");
     if (acc_baseline(*mu) == 1) {
      // calculate d0 and z0
      const xAOD::Vertex* pv = m_susytools_handle->GetPrimVtx();
      const Float_t primary_vertex_z = pv ? pv->z() : 0;
      //content.vtx_sumpt2 = sumPt2(*pv);
      dec_new_d0(*mu) = HelperFunctions::getD0(mu);
      dec_new_d0sig(*mu) = HelperFunctions::getD0sig(mu, content.eventInfo);
      dec_new_z0(*mu) = HelperFunctions::getZ0(mu, primary_vertex_z);
      dec_new_z0sig(*mu) = HelperFunctions::getZ0sig(mu);
      if(debug)
        ANA_MSG_INFO("d0  >>  " << HelperFunctions::getD0sig(mu, content.eventInfo) << ", z0 >> " << HelperFunctions::getZ0(mu, primary_vertex_z));
      content.allMuons.push_back(mu);
    }
  }
  content.allMuons.sort(&HelperFunctions::comparePt);
    } // done with muons

   //-- ELECTRONS --
    if (content.doElectrons) {
      content.electrons = nullptr;
      content.electronsAux = nullptr;
      content.allElectrons.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetElectrons(content.electrons, content.electronsAux, kTRUE));

      for (auto el : *content.electrons) {
        //static SG::AuxElement::Accessor<char> acc_baseline("baseline");
        if (acc_baseline(*el) == 1) {
          // impact parameters
          const xAOD::Vertex* pv = m_susytools_handle->GetPrimVtx();
          const Float_t primary_vertex_z = pv ? pv->z() : 0;
          dec_new_d0(*el) = HelperFunctions::getD0(el);
          dec_new_d0sig(*el) = HelperFunctions::getD0sig(el, content.eventInfo);
          dec_new_z0(*el) = HelperFunctions::getZ0(el, primary_vertex_z);
          dec_new_z0sig(*el) = HelperFunctions::getZ0sig(el);
          content.allElectrons.push_back(el);
        }
      }
      content.allElectrons.sort(&HelperFunctions::comparePt);
    } // done with electrons

   //-- PHOTONS --
    if (content.doPhotons) {
      content.photons = nullptr;
      content.photonsAux = nullptr;
      content.allPhotons.clear(SG::VIEW_ELEMENTS);
      ANA_CHECK(m_susytools_handle->GetPhotons(content.photons, content.photonsAux, kTRUE));
      for (auto ph : *content.photons) {
       //static SG::AuxElement::Accessor<char> acc_baseline("baseline");
       //if (acc_baseline(*ph) == 1) {
        content.allPhotons.push_back(ph);
	//}
      }
      content.allPhotons.sort(&HelperFunctions::comparePt);
    }

   //-- TAUS --
    /*
  if (content.doTaus) {
    content.taus = nullptr;
    content.tausAux = nullptr;
    content.allTaus.clear(SG::VIEW_ELEMENTS);
    ANA_CHECK(m_susytools_handle->GetTaus(content.taus, content.tausAux, kTRUE));
    for (auto tau : *content.taus) {
      static SG::AuxElement::Accessor<char> acc_baseline("baseline");
      ANA_MSG_INFO ("tau pt=" << tau->pt()*0.001 );
      if (acc_baseline(*tau) == 1) {
        content.allTaus.push_back(tau);
      }
    }
    content.allTaus.sort(&HelperFunctions::comparePt);
  }
    */

  //-----------------------------------------------------------------------
  // Select good objects after overlap removal
  //-----------------------------------------------------------------------

   //Good objects containers clear
    content.goodMuons.clear(SG::VIEW_ELEMENTS);
    content.baselineMuons.clear(SG::VIEW_ELEMENTS);
    content.goodElectrons.clear(SG::VIEW_ELEMENTS);
    content.baselineElectrons.clear(SG::VIEW_ELEMENTS);
    content.goodJets.clear(SG::VIEW_ELEMENTS);
    content.goodPhotons.clear(SG::VIEW_ELEMENTS);
    content.baselinePhotons.clear(SG::VIEW_ELEMENTS);
    content.goodTaus.clear(SG::VIEW_ELEMENTS);
    content.baselineTaus.clear(SG::VIEW_ELEMENTS);

   //-- Overlap removal --
    m_susytools_handle->OverlapRemoval(content.electrons, content.muons, content.jets, content.photons);

  //-- JETS --
    if(debug){
      for (auto jet : *content.jets) {
        ANA_MSG_INFO ("jet pt=" << jet->pt()*0.001 << ", eta=" << jet->eta() << ", phi=" << jet->phi() << ", bad=" << (acc_bad(*jet)==1) << ", passOR=" << (acc_passOR(*jet)==1) << ", passJVT=" << (acc_passJvt(*jet)==1)<< ", passFJVT=" << (acc_passFJvt(*jet)==1) << " => BASELINE=" << (acc_baseline(*jet)==1) << ", SIGNAL=" << (acc_signal(*jet)==1) << ", No JVT SIGNAL=" << (acc_signal_less_JVT(*jet)==1));
      }
    }

    Float_t mhtx = 0;
    Float_t mhty = 0;
    for (auto jet : content.allJets){
      if (acc_passOR(*jet) == 1 && acc_bad(*jet) == 0){
        mhtx += jet->pt() * TMath::Cos(jet->phi());
        mhty += jet->pt() * TMath::Sin(jet->phi());
        if (acc_signal(*jet) == 1)
          content.goodJets.push_back(jet);
      }
    }
  // Calculated MHT by hand
    double mht = sqrt(mhtx*mhtx+mhty*mhty);

  //-- MUONS --
    for (auto muon : content.allMuons)
     if (acc_passOR(*muon) == 1 && acc_cosmic(*muon) == 0 && acc_bad(*muon) == 0) {
      content.baselineMuons.push_back(muon);
      if (acc_signal(*muon) == 1){
      content.goodMuons.push_back(muon); // CR muons
      }
  }

  //-- ELECTRONS --
  for (auto electron : content.allElectrons)
   if (acc_passOR(*electron) == 1) {
    content.baselineElectrons.push_back(electron);
    if (acc_signal(*electron) == 1)
      content.goodElectrons.push_back(electron); // CR electrons
  }

  //-- PHOTONS --
  for (auto photon : content.allPhotons)
   if (acc_passOR(*photon) == 1) {
    content.baselinePhotons.push_back(photon);
    if (acc_signal(*photon) == 1)
     content.goodPhotons.push_back(photon);
 }

  //-- TAUS --
  // No selection
  
 for (auto tau : content.allTaus) {
   content.goodTaus.push_back(tau);
 }

//-- MET --

// real MET, with el mu ph soft
 TLorentzVector myMET_tst;
 double myMETsig_tst;
 getMET(content.met_tst,
  content.met_tstAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          content.electrons,
          content.muons,
          content.photons,
          kTRUE, // do TST
          kTRUE, // do JVT
          nullptr, // invisible particles
          myMET_tst,
	myMETsig_tst,
          0);
 content.metsig_tst = myMETsig_tst;

 TLorentzVector myMET_Tight_tst;
 double myMETsig_Tight_tst;
 getMET(content.met_tight_tst,
	content.met_tight_tstAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, nullptr,myMET_Tight_tst,myMETsig_Tight_tst,1);
 TLorentzVector myMET_Tighter_tst;
 double myMETsig_Tighter_tst;
 getMET(content.met_tighter_tst,
	content.met_tighter_tstAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, nullptr,myMET_Tighter_tst,myMETsig_Tighter_tst,2);
 TLorentzVector myMET_Tenacious_tst;
 double myMETsig_Tenacious_tst;
 getMET(content.met_tenacious_tst,
	content.met_tenacious_tstAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, nullptr,myMET_Tenacious_tst,myMETsig_Tenacious_tst,3);

 double met_tst_j1_dphi=-1., met_tst_j2_dphi=-1.;
 HelperFunctions::computeMETj(myMET_tst, content.goodJets, met_tst_j1_dphi, met_tst_j2_dphi);
 if (debug){
   print("met_tst_j1_dphi: ", met_tst_j1_dphi);
   print("met_tst_j2_dphi: ", met_tst_j2_dphi);
 }
 content.met_tst_j1_dphi = met_tst_j1_dphi;
 content.met_tst_j2_dphi = met_tst_j2_dphi;

 // invisible particles
 //DataVector<const xAOD::IParticle *> invis;
 xAOD::IParticleContainer invis(SG::VIEW_ELEMENTS);
 for (auto muon : content.goodMuons) {
   //invis.push_back((const xAOD::IParticle*) muon);
   invis.push_back(muon);
 }
 for (auto electron : content.goodElectrons) {
   //invis.push_back((const xAOD::IParticle*) electron);
   invis.push_back(electron);
 }
// MET, with invisble leptons
 TLorentzVector myMET_tst_nolep;
 double myMETsig_tst_nolep;
 getMET(content.met_tst_nolep,
  content.met_tst_nolepAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          content.electrons,
          content.muons,
          content.photons,
          kTRUE, // do TST
          kTRUE, // do JVT
          nullptr, // invisible particles
          myMET_tst_nolep,
	myMETsig_tst_nolep,
          0);
 content.metsig_tst_nolep = myMETsig_tst_nolep;

 TLorentzVector myMET_Tight_tst_nolep;
 double myMETsig_Tight_tst_nolep;
 getMET(content.met_tight_tst_nolep,
	content.met_tight_tst_nolepAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, &invis,myMET_Tight_tst_nolep,myMETsig_Tight_tst_nolep,1);
 TLorentzVector myMET_Tighter_tst_nolep;
 double myMETsig_Tighter_tst_nolep;
 getMET(content.met_tighter_tst_nolep,
	content.met_tighter_tst_nolepAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, &invis,myMET_Tighter_tst_nolep,myMETsig_Tighter_tst_nolep,2);
 TLorentzVector myMET_Tenacious_tst_nolep;
 double myMETsig_Tenacious_tst_nolep;
 getMET(content.met_tenacious_tst_nolep,
	content.met_tenacious_tst_nolepAux,
	content.jets, content.electrons,content.muons,content.photons,kTRUE,kTRUE, &invis,myMET_Tenacious_tst_nolep,myMETsig_Tenacious_tst_nolep,3);
 //std::cout << "tight: " << myMET_Tight_tst.Pt() << " Loose: " << myMET_tst.Pt() << " Tighter: " << myMET_Tighter_tst.Pt() << " Tenacious: " << myMET_Tenacious_tst.Pt() << std::endl; 
    // create sum of muon and electron pts
 {
  Float_t px = 0;
  Float_t py = 0;
  for (auto muon : content.goodMuons) {
   px += muon->pt() * TMath::Cos(muon->phi());
   py += muon->pt() * TMath::Sin(muon->phi());
 }
 for (auto electron : content.goodElectrons) {
   px += electron->pt() * TMath::Cos(electron->phi());
   py += electron->pt() * TMath::Sin(electron->phi());
 }
 const Float_t mpx = (*content.met_tst_nolep)["Final"]->mpx();
 const Float_t mpy = (*content.met_tst_nolep)["Final"]->mpy();
 (*content.met_tst_nolep)["Final"]->setMpx(mpx + px);
 (*content.met_tst_nolep)["Final"]->setMpy(mpy + py);
}

double met_tst_nolep_j1_dphi=-1., met_tst_nolep_j2_dphi=-1.;
HelperFunctions::computeMETj(myMET_tst_nolep, content.goodJets, met_tst_nolep_j1_dphi, met_tst_nolep_j2_dphi);
if (debug){
 print("met_tst_nolep_j1_dphi: ", met_tst_nolep_j1_dphi);
 print("met_tst_nolep_j2_dphi: ", met_tst_nolep_j2_dphi);
}
content.met_tst_nolep_j1_dphi = met_tst_nolep_j1_dphi;
content.met_tst_nolep_j2_dphi = met_tst_nolep_j2_dphi;

 // MET CST, for HT
TLorentzVector myMET_cst;
double myMETsig_cst;
getMET(content.met_cst,
	content.met_cstAux,
	content.jets, // use all objects (before OR and after corrections) for MET utility
	content.electrons,
	content.muons,
	content.photons,
	kFALSE, // do TST
	kFALSE, // do JVT
	nullptr, // invisible particles
	myMET_cst,
  myMETsig_cst
  );

double met_cst_jet = -1.;
met_cst_jet = (*content.met_cst)["RefJet"]->met();
content.met_cst_jet = met_cst_jet;

if(debug) {
  print("MET CST", met_cst_jet*1e-3);
  print("MHT without jVT", mht*1e-3);
}

/*
// MET, with invisble muons
 TLorentzVector myMET_tst_nomuon;
 double myMETsig_tst_nomuon;
 getMET(content.met_tst_nomuon,
  content.met_tst_nomuonAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          content.electrons,
          content.muons,
          content.photons,
          kTRUE, // do TST
          kTRUE, // do JVT
          nullptr, // invisible particles
          myMET_tst_nomuon,
          myMETsig_tst_nomuon
          );
    // create sum of muon pts
 {
  Float_t px = 0;
  Float_t py = 0;
  for (auto muon : content.goodMuons) {
   px += muon->pt() * TMath::Cos(muon->phi());
   py += muon->pt() * TMath::Sin(muon->phi());
 }
 const Float_t mpx = (*content.met_tst_nomuon)["Final"]->mpx();
 const Float_t mpy = (*content.met_tst_nomuon)["Final"]->mpy();
 (*content.met_tst_nomuon)["Final"]->setMpx(mpx + px);
 (*content.met_tst_nomuon)["Final"]->setMpy(mpy + py);
}

 double met_tst_nomuon_j1_dphi=-1., met_tst_nomuon_j2_dphi=-1.;
 HelperFunctions::computeMETj(myMET_tst_nomuon, content.goodJets, met_tst_nomuon_j1_dphi, met_tst_nomuon_j2_dphi);
 if (debug){
   print("met_tst_nomuon_j1_dphi: ", met_tst_nomuon_j1_dphi);
   print("met_tst_nomuon_j2_dphi: ", met_tst_nomuon_j2_dphi);
 }
 content.met_tst_nomuon_j1_dphi = met_tst_nomuon_j1_dphi;
 content.met_tst_nomuon_j2_dphi = met_tst_nomuon_j2_dphi;

// MET, with invisble electrons
TLorentzVector myMET_tst_noelectron;
double myMETsig_tst_noelectron;
getMET(content.met_tst_noelectron,
  content.met_tst_noelectronAux,
          content.jets, // use all objects (before OR and after corrections) for MET utility
          content.electrons,
          content.muons,
          content.photons,
          kTRUE, // do TST
          kTRUE, // do JVT
          nullptr, // invisible particles
          myMET_tst_noelectron,
          myMETsig_tst_noelectron
          );
   // create sum of of electron pts
{
  Float_t px = 0;
  Float_t py = 0;
  for (auto electron : content.goodElectrons) {
   px += electron->pt() * TMath::Cos(electron->phi());
   py += electron->pt() * TMath::Sin(electron->phi());
 }
 const Float_t mpx = (*content.met_tst_noelectron)["Final"]->mpx();
 const Float_t mpy = (*content.met_tst_noelectron)["Final"]->mpy();
 (*content.met_tst_noelectron)["Final"]->setMpx(mpx + px);
 (*content.met_tst_noelectron)["Final"]->setMpy(mpy + py);
}

 double met_tst_noelectron_j1_dphi=-1., met_tst_noelectron_j2_dphi=-1.;
 HelperFunctions::computeMETj(myMET_tst_noelectron, content.goodJets, met_tst_noelectron_j1_dphi, met_tst_noelectron_j2_dphi);
 if (debug){
   print("met_tst_noelectron_j1_dphi: ", met_tst_noelectron_j1_dphi);
   print("met_tst_noelectron_j2_dphi: ", met_tst_noelectron_j2_dphi);
 }
 content.met_tst_noelectron_j1_dphi = met_tst_noelectron_j1_dphi;
 content.met_tst_noelectron_j2_dphi = met_tst_noelectron_j2_dphi;
*/
// track MET
getTrackMET(content.met_track,
 content.met_trackAux,
               content.jets, // use all objects (before OR and after corrections) for MET utility
               content.electrons,
               content.muons
               );

// truth MET
TLorentzVector myTruthMET;
if (m_isMC) {
  content.met_truth = nullptr;
      if (!event->retrieve(content.met_truth, "MET_Truth").isSuccess()) { // retrieve arguments: container type, container key
       ANA_MSG_ERROR("VBFInv::getObjects : Failed to retrieve MET_Truth container");
       return EL::StatusCode::FAILURE;
     }

     xAOD::MissingETContainer::const_iterator met_truth_it = content.met_truth->find("NonInt");

     if (met_truth_it == content.met_truth->end()) {
       ANA_MSG_ERROR("VBFInv::getObjects : No NonInt inside truth MET container");
       return EL::StatusCode::FAILURE;
     }
     else {
      float Etmiss_Etx = (*met_truth_it)->mpx();
      float Etmiss_Ety = (*met_truth_it)->mpx();
      float Etmiss_Et = sqrt(Etmiss_Etx * Etmiss_Etx + Etmiss_Ety * Etmiss_Ety);
      myTruthMET.SetPxPyPzE(Etmiss_Etx, Etmiss_Ety, 0., Etmiss_Et);
    }
  }

  //-----------------------------------------------------------------------
  // Debugs and sanity checks
  //-----------------------------------------------------------------------
  if(debug){
  //-- JETS --
    printObjects(content.allJets, "allJets");
    printObjects(content.goodJets, "goodJets");
  //-- MUONS --
    printObjects(content.allMuons, "allMuons");
    printObjects(content.baselineMuons, "baselineMuons");
    printObjects(content.goodMuons, "goodMuons");
  //-- ELECTRONS --
    printObjects(content.allElectrons, "allElectrons");
    printObjects(content.baselineElectrons, "baselineElectrons");
    printObjects(content.goodElectrons, "goodElectrons");
  //-- MET --
    printMET(myMET_tst, myMETsig_tst, myTruthMET,"MET");
    printMET(content.met_tst_nolep, "MET no leptons");
//    printMET(content.met_tst_noelectron, "MET no electrons");
//    printMET(content.met_tst_nomuon, "MET no muons");
    printTrackMET(content.met_track, "track MET");
    //-- PHOTONS --
    printObjects(content.allPhotons, "photons");
  }

  //-----------------------------------------------------------------------
  // Continue cleaning...
  //-----------------------------------------------------------------------

//asg::AnaToolHandle< ST::ISUSYObjDef_xAODTool >&  handle{"ST::SUSYObjDef_xAOD/SUSYTools"};
const asg::AsgTool* toolPtr = dynamic_cast< const asg::AsgTool* >( m_susytools_handle.get() );
if( ! toolPtr ) {
  // Handle the error...
  std::cout << "Error with tool!" << std::endl;
}
//auto testProp = toolPtr->getProperty< int >( "Jet.InputType" );
//std::cout << testProp << std::endl;

// Jet cleaning
// https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJets2017
  static SG::AuxElement::Accessor<char> acc_eventClean("DFCommonJets_eventClean_LooseBad");
  if(debug)
    print("eventClean_LooseBad", (bool)acc_eventClean(*content.eventInfo));
  Bool_t passesJetCleanLoose = !(acc_eventClean(*content.eventInfo) == 0);
  if( !passesJetCleanLoose && doSkim){
    return EL::StatusCode::SUCCESS;
  }

  // Tight cleaning for EMTopo
  Bool_t passesJetCleanTight = true;
  static SG::AuxElement::Accessor<char> acc_jetCleanTight("DFCommonJets_jetClean_TightBad");
  for (auto jet : content.goodJets) {
    if(debug)
      print("jetClean_TightBad", (bool)acc_jetCleanTight(*jet));
    if (acc_jetCleanTight(*jet) == 0) {
      passesJetCleanTight = false;
    }
  }

  // Tight cleaning for PFlow
  Bool_t passesJetCleanTightCustom = true;
  for (auto jet : content.goodJets) {
    std::vector<float> SumTrkPt500vec;
    SumTrkPt500vec = jet->auxdata<std::vector<float>>("SumPtTrkPt500");
    float SumTrkPt500 = -1;
    if (SumTrkPt500vec.size()>0) SumTrkPt500 = SumTrkPt500vec.at(0);
    float chf = SumTrkPt500/jet->pt();
    float fmax = jet->auxdata<float>("FracSamplingMax");
    if (std::fabs(jet->eta())<2.4 && chf/fmax<0.1) {passesJetCleanTightCustom = false;}
  }

//if (passesJetCleanTightCustom != passesJetCleanTight )
//  std::cout << evtNbr << " cleaning: DFCommonJets_eventClean_LooseBad=" << passesJetCleanLoose << ", DFCommonJets_jetClean_TightBad=" << passesJetCleanTight << " > passesJetCleanTightCustom=" << passesJetCleanTightCustom << std::endl;

  m_CutFlow.hasPassed(VBFInvCuts::JetBad, event_weight);
  content.passJetCleanLoose = passesJetCleanLoose;
  content.passJetCleanTight = passesJetCleanTight;

// Investigating jet cleaning
/*
int val = 0;
if(passesJetCleanLoose)
  val += 1;
if(passesJetCleanTight)
  val += 2;
if(passesJetCleanTightCustom)
  val += 3;
content.passJetCleanLoose = passesJetCleanLoose;
*/

  //-----------------------------------------------------------------------
  // Fill tree
  //-----------------------------------------------------------------------

  // skimming
  const Double_t pt1SkimToUse = (content.isNominal) ? pt1Skim : pt1SkimForSyst;
  const Double_t pt2SkimToUse = (content.isNominal) ? pt2Skim : pt2SkimForSyst;
  const Double_t metSkimToUse = (content.isNominal) ? metSkim : metSkimForSyst;
  const Double_t mjjSkimToUse = (content.isNominal) ? mjjSkim : mjjSkimForSyst;
  const Double_t detajjSkimToUse = (content.isNominal) ? detajjSkim : detajjSkimForSyst;
  //TVector2 met_nomuon_to_use = TVector2((*content.met_tst_nomuon)["Final"]->mpx(), (*content.met_tst_nomuon)["Final"]->mpy());
  //TVector2 met_noelectron_to_use = TVector2((*content.met_tst_noelectron)["Final"]->mpx(), (*content.met_tst_noelectron)["Final"]->mpy());
  TVector2 met_nolep_to_use = TVector2((*content.met_tst_nolep)["Final"]->mpx(), (*content.met_tst_nolep)["Final"]->mpy());
  double detajjMax=-1., mjjMax=-1., dphijjMax=-1.; // not really max here, ?TO DO?
  HelperFunctions::computejj(content.goodJets, mjjMax, detajjMax, dphijjMax);
  if(debug){
    print("Max Mjj", mjjMax);
    print("Max DEta", detajjMax);
    print("Max DPhi", dphijjMax);
  }

  // Skimming
  //Bool_t saveMe = ( met_nomuon_to_use.Mod() > metSkimToUse || met_noelectron_to_use.Mod() > metSkimToUse );
  Bool_t saveMe = ( met_nolep_to_use.Mod() > metSkimToUse );
  if(doMETDetail) saveMe = saveMe || ( (*content.met_tight_tst_nolep)["Final"]->met() > metSkimToUse ) || ( (*content.met_tight_tst)["Final"]->met() > metSkimToUse )
		    || ( (*content.met_tighter_tst_nolep)["Final"]->met() > metSkimToUse ) || ( (*content.met_tighter_tst)["Final"]->met() > metSkimToUse )
		    || ( (*content.met_tenacious_tst_nolep)["Final"]->met() > metSkimToUse ) || ( (*content.met_tenacious_tst)["Final"]->met() > metSkimToUse );
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::MET_skim, event_weight);
  if(content.goodJets.size() < 2 ) // At least two good jet
    return EL::StatusCode::SUCCESS;
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetN_skim, event_weight );
  saveMe &= ( content.goodJets[0]->pt() > pt1SkimToUse );
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetpT1_skim, event_weight );
  saveMe &= ( content.goodJets[1]->pt() > pt2SkimToUse );
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::JetpT2_skim, event_weight );
  saveMe &= ( mjjMax > mjjSkimToUse );
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::Mjj_skim, event_weight);
  saveMe &= ( detajjMax > detajjSkimToUse );
  if(saveMe || !doSkim) m_CutFlow.hasPassed(VBFInvCuts::DEta_skim, event_weight);
  saveMe |= !doSkim; // user flag to skim or not

  if(saveMe)
    fillTree(content, cand);

  if(debug)
    ANA_MSG_INFO ("====================================================================");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::fillTree(Analysis::ContentHolder &content, Analysis::outHolder &cand)
{

 xAOD::TEvent *event = wk()->xaodEvent();

 cand.reset();

 // Deal with Rebalance and Smear first:
 if(doRnS && !m_isMC ) {
  cand.rns.rnsPSweight = cand.rns.getPSweight(m_susytools_handle, event,content.eventInfo->runNumber(), debug);
}

   // trigger
for (auto &kv : cand.evt.trigger) {
  kv.second = m_susytools_handle->IsTrigPassed(kv.first.Data());
}
//cand.evt.passTrigger = -1;
cand.evt.trigger_lep =
// el 2015
cand.evt.trigger["HLT_e24_lhmedium_L1EM20VH"] ||
cand.evt.trigger["HLT_e60_lhmedium"] ||
cand.evt.trigger["HLT_e120_lhloose"] ||
// el 2016
cand.evt.trigger["HLT_e24_lhtight_nod0_ivarloose"] ||
cand.evt.trigger["HLT_e26_lhtight_nod0_ivarloose"] ||
cand.evt.trigger["HLT_e60_lhmedium_nod0"] ||
cand.evt.trigger["HLT_e60_medium"] ||
cand.evt.trigger["HLT_e120_lhloose_nod0"] ||
cand.evt.trigger["HLT_e140_lhloose_nod0"] ||
cand.evt.trigger["HLT_e300_etcut"] ||
// mu 2015
cand.evt.trigger["HLT_mu20_iloose_L1MU15"] ||
cand.evt.trigger["HLT_mu40"] ||
cand.evt.trigger["HLT_mu60_0eta105_msonly"] ||
// mu 2016
cand.evt.trigger["HLT_mu24_iloose"] ||
cand.evt.trigger["HLT_mu24_ivarloose"] ||
cand.evt.trigger["HLT_mu40"] ||
cand.evt.trigger["HLT_mu50"] ||
cand.evt.trigger["HLT_mu24_ivarmedium"] ||
cand.evt.trigger["HLT_mu24_imedium"] ||
cand.evt.trigger["HLT_mu26_ivarmedium"] ||
cand.evt.trigger["HLT_mu26_imedium"];

   // raw event info
cand.evt.runNumber = (m_isMC) ? content.eventInfo->mcChannelNumber() : content.eventInfo->runNumber();
cand.evt.eventNumber = (ULong64_t)content.eventInfo->eventNumber();
cand.evt.lumiBlock = content.eventInfo->lumiBlock();
cand.evt.bcid = content.eventInfo->bcid();
cand.evt.averageIntPerXing = content.eventInfo->averageInteractionsPerCrossing();
cand.evt.corAverageIntPerXing = m_susytools_handle->GetCorrectedAverageInteractionsPerCrossing();

// Which year are we in?
  cand.evt.year = (m_isMC) ? m_susytools_handle->treatAsYear() : 0; // RandomRunNumber from the PRWTool

  Bool_t is2015(kFALSE);
  Bool_t is2016(kFALSE);
/*  Bool_t is2017(kFALSE);
  Bool_t is2018(kFALSE);*/

   if ((cand.evt.year == 0 && cand.evt.runNumber >= 276262 && cand.evt.runNumber <= 284484) || cand.evt.year == 2015) is2015 = kTRUE; // data2015
   if ((cand.evt.year == 0 && cand.evt.runNumber >= 296939 && cand.evt.runNumber<=311481 ) || cand.evt.year == 2016) is2016 = kTRUE; // data2016
/*   if ((cand.evt.year == 0 && cand.evt.runNumber >= 324320 && cand.evt.runNumber<=341649 ) || cand.evt.year == 2017) is2017 = kTRUE; // data2017
  if ((cand.evt.year == 0 && cand.evt.runNumber >= 348197) || cand.evt.year == 2018) is2018 = kTRUE; // data2018*/

// Testing trigger configuration:
/*cand.evt.trigger_met = (is2015 && cand.evt.trigger["HLT_xe70_mht"]) ||
               (is2016 && ( cand.evt.trigger["HLT_xe90_mht_L1XE50"] || cand.evt.trigger["HLT_xe110_mht_L1XE50"] )) ||
               cand.evt.trigger["HLT_noalg_L1J400"] ;*/
   cand.evt.randomRunNumber = m_susytools_handle->GetRandomRunNumber();

   Bool_t customMETtrig(kFALSE);
   if(is2015 && cand.evt.trigger["HLT_xe70_mht"])
    customMETtrig = kTRUE;
  else if(is2016 && cand.evt.randomRunNumber <= 304008 && cand.evt.trigger["HLT_xe90_mht_L1XE50"])
    customMETtrig = kTRUE;
  else if(is2016 && cand.evt.randomRunNumber > 304008 && cand.evt.trigger["HLT_xe110_mht_L1XE50"])
    customMETtrig = kTRUE;
  else if (cand.evt.trigger["HLT_noalg_L1J400"])
    customMETtrig = kTRUE;

  cand.evt.trigger_met = customMETtrig;

  // pass event flags
  cand.evt.passGRL = content.passGRL;
  cand.evt.passPV = content.passPV;
//   cand.evt.passTrigger = content.passTrigger;
  cand.evt.passDetErr = content.passDetErr;
  cand.evt.passJetCleanLoose = content.passJetCleanLoose;
  cand.evt.passJetCleanTight = content.passJetCleanTight;

   // vertex information
   cand.evt.n_vx = content.vertices->size(); // absolute number of PV's (i.e. no track cut)

   // jj and met_j
   double jj_deta = -1., jj_mass=-1., jj_dphi=-1.;
   HelperFunctions::computejj(content.goodJets, jj_mass, jj_deta, jj_dphi);
   if(debug){
    print(" Mjj", jj_mass);
    print(" DEta", jj_deta);
    print(" DPhi", jj_dphi);
  }


   cand.evt.jj_mass = jj_mass;
   cand.evt.jj_deta = jj_deta;
   cand.evt.jj_dphi = jj_dphi;
   cand.evt.met_tst_j1_dphi = content.met_tst_j1_dphi;
   cand.evt.met_tst_j2_dphi = content.met_tst_j2_dphi;
   cand.evt.met_tst_nolep_j1_dphi = content.met_tst_nolep_j1_dphi;
   cand.evt.met_tst_nolep_j2_dphi = content.met_tst_nolep_j2_dphi;
   cand.evt.met_cst_jet = content.met_cst_jet;
   cand.evt.metsig_tst = content.metsig_tst;
   cand.evt.metsig_tst_nolep = content.metsig_tst_nolep;
//   cand.evt.met_tst_nomuon_j1_dphi = content.met_tst_nomuon_j1_dphi;
//   cand.evt.met_tst_nomuon_j2_dphi = content.met_tst_nomuon_j2_dphi;
//   cand.evt.met_tst_noelectron_j1_dphi = content.met_tst_noelectron_j1_dphi;
//   cand.evt.met_tst_noelectron_j2_dphi = content.met_tst_noelectron_j2_dphi;

   // MC-only information

  if (m_isMC) {
      // Record all weights
    cand.evt.mcEventWeight     = content.eventInfo->mcEventWeight();
    cand.evt.mcEventWeights    = content.eventInfo->mcEventWeights();
    cand.evt.puWeight          = m_susytools_handle->GetPileupWeight();
    cand.evt.btagSFWeight      = m_susytools_handle->BtagSF(&content.goodJets);

// GetTotalJetSF(jets, bool btagSF, bool jvtSF)
    cand.evt.jvtSFWeight       = m_susytools_handle->GetTotalJetSF(content.jets, false, true);

    // Lepton Scale Factors
    // See definition of Trig.Singlelep20XX in SUSYObjDef_xAOD.cxx
    // You can modify it in the ST config under Trigger SFs configuration
    // Total Electron SF: GetTotalElectronSF(const xAOD::ElectronContainer& electrons, const bool recoSF, const bool idSF, const bool triggerSF, const bool isoSF, const std::string& trigExpr, const bool chfSF)
    cand.evt.elSFWeight        = m_susytools_handle->GetTotalElectronSF(content.goodElectrons, true, true, false, true, "");
    cand.evt.elSFTrigWeight    = m_susytools_handle->GetTotalElectronSF(content.goodElectrons, false, false, true, false, "singleLepton");
    // Total Muon SF: GetTotalMuonTriggerSF(const xAOD::MuonContainer& sfmuons, const std::string& trigExpr)
    cand.evt.muSFWeight        = m_susytools_handle->GetTotalMuonSF(content.goodMuons, true, true, "");
    cand.evt.muSFTrigWeight    = m_susytools_handle->GetTotalMuonSF(content.goodMuons, false, false, is2015 ? "HLT_mu20_iloose_L1MU15_OR_HLT_mu50" : "HLT_mu26_ivarmedium_OR_HLT_mu50" );

    if(debug)
    {
      print("Electron SF", cand.evt.elSFWeight);
      print("Electron Trig SF", cand.evt.elSFTrigWeight);
      print("Muon SF", cand.evt.muSFWeight);
      print("Muon Trig SF", cand.evt.muSFTrigWeight);
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
    if(doRnS && !m_isMC ) {
      cand.rns.rnsPSweight = cand.rns.getPSweight(m_susytools_handle, event,content.eventInfo->runNumber(), debug);
    }

  // Truth
      //-- JETS --
    const xAOD::JetContainer * truthJets(nullptr);
      static Bool_t failedLookingFor(kFALSE); // trick to avoid infinite RuntimeWarning's for EXOT5
      if (!failedLookingFor) {
       if (!event->retrieve(truthJets, "AntiKt4TruthJets").isSuccess()) {
        ANA_MSG_ERROR("VBFInv::analyzeEvent : Failed to access Truth Jets container; not attempting again, truth_jet* variables will be empty");
        failedLookingFor = kTRUE;
      } else {
            //number of truth jets with pt > 20 GeV and OR
            // loop over all truth jets passing basic cuts
        Int_t nTruthJets(0);
        const xAOD::TruthParticleContainer * truthP(0);
        ANA_CHECK(event->retrieve(truthP, "TruthParticles"));
        for (const auto& truthJ_itr : *truthJets) {
         if (truthJ_itr->pt() > 20000. ) { // no eta cut
          float minDR2 = 9999999.;
                  // for each jet, loop over all electrons and muons passing basic cuts to check if the jets should be counted or not
          for (const auto& truthP_itr : *truthP) {
           if (truthP_itr->pt() > 25000. && truthP_itr->status() == 1 && (abs(truthP_itr->pdgId()) == 11 || abs(truthP_itr->pdgId()) == 13)) { // no eta cut
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
  for (const auto& part : *truthJets) {
    if(part->pt()<7.0e3) continue;
       cand.evt.truth_jet_pt.push_back(part->pt());
       cand.evt.truth_jet_eta.push_back(part->eta());
       cand.evt.truth_jet_phi.push_back(part->phi());
       cand.evt.truth_jet_m.push_back(part->m());
     }
   }
}

      //-- MUONS --
const xAOD::TruthParticleContainer *truthMuons = nullptr;
if (!m_determinedDerivation) {
 ANA_MSG_INFO("Determining derivation type");
 m_isEXOT5 = event->retrieve(truthMuons, "EXOT5TruthMuons").isSuccess();
 m_determinedDerivation = kTRUE;
 TString isEXOT5 = (m_isEXOT5) ? "YES" : "NO";
 ANA_MSG_INFO("Is it EXOT5? (will trigger access to dedicated truth electron and muon containers): " << isEXOT5);
}
const TString mu_container = (m_isEXOT5) ? "EXOT5TruthMuons" : "TruthMuons";
      if (!event->retrieve(truthMuons, mu_container.Data()).isSuccess()) {    // retrieve arguments: container type, container key
       ANA_MSG_ERROR("Failed to retrieve TruthMuons container");
       return EL::StatusCode::FAILURE;
     }
     cand.evt.n_mu_truth = truthMuons->size();
     for (const auto& part : *truthMuons) {
       if(part->pt()<4.0e3) continue;
       if(part->status()!=1) continue;
       cand.evt.truth_mu_pt.push_back(part->pt());
       cand.evt.truth_mu_eta.push_back(part->eta());
       cand.evt.truth_mu_phi.push_back(part->phi());
       cand.evt.truth_mu_m.push_back(part->m());
       cand.evt.truth_mu_status.push_back(part->status());
     }

      //-- ELECTRONS --
     const xAOD::TruthParticleContainer *truthElectrons = nullptr;
     const TString el_container = (m_isEXOT5) ? "EXOT5TruthElectrons" : "TruthElectrons";
      if (!event->retrieve(truthElectrons, el_container.Data()).isSuccess()) {    // retrieve arguments: container type, container key
       ANA_MSG_ERROR("Failed to retrieve TruthElectrons container");
       return EL::StatusCode::FAILURE;
     }
     cand.evt.n_el_truth = truthElectrons->size();
     for (const auto& part : *truthElectrons) {
       if(part->pt()<4.0e3) continue;
       if(part->status()!=1) continue;
       cand.evt.truth_el_pt.push_back(part->pt());
       cand.evt.truth_el_eta.push_back(part->eta());
       cand.evt.truth_el_phi.push_back(part->phi());
       cand.evt.truth_el_m.push_back(part->m());
       cand.evt.truth_el_status.push_back(part->status());
     }

      //-- TAUS --
     const xAOD::TruthParticleContainer *truthTaus = nullptr;
     if (!event->retrieve(truthTaus, "TruthTaus").isSuccess()) {    // retrieve arguments: container type, container key
       ANA_MSG_ERROR("Failed to retrieve TruthTaus container");
       return EL::StatusCode::FAILURE;
     }
     cand.evt.n_tau_truth = truthTaus->size();
     for (const auto& part : *truthTaus) {
       if(part->pt()<4.0e3) continue;
       if(part->status()!=1) continue;
       cand.evt.truth_tau_pt.push_back(part->pt());
       cand.evt.truth_tau_eta.push_back(part->eta());
       cand.evt.truth_tau_phi.push_back(part->phi());
       cand.evt.truth_tau_m.push_back(part->m());
       cand.evt.truth_tau_status.push_back(part->status());
     }

      //-- BOSONS --
     const xAOD::TruthParticleContainer *truthParticles(nullptr);
     ANA_CHECK(event->retrieve(truthParticles, "TruthParticles"));
      // Dressed
     const TLorentzVector truth_V_dressed = VBFInvAnalysis::getTruthBosonP4(truthParticles, truthElectrons, truthMuons, truthParticles);
     cand.evt.truth_V_dressed_pt = truth_V_dressed.Pt();
     cand.evt.truth_V_dressed_eta = truth_V_dressed.Eta();
     cand.evt.truth_V_dressed_phi = truth_V_dressed.Phi();
     cand.evt.truth_V_dressed_m = truth_V_dressed.M();
      // Bare
      /*
     const TLorentzVector truth_V_bare = VBFInvAnalysis::getTruthBosonP4(truthParticles, truthElectrons, truthMuons, truthParticles, kFALSE);
     cand.evt.truth_V_bare_pt = truth_V_bare.Pt();
     cand.evt.truth_V_bare_eta = truth_V_bare.Eta();
     cand.evt.truth_V_bare_phi = truth_V_bare.Phi();
     cand.evt.truth_V_bare_m = truth_V_bare.M();
     */

   } // done with MC only

  //-----------------------------------------------------------------------
  // Selected jets
  //-----------------------------------------------------------------------
   cand.evt.n_jet = content.goodJets.size();
   static SG::AuxElement::Accessor<char> acc_bjet("bjet");
   UInt_t tmpNBJet(0);
   for (auto jet : content.goodJets) {
    cand.jet["jet"].add(*jet);
    if (acc_bjet(*jet))  tmpNBJet++;
  }
  cand.evt.n_bjet = tmpNBJet;

  //-----------------------------------------------------------------------
  // Selected muons
  //-----------------------------------------------------------------------
  cand.evt.n_mu = content.goodMuons.size();
  cand.evt.n_mu_baseline = content.baselineMuons.size();
  //if( cand.evt.n_mu_baseline != 0)
  //std::cout << "Number of muons in event=" << cand.evt.n_mu << ", baseline=" << cand.evt.n_mu_baseline << std::endl;
  static SG::AuxElement::Accessor<char> acc_bad("bad");
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
         std::cout << "Muon pT=" << muon->pt()*1e-3 << ", ptvarcone30_TightTTVA_pt1000/pt=" << tmp_ptvarcone30_TightTTVA_pt1000/muon->pt() << ", topoetcone20/pt=" << tmp_topoetcone20/muon->pt() << std::endl;
    */
  }
  if(doMuonDetail){
    for (auto muon : content.baselineMuons) {
      cand.mu["basemu"].add(*muon);
    }
  }


  //-----------------------------------------------------------------------
  // Selected electrons
  //-----------------------------------------------------------------------
  cand.evt.n_el = content.goodElectrons.size();
  cand.evt.n_el_baseline = content.baselineElectrons.size();
  for (auto electron : content.goodElectrons) {
    cand.el["el"].add(*electron);
  }
  if(doElectronDetail){
    for (auto electron : content.baselineElectrons) {
      cand.el["baseel"].add(*electron);
    }
  }

   /////////////////////////////
   // Selected taus
   ////////////////////////////
for (auto thisTau : content.goodTaus) {
   cand.tau["tau"].add(*thisTau);
}

  //-----------------------------------------------------------------------
   // MET
  //-----------------------------------------------------------------------
  cand.met["met_tst"].add(*((*content.met_tst)["Final"]));
  cand.met["met_tight_tst"].add(*((*content.met_tight_tst)["Final"]));
  cand.met["met_tighter_tst"].add(*((*content.met_tighter_tst)["Final"]));
  cand.met["met_tenacious_tst"].add(*((*content.met_tenacious_tst)["Final"]));
  cand.met["met_soft_tst"].add(*((*content.met_tst)["PVSoftTrk"]));
  cand.met["met_tight_tst_nolep"].add(*((*content.met_tight_tst_nolep)["Final"]));
  cand.met["met_tighter_tst_nolep"].add(*((*content.met_tighter_tst_nolep)["Final"]));
  cand.met["met_tenacious_tst_nolep"].add(*((*content.met_tenacious_tst_nolep)["Final"]));

//  cand.met["met_tst_nomuon"].add(*((*content.met_tst_nomuon)["Final"]));
//  cand.met["met_tst_noelectron"].add(*((*content.met_tst_noelectron)["Final"]));
  cand.met["met_tst_nolep"].add(*((*content.met_tst_nolep)["Final"]));
  cand.met["met_track"].add(*((*content.met_track)["Track"]));
  if (m_isMC) cand.met["met_truth"].add(*((*content.met_truth)["NonInt"]));
  //-----------------------------------------------------------------------
   // write tree
  //-----------------------------------------------------------------------
  cand.save();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::getMET(std::shared_ptr<xAOD::MissingETContainer> &met, std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer * jet, xAOD::ElectronContainer * el, xAOD::MuonContainer * mu, xAOD::PhotonContainer * /*ph*/, Bool_t doTST, Bool_t doJVT, xAOD::IParticleContainer * invis, TLorentzVector &myMET, double &myMETsig, int METType)
{
 myMETsig = 0;
 met = std::make_shared<xAOD::MissingETContainer>();
 metAux = std::make_shared<xAOD::MissingETAuxContainer>();
 met->setStore(metAux.get());

 if(METType==0){
   ANA_CHECK(m_susytools_handle->GetMET(*met,
					jet, // use all objects (before OR and after corrections) for MET utility
					el,
					mu,
					0,//ph, // ph term
					0, // tau term
					doTST,
					doJVT,
					invis// invisible particles
					));
   ANA_CHECK(m_susytools_handle->GetMETSig(*met,
					   myMETsig,
					   doTST,
					   doJVT
					   ));
 }else if(METType==1){
   ANA_CHECK(m_susytools_Tight_handle->GetMET(*met,
					jet, // use all objects (before OR and after corrections) for MET utility
					el,
					mu,
					0,//ph, // ph term
					0, // tau term
					doTST,
					doJVT,
					invis// invisible particles
					));
   ANA_CHECK(m_susytools_Tight_handle->GetMETSig(*met,
					   myMETsig,
					   doTST,
					   doJVT
					   ));
 }else if(METType==2){
   ANA_CHECK(m_susytools_Tighter_handle->GetMET(*met,
					jet, // use all objects (before OR and after corrections) for MET utility
					el,
					mu,
					0,//ph, // ph term
					0, // tau term
					doTST,
					doJVT,
					invis// invisible particles
					));
   ANA_CHECK(m_susytools_Tighter_handle->GetMETSig(*met,
					   myMETsig,
					   doTST,
					   doJVT
					   ));
 }else if(METType==3){
   ANA_CHECK(m_susytools_Tenacious_handle->GetMET(*met,
					jet, // use all objects (before OR and after corrections) for MET utility
					el,
					mu,
					0,//ph, // ph term
					0, // tau term
					doTST,
					doJVT,
					invis// invisible particles
					));
   ANA_CHECK(m_susytools_Tenacious_handle->GetMETSig(*met,
					   myMETsig,
					   doTST,
					   doJVT
					   ));
   }

 xAOD::MissingETContainer::const_iterator met_it = met->find("Final");

 if (met_it == met->end()) {
  ANA_MSG_ERROR("VBFInv::getMET : No RefFinal inside MET container");
  return EL::StatusCode::FAILURE;
 }
 else {
   float Etmiss_Etx = (*met_it)->mpx();
   float Etmiss_Ety = (*met_it)->mpy();
   float Etmiss_Et = sqrt(Etmiss_Etx * Etmiss_Etx + Etmiss_Ety * Etmiss_Ety);
   myMET.SetPxPyPzE(Etmiss_Etx, Etmiss_Ety, 0., Etmiss_Et);
 }

return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::getTrackMET(std::shared_ptr<xAOD::MissingETContainer> &met, std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer * jet, xAOD::ElectronContainer * el, xAOD::MuonContainer * mu)
{
 met = std::make_shared<xAOD::MissingETContainer>();
 metAux = std::make_shared<xAOD::MissingETAuxContainer>();
 met->setStore(metAux.get());

 ANA_CHECK(m_susytools_handle->GetTrackMET(*met, // note that we don't need to pass the Aux container here
                                    jet, // use all objects (before OR and after corrections) for MET utility
                                    el,
                                    mu
                                    ));
 xAOD::MissingETContainer::const_iterator met_it = met->find("Track");

 if (met_it == met->end()) {
  ANA_MSG_ERROR("VBFInv::getMET : No Track inside MET container");
  return EL::StatusCode::FAILURE;
}

return EL::StatusCode::SUCCESS;
}

void VBFInv::printObjects(xAOD::IParticleContainer obj, TString label){
  if(obj.size() !=0){
    ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
    int iobj=0;
    ANA_MSG_INFO(label << " size is " << obj.size());
    for (auto thisObj : obj){
      ANA_MSG_INFO( label << " " << iobj << ": pt=" << thisObj->pt()*0.001 <<", eta=" << thisObj->eta() << ", phi=" << thisObj->phi() );
      iobj++;
    }
  }
}

void VBFInv::printMET(TLorentzVector myMET, double myMETsig, TLorentzVector myTruthMET, TString label){
  ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
  ANA_MSG_INFO(label << ": met=" << myMET.Perp()*0.001 << ", phi=" << myMET.Phi());
  ANA_MSG_INFO(label << ": metSig=" << myMETsig);
  if(m_isMC)
    ANA_MSG_INFO(label << ": truthMET=" << myTruthMET.Perp()*0.001 << ", truthPhi=" << myTruthMET.Phi());
}

void VBFInv::printMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label){
  ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
  const Float_t mpx = (*met)["Final"]->mpx();
  const Float_t mpy = (*met)["Final"]->mpy();
  float Etmiss_Et = sqrt(mpx*mpx + mpy*mpy);
  ANA_MSG_INFO(label << ": met=" << Etmiss_Et*0.001);
}

void VBFInv::printTrackMET(std::shared_ptr<xAOD::MissingETContainer> met, TString label){
  ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
  const Float_t mpx = (*met)["Track"]->mpx();
  const Float_t mpy = (*met)["Track"]->mpy();
  float Etmiss_Et = sqrt(mpx*mpx + mpy*mpy);
  ANA_MSG_INFO(label << ": met=" << Etmiss_Et*0.001);
}

void VBFInv::print(TString label, float val){
  ANA_MSG_INFO("---------------------------- " << label << " ----------------------------");
  ANA_MSG_INFO(label << "=" << val);
}





