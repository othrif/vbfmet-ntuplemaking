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
doRnS(false),
m_isMC(false),
m_isAFII(false),
m_eventCounter(0),
m_determinedDerivation(false),
m_isEXOT5(false),
m_grl("GoodRunsListSelectionTool/grl", this),
m_susytools_handle("ST::SUSYObjDef_xAOD/ST", this)
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
 m_NumberEvents = new TH1D("NumberEvents", "Number Events", 4, 0, 4);
 m_NumberEvents->GetXaxis()->SetBinLabel(1, "Raw");
 m_NumberEvents->GetXaxis()->SetBinLabel(2, "Weights");
 m_NumberEvents->GetXaxis()->SetBinLabel(3, "WeightsSquared");
 m_NumberEvents->GetXaxis()->SetBinLabel(4, "XsecXEff");

  // CutFlow
 TString NameCut("Nominal");
 m_CutFlow = Analysis::CutFlowTool(NameCut);
 m_CutFlow.addCut("Pre-Derivation");
 m_CutFlow.addCut("Processed");
 m_CutFlow.addCut("GRL");
 m_CutFlow.addCut("Vertex");
 m_CutFlow.addCut("Trigger");
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
  std::string xSecFilePath = "dev/PMGTools/PMGxsecDB_mc15.txt";
  xSecFilePath = PathResolverFindCalibFile(xSecFilePath);
  my_XsecDB = new SUSY::CrossSectionDB(xSecFilePath);
  if (debug)
    ANA_MSG_INFO("xsec DB initialized using file:" <<  xSecFilePath);
  const xAOD::EventInfo *eventInfo = nullptr;
  ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));
  float crossSection = my_XsecDB->xsectTimesEff(eventInfo->mcChannelNumber());
  if (debug)
    print("cross section", crossSection );
  m_NumberEvents->Fill(0., nEventsProcessed);
  m_NumberEvents->Fill(1., sumOfWeights);
  m_NumberEvents->Fill(2., sumOfWeightsSquared);
  m_NumberEvents->Fill(3., crossSection);
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

  // SUSYTools
 const ST::ISUSYObjDef_xAODTool::DataSource datasource = (!m_isMC ? ST::ISUSYObjDef_xAODTool::Data : (m_isAFII ? ST::ISUSYObjDef_xAODTool::AtlfastII : ST::ISUSYObjDef_xAODTool::FullSim));
 ANA_CHECK(m_susytools_handle.setProperty("DataSource", datasource));
 ANA_CHECK(m_susytools_handle.setProperty("ConfigFile", ST_config_file.Data()));
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
      std::string prwConfigFile = "dev/SUSYTools/PRW_AUTOCONFIG/files/pileup_" + mc_campaign + "_dsid" + std::to_string(eventInfo->mcChannelNumber()) + ".root";
      prwConfigFile = PathResolverFindCalibFile(prwConfigFile);
      if(prwConfigFile.empty()) {
        std::vector<std::string> prw_conf_tmp = getTokens(prw_file, ",");
        std::transform(prw_conf_tmp.begin(), prw_conf_tmp.end(), prw_conf_tmp.begin(), [](std::string x) -> std::string { return PathResolverFindCalibFile(x); });
        ANA_MSG_INFO( "File " << prwConfigFile << " not found in ST database. Set manual prw configuration with the following files:" );
        for (uint i = 0; i< prw_conf_tmp.size(); i++) {
          ANA_MSG_INFO( prw_conf_tmp.at(i) );
          prw_conf.push_back(prw_conf_tmp.at(i));}
        }
        else {
          prw_conf.push_back(prwConfigFile);
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
      const Bool_t trim = !isNominal;

      ANA_MSG_INFO("Creating TTree named " <<  treeName.Data() << " for systematic named \"" << thisSyst.Data() << "\"");

      m_tree[thisSyst] = new TTree(treeName, treeTitle);
      m_tree[thisSyst]->SetDirectory(outputFile);

      // save, in all trees, the trigger decision
      std::vector<std::string> triggersToSave = getTokens(trigger_list, ",");
      for (uint i = 0; i< triggersToSave.size();i++)    boost::algorithm::trim(triggersToSave.at(i));
        if(debug) for (uint i = 0; i< triggersToSave.size();i++) ANA_MSG_INFO("Save trigger decision of: " << triggersToSave.at(i));
      m_cand[thisSyst].evt.setTriggers(triggersToSave);

      // define all elements of the output tree

      m_cand[thisSyst].met["met_tst"] = Analysis::outMET("met_tst", trim, doMETDetail);
      m_cand[thisSyst].met["met_tst_nomuon"] = Analysis::outMET("met_tst_nomuon", trim, doMETDetail);
      m_cand[thisSyst].met["met_tst_noelectron"] = Analysis::outMET("met_tst_noelectron", trim, doMETDetail);
      m_cand[thisSyst].met["met_track"] = Analysis::outMET("met_track", trim, doMETDetail);
      m_cand[thisSyst].met["met_truth"] = Analysis::outMET("met_truth", trim, doMETDetail);
      m_cand[thisSyst].mu["mu"] = Analysis::outMuon("mu", trim, doMuonDetail);
      m_cand[thisSyst].el["el"] = Analysis::outElectron("el", trim, doElectronDetail);
      m_cand[thisSyst].jet["jet"] = Analysis::outJet("jet", trim, doJetDetail);
      //m_cand[thisSyst].tau["tau"] = Analysis::outTau("tau", trim, doTauDetail);

      // trim non-nominal trees (might be redundant)
      m_cand[thisSyst].setDoTrim(trim);
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
  //  const Bool_t syst_affectsTaus      = ST::testAffectsObject(xAOD::Type::Tau, systInfo.affectsType);

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
    content.doTaus = false;//(syst_affectsTaus || content.isNominal);
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

    if(debug)
      ANA_MSG_INFO ("in execute, runNumber = " << content.eventInfo->runNumber() << ", eventNumber = " << content.eventInfo->eventNumber());

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
    Bool_t passesTrigger(kTRUE);
    if (!passesTrigger && doSkim)
      return EL::StatusCode::SUCCESS;
    m_CutFlow.hasPassed(VBFInvCuts::Trigger, event_weight);
    content.passTrigger = passesTrigger;

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
       if (acc_baseline(*ph) == 1) {
        content.allPhotons.push_back(ph);
      }
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
      ANA_MSG_INFO ("jet pt=" << jet->pt()*0.001 << ", eta=" << jet->eta() << ", bad=" << (acc_bad(*jet)==1) << ", passOR=" << (acc_passOR(*jet)==1) << ", passJVT=" << (acc_passJvt(*jet)==1)<< ", passFJVT=" << (acc_passFJvt(*jet)==1) << " => BASELINE=" << (acc_baseline(*jet)==1) << ", SIGNAL=" << (acc_signal(*jet)==1) );
    }
  }
  for (auto jet : content.allJets){
    // Good jets: pt>25GeV, jet cleaning, |eta| < 4.8
    // Give user passOR, passJvt, passFJvt to decide on the jet
      if (/*acc_signal(*jet) == 1 && acc_passOR(*jet) == 1 &&*/ acc_bad(*jet) == 0){
    content.goodJets.push_back(jet);
  }
}

  //-- MUONS --
