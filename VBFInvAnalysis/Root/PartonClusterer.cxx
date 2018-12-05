// Implementation of Analysis::PartonClusterer, a class which is passed
// a set of TruthParticles. The partons are extracted, clustered using
// fastjet, and then various dijet pairs computed from them.

#include "VBFInvAnalysis/PartonClusterer.h"

#include "TMath.h"

// Use the PartonClusterer class to avoid having to declare
// methods as "Analysis::PartonClusterer".
using Analysis::PartonClusterer;

PartonClusterer::PartonClusterer(std::string prefix, float minPt, float antiktDR, bool noCluster)
   : m_prefix(prefix), m_minPt(minPt), m_antiktDR(antiktDR), m_noCluster(noCluster)
{
}

void PartonClusterer::attachToTree(TTree *tree)
{
   // Create branches to store the particle kinematics
   tree->Branch((m_prefix + "_Pt").c_str(), &m_particlePt);
   tree->Branch((m_prefix + "_Eta").c_str(), &m_particleEta);
   tree->Branch((m_prefix + "_Phi").c_str(), &m_particlePhi);
   tree->Branch((m_prefix + "_E").c_str(), &m_particleE);
   tree->Branch((m_prefix + "_Mass").c_str(), &m_particleMass);

   // Store the particle pdg ID.
   tree->Branch((m_prefix + "_PID").c_str(), &m_particlePID);

   // Store the number of jets, as well, because that's good to know.
   tree->Branch((m_prefix + "_numParticles").c_str(), &m_numParticles);

   // Store the computed HT, pTV, and max(HT, pTV) here.
   tree->Branch((m_prefix + "_HT").c_str(), &m_ht);
   tree->Branch((m_prefix + "_PTV").c_str(), &m_ptv);
   tree->Branch((m_prefix + "_maxHTPTV").c_str(), &m_maxhtptv);

   // For fun! Store s (e.g. |p[0] + p[1]|^2) from the incoming particles.
   tree->Branch((m_prefix + "_S").c_str(), &m_s);
}

void PartonClusterer::reset()
{
   // Clear all the truth particle vectors.
   m_particlePt.clear();
   m_particleEta.clear();
   m_particlePhi.clear();
   m_particleE.clear();
   m_particleMass.clear();
   m_particlePID.clear();

   // Clear the number of truth particles.
   m_numParticles = 0;

   // Clear the stored/cached parton jets.
   m_partonJets.clear();

   // Reset the max(HT, pTV) variables.
   m_ht       = 0;
   m_ptv      = 0;
   m_maxhtptv = 0;

   // Reset Mandelstam 's' variable.
   m_s = 0;
}

void PartonClusterer::calcMaxHTPTV(std::vector<const xAOD::TruthParticle *> *particles)
{

   // Only look at parton jets that pass the min pT cut.
   for (unsigned int i = 0; i < m_partonJets.size(); i++) {
      TLorentzVector jet = m_partonJets.at(i);
      if (((jet.Pt()) / 1000.) >= m_minPt) {
         m_ht += (jet.Pt() / 1000);
      }
   }

   // Now compute the pTV by extracting the leptons.
   std::vector<TLorentzVector> leptons;
   for (const auto *particle : *particles) {
      if (particle->isLepton()) {
         TLorentzVector rootParticle;
         rootParticle.SetPxPyPzE(particle->px(), particle->py(), particle->pz(), particle->e());
         leptons.push_back(rootParticle);
      }
   }

   // Take the first two leptons-- without sorting by pT-- as is done in the plugin.
   // (this is safe here because there should only _be_ two, but...).
   if (leptons.size() >= 2) {
      m_ptv = (leptons.at(0) + leptons.at(1)).Pt() / 1000.;
   }

   // Take the maximum and assign to maxhtptv.
   m_maxhtptv = TMath::Max(m_ht, m_ptv);
}

void PartonClusterer::storeParticles(std::vector<const xAOD::TruthParticle *> *particles)
{

   TLorentzVector rootParticle;

   unsigned int   count = 0;
   TLorentzVector incoming;

   // Loop through the truth particles, store them in the arrays.
   for (const auto *particle : *particles) {

      // Create a TLorentzVector particle.
      rootParticle.SetPxPyPzE(particle->px(), particle->py(), particle->pz(), particle->e());

      // Compute Mandelstam variable s.
      if (count < 2) {
         count += 1;
         incoming += rootParticle;
      }

      // Push the kinematics (and mass) back.
      m_particlePt.push_back(rootParticle.Pt());
      m_particleEta.push_back(rootParticle.Eta());
      m_particlePhi.push_back(rootParticle.Phi());
      m_particleE.push_back(rootParticle.E());
      m_particleMass.push_back(particle->m());

      // Push the particle PID back too.
      m_particlePID.push_back(particle->pdgId());
   }

   // s = |p[0] + p[1]|**2. Convert it to GeV^2.
   m_s = (incoming.Mag2()) / (1e6);

   // Store the size, too.
   m_numParticles = particles->size();

   // Calculate (and store) max(HT, pTV).
   this->calcMaxHTPTV(particles);
}

std::vector<TLorentzVector> *PartonClusterer::clusterPartons(std::vector<const xAOD::TruthParticle *> *particles)
{

   std::vector<fastjet::PseudoJet> clusterInputs;
   std::vector<fastjet::PseudoJet> partonPseudoJets;

   TLorentzVector     rootJet;
   fastjet::PseudoJet pseudoJet;

   // Loop over the particles, extract particles which are actually partons.
   for (const auto *particle : *particles) {
      if (!particle->isParton()) {
         continue;
      }

      // Add the particle to the array of things to cluster.
      fastjet::PseudoJet parton(particle->px(), particle->py(), particle->pz(), particle->e());
      clusterInputs.push_back(parton);
   }

   // Now, run fastjet with the given jet definition, unless we were told not to.
   // When we do this, assign m_partonJets local variable (cache the result).
   if (m_noCluster) {
      partonPseudoJets = clusterInputs;
   } else {
      fastjet::JetDefinition   jetdef(fastjet::antikt_algorithm, m_antiktDR);
      fastjet::ClusterSequence sequence(clusterInputs, jetdef);
      partonPseudoJets = fastjet::sorted_by_pt(sequence.inclusive_jets());
   }

   // Now, reprocess the parton jets into ROOT TLorentzVectors.
   for (unsigned int i = 0; i < partonPseudoJets.size(); i++) {
      pseudoJet = partonPseudoJets.at(i);

      // I don't think there should be any of these objects with pt <= 0, but ju tin case.
      if (pseudoJet.perp() > 0) {
         rootJet.SetPtEtaPhiE(pseudoJet.perp(), pseudoJet.eta(), pseudoJet.phi(), pseudoJet.e());
         m_partonJets.push_back(rootJet);
      }
   }

   // Also, store the truth particles.
   this->storeParticles(particles);

   return &m_partonJets;
}

std::vector<TLorentzVector> *PartonClusterer::getPartonJets()
{
   return &m_partonJets;
}
