#ifndef GETTRUTHBOSONP4_H
#define GETTRUTHBOSONP4_H

#include <xAODTruth/TruthParticleContainer.h>
#include <TLorentzVector.h>
#include <vector>

namespace VBFInvAnalysis {

// This function should run on derivations as well on xAOD
// in case of xAOD:
// - key: "TruthParticles"
// in case of DxAOD:
// - keys: "TruthParticles", "TruthElectrons", "TruthMuons", "TruthPhotons"
TLorentzVector getTruthBosonP4(const xAOD::TruthParticleContainer *truthParticles,
                               const xAOD::TruthParticleContainer *truthElectrons,
                               const xAOD::TruthParticleContainer *truthMuons,
                               const xAOD::TruthParticleContainer *truthPhotons, Bool_t doDressing = kTRUE);
  // This function gives a simple boson Pt
  // This function should run on derivations as well on xAOD
  // in case of xAOD:
  // - key: "TruthParticles"
  // in case of DxAOD:
  // - keys: "TruthParticles", "TruthElectrons", "TruthMuons", "TruthPhotons"
  TLorentzVector getTruthBosonP4_simple(const xAOD::TruthParticleContainer *truthParticles);
} // namespace VBFInvAnalysis


#endif
