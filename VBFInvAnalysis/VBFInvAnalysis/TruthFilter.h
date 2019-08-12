#ifndef TRUTHFILTER_H
#define TRUTHFILTER_H

#include "xAODJet/JetContainer.h"
#include <TLorentzVector.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <vector>

namespace VBFInvAnalysis {

bool passTruthFilter(const xAOD::JetContainer *truthJets, double JetEtaFilter, double JetpTFilter, double MjjFilter,
                     double PhijjFilter, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi,
		     const xAOD::TruthParticleContainer *truthElectrons=nullptr, const xAOD::TruthParticleContainer *truthTaus=nullptr);

} // namespace VBFInvAnalysis

#endif
