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


}

