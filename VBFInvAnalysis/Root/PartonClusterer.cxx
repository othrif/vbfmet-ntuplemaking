// Implementation of Analysis::PartonClusterer, a class which is passed
// a set of TruthParticles. The partons are extracted, clustered using
// fastjet, and then various dijet pairs computed from them.

#include "VBFInvAnalysis/PartonClusterer.h"

// Use the PartonClusterer class to avoid having to declare
// methods as "Analysis::PartonClusterer".
using Analysis::PartonClusterer;

PartonClusterer::PartonClusterer(std::string prefix, float antiktDR, bool noCluster)
    : m_prefix(prefix), m_antiktDR(antiktDR), m_noCluster(noCluster) {

}

void PartonClusterer::attachToTree(TTree* tree) {
    // Create branches to store the particle kinematics
    tree->Branch((m_prefix + "_Pt").c_str(), &m_particlePt);
    tree->Branch((m_prefix + "_Eta").c_str(), &m_particleEta);
    tree->Branch((m_prefix + "_Phi").c_str(), &m_particlePhi);
    tree->Branch((m_prefix + "_E").c_str(), &m_particleE);
    tree->Branch((m_prefix + "_Mass").c_str(), &m_particleMass);

    // Store the particle pdg ID.
    tree->Branch((m_prefix + "_PID").c_str(), &m_particlePID);

    // Store the number of jets, as well, because that's good to know.
    tree->Branch((m_prefix + "_numParticles").c_str(), &m_numParticles);
}

void PartonClusterer::reset() {
    // Clear all the truth particle vectors.
    m_particlePt.clear();
    m_particleEta.clear();
    m_particlePhi.clear();
    m_particleE.clear();
    m_particleMass.clear();
    m_particlePID.clear();

    // Clear the number of truth particles.
    m_numParticles = 0;

    // Clear the stored/cached parton jets.
    m_partonJets.clear();
}

void PartonClusterer::storeParticles(std::vector<const xAOD::TruthParticle*> particles) {

    TLorentzVector rootParticle;

    // Loop through the truth particles, store them in the arrays.
    for (const auto* particle: particles) {

        // Create a TLorentzVector particle.
        rootParticle.SetPxPyPzE(particle->px(), particle->py(), particle->pz(), particle->e());

        // Push the kinematics (and mass) back.
        m_particlePt.push_back(rootParticle.Pt());
        m_particleEta.push_back(rootParticle.Eta());
        m_particlePhi.push_back(rootParticle.Phi());
        m_particleE.push_back(rootParticle.E());
        m_particleMass.push_back(particle->m());

        // Push the particle PID back too.
        m_particlePID.push_back(particle->pdgId());
    }

    // Store the size, too.
    m_numParticles = particles.size();
}

std::vector<TLorentzVector>* PartonClusterer::clusterPartons(std::vector<const xAOD::TruthParticle*> particles) {

    std::vector<fastjet::PseudoJet> clusterInputs;
    std::vector<fastjet::PseudoJet> partonPseudoJets;

    TLorentzVector rootJet;
    fastjet::PseudoJet pseudoJet;

    // First, store the truth particles.
    this->storeParticles(particles);

    // Loop over the particles, extract particles which are actually partons.
    for (const auto* particle: particles) {
        if (!particle->isParton()) {
            continue;
        }

        // Add the particle to the array of things to cluster.
        fastjet::PseudoJet parton(particle->px(), particle->py(), particle->pz(), particle->e());
        clusterInputs.push_back(parton);
    }

    // Now, run fastjet with the given jet definition, unless we were told not to.
    // When we do this, assign m_partonJets local variable (cache the result).
    if (m_noCluster) {
        partonPseudoJets = clusterInputs;
    } else {
        fastjet::JetDefinition jetdef(fastjet::antikt_algorithm, m_antiktDR);
        fastjet::ClusterSequence sequence(clusterInputs, jetdef);
        partonPseudoJets = fastjet::sorted_by_pt(sequence.inclusive_jets());
    }

    // Now, reprocess the parton jets into ROOT TLorentzVectors.
    for (unsigned int i = 0; i < partonPseudoJets.size(); i++) {
        pseudoJet = partonPseudoJets.at(i);

        // I don't think there should be any of these objects with pt <= 0, but ju tin case.
        if (pseudoJet.perp() > 0) {
            rootJet.SetPtEtaPhiE(pseudoJet.perp(), pseudoJet.eta(), pseudoJet.phi(), pseudoJet.e());
            m_partonJets.push_back(rootJet);
        }
    }

    return &m_partonJets;
}

std::vector<TLorentzVector>* PartonClusterer::getPartonJets() {
    return &m_partonJets;
}
