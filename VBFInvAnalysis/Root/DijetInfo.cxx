// Implementation of Analysis::DijetFinder, a class which-- given
// a set of TLorentzVectors corresponding to (truth/parton) jets--
// computes various dijet pairs and observables.

#include "VBFInvAnalysis/DijetInfo.h"

#include "TMath.h"

// Use the DijetInfo class and its subclasses.
using Analysis::DijetInfo;
using Analysis::LeadingDijetInfo;

DijetInfo::DijetInfo(std::string prefix, std::string algorithm)
    : m_prefix(prefix), m_algorithm(algorithm) {

}

void DijetInfo::reset() {
    m_mass = 0;
    m_deta = -100;
    m_dphi = -100;
}

void DijetInfo::attachToTree(TTree* tree) {
    // Always create branches for mjj, deta, and dphi.
    tree->Branch((m_prefix + "_Mjj_" + m_algorithm).c_str(), &m_mass);
    tree->Branch((m_prefix + "_deta_" + m_algorithm).c_str(), &m_deta);
    tree->Branch((m_prefix + "_dphi_" + m_algorithm).c_str(), &m_dphi);
}

float DijetInfo::getMass() {
    return this->m_mass;
}

float DijetInfo::getDEta() {
    return this->m_deta;
}

float DijetInfo::getDPhi() {
    return this->m_dphi;
}

LeadingDijetInfo::LeadingDijetInfo(std::string prefix)
    : DijetInfo(prefix, "lead") {
}

void LeadingDijetInfo::compute(std::vector<TLorentzVector> jets) {
    // If there are at least two jets, pick the leading two jets!
    if (jets.size() >= 2) {
        m_mass = (jets.at(0) + jets.at(1)).M();
        m_deta = TMath::Abs(jets.at(0).Eta() - jets.at(1).Eta());
        m_dphi = TMath::Abs(jets.at(0).Phi() - jets.at(1).Phi());
    }
}
