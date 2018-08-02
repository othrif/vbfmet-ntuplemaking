
#ifndef __Analysis_outObject__
#define __Analysis_outObject__

#include <TString.h>

class TTree;

namespace Analysis {
   class outObject {
   private:
      TString m_name;
      Bool_t m_doTrim;
      Bool_t m_write;

   public:

      inline outObject(TString name = "", Bool_t doTrim = kFALSE)
      {
         m_name = name;
         m_doTrim = doTrim;
         m_write = true;
      }

      inline TString name()
      {
         return m_name;
      }

      inline Bool_t doTrim()
      {
         return m_doTrim;
      }

      inline Bool_t write()
      {
         return m_write;
      }

      inline void setName(TString val)
      {
         m_name = val;
      }

      inline void setDoTrim(Bool_t val)
      {
         m_doTrim = val;
      }

      inline void setWrite(Bool_t val)
      {
         m_write = val;
      }


   public:
      inline virtual ~outObject() = 0;
      virtual void reset() = 0;
      virtual void attachToTree(TTree *tree) = 0;
   };
}

Analysis::outObject::~outObject()
{
}

#endif
