
#ifndef __Analysis_outObject__
#define __Analysis_outObject__

#include <TString.h>

class TTree;

namespace Analysis {
class outObject {
private:
   TString m_name;
   Bool_t  m_doTrim;
   Bool_t  m_write;
   Bool_t  m_doExtraTrim;
   Bool_t  m_doVertextDetail;

public:
   inline outObject(TString name = "", Bool_t doTrim = kFALSE)
   {
      m_name        = name;
      m_doTrim      = doTrim;
      m_write       = true;
      m_doExtraTrim = doTrim;
   }

   inline TString name() { return m_name; }

   inline Bool_t doTrim() { return m_doTrim; }
   inline Bool_t doExtraTrim() { return m_doExtraTrim; }
   inline Bool_t doVertexDetail() { return m_doVertextDetail; }

   inline Bool_t write() { return m_write; }

   inline void setName(TString val) { m_name = val; }

   inline void setDoTrim(Bool_t val) { m_doTrim = val; }
   inline void setDoExtraTrim(Bool_t val) { m_doExtraTrim = val; }
   inline void setDoVertexDetail(Bool_t val) { m_doVertextDetail = val; }

   inline void setWrite(Bool_t val) { m_write = val; }

public:
   inline virtual ~outObject()            = 0;
   virtual void reset()                   = 0;
   virtual void attachToTree(TTree *tree) = 0;
};
} // namespace Analysis

Analysis::outObject::~outObject() {}

#endif
