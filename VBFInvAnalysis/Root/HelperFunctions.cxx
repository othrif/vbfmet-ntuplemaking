#include "CxxUtils/fpcompare.h"
#include "VBFInvAnalysis/HelperFunctions.h"
#include "xAODBase/IParticle.h"

namespace HelperFunctions {
   Bool_t comparePt(const xAOD::IParticle *a, const xAOD::IParticle *b)
   {
      return CxxUtils::fpcompare::greater(a->pt(), b->pt());
   }


  void computeMaxjj(xAOD::JetContainer jets, double &e_DiJetMass_Max, double &e_JetsDEta_Max)
{

  std::vector<TLorentzVector> jet_tlv;
  TLorentzVector jet_tmp;
  int num_jet = 0;
  float tmpJetMass, tmpDEta;

  for (auto jet : jets){
    if(jet->pt() > 40000){
      jet_tmp.SetPtEtaPhiE(jet->pt(),jet->eta(),jet->phi(),jet->e());
      jet_tlv.push_back(jet_tmp);
      num_jet++;
    }
  }
  if (num_jet >= 2)
   for (int i = 0; i < num_jet; i++)
     for (int j = i + 1; j < num_jet; j++) {
       TLorentzVector jet_sum = jet_tlv.at(i) + jet_tlv.at(j);
       tmpDEta = fabs(jet_tlv.at(i).Eta()-jet_tlv.at(j).Eta());
       tmpJetMass = (jet_sum).M();
       if (tmpJetMass > e_DiJetMass_Max) e_DiJetMass_Max = tmpJetMass;
       if (tmpDEta > e_JetsDEta_Max) e_JetsDEta_Max = tmpDEta;
     }
   }

  void computejj(xAOD::JetContainer jets, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi)
  {

    std::vector<TLorentzVector> jet_tlv;
    TLorentzVector jet_tmp;

    if (jets.size() >= 2 && jets.at(0)->pt() > 40000 && jets.at(1)->pt() > 40000){
      jet_tmp.SetPtEtaPhiE(jets.at(0)->pt(),jets.at(0)->eta(),jets.at(0)->phi(),jets.at(0)->e());
      jet_tlv.push_back(jet_tmp);
      jet_tmp.SetPtEtaPhiE(jets.at(1)->pt(),jets.at(1)->eta(),jets.at(1)->phi(),jets.at(1)->e());
      jet_tlv.push_back(jet_tmp);
      
      TLorentzVector jet_sum = jet_tlv.at(0) + jet_tlv.at(1);
      e_JetsDEta = fabs(jet_tlv.at(0).Eta()-jet_tlv.at(1).Eta());
      e_DiJetMass = (jet_sum).M();
      e_JetsDPhi = fabs(jet_tlv.at(0).DeltaPhi(jet_tlv.at(1)));
    }
  }

  void computeMETj(TLorentzVector met, xAOD::JetContainer jets, double &e_met_j1_dphi, double &e_met_j2_dphi)
  {

    std::vector<TLorentzVector> jet_tlv;
    TLorentzVector jet_tmp;
    
    if (jets.size() >= 2 && jets.at(0)->pt() > 40000 && jets.at(1)->pt() > 40000){
      jet_tmp.SetPtEtaPhiE(jets.at(0)->pt(),jets.at(0)->eta(),jets.at(0)->phi(),jets.at(0)->e());
      jet_tlv.push_back(jet_tmp);
      jet_tmp.SetPtEtaPhiE(jets.at(1)->pt(),jets.at(1)->eta(),jets.at(1)->phi(),jets.at(1)->e());
      jet_tlv.push_back(jet_tmp);

      TLorentzVector jet_sum = jet_tlv.at(0) + jet_tlv.at(1);
      e_met_j1_dphi = fabs(jet_tlv.at(0).DeltaPhi(met));
      e_met_j2_dphi = fabs(jet_tlv.at(1).DeltaPhi(met));
    }
  }


}

