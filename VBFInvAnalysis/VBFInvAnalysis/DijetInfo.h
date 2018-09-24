/* DijetInfo.h
 * The DijetInfo base class is a collection of m_jj, deta_jj, and dphi_jj
 * for a pair of jets. A bunch of different subclasses implement different
 * algorithms for picking different jet pairs.
 * The DijetInfo class is responsible for writing itself to the output tree,
 * since different combinations might do different things. e.g. picking the
 * leading two jets does not require storing the jet indices, but picking
 * the biggest combination does.
 * Ben Rosser <bjr@sas.upenn.edu>
 */

#ifndef __Analysis_DijetInfo__
#define __Analysis_DijetInfo__

#include "TTree.h"
#include "TLorentzVector.h"

namespace Analysis {
    // Dijet Info base class.
    class DijetInfo;

    // Dijet Info - select the leading two jets by p_T.
    class LeadingDijetInfo;

    // Dijet Info - select the jets which give biggest p_T.
    class MaxDijetInfo;

    // Version of MaxDijetInfo algorithm that can be more than
    class MaxAlljetInfo;

    // DijetInfo - select the jets which give value closest to some other value.
    //class BestDijetInfo;
}

class Analysis::DijetInfo {

public:
    DijetInfo(std::string prefix, std::string algorithm);

    // Reset the dijet info object.
    virtual void reset();

    // Create branches for this dijet info.
    // Default version of this creates branches for mass, deta, and dphi.
    virtual void attachToTree(TTree *tree);

    // Compute the different quantities. Must be implemented by subclasses!
    virtual void compute(std::vector<TLorentzVector> jets) = 0;

    // Define some getters!
    float getMass();
    float getDEta();
    float getDPhi();

// These things want to be accessed by subclasses, so... use protected instead of private.
protected:

    // Prefix when naming branches.
    std::string m_prefix;

    // Algorithm name, used when naming branches.
    std::string m_algorithm;

    // Quantities that every version of this will have.
    float m_mass = 0.0;
    float m_deta = -100.0;
    float m_dphi = -100.0;
};

class Analysis::LeadingDijetInfo : public Analysis::DijetInfo {

public:
    LeadingDijetInfo(std::string prefix);

    virtual void compute(std::vector<TLorentzVector> jets);

};

class Analysis::MaxDijetInfo : public Analysis::DijetInfo {

public:
    MaxDijetInfo(std::string prefix);

    virtual void reset();
    virtual void compute(std::vector<TLorentzVector> jets);
    virtual void attachToTree(TTree *tree);

private:
    int m_firstJet = -1;
    int m_secondJet = -1;

};

#endif
