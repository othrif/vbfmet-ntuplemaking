#define __CutFlowTool_C__

#include "VBFInvAnalysis/CutFlowTool.h"
#include <TError.h>
#include <TH1F.h>

void Analysis::CutFlowTool::addCut(TString cut_name)
{
   UInt_t index     = cuts_map_.size();
   cuts_map_[index] = cut_name;
   cut_npass_.push_back(0.);
   cut_npass_unw_.push_back(0.);
   return;
}

void Analysis::CutFlowTool::addCutCounter(Int_t cut_index, Double_t nevents)
{
   for (int i = 0; i < cut_index + 1; i++) {
      double old_counter     = cut_npass_.at(i);
      cut_npass_.at(i)       = old_counter + nevents;
      double old_counter_unw = cut_npass_unw_.at(i);
      cut_npass_unw_.at(i)   = old_counter_unw + 1.0;
   }
   return;
}

void Analysis::CutFlowTool::addContent(Int_t cut_index, Double_t nevents_unw, Double_t nevents)
{
   double old_counter           = cut_npass_.at(cut_index);
   cut_npass_.at(cut_index)     = old_counter + nevents;
   double old_counter_unw       = cut_npass_unw_.at(cut_index);
   cut_npass_unw_.at(cut_index) = old_counter_unw + nevents_unw;
}

void Analysis::CutFlowTool::hasPassed(Int_t cut_index, Double_t nevents)
{
   double old_counter           = cut_npass_.at(cut_index);
   cut_npass_.at(cut_index)     = old_counter + nevents;
   double old_counter_unw       = cut_npass_unw_.at(cut_index);
   cut_npass_unw_.at(cut_index) = old_counter_unw + 1.0;
}

void Analysis::CutFlowTool::resetCounter()
{
   std::map<UInt_t, TString>::iterator it;
   for (it = cuts_map_.begin(); it != cuts_map_.end(); it++) {
      UInt_t index             = (*it).first;
      cut_npass_.at(index)     = 0.;
      cut_npass_unw_.at(index) = 0.;
   }
   return;
}

void Analysis::CutFlowTool::print()
{
   Info("CutFlowTool", "Printing %s weighted cutflow:", name_.Data());
   Info("CutFlowTool", "|	CUT		 | EVENTS\t| ABS. EFF.\t| REL. EFF. |");
   std::map<UInt_t, TString>::iterator it;
   for (it = cuts_map_.begin(); it != cuts_map_.end(); it++) {
      int index = (*it).first;
      if (index == 0) {
         Info("CutFlowTool", "| %20s | %.2lf\t | -- \t | -- |", ((*it).second).Data(), cut_npass_.at(index));
         continue;
      }
      int index_minus_one = (index > 1) ? (index - 1) : index;
      if (cut_npass_.at(index_minus_one) != 0 && cut_npass_.at(0) != 0) {
         Info("CutFlowTool", "| %20s | %.2lf\t| %.2lf\t | %.2lf |", ((*it).second).Data(), cut_npass_.at(index),
              (double)cut_npass_.at(index) / (double)cut_npass_.at(1),
              (double)cut_npass_.at(index) / (double)cut_npass_.at(index_minus_one));
      } else {
         std::cout << (*it).second << ": " << cut_npass_.at(index)
                   << ", efficiencies could not be computed since the denominators are 0" << std::endl;
      }
   }

   return;
}

void Analysis::CutFlowTool::printUnw()
{
   Info("CutFlowTool", "Printing %s unweighted cutflow:", name_.Data());
   Info("CutFlowTool", "|   CUT      | EVENTS\t| ABS. EFF.\t| REL. EFF. |");
   std::map<UInt_t, TString>::iterator it;
   for (it = cuts_map_.begin(); it != cuts_map_.end(); it++) {
      int index = (*it).first;
      if (index == 0) {
         Info("CutFlowTool", "| %20s | %.2lf\t | -- \t | -- |", ((*it).second).Data(), cut_npass_unw_.at(index));
         continue;
      }
      int index_minus_one = (index > 1) ? (index - 1) : index;
      if (cut_npass_unw_.at(index_minus_one) != 0 && cut_npass_unw_.at(0) != 0) {
         Info("CutFlowTool", "| %20s | %.2lf\t| %.2lf\t | %.2lf |", ((*it).second).Data(), cut_npass_unw_.at(index),
              (double)cut_npass_unw_.at(index) / (double)cut_npass_unw_.at(1),
              (double)cut_npass_unw_.at(index) / (double)cut_npass_unw_.at(index_minus_one));
      } else {
         std::cout << (*it).second << ": " << cut_npass_unw_.at(index)
                   << ", efficiencies could not be computed since the denominators are 0" << std::endl;
      }
   }

   return;
}

TH1F *Analysis::CutFlowTool::createTH1F()
{
   TH1F *h = new TH1F("cutflowhisto_" + name_, name_, cuts_map_.size() + 1, 0, cuts_map_.size() + 1);

   for (std::map<UInt_t, TString>::iterator c = cuts_map_.begin(); c != cuts_map_.end(); ++c) {

      UInt_t   idx  = c->first;
      TString  name = c->second;
      Double_t val  = cut_npass_.at(idx);

      h->Fill(name, val);
   }

   return h;
}

TH1F *Analysis::CutFlowTool::createTH1Fraw()
{
   TH1F *h = new TH1F("cutflowhisto_raw_" + name_, name_, cuts_map_.size() + 1, 0, cuts_map_.size() + 1);

   for (std::map<UInt_t, TString>::iterator c = cuts_map_.begin(); c != cuts_map_.end(); ++c) {
      // h->Fill(name_, 0.0); // to define bin name

      UInt_t   idx  = c->first;
      TString  name = c->second;
      Double_t val  = cut_npass_unw_.at(idx);

      h->Fill(name, val);
   }

   return h;
}
