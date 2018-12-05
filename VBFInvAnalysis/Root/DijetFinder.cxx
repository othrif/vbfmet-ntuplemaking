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
   bool operator()(const TLorentzVector &a, const TLorentzVector &b) const { return (a.Perp() > b.Perp()); }
};
} // namespace Analysis

DijetFinder::DijetFinder(std::string prefix, float minPt) : m_prefix(prefix), m_minPt(minPt)
{

   // Create dijet info objects.
   m_leadingDijets = new Analysis::LeadingDijetInfo(prefix);
   m_maxDijets     = new Analysis::MaxDijetInfo(prefix);
   m_bestDijets    = new Analysis::BestDijetInfo(prefix, "bestLead");
   m_maxAlljets    = new Analysis::MaxAlljetInfo(prefix);

   // Insert pointers into this map, for easier reference.
   m_dijetAlgos[m_leadingDijets->getName()] = m_leadingDijets;
   m_dijetAlgos[m_maxDijets->getName()]     = m_maxDijets;
   m_dijetAlgos[m_bestDijets->getName()]    = m_bestDijets;
   m_dijetAlgos[m_maxAlljets->getName()]    = m_maxAlljets;
}

Analysis::DijetInfo *DijetFinder::getDijetInfo(std::string name)
{
   return m_dijetAlgos[name];
}

void DijetFinder::attachToTree(TTree *tree)
{

   // Create branches to store the jet kinematics.
   tree->Branch((m_prefix + "_jetPt").c_str(), &m_jetPt);
   tree->Branch((m_prefix + "_jetEta").c_str(), &m_jetEta);
   tree->Branch((m_prefix + "_jetPhi").c_str(), &m_jetPhi);
   tree->Branch((m_prefix + "_jetE").c_str(), &m_jetE);

   // Store the number of jets, as well, because that's good to know.
   tree->Branch((m_prefix + "_numJets").c_str(), &m_numJets);

   // Store matching information, because there's nowhere better to do it.
   tree->Branch((m_prefix + "_numUnmatched").c_str(), &m_numUnmatched);
   tree->Branch((m_prefix + "_jetPartonMatchID").c_str(), &m_matches);
   tree->Branch((m_prefix + "_jetPartonDeltaR").c_str(), &m_deltaRs);

   // Call DijetInfo classes, pass the tree to them too!
   m_leadingDijets->attachToTree(tree);
   m_maxDijets->attachToTree(tree);
   m_bestDijets->attachToTree(tree);
   m_maxAlljets->attachToTree(tree);
}

void DijetFinder::reset()
{
   // Clear the jet kinematics vectors.
   m_jetPt.clear();
   m_jetEta.clear();
   m_jetPhi.clear();
   m_jetE.clear();

   // Reset number of jets.
   m_numJets = 0;

   // Reset number of matched jets.
   m_numUnmatched = 0;

   // Reset pruned jet objects.
   m_prunedJets.clear();

   // Reset matching information.
   m_matches.clear();
   m_deltaRs.clear();

   // Reset dijet info classes.
   m_leadingDijets->reset();
   m_maxDijets->reset();
   m_bestDijets->reset();
   m_maxAlljets->reset();
}

void DijetFinder::computeMjj(std::vector<TLorentzVector> *jets)
{

   TLorentzVector jet;

   // Sort the jets by pT, just to be safe.
   std::sort(jets->begin(), jets->end(), SortByPt());

   // Loop over the jets, write out their kinematics.
   for (unsigned int i = 0; i < jets->size(); i++) {
      jet = jets->at(i);
      m_jetPt.push_back(jet.Pt());
      m_jetEta.push_back(jet.Eta());
      m_jetPhi.push_back(jet.Phi());
      m_jetE.push_back(jet.E());
   }

   // Assign number of jets.
   m_numJets = jets->size();

   // Store a pointer to these jets.
   m_jets = jets;

   // Now we want to take the jets, and apply the minPt cut.
   // (Any other cuts, for example an eta/phi/E cut, could also be done here).
   for (unsigned int i = 0; i < jets->size(); i++) {
      jet = jets->at(i);
      if (((jet.Pt()) / 1000.) >= m_minPt) {
         m_prunedJets.push_back(jet);
      }
   }

   // Call the relevant DijetInfo classes too, pass m_prunedJets.
   m_leadingDijets->compute(m_prunedJets);
   m_maxDijets->compute(m_prunedJets);
   m_maxAlljets->compute(m_prunedJets);

   // We *don't* call the best version explicitly here, unfortunately.
   // We need to get the right value from elsewhere and pass it in.
   // That's done in the main algorithm.
}

std::vector<TLorentzVector> *DijetFinder::getPrunedJets()
{
   return &m_prunedJets;
}

void DijetFinder::matchJets(std::vector<TLorentzVector> *otherJets, float minDR)
{

   TLorentzVector jet;
   TLorentzVector otherJet;

   // Prepare the vectors of matches and of recorded deltaRs.
   for (unsigned int i = 0; i < m_jets->size(); i++) {
      m_matches.push_back(-1);
      m_deltaRs.push_back(-1);
   }

   // Step through our set of jets.
   for (unsigned int i = 0; i < m_jets->size(); i++) {
      jet = m_jets->at(i);

      // Now, loop over the other jets.
      for (unsigned int j = 0; j < otherJets->size(); j++) {
         otherJet = otherJets->at(j);

         // Compute delta R between this jet and the other jet.
         // Check if this delta R is better than any other delta R we've seen before
         // for this jet. If it is, record it.
         float deltaR = jet.DeltaR(otherJet);
         if (m_deltaRs[i] == -1 || deltaR < m_deltaRs[i]) {
            m_deltaRs[i] = deltaR;

            // Now, check if this delta R is within minDR. If it is, interpret thi
            // as a "match" with the other jet and save that.
            if (deltaR <= minDR) {
               m_matches[i] = j;
            }
         }
      }
   }

   // Now, count the number of unmatched jets.
   for (unsigned int i = 0; i < m_jets->size(); i++) {
      if (m_matches[i] == -1) {
         m_numUnmatched += 1;
      }
   }
}
