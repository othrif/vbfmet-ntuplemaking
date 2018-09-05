/* PartonCluster - clusters partons from truth using fastjet.
 * Attempts to reproduce "pre-parton-shower" jets.
 * Ben Rosser <bjr@sas.upenn.edu> 7/9/2018
 */

#ifndef VBFInvAnalysis_SherpaPartonCluster_H
#define VBFInvAnalysis_SherpaPartonCluster_H

#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/TruthParticleAuxContainer.h>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

#include <vector>
#include <TLorentzVector.h>

class SherpaPartonCluster {

public:
    // Constructor for SherpaPartonCluster class.
    SherpaPartonCluster(fastjet::JetDefinition* jetDef, const xAOD::TruthParticleContainer* truthParticles);

    // Retrieve parton jets of a given status code, using the JetDefinition.
    std::vector<fastjet::PseudoJet> getPartonJets(int status, bool noCluster);

    // Retrieves parton jets as TLorentzVectors, first filtering by pT and eT.
    std::vector<TLorentzVector> getRootPartonJets(int status, int ptMin, bool noCluster);

    // Retrieves the truth particles of a status code.
    std::vector<const xAOD::TruthParticle_v1*> getParticles(int status);

    // Fills the map from the truth particle container.
    // Maybe shouldn't be public.
    void fillMapFromContainer();

private:

    // Jet definition, used for clustering.
    fastjet::JetDefinition* jetDef;

    // Truth particles, retrieved from xAOD.
    const xAOD::TruthParticleContainer* truthParticles;

    // TODO: this class should somehow cache the results of computation.
    // That will mean less accessing the truthParticles container, I think.

    // Truth particles, sorted by status code.
    std::map<int, std::vector<const xAOD::TruthParticle_v1*>> truthByStatus;

};


#endif
