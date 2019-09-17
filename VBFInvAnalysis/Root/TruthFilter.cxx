#include <VBFInvAnalysis/TruthFilter.h>

namespace VBFInvAnalysis {

  static bool GreaterPt(const TLorentzVector &a, const TLorentzVector &b){
    return a.Pt()>b.Pt();
  }

bool passTruthFilter(const xAOD::JetContainer *truthJets, double JetEtaFilter, double JetpTFilter, double MjjFilter,
                     double PhijjFilter, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi, const xAOD::TruthParticleContainer *truthElectrons, const xAOD::TruthParticleContainer *truthTaus)
{

   bool                        passFilter = false;
   std::vector<TLorentzVector> jets;
   TLorentzVector              tmp;
   static SG::AuxElement::Accessor<unsigned int> acc_classifierParticleOrigin("classifierParticleOrigin");
   for (const auto &jet : *truthJets) {
      tmp.SetPtEtaPhiM(jet->pt(), jet->eta(), jet->phi(), jet->m());
      if (fabs(jet->eta()) < JetEtaFilter && jet->pt() > JetpTFilter){
	bool passOR=true;
	if(truthElectrons){
	  for (const auto &part : *truthElectrons) {
	    if (part->pt() < 20.0e3) continue;
	    if (fabs(part->eta()) > 5.0) continue;
	    if (part->status() != 1) continue;
	    if(!(acc_classifierParticleOrigin(*part)==12 || acc_classifierParticleOrigin(*part)==13)) continue; //require it is w or z
	    if(jet->p4().DeltaR(part->p4())<0.3) passOR=false;
	  }
	}
	if(truthTaus){
	  for (const auto &part : *truthTaus) {
	    if (part->pt() < 20.0e3) continue;
	    if (fabs(part->eta()) > 5.0) continue;
	    if (part->status() == 3) continue;
	    if(!(acc_classifierParticleOrigin(*part)==12 || acc_classifierParticleOrigin(*part)==13)) continue; //require it is w or z
	    if(jet->p4().DeltaR(part->p4())<0.3) passOR=false;
	  }
	}
	
	if(passOR) jets.push_back(tmp);
      }
   }

   std::sort(jets.begin(), jets.end(), GreaterPt);
   //for(unsigned j=0; j<jets.size(); ++j) std::cout << "jet: " << j << " pT: " << jets.at(j).Pt() << std::endl;
   if (jets.size() >= 2) {
      e_DiJetMass = (jets.at(0) + jets.at(1)).M();
      e_JetsDEta  = fabs(jets.at(0).Eta() - jets.at(1).Eta());
      e_JetsDPhi  = fabs(jets.at(0).DeltaPhi(jets.at(1)));
      // std::cout << "mjj " << e_DiJetMass << ", Etajj " << e_JetsDEta << ", Phijj " << e_JetsDPhi << std::endl;
      if (e_DiJetMass > MjjFilter && e_JetsDPhi < PhijjFilter) passFilter = true;
   }

   /* // debug statements
   if(truthTaus){
     if(e_DiJetMass<800.0e3 || !passFilter){
       unsigned iJet=0;
       std::cout << "mjj: " << e_DiJetMass << std::endl;
       for (const auto &jet : *truthJets) {
	 std::cout << "  jet: " << iJet << " pT: " << jet->pt() << " eta: " << jet->eta() << std::endl;
	  for (const auto &part : *truthElectrons) {
	    if (part->pt() < 20.0e3) continue;
	    if (part->status() != 1) continue;
	    std::cout << "    ele: " << " pT: " << part->pt() << " eta: " << part->eta() << " dr: " << part->p4().DeltaR(jet->p4()) << std::endl;
	  }	 
	  for (const auto &part : *truthTaus) {
	    if (part->pt() < 20.0e3) continue;
	    if (part->status() != 1) continue;
	    std::cout << "    tau: " << " pT: " << part->pt() << " eta: " << part->eta() << " dr: " << part->p4().DeltaR(jet->p4()) << std::endl;
	  }	 
	 ++iJet;
       }
     }
     }*/

   return passFilter;
}

} // namespace VBFInvAnalysis
