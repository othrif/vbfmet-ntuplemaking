#ifndef __CutFlowTool_h__
#define __CutFlowTool_h__

#include <iostream>
#include <vector>
#include <map>
#include <TString.h>

class TH1F;

namespace Analysis {

   class CutFlowTool {
   public:

      CutFlowTool(TString name = "")
      {
         name_ = name;
         cuts_map_.clear();
         cut_npass_.clear();
      };
      ~CutFlowTool() {};

      TString getName()
      {
         return name_;
      }
      std::map<UInt_t, TString> getCutFlowToolMap()
      {
         return cuts_map_;
      }
      std::vector<Double_t> getNPassVec()
      {
         return cut_npass_;
      }
      Double_t getNPass(UInt_t index)
      {
         return cut_npass_[index];
      }
      TString getCutName(UInt_t index)
      {
         return cuts_map_[index];
      }

      void addCut(TString cut_name);
      void addCutCounter(Int_t cut_index, Double_t nevents);
      void addContent(Int_t cut_index, Double_t nevents_unw, Double_t nevents);
      void hasPassed(Int_t cut_index, Double_t nevents);
      void resetCounter();
      void print();
      void printUnw();
      TH1F* createTH1F();
      TH1F* createTH1Fraw();

   private:
      TString name_;
      std::map<UInt_t, TString> cuts_map_;
      std::vector<Double_t> cut_npass_;
      std::vector<Double_t> cut_npass_unw_;
   };
}

#endif
