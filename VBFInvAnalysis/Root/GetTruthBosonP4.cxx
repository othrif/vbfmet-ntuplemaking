#include <VBFInvAnalysis/GetTruthBosonP4.h>

namespace VBFInvAnalysis {

TLorentzVector getTruthBosonP4(const xAOD::TruthParticleContainer *truthParticles,
                               const xAOD::TruthParticleContainer *truthElectrons,
                               const xAOD::TruthParticleContainer *truthMuons,
                               const xAOD::TruthParticleContainer *truthPhotons, Bool_t doDressing)
{

   // Select neutrinos:
   // - Status 3 neutrinos from original truth particle container.
   // - DxAOD key: "TruthParticles"
   std::vector<TLorentzVector> neutrinos;
   for (const auto &part : *truthParticles) {
      int pdg = abs(part->pdgId());
      if (pdg != 12 && pdg != 14 && pdg != 16) continue;
      if (part->status() != 3) continue;
      TLorentzVector v(part->px(), part->py(), part->pz(), part->e());
      neutrinos.push_back(v);
   }

   // Select taus:
   // - Status 3 taus from original truth particle container.
   // - DxAOD key: "TruthParticles"
   std::vector<TLorentzVector> taus;
   for (const auto &part : *truthParticles) {
      int pdg = abs(part->pdgId());
      if (pdg != 15) continue;
      if (part->status() != 3) continue;
      TLorentzVector v(part->px(), part->py(), part->pz(), part->e());
      taus.push_back(v);
   }

   // Count number of electrons and muons from W/Z decay
   // - Use status 3 electrons/muon from original truth particle container.
   unsigned int nElectrons = 0;
   unsigned int nMuons     = 0;
   for (const auto &part : *truthParticles) {
      if (part->status() != 3) continue;
      int pdg = abs(part->pdgId());
      if (pdg == 11) nElectrons++;
      if (pdg == 13) nMuons++;
   }

   // Select electrons:
   // - Status 1 from truth electrons container.
   // - check number of children: assumes W/Z decays appear first in the container!!
   // - Dressing: check if the truth particles are already dressed in derivation
   // - DxAOD key: "TruthElectrons"
   // - Can use original truth particle container if not skimmed
   std::vector<TLorentzVector> electrons;
   bool                        needDressing = false;
   for (const auto &part : *truthElectrons) {
      if (part->status() != 1) continue;
      if (abs(part->pdgId()) != 11) continue;
      TLorentzVector v(part->px(), part->py(), part->pz(), part->e());
      if (doDressing) {
         try {
            v.SetPtEtaPhiE(part->auxdata<float>("pt_dressed"), part->auxdata<float>("eta_dressed"),
                           part->auxdata<float>("phi_dressed"), part->auxdata<float>("e_dressed"));
         } catch (...) {
            needDressing = true;
         }
      }
      electrons.push_back(v);
      if (electrons.size() >= nElectrons) break;
   }

   // Select muons:
   // - Status 1 from truth muons container.
   // - check number of children: assumes W/Z decays appear first in the container!!
   // - Dressing: check if the truth particles are already dressed in derivation
   // - DxAOD key: "TruthMuons"
   // - Can use original truth particle container if not skimmed
   std::vector<TLorentzVector> muons;
   for (const auto &part : *truthMuons) {
      if (part->status() != 1) continue;
      if (abs(part->pdgId()) != 13) continue;
      TLorentzVector v(part->px(), part->py(), part->pz(), part->e());
      if (doDressing) {
         try {
            v.SetPtEtaPhiE(part->auxdata<float>("pt_dressed"), part->auxdata<float>("eta_dressed"),
                           part->auxdata<float>("phi_dressed"), part->auxdata<float>("e_dressed"));
         } catch (...) {
            needDressing = true;
         }
      }
      muons.push_back(v);
      if (muons.size() >= nMuons) break;
   }

   // Dressing electrons and muons
   // - if not already done in derivations
   // - Use status 1 photons from truth photon container
   // - DxAOD key: TruthPhotons
   // - Can use original truth particle container if not skimmed
   if (doDressing && needDressing) {
      for (auto &electron : electrons) {
         TLorentzVector undressedEl = electron;
         for (const auto &part : *truthPhotons) {
            if (part->pdgId() != 22) continue;
            if (part->status() != 1) continue;
            TLorentzVector ph(part->px(), part->py(), part->pz(), part->e());
            if (ph.DeltaR(undressedEl) < 0.1) {
               electron += ph;
            }
         }
      }
      for (auto &muon : muons) {
         TLorentzVector undressedMu = muon;
         for (const auto &part : *truthPhotons) {
            if (part->pdgId() != 22) continue;
            if (part->status() != 1) continue;
            TLorentzVector ph(part->px(), part->py(), part->pz(), part->e());
            if (ph.DeltaR(undressedMu) < 0.1) {
               muon += ph;
            }
         }
      }
   }

   // Compute vector boson 4-momentum
   TLorentzVector vV(0, 0, 0, 0);
   for (auto &neutrino : neutrinos) {
      vV += neutrino;
   }
   for (auto &tau : taus) {
      vV += tau;
   }
   for (auto &electron : electrons) {
      vV += electron;
   }
   for (auto &muon : muons) {
      vV += muon;
   }

   return vV;
}
TLorentzVector getTruthBosonP4_simple(const xAOD::TruthParticleContainer *truthParticles)
  {
    // Select neutrinos:
    // - Status 3 neutrinos from original truth particle container.
    // - DxAOD key: "TruthParticles"

    std::vector<TLorentzVector> leptons;
    for (const auto& part : *truthParticles) {
      int pdg = abs(part->pdgId());
      if (pdg != 12 && pdg != 14 && pdg != 16 && pdg != 11 && pdg != 13 && pdg != 15) continue;//3 neutrinos, el, mu, tau
      if (part->status() != 3) continue;
      TLorentzVector v(part->px(), part->py(), part->pz(), part->e());
      leptons.push_back(v);
    }

    // Compute vector boson 4-momentum
    TLorentzVector vV(0, 0, 0, 0);

    for (auto& lepton : leptons) {
      vV += lepton;
    }

    return vV;
  }
} // namespace VBFInvAnalysis
