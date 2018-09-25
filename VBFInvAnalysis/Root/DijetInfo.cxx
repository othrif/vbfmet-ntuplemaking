// Implementation of Analysis::DijetFinder, a class which-- given
// a set of TLorentzVectors corresponding to (truth/parton) jets--
// computes various dijet pairs and observables.

#include "VBFInvAnalysis/DijetInfo.h"

#include "TMath.h"

// Use the DijetInfo class and its subclasses.
using Analysis::DijetInfo;
using Analysis::LeadingDijetInfo;
using Analysis::MaxDijetInfo;
using Analysis::BestDijetInfo;

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

std::string DijetInfo::getName() {
    return this->m_algorithm;
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

MaxDijetInfo::MaxDijetInfo(std::string prefix)
    : DijetInfo(prefix, "max") {

}

void MaxDijetInfo::reset() {
    // In addition to resetting everything else, reset the jet indices.
    DijetInfo::reset();
    m_firstJet = -1;
    m_secondJet = -1;
}

void MaxDijetInfo::attachToTree(TTree* tree) {
    // Call the base attach to tree.
    DijetInfo::attachToTree(tree);

    // Create branches for the jet indices.
    tree->Branch((m_prefix + "_firstJet_" + m_algorithm).c_str(), &m_firstJet);
    tree->Branch((m_prefix + "_secondJet_" + m_algorithm).c_str(), &m_secondJet);
}

void MaxDijetInfo::compute(std::vector<TLorentzVector> jets) {
    if (jets.size() >= 2) {
        // Loop through all jets.
        for (unsigned int i = 0; i < jets.size(); i++) {
            // Loop over (i, j) pairs such that i != j and we never get (j, i).
            for (unsigned int j = i + 1; j < jets.size(); j++) {
                float mjj = (jets.at(i) + jets.at(j)).M();

                // If this mjj is better than the current maximum, use it.
                if (mjj > m_mass) {
                    m_mass = mjj;
                    m_deta = TMath::Abs(jets.at(i).Eta() - jets.at(j).Eta());
                    m_dphi = TMath::Abs(jets.at(i).Phi() - jets.at(j).Phi());
                    m_firstJet = i;
                    m_secondJet = j;
                }
            }
        }
    }

}

BestDijetInfo::BestDijetInfo(std::string prefix, std::string algorithm)
    : DijetInfo(prefix, algorithm) {

}

void BestDijetInfo::reset() {
    // In addition to resetting everything else, reset the jet indices.
    DijetInfo::reset();
    m_firstJet = -1;
    m_secondJet = -1;
    m_truthMjj = 0;
}

void BestDijetInfo::attachToTree(TTree* tree) {
    // Call the base attach to tree.
    DijetInfo::attachToTree(tree);

    // Create branches for the jet indices.
    tree->Branch((m_prefix + "_firstJet_" + m_algorithm).c_str(), &m_firstJet);
    tree->Branch((m_prefix + "_secondJet_" + m_algorithm).c_str(), &m_secondJet);
}

void BestDijetInfo::compute(std::vector<TLorentzVector> jets) {
    // If the "truth" mjj is 0 or less, just use that.
    // This likely means there were not two jets.
    if (m_truthMjj <= 0) {
        m_mass = m_truthMjj;
    }

    if (jets.size() >= 2) {
        // Loop through all jets.
        for (unsigned int i = 0; i < jets.size(); i++) {
            // Loop over (i, j) pairs such that i != j and we never get (j, i).
            for (unsigned int j = i + 1; j < jets.size(); j++) {
                float mjj = (jets.at(i) + jets.at(j)).M();

                // Figure out how close we are to the 'true' mjj.
                float deltaNew = TMath::Abs(mjj - m_truthMjj);
                float deltaOld = TMath::Abs(m_mass - m_truthMjj);
                if (deltaNew < deltaOld || m_mass <= 0) {
                    m_mass = mjj;
                    m_deta = TMath::Abs(jets.at(i).Eta() - jets.at(j).Eta());
                    m_dphi = TMath::Abs(jets.at(i).Phi() - jets.at(j).Phi());
                    m_firstJet = i;
                    m_secondJet = j;
                }
            }
        }
    }
}

void BestDijetInfo::setTruthMjj(float truthMjj) {
    m_truthMjj = truthMjj;
}
