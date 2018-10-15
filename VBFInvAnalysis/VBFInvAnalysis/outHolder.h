#ifndef __Analysis_outHolder__
#define __Analysis_outHolder__

#include <VBFInvAnalysis/outEvent.h>
#include <VBFInvAnalysis/outRnSmear.h>
#include <VBFInvAnalysis/outMET.h>
#include <VBFInvAnalysis/outMuon.h>
#include <VBFInvAnalysis/outElectron.h>
#include <VBFInvAnalysis/outJet.h>
#include <VBFInvAnalysis/outPhoton.h>
#include <VBFInvAnalysis/outTau.h>

#include <TString.h>
#include <map>

namespace Analysis {
   class outHolder {
   private:
      TTree *m_attachedTree;
      Bool_t m_doTrim;

   public:
      outEvent evt;
      outRnSmear rns;
      std::map<TString, outMET> met;
      std::map<TString, outMuon> mu;
      std::map<TString, outElectron> el;
      std::map<TString, outJet> jet;
      std::map<TString, outPhoton> ph;
      std::map<TString, outTau> tau;


   public:
      outHolder();
      ~outHolder();
      void reset();
      void setDoTrim(Bool_t val);
      void attachToTree(TTree *tree);
      void save();
   };
}

#endif
