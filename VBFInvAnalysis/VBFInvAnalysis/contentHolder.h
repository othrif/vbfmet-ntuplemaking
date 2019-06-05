#ifndef VBFInvAnalysis_contentHolder_H
#define VBFInvAnalysis_contentHolder_H

#include <xAODCore/ShallowCopy.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTracking/VertexContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMissingET/MissingETAuxContainer.h>

namespace Analysis {
class ContentHolder {
   // this class holds (smart) pointers to xAOD containers
   // when destructed, owned elements are destroyed

public:
   Bool_t isNominal;

   Bool_t doMuons;
   Bool_t doElectrons;
   Bool_t doJets;
   Bool_t doFatJets;
   Bool_t doTrackJets;
   Bool_t doPhotons;
   Bool_t doTaus;
   Bool_t doOverlapRemoval;
   Bool_t doMET;

   Bool_t passGRL;
   //      Bool_t passTrigger;
   Bool_t passPV;
   Bool_t passJetCleanLoose;
   Bool_t passJetCleanTight;
   Bool_t passDetErr;
   Bool_t passBatman;

   Double_t met_tst_j1_dphi;
   Double_t met_tst_j2_dphi;
   Double_t met_tst_nolep_j1_dphi;
   Double_t met_tst_nolep_j2_dphi;
   Double_t met_cst_jet;
   Double_t metsig_tst;
   Double_t metsig_tst_nolep;
   //      Double_t met_tst_nomuon_j1_dphi;
   //      Double_t met_tst_nomuon_j2_dphi;
   //      Double_t met_tst_noelectron_j1_dphi;
   //      Double_t met_tst_noelectron_j2_dphi;
   //
   // objects we do NOT own
   //
   const xAOD::EventInfo       *eventInfo; //!
   const xAOD::VertexContainer *vertices;  //!

   // MET
   std::shared_ptr<xAOD::MissingETContainer>    met_tst;                    //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tstAux;                 //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tight_tst;              //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tight_tstAux;           //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tighter_tst;            //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tighter_tstAux;         //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tenacious_tst;          //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tenacious_tstAux;       //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tight_tst_nolep;        //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tight_tst_nolepAux;     //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tighter_tst_nolep;      //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tighter_tst_nolepAux;   //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tenacious_tst_nolep;    //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tenacious_tst_nolepAux; //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tst_nolep;              //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tst_nolepAux;           //!
   std::shared_ptr<xAOD::MissingETContainer>    met_cst;                    //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_cstAux;                 //!
   //      std::shared_ptr<xAOD::MissingETContainer> met_tst_nomuon; //!
   //      std::shared_ptr<xAOD::MissingETAuxContainer> met_tst_nomuonAux; //!
   //      std::shared_ptr<xAOD::MissingETContainer> met_tst_noelectron; //!
   //      std::shared_ptr<xAOD::MissingETAuxContainer> met_tst_noelectronAux; //!
   std::shared_ptr<xAOD::MissingETContainer>    met_tst_nophoton;    //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_tst_nophotonAux; //!
   std::shared_ptr<xAOD::MissingETContainer>    met_track;           //!
   std::shared_ptr<xAOD::MissingETAuxContainer> met_trackAux;        //!
   const xAOD::MissingETContainer *             met_truth;           //! // we do not own it!!!

   //
   // objects we own
   //

   // before selection criteria
   xAOD::MuonContainer       *muons;        //!
   xAOD::ShallowAuxContainer *muonsAux;     //!
   xAOD::ElectronContainer   *electrons;    //!
   xAOD::ShallowAuxContainer *electronsAux; //!
   xAOD::JetContainer        *jets;         //!
   xAOD::ShallowAuxContainer *jetsAux;      //!
   xAOD::JetContainer        *fatjets;      //!
   xAOD::ShallowAuxContainer *fatjetsAux;   //!
   xAOD::JetContainer        *trackjets;    //!
   xAOD::ShallowAuxContainer *trackjetsAux; //!
   xAOD::PhotonContainer     *photons;      //!
   xAOD::ShallowAuxContainer *photonsAux;   //!
   xAOD::TauJetContainer     *taus;         //!
   xAOD::ShallowAuxContainer *tausAux;      //!

   // after selection criteria
   xAOD::MuonContainer     allMuons;          //!
   xAOD::MuonContainer     contMuons;         //!
   xAOD::MuonContainer     baselineMuons;     //!
   xAOD::MuonContainer     goodMuons;         //!
   xAOD::ElectronContainer allElectrons;      //!
   xAOD::ElectronContainer contElectrons;     //!
   xAOD::ElectronContainer baselineElectrons; //!
   xAOD::ElectronContainer goodElectrons;     //!
   xAOD::JetContainer      allJets;           //!
   xAOD::JetContainer      goodJets;          //!
   xAOD::JetContainer      allFatJets;        //!
   xAOD::JetContainer      goodFatJets;       //!
   xAOD::JetContainer      allTrackJets;      //!
   xAOD::JetContainer      goodTrackJets;     //!
   xAOD::PhotonContainer   allPhotons;        //!
   xAOD::PhotonContainer   baselinePhotons;   //!
   xAOD::PhotonContainer   goodPhotons;       //!
   xAOD::TauJetContainer   allTaus;           //!
   xAOD::TauJetContainer   baselineTaus;      //!
   xAOD::TauJetContainer   goodTaus;          //!
};
} // namespace Analysis

#endif
