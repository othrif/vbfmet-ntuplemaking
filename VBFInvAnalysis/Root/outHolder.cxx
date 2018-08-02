#include "VBFInvAnalysis/outHolder.h"

#include <TTree.h>
#include <stdexcept>

Analysis::outHolder::outHolder() : m_attachedTree(nullptr)
{
   reset();
}

Analysis::outHolder::~outHolder()
{
}

void Analysis::outHolder::reset()
{
   evt.reset();
   rns.reset();
   for (auto &kv : met) kv.second.reset();
   for (auto &kv : mu) kv.second.reset();
   for (auto &kv : el) kv.second.reset();
   for (auto &kv : jet) kv.second.reset();
   //for (auto &kv : ph) kv.second.reset();
   //for (auto &kv : tau) kv.second.reset();
}

void Analysis::outHolder::setDoTrim(Bool_t val)
{
   m_doTrim = val;
}

void Analysis::outHolder::attachToTree(TTree *tree)
{
   // then, attach to TTree
   if (tree != m_attachedTree) {
      m_attachedTree = tree; // do NOT take ownership

      evt.attachToTree(tree);
      rns.attachToTree(tree);
      for (auto &kv : met) kv.second.attachToTree(tree);
      for (auto &kv : mu)  kv.second.attachToTree(tree);
      for (auto &kv : el)  kv.second.attachToTree(tree);
      for (auto &kv : jet) kv.second.attachToTree(tree);
      //for (auto &kv : ph)  kv.second.attachToTree(tree);
      //for (auto &kv : tau) kv.second.attachToTree(tree);
   }
}

void Analysis::outHolder::save()
{
   if (!m_attachedTree) throw std::runtime_error("out candidate not attached to any TTree");
   m_attachedTree->Fill();
}
