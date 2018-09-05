/* PartonCluster - clusters partons from truth using fastjet.
* Attempts to reproduce "pre-parton-shower" jets.
* Ben Rosser <bjr@sas.upenn.edu> 7/9/2018
*/

#include <VBFInvAnalysis/SherpaPartonCluster.h>

SherpaPartonCluster::SherpaPartonCluster(fastjet::JetDefinition* jetDef,
                            const xAOD::TruthParticleContainer* truthParticles) {
    // I know there's some fancier way to do this, but this is fine.
    this->jetDef = jetDef;
    this->truthParticles = truthParticles;

    // Fill the map from the container.
    this->fillMapFromContainer();
}

void SherpaPartonCluster::fillMapFromContainer() {

    int status;
    for (const auto* particle: *(this->truthParticles)) {
        status = particle->status();

        // Check if we already created the vector. If not, create it!
        if (this->truthByStatus.count(status) == 0) {
            this->truthByStatus[status] = std::vector<const xAOD::TruthParticle_v1*>();
        }

        // Add the particle to a status-code-indexed vector.
        this->truthByStatus[status].push_back(particle);
    }
}

std::vector<fastjet::PseudoJet> SherpaPartonCluster::getPartonJets(int status, bool noCluster) {

    // Inputs to fastjet.
    std::vector<fastjet::PseudoJet> clusterInputs;

    // Iterate through the partons. We have already indexed them by status code!
    std::vector<const xAOD::TruthParticle_v1*> particleVector = this->truthByStatus[status];
    for (const auto* particle: particleVector) {

        // Reject anything that's not a parton.
        if (!particle->isParton()) {
            continue;
        }

        // Add the particle to the array of things to cluster.
        fastjet::PseudoJet parton(particle->px(), particle->py(), particle->pz(), particle->e());
        clusterInputs.push_back(parton);
    }

    // Now, run fastjet with the given jet definition, unless we were told not to.
    if (noCluster) {
        return clusterInputs;
    } else {
        fastjet::ClusterSequence sequence(clusterInputs, *(this->jetDef));
        std::vector<fastjet::PseudoJet> partonJets = fastjet::sorted_by_pt(sequence.inclusive_jets());
        return partonJets;
    }
}

// XXX: This function only supports pT filtering. We could add (eta, y, ET cuts).
std::vector<TLorentzVector> SherpaPartonCluster::getRootPartonJets(int status, int ptMin, bool noCluster) {

    // First, get partonJets.
    std::vector<fastjet::PseudoJet> partonJets = this->getPartonJets(status, noCluster);

    // Now, process them into TLorentzVectors.
    std::vector<TLorentzVector> rootPartonJets;
    fastjet::PseudoJet partonJet;
    for (unsigned int i = 0; i < partonJets.size(); i++) {
        partonJet = partonJets.at(i);

        // Ignore things with pT = 0, ROOT complains about them... and they're probably beams.
        if (partonJet.perp() > 0) {
            TLorentzVector rootJet;
            rootJet.SetPtEtaPhiE(partonJet.perp(), partonJet.eta(), partonJet.phi(), partonJet.e());

            // Apply (optional!) pt cut.
            if (rootJet.Perp() > (ptMin * 1e3)) {
                rootPartonJets.push_back(rootJet);
            }
        }
    }

    // Return the array of (filtered) TLorentzVectors.
    return rootPartonJets;
}

std::vector<const xAOD::TruthParticle_v1*> SherpaPartonCluster::getParticles(int status) {
    // This is really simple now!
    return this->truthByStatus[status];
}
