/* PartonClusterer.h
 * Given a set of truth particles, the PartonClusterer class will cluster
 * them (using fastjet) and produce a set of "parton jets".
 * The parton jets are then passed to a DijetFinder class in order to
 * compute various dijet kinematics (primarily mjj).
 * It will also write the particles themselves to a TTree.
 * Ben Rosser <bjr@sas.upenn.edu>
 */

#ifndef __Analysis_PartonClusterer__
#define __Analysis_PartonClusterer__

#include "TTree.h"
#include "TLorentzVector.h"

// We need the truth particle object.
#include <xAODTruth/TruthParticle.h>

// We need some fastjet headers, too.
#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

// A PartonClusterer contains a DijetFinder, so we need that header.
#include <VBFInvAnalysis/DijetFinder.h>

namespace Analysis {
    class PartonClusterer;
}

class Analysis::PartonClusterer {

public:
    PartonClusterer(std::string prefix, float minPt, float antiktDR, bool noCluster);

    // Similar to the outObject classes, define an attachToTree.
    // This will set up branches.
    void attachToTree(TTree *tree);

    // Helper function to store truth particles.
    void storeParticles(std::vector<const xAOD::TruthParticle*> particles);

    // Cluster the partons.
    void clusterPartons(std::vector<const xAOD::TruthParticle*> particles);

    // Returns the computed parton jets.
    std::vector<TLorentzVector> getPartonJets();

    // Compute mjj, given the parton jets (or not), by calling dijet finder.
    void computeMjj(std::vector<TLorentzVector> partonJets);
    void computeMjj();

    // Resets everything from event to event.
    void reset();

private:
    // The name to prefix all branches with!
    std::string m_prefix;

    // Minimum pT used as a cut when computing dijet infos.
    float m_minPt;

    // The anti-k_T delta R definition for running fastjet.
    float m_antiktDR;

    // A boolean flag indicating whether we should or should not actually cluster.
    // Defaults to false.
    bool m_noCluster = false;

    // The associated DijetFinder object.
    Analysis::DijetFinder* m_dijetFinder; //!

    // Vectors for the particle kinematics, plus mass, plus pdg ID.
    std::vector<float> m_particlePt; //!
    std::vector<float> m_particleEta; //!
    std::vector<float> m_particlePhi; //!
    std::vector<float> m_particleE; //!
    std::vector<float> m_particleMass; //!
    std::vector<int> m_particlePID; //!

    // The number of particles per event.
    unsigned int m_numParticles = 0;

    // A cached/stored copy of the parton jets.
    std::vector<TLorentzVector> m_partonJets; //!

};

#endif
