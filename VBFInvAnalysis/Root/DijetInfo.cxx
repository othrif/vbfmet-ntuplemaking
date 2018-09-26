// Implementation of Analysis::DijetFinder, a class which-- given
// a set of TLorentzVectors corresponding to (truth/parton) jets--
// computes various dijet pairs and observables.

#include "VBFInvAnalysis/DijetInfo.h"

#include "TMath.h"

// std::bitset seems to be what I want as a binary array representation
// (used in AlljetInfo in order to easily sum over an arbitrary number of jets).
#include <bitset>

// Use the DijetInfo class and its subclasses.
using Analysis::DijetInfo;
using Analysis::LeadingDijetInfo;
using Analysis::MaxDijetInfo;
using Analysis::BestDijetInfo;
using Analysis::MaxAlljetInfo;

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

MaxAlljetInfo::MaxAlljetInfo(std::string prefix)
    : DijetInfo(prefix, "maxall") {

}

void MaxAlljetInfo::reset() {
    // In addition to resetting everything else, reset the jet indices.
    DijetInfo::reset();
    m_jetIDs.clear();
}

void MaxAlljetInfo::attachToTree(TTree* tree) {
    // Create branches for the jet indices and m_allj. Don't call base version.
    // We can't meaningfully compute an eta or phi here.
    tree->Branch((m_prefix + "_Mjj_" + m_algorithm).c_str(), &m_mass);
    tree->Branch((m_prefix + "_jets_" + m_algorithm).c_str(), &m_jetIDs);
}

void MaxAlljetInfo::compute(std::vector<TLorentzVector> jets) {
    if (jets.size() == 2) {
        // There's no ambiguity, just pick the two jets.
        m_mass = (jets.at(0) + jets.at(1)).M();
        m_jetIDs.push_back(0);
        m_jetIDs.push_back(1);
    } else if (jets.size() > 2) {

        // This is a tad hacky.
        // We want to pick all possible sets of jet indices of size >= 2.
        // One very naive way to do this is to use a binary counter.
        // We sum from 0 to 2**numjets and convert to binary. Then if there
        // are at least two 1s in the binary representation, we sum over the
        // binary representation and pick those jets.
        // I feel like there is a better algorithm but I honestly can't think of one...

        std::vector<int> jetIDs;
        unsigned int jetsize = jets.size();

        for (unsigned int count = 0; count < pow(2, jetsize); count++) {

            // The size here must be a constant expression, unfortunately.
            // But I doubt there will ever be more than 2**16 jets.
            // TODO: make not magic number or revisit approach entirely.
            std::bitset<16> counter = std::bitset<16>(count);

            // If there aren't at least two bits "on" in our counter, continue.
            if (counter.count() < 2) {
                continue;
            }
            else {

                // Loop through each bit in the counter. If it's on, select the corresponding jet.
                // Also record that we selected the particular jet in a vector.
                TLorentzVector combination;
                for (unsigned int i = 0; i < jets.size(); i++) {
                    if (counter[i]) {
                        combination = combination + jets.at(i);
                        jetIDs.push_back(i);
                    }
                }

                // If this mjj is better than the current maximum, use it.
                // Otherwise, clear the jet IDs vector.
                float mjj = combination.M();
                if (mjj > m_mass) {
                    m_mass = mjj;
                    m_jetIDs = jetIDs;
                } else {
                    jetIDs.clear();
                }
            }
        }
    }
}
