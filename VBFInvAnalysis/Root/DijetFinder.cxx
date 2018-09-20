// Implementation of Analysis::DijetFinder, a class which-- given
// a set of TLorentzVectors corresponding to (truth/parton) jets--
// computes various dijet pairs and observables.

#include "VBFInvAnalysis/DijetFinder.h"

// Use the DijetFinder class. This avoids having to define all the
// methods as "Analysis::DijetFinder" or inside a namespace block.
using Analysis::DijetFinder;

namespace Analysis {

    class SortByPt {
    public:
        bool operator() (const TLorentzVector &a, const TLorentzVector &b) const {
            return (a.Perp() > b.Perp());
        }
    };
}


DijetFinder::DijetFinder(std::string prefix, float minPt)
    : m_prefix(prefix), m_minPt(minPt) {

    // Create dijet info objects.
    m_leadingDijets = new Analysis::LeadingDijetInfo(prefix);

}

void DijetFinder::attachToTree(TTree* tree) {

    // Create branches to store the jet kinematics.
    tree->Branch((m_prefix + "_jetPt").c_str(), &m_jetPt);
    tree->Branch((m_prefix + "_jetEta").c_str(), &m_jetEta);
    tree->Branch((m_prefix + "_jetPhi").c_str(), &m_jetPhi);
    tree->Branch((m_prefix + "_jetE").c_str(), &m_jetE);

    // Store the number of jets, as well, because that's good to know.
    tree->Branch((m_prefix + "_numJets").c_str(), &m_numJets);

    // Call DijetInfo classes, pass the tree to them too!
    m_leadingDijets->attachToTree(tree);
}

void DijetFinder::reset() {
    // Clear the jet kinematics vectors.
    m_jetPt.clear();
    m_jetEta.clear();
    m_jetPhi.clear();
    m_jetE.clear();

    // Reset number of jets.
    m_numJets = 0;

    // Reset pruned jet objects.
    m_prunedJets.clear();

    // Reset dijet info classes.
    m_leadingDijets->reset();
}

void DijetFinder::computeMjj(std::vector<TLorentzVector> jets) {

    TLorentzVector jet;

    // Sort the jets by pT, just to be safe.
    std::sort(jets.begin(), jets.end(), SortByPt());

    // Loop over the jets, write out their kinematics.
    for (unsigned int i = 0; i < jets.size(); i++) {
        jet = jets.at(i);
        m_jetPt.push_back(jet.Pt());
        m_jetEta.push_back(jet.Eta());
        m_jetPhi.push_back(jet.Phi());
        m_jetE.push_back(jet.E());
    }

    // Assign number of jets.
    m_numJets = jets.size();

    // Now we want to take the jets, and apply the minPt cut.
    // (Any other cuts, for example an eta/phi/E cut, could also be done here).
    for (unsigned int i = 0; i < jets.size(); i++) {
        jet = jets.at(i);
        if (((jet.Pt())/1000.) >= m_minPt) {
            m_prunedJets.push_back(jet);
        }
    }

    // Call the relevant DijetInfo classes too, pass m_prunedJets.
    m_leadingDijets->compute(m_prunedJets);

}
