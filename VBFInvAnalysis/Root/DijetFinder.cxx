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
    m_maxDijets = new Analysis::MaxDijetInfo(prefix);
    m_bestDijets = new Analysis::BestDijetInfo(prefix, "bestLead");

    // Insert pointers into this map, for easier reference.
    m_dijetAlgos[m_leadingDijets->getName()] = m_leadingDijets;
    m_dijetAlgos[m_maxDijets->getName()] = m_maxDijets;
    m_dijetAlgos[m_bestDijets->getName()] = m_bestDijets;

}

Analysis::DijetInfo* DijetFinder::getDijetInfo(std::string name) {
    return m_dijetAlgos[name];
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
    m_maxDijets->attachToTree(tree);
    m_bestDijets->attachToTree(tree);
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
    m_maxDijets->reset();
    m_bestDijets->reset();
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
    m_maxDijets->compute(m_prunedJets);

    // We *don't* call the best version explicitly here, unfortunately.
    // We need to get the right value from elsewhere and pass it in.

}

std::vector<TLorentzVector>* DijetFinder::getPrunedJets() {
    return &m_prunedJets;
}
