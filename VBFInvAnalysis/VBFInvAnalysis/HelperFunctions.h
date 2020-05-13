#ifndef __VBFInvAnalysis_HelperFunctions_h__
#define __VBFInvAnalysis_HelperFunctions_h__

// various xAOD includes

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include <Rtypes.h>

namespace xAOD {
class IParticle;
}

namespace HelperFunctions {
// convenience methods (not class members)
Bool_t comparePt(const xAOD::IParticle *a, const xAOD::IParticle *b);

inline const xAOD::TrackParticle *getTrackParticle(xAOD::Electron *p)
{
   return p->trackParticle();
}

inline const xAOD::TrackParticle *getTrackParticle(xAOD::Muon *p)
{
   const ElementLink<xAOD::TrackParticleContainer> tp_prime = p->primaryTrackParticleLink();
   if (!tp_prime.isValid()) return nullptr;
   return *tp_prime;
}

// d0 calculations
template <typename T>
float getD0(T p)
{
   auto track = getTrackParticle(p);
   if (!track) return -99.;
   double d0 = track->d0();
   return d0;
}
template <typename T>
float getD0sig(T p, const xAOD::EventInfo *info)
{
   auto track = getTrackParticle(p);
   if (!track) return -99.;
   double d0_sig = xAOD::TrackingHelpers::d0significance(track, info->beamPosSigmaX(), info->beamPosSigmaY(),
                                                         info->beamPosSigmaXY());
   return d0_sig;
}

// z0 calculations
template <typename T>
float getZ0(T p, float primvertex_z)
{
   auto track = getTrackParticle(p);
   if (!track) return -99.;
   double z0 = track->z0() + track->vz() - primvertex_z;
   return z0;
}
template <typename T>
float getZ0sig(T p)
{
   auto track = getTrackParticle(p);
   if (!track) return -99.;
   double z0_sig = TMath::Sqrt(track->definingParametersCovMatrix()(1, 1));
   return z0_sig;
}

void computeMaxjj(xAOD::JetContainer jets, double &e_DiJetMass_Max, double &e_JetsDEta_Max);
void computejj(xAOD::JetContainer jets, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi);
void computejj(std::vector<TLorentzVector> *jet_tlv, double &e_DiJetMass, double &e_JetsDEta, double &e_JetsDPhi);
void computeMETj(TLorentzVector met, xAOD::JetContainer jets, double &e_met_j1_dphi, double &e_met_j2_dphi);
} // namespace HelperFunctions

#define SET_DUAL_TOOL(TOOLHANDLE, TOOLTYPE, TOOLNAME) \
   ASG_SET_ANA_TOOL_TYPE(TOOLHANDLE, TOOLTYPE);       \
   TOOLHANDLE.setName(TOOLNAME);

#define LINE std::cerr << __FILE__ << "::" << __FUNCTION__ << "::" << __LINE__ << std::endl;

#endif
