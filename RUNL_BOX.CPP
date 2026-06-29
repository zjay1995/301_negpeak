#include "includes.h"
#pragma hdrstop


#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _RUNLBOX_H
#include "runlbox.h"
#endif

RunListBox::RunListBox(StreamableInit)
 : TListBox(streamableInit)
{
};

TStreamableClass RegRunListBox(RunListBox::name, RunListBox::build, __DELTA(RunListBox));
char far RunListBox::name[]="RunListBox";
PTStreamable RunListBox::build()
{
  return new RunListBox(streamableInit);
}


void RunListBox::WMChar(RTMessage Msg)
{
  switch(Msg.WParam) {
    case VK_RETURN:
//      PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_EDITITEM, 0L);
    break;
    case VK_INSERT:
//      PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_ADDITEM, 0L);
    break;
    case VK_DELETE:
//      PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_DELITEM, 0L);
    break;
  }
  DefWndProc(Msg);
}

