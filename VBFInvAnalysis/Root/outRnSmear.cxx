#include "VBFInvAnalysis/outRnSmear.h"

Analysis::outRnSmear::outRnSmear(TString name, Bool_t doTrim, Bool_t doDetail) : Analysis::outObject::outObject(name, doTrim, doDetail)
{
 reset();
}

Analysis::outRnSmear::~outRnSmear()
{
}

void Analysis::outRnSmear::reset()
{
 rnsPSweight = 1.0;


 return;
}


void Analysis::outRnSmear::attachToTree(TTree *tree)
{
   const TString prefix = (name() != "") ? name() + "_" : ""; // no prefix by default

   tree->Branch(prefix + "rnsPSweight", &rnsPSweight);

   return;
}

double Analysis::outRnSmear::getPSweight(asg::AnaToolHandle<ST::ISUSYObjDef_xAODTool> susytools_handle, xAOD::TEvent*& event, int RunNumber, bool debug )
{

  if(debug)
    Info("getPSweight()", "---------------------------- Jet trigger pre-scales for Rebalance and Smear ----------------------------");

double PSweight = -1.0;

    // Get list of single jet triggers
const Trig::ChainGroup*  chainGroup = susytools_handle->GetTrigChainGroup("HLT_j.*");
std::map<int, std::string> PresenTriggers;
for (auto &trig : chainGroup->getListOfTriggers()) {
    std::string thisTrig = trig;
    if (thisTrig.length() < 9) {
            // Extract threshold
        std::string str_thres = thisTrig.substr(5);
        PresenTriggers[atoi(str_thres.c_str())] = thisTrig;
            //std::cout << "trigger: " << thisTrig << ", threshold: " << str_thres << std::endl;
    }
}

std::vector<std::string> single_jet_triggers;
std::vector<double> single_jet_theshold;
for (std::map<int, std::string>::iterator it = PresenTriggers.begin(); it != PresenTriggers.end(); it++) {
    single_jet_triggers.push_back(it->second);
    single_jet_theshold.push_back(int(it->first));
}

    // check which single jet trigger has fired
std::vector<bool > triggersPass;
for(const std::string &sjt : single_jet_triggers) {
    triggersPass.push_back(susytools_handle->IsTrigPassed(sjt));
    if(debug)
        std::cout << "Trigger, fired: " << sjt << ", " << susytools_handle->IsTrigPassed(sjt) << std::endl;
}

    // retrieve the HLT jets
const xAOD::JetContainer *hlt_jet = nullptr;
TString mc_name="HLT_xAOD__JetContainer_GSCJet";
if (RunNumber <= 320000)
    mc_name="HLT_xAOD__JetContainer_a4tcemsubjesFS";
if( ! event->retrieve(hlt_jet, mc_name.Data()) )
    Error("getPSweight()", Form("failed to retrieve %s", mc_name.Data()));
    // Get leading HLT jet
double HLTjetPt = 0;
double HLTjetEta = 0;
double HLTjetPhi = 0;
if (hlt_jet->size() > 0) {
    HLTjetPt = hlt_jet->at(0)->pt()/1000.;
    HLTjetEta = hlt_jet->at(0)->eta();
    HLTjetPhi = hlt_jet->at(0)->phi();
    for  (unsigned int i = 0; i < hlt_jet->size(); ++i) {
        if (hlt_jet->at(i)->pt()/1000. > HLTjetPt) {
            HLTjetPt = hlt_jet->at(i)->pt()/1000.;
            HLTjetEta = hlt_jet->at(i)->eta();
            HLTjetPhi = hlt_jet->at(i)->phi();
        }
    }
} else {
   Info( "getPSweight()", "No trigger object (jet) available" );
   return PSweight;
}
if(debug){
   Info( "getPSweight()", "Leading jet pT: %.1f ", HLTjetPt);
   Info( "getPSweight()", "Leading jet eta: %.1f ", HLTjetEta);
   Info( "getPSweight()", "Leading jet phi: %.1f ", HLTjetPhi);
}

      // Determine highest threshold trigger, which fired
int i_highest_fire = -1;
for (int i  = triggersPass.size()-1; i >= 0; i--) {
    if (triggersPass.at(i)) {
        i_highest_fire = i;
        break;
    }
}

        // Determine highest threshold trigger, which could have fired
int i_highest_threshold = -1;
for (int i  = single_jet_theshold.size()-1; i >= 0; i--) {
    if (float(single_jet_theshold.at(i)) <= HLTjetPt) {
        i_highest_threshold = i;
        break;
    }
}

        //// Calculating effective pre-scale
double combined_PS = 1.;
if (i_highest_fire == -1 || i_highest_threshold == -1) {
    PSweight = 0.;
    Info( "getPSweight()", "No relevant single jet trigger has fired" );
    return PSweight;
} else {
    for (int i = 0; i <= i_highest_threshold; i++) {
        auto chainGroup = susytools_handle->GetTrigChainGroup(single_jet_triggers.at(i));
        for (auto &trig : chainGroup->getListOfTriggers()) {
            auto cg = susytools_handle->GetTrigChainGroup(trig);
            std::string thisTrig = trig;
            double ps = cg->getPrescale();
            if(debug)
                std::cout << "Trigger, fire, prescale" << trig << ", " <<  triggersPass.at(i) << ", " << ps << std::endl;
            if (ps > 0) {
                combined_PS *= (1. - 1./ps);
            }
        }
    }
    PSweight = 1. / (1. - combined_PS);
    if(debug)
        std::cout << "Combined pre-scale = " << PSweight << std::endl;
}


return PSweight;
}


