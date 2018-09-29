/* DijetFinder.h
 * Class to "find" various pairs of jets, and compute quantities
 * like m_jj, deta_jj, dphi_jj, for each pairing.
 * The quantities get written to a provided ttree automatically.
 * This is used from the VBFInvSherpaTruth algorithm.
 * This class is also responsible for writing out the jets themselves.
 * Ben Rosser <bjr@sas.upenn.edu>
 */

#ifndef __Analysis_DijetFinder__
#define __Analysis_DijetFinder__

#include "TTree.h"
#include "TLorentzVector.h"

#include <VBFInvAnalysis/DijetInfo.h>

namespace Analysis {
    class DijetFinder;
}

class Analysis::DijetFinder {

public:
    DijetFinder(std::string prefix, float minPt);

    // Similar to the outObject classes, define an attachToTree.
    // This will set up branches.
    void attachToTree(TTree *tree);

    // Computes mjj given a pointer to a collection of jets.
    void computeMjj(std::vector<TLorentzVector>* jets);

    // Returns (pointer to) dijet info object given the algorithm name
    // used to calculate this dijet info.
    Analysis::DijetInfo* getDijetInfo(std::string name);

    // Retrieve the 'pruned' jets (after applying the pt cut).
    std::vector<TLorentzVector>* getPrunedJets();

    // Try to match the jets stored here against another set of jet vectors
    // e.g. truth parton matching.
    void matchJets(std::vector<TLorentzVector>* otherJets, float minDR);

    // Resets the arrays in the tree.
    void reset();

private:
    // The name to prefix all branches with!
    std::string m_prefix;

    // Minimum pT used as a cut when computing dijet infos.
    float m_minPt;

    // Jet kinematics, to write to the attached ttree.
    std::vector<float> m_jetPt; //!
    std::vector<float> m_jetEta; //!
    std::vector<float> m_jetPhi; //!
    std::vector<float> m_jetE; //!

    // The pruned set of jets that actually get passed to the DijetInfos.
    std::vector<TLorentzVector> m_prunedJets; //!

    // Number of jets.
    unsigned int m_numJets = 0;

    // Pointer to jet vector.
    std::vector<TLorentzVector>* m_jets; //!

    // The dijet info objects that we are going to use.
    Analysis::LeadingDijetInfo* m_leadingDijets; //!
    Analysis::MaxDijetInfo* m_maxDijets; //!
    Analysis::BestDijetInfo* m_bestDijets; //!
    Analysis::MaxAlljetInfo* m_maxAlljets; //!

    // A map of algorithm names to analysis objects,
    std::map<std::string, Analysis::DijetInfo*> m_dijetAlgos; //!

    // Store information about jet/parton matching. This probably belongs
    // in another class, except... all the necessary pieces exist here.
    std::vector<int> m_matches; //!
    std::vector<float> m_deltaRs; //!
    unsigned int m_numUnmatched = 0;
};

#endif
