#ifndef  _PEAKLIST_H
#define  _PEAKLIST_H

#ifndef  _WCPPLIB_WLIST_H
#include "wcpplib\wlist.h"
#endif

#ifndef  _PEAK_H
#include "peak.h"
#endif

class MainWindow;

class PeakList : public List
{
  typedef List Inherited;
 public:
  class Node : public List::Node, public Peak {
    typedef List::Node Inherited;
   public:
    Node(const Peak &peak) : Peak(peak) {}
    Node *nextNode() const { return (Node *)Inherited::nextNode(); }
  };

  Node *first(void) const { return (Node *)Inherited::first(); }
  Node *GetChild(short index) const { return (Node *)Inherited::GetChild(index); }


  long baseline;

  double _baseline;
  double _baselineslope;

  MainWindow *Window;
  int id;
 public:

  PeakList(MainWindow *_Window, int _id);
  virtual ~PeakList();

  void append(const Peak &peak, int bNotNotify=0);
};

#endif
