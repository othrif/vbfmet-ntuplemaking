#include <VBFInvAnalysis/TruthFilter.h>

namespace VBFInvAnalysis {

bool GreaterPt(const TLorentzVector &a, const TLorentzVector &b)
{
   return a.Pt() > b.Pt();
}

bool passTruthFilter(const xAOD::JetContainer *truthJets, double JetEtaFilter, double JetpTFilter, double MjjFilter,
                     double PhijjFilter, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi)
{

   bool                        passFilter = false;
   std::vector<TLorentzVector> jets;
   TLorentzVector              tmp;
   for (const auto &jet : *truthJets) {
      tmp.SetPtEtaPhiM(jet->pt(), jet->eta(), jet->phi(), jet->m());
      if (fabs(jet->eta()) < JetEtaFilter && jet->pt() > JetpTFilter) jets.push_back(tmp);
   }

   std::sort(jets.begin(), jets.end(), GreaterPt);

   if (jets.size() >= 2) {
      e_DiJetMass = (jets.at(0) + jets.at(1)).M();
      e_JetsDEta  = fabs(jets.at(0).Eta() - jets.at(1).Eta());
      e_JetsDPhi  = fabs(jets.at(0).DeltaPhi(jets.at(1)));
      // std::cout << "mjj " << e_DiJetMass << ", Etajj " << e_JetsDEta << ", Phijj " << e_JetsDPhi << std::endl;
      if (e_DiJetMass > MjjFilter && e_JetsDPhi < PhijjFilter) passFilter = true;
   }

   return passFilter;
}

} // namespace VBFInvAnalysis