for (auto muon : content.allMuons)
 if (acc_passOR(*muon) == 1 && acc_cosmic(*muon) == 0 && acc_bad(*muon) == 0) {
  content.baselineMuons.push_back(muon);
  if (acc_signal(*muon) == 1)
      content.goodMuons.push_back(muon); // CR muons
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
  /*
 for (auto tau : content.allTaus) {
   content.goodTaus.push_back(tau);
 }*/

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
          myMETsig_tst
          );

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
    printMET(content.met_tst_noelectron, "MET no electrons");
    printMET(content.met_tst_nomuon, "MET no muons");
    printTrackMET(content.met_track, "track MET");
  }

  //-----------------------------------------------------------------------
  // Continue cleaning...
  //-----------------------------------------------------------------------
// Jet cleaning
// https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJets2017
  static SG::AuxElement::Accessor<char> acc_eventClean("DFCommonJets_eventClean_LooseBad");
  if(debug)
    print("eventClean_LooseBad", (bool)acc_eventClean(*content.eventInfo));
  Bool_t passesJetClean = !(acc_eventClean(*content.eventInfo) == 0);
  if( !passesJetClean && doSkim){
    return EL::StatusCode::SUCCESS;
  }

  Bool_t passesJetCleanTight = true;
  static SG::AuxElement::Accessor<char> acc_jetCleanTight("DFCommonJets_jetClean_TightBad");
  for (auto jet : content.allJets) {
    if(debug)
      print("jetClean_TightBad", (bool)acc_jetCleanTight(*jet));
    if (acc_jetCleanTight(*jet) == 0) {
      passesJetCleanTight = false;
    }
  }
  
  m_CutFlow.hasPassed(VBFInvCuts::JetBad, event_weight);
  content.passJetClean = passesJetClean;
  content.passJetCleanTight = passesJetCleanTight;

  //-----------------------------------------------------------------------
  // Fill tree
  //-----------------------------------------------------------------------

  // skimming
  const Double_t pt1SkimToUse = (content.isNominal) ? pt1Skim : pt1SkimForSyst;
  const Double_t pt2SkimToUse = (content.isNominal) ? pt2Skim : pt2SkimForSyst;
  const Double_t metSkimToUse = (content.isNominal) ? metSkim : metSkimForSyst;
  const Double_t mjjSkimToUse = (content.isNominal) ? mjjSkim : mjjSkimForSyst;
  const Double_t detajjSkimToUse = (content.isNominal) ? detajjSkim : detajjSkimForSyst;
  TVector2 met_nomuon_to_use = TVector2((*content.met_tst_nomuon)["Final"]->mpx(), (*content.met_tst_nomuon)["Final"]->mpy());
  TVector2 met_noelectron_to_use = TVector2((*content.met_tst_noelectron)["Final"]->mpx(), (*content.met_tst_noelectron)["Final"]->mpy());
  double detajjMax=-1., mjjMax=-1.;
  HelperFunctions::computeMaxjj(content.goodJets, mjjMax, detajjMax);
  if(debug){
    print("Max Mjj", mjjMax);
    print("Max DEta", detajjMax);
  }

  // Skimming
  Bool_t saveMe = ( met_nomuon_to_use.Mod() > metSkimToUse || met_noelectron_to_use.Mod() > metSkimToUse );
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
cand.evt.passTrigger = -1;
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
/*  Bool_t is2016(kFALSE);
  Bool_t is2017(kFALSE);
  Bool_t is2018(kFALSE);*/

   if ((cand.evt.year == 0 && cand.evt.runNumber >= 276262 && cand.evt.runNumber <= 284484) || cand.evt.year == 2015) is2015 = kTRUE; // data2015
