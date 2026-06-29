#ifndef  _RUNLIST_H
#define  _RUNLIST_H

#ifndef  _WCPPLIB_WLIST_H
#include "wcpplib\wlist.h"
#endif
class WCPPLIB_CLASS iwstream;
class WCPPLIB_CLASS owstream;

#ifndef  _RUN_H
#include "run.h"
#endif

class RunList : public List {
  typedef List Inherited;
 public:
  class Node : public List::Node, public Run {
    typedef List::Node Inherited;
   public:
    Node *nextNode() const { return (Node *)Inherited::nextNode(); }
  };

  Node *first(void) const { return (Node *)Inherited::first(); }
  Node *last(void) const { return (Node *)Inherited::last(); }
  Node *GetChild(short index) const { return (Node *)Inherited::GetChild(index); }

  RunList();
  virtual ~RunList();

  virtual void *read(iwstream &is, unsigned long version);
  virtual void write(owstream &os);
};

#endif
