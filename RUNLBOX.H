#ifndef  _RUNLBOX_H
#define  _RUNLBOX_H

#ifndef __LISTBOX_H
#include <listbox.h>
#endif


class RunListBox : public TListBox {
  typedef TListBox Inherited;
 public:
  RunListBox(PTWindowsObject AParent, int AnId, int X, int Y, int W, int H, PTModule AModule = NULL)
    : TListBox(AParent, AnId, X, Y, W, H, AModule ) {}
  RunListBox(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL)
    : TListBox(AParent, ResourceId, AModule ) {}
  virtual void WMChar(RTMessage Msg) = [WM_FIRST + WM_CHAR];

  RunListBox(StreamableInit);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];
};

#endif