/*   if ((cand.evt.year == 0 && cand.evt.runNumber >= 296939 && cand.evt.runNumber<=311481 ) || cand.evt.year == 2016) is2016 = kTRUE; // data2016
   if ((cand.evt.year == 0 && cand.evt.runNumber >= 324320 && cand.evt.runNumber<=341649 ) || cand.evt.year == 2017) is2017 = kTRUE; // data2017
  if ((cand.evt.year == 0 && cand.evt.runNumber >= 348197) || cand.evt.year == 2018) is2018 = kTRUE; // data2018*/


   // event-level artifacts from system flags
   cand.evt.flag_lar = (content.eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error );
   cand.evt.flag_tile =    (content.eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error );
   cand.evt.flag_sct = (content.eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error);
   cand.evt.flag_core = (content.eventInfo->eventFlags(xAOD::EventInfo::Core) & 0x40000);
   cand.evt.flag_bib = content.eventInfo->eventFlags(xAOD::EventInfo::Background) & (1 << 20);
   cand.evt.flag_bib_raw = content.eventInfo->eventFlags(xAOD::EventInfo::Background);

  // pass event flags
   cand.evt.passGRL = content.passGRL;
   cand.evt.passPV = content.passPV;
   cand.evt.passTrigger = content.passTrigger;
   cand.evt.passDetErr = content.passDetErr;
   cand.evt.passJetClean = content.passJetClean;
   cand.evt.passJetCleanTight = content.passJetCleanTight;

   // vertex information
   cand.evt.n_vx = content.vertices->size(); // absolute number of PV's (i.e. no track cut)
   // MC-only information

   if (m_isMC) {
      // Record all weights
    cand.evt.mcEventWeight     = content.eventInfo->mcEventWeight();
    cand.evt.mcEventWeights    = content.eventInfo->mcEventWeights();
    cand.evt.puWeight          = m_susytools_handle->GetPileupWeight();
    cand.evt.btagSFWeight      = m_susytools_handle->BtagSF(&content.goodJets);

// GetTotalJetSF(jets, bool btagSF, bool jvtSF)
    cand.evt.jvtSFWeight       = m_susytools_handle->GetTotalJetSF(content.jets, false, true);

    // Lepton Scale Facgtors
    // See definition of Trig.Singlelep20XX in SUSYObjDef_xAOD.cxx
    // You can modify it in the ST config under Trigger SFs configuration
    // Total Electron SF: GetTotalElectronSF(const xAOD::ElectronContainer& electrons, const bool recoSF, const bool idSF, const bool triggerSF, const bool isoSF, const std::string& trigExpr, const bool chfSF)
    cand.evt.elSFWeight        = m_susytools_handle->GetTotalElectronSF(content.goodElectrons, true, true, false, true, "");
    cand.evt.elSFTrigWeight    = m_susytools_handle->GetTotalElectronSF(content.goodElectrons, false, false, true, false, "singleLepton");
    // Total Muon SF: GetTotalMuonTriggerSF(const xAOD::MuonContainer& sfmuons, const std::string& trigExpr)
    cand.evt.muSFWeight        = m_susytools_handle->GetTotalMuonSF(content.goodMuons, true, true, "");
    cand.evt.muSFTrigWeight    = m_susytools_handle->GetTotalMuonSF(content.goodMuons, false, false, is2015 ? "HLT_mu20_iloose_L1MU15_OR_HLT_mu50" : "HLT_mu24_imedium_OR_HLT_mu50" );
    if(debug)
    {
      print("Electron SF", cand.evt.elSFWeight);
      print("Electron Trig SF", cand.evt.elSFTrigWeight);
      print("Muon SF", cand.evt.muSFWeight);
      print("Muon Trig SF", cand.evt.muSFTrigWeight);
    }


      // PDF
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
  if (truthJets->size() > 0) {
   cand.evt.truth_jet1_pt = (*truthJets)[0]->p4().Pt();
   cand.evt.truth_jet1_eta = (*truthJets)[0]->p4().Eta();
   cand.evt.truth_jet1_phi = (*truthJets)[0]->p4().Phi();
   cand.evt.truth_jet1_m = (*truthJets)[0]->p4().M();
   if (truthJets->size() > 1) {
    cand.evt.truth_jet2_pt = (*truthJets)[1]->p4().Pt();
    cand.evt.truth_jet2_eta = (*truthJets)[1]->p4().Eta();
    cand.evt.truth_jet2_phi = (*truthJets)[1]->p4().Phi();
    cand.evt.truth_jet2_m = (*truthJets)[1]->p4().M();
  }
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
       ANA_MSG_ERROR("Failed to retrieve Muons container");
       return EL::StatusCode::FAILURE;
     }
     for (const auto& part : *truthMuons) {
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
       ANA_MSG_ERROR("Failed to retrieve Electrons container");
       return EL::StatusCode::FAILURE;
     }
     for (const auto& part : *truthElectrons) {
       cand.evt.truth_el_pt.push_back(part->pt());
       cand.evt.truth_el_eta.push_back(part->eta());
       cand.evt.truth_el_phi.push_back(part->phi());
       cand.evt.truth_el_m.push_back(part->m());
       cand.evt.truth_el_status.push_back(part->status());
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
     const TLorentzVector truth_V_bare = VBFInvAnalysis::getTruthBosonP4(truthParticles, truthElectrons, truthMuons, truthParticles, kFALSE);
     cand.evt.truth_V_bare_pt = truth_V_bare.Pt();
     cand.evt.truth_V_bare_eta = truth_V_bare.Eta();
     cand.evt.truth_V_bare_phi = truth_V_bare.Phi();
     cand.evt.truth_V_bare_m = truth_V_bare.M();

    // -- Generator MET  and MHT --
    // MET calculated from all status==1 gen particles
    // MHT calculated from all status==1 gen particles matched (dR < 0.4) to jets above rebalancing threshold

    // get truth container of interest
     const xAOD::TruthParticleContainer* genparticles = nullptr;
     ANA_CHECK(event->retrieve(genparticles, "TruthParticles"));

     TLorentzVector vgenMET(0, 0, 0, 0);
     TLorentzVector vtrueMHTreb(0, 0, 0, 0);

     for ( const auto* it : *genparticles ) {
            if (it->pt() > 0. && it->status() == 1 && !(it->hasDecayVtx())) { // status() == 1 for pythia samples
              if (abs(it->pdgId()) == 12 || abs(it->pdgId()) == 14 || abs(it->pdgId()) == 16 )
                vgenMET += it->p4();
              if (abs(it->pdgId()) == 12 || abs(it->pdgId()) == 14 || abs(it->pdgId()) == 16 || abs(it->pdgId()) == 11 || abs(it->pdgId()) == 13 || abs(it->pdgId()) == 15 ){
                if (debug)
                  ANA_MSG_INFO("truth lepton (pt, eta, phi, status): " << it->pdgId() << ", " << it->pt()/1000. << ", " << it->eta() << ", " << it->phi() << ", " << it->status());
              }
              bool particleAdded = false;

              // loop over reco jet container
              for (auto jet : content.goodJets)
                if (!particleAdded && jet->pt() > rebalancedJetPt) {

                  if (xAOD::P4Helpers::isInDeltaR(*jet, *it, 0.4, true)) {
                   float dR2 = xAOD::P4Helpers::deltaR2(jet, it, true);
                   if (dR2 < 0.4 * 0.4) {
                    vtrueMHTreb += it->p4();
                    particleAdded = true;
                  }
                }
              }
            }
          }

          cand.evt.GenMET_pt = vgenMET.Pt();
          cand.evt.GenMET_phi = vgenMET.Phi();
          cand.evt.TrueMHT_pt = vtrueMHTreb.Pt();
          cand.evt.TrueMHT_phi = vtrueMHTreb.Phi();

          // Generator MET and HT
          if(debug) {
            print("Generator MET pt",cand.evt.GenMET_pt);
            print("Generator MET phi",cand.evt.GenMET_phi);
            print("True MHT pt",cand.evt.TrueMHT_pt);
            print("Truth MHT phi",cand.evt.TrueMHT_phi);
          }


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
  static SG::AuxElement::Accessor<char> acc_bad("bad");
  for (auto muon : content.goodMuons) {
    cand.mu["mu"].add(*muon);
  }

  //-----------------------------------------------------------------------
  // Selected electrons
  //-----------------------------------------------------------------------
  cand.evt.n_el = content.goodElectrons.size();
  cand.evt.n_el_baseline = content.baselineElectrons.size();
  for (auto electron : content.goodElectrons) {
    cand.el["el"].add(*electron);
  }

   /////////////////////////////
   // Selected taus
   ////////////////////////////
   /*
for (auto thisTau : content.goodTaus) {
   cand.tau["tau"].add(*thisTau);
}*/

  //-----------------------------------------------------------------------
   // MET
  //-----------------------------------------------------------------------
  cand.met["met_tst"].add(*((*content.met_tst)["Final"]));
  cand.met["met_tst_nomuon"].add(*((*content.met_tst_nomuon)["Final"]));
  cand.met["met_tst_noelectron"].add(*((*content.met_tst_noelectron)["Final"]));
  cand.met["met_track"].add(*((*content.met_track)["Track"]));
  if (m_isMC) cand.met["met_truth"].add(*((*content.met_truth)["NonInt"]));
  //-----------------------------------------------------------------------
   // write tree
  //-----------------------------------------------------------------------
  cand.save();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode VBFInv::getMET(std::shared_ptr<xAOD::MissingETContainer> &met, std::shared_ptr<xAOD::MissingETAuxContainer> &metAux, xAOD::JetContainer * jet, xAOD::ElectronContainer * el, xAOD::MuonContainer * mu, xAOD::PhotonContainer * ph, Bool_t doTST, Bool_t doJVT, xAOD::IParticleContainer * invis, TLorentzVector &myMET, double &myMETsig)
{
 myMETsig = 0;
 met = std::make_shared<xAOD::MissingETContainer>();
 metAux = std::make_shared<xAOD::MissingETAuxContainer>();
 met->setStore(metAux.get());

 ANA_CHECK(m_susytools_handle->GetMET(*met,
                               jet, // use all objects (before OR and after corrections) for MET utility
                               el,
                               mu,
                               ph,
                               0, // tau term
                               doTST,
                               doJVT,
                               invis// invisible particles
                               ));
 ANA_CHECK(m_susytools_handle->GetMETSig(*met,
			      myMETsig,
			      /*
			      jet, // use all objects (before OR and after corrections) for MET utility
                              el,
                              mu,
                              ph,
                              0, // tau term
			      */
                              doTST,
                              doJVT
			      //invis
                              ));

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





