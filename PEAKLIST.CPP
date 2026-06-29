#pragma hdrstop

#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif

#ifndef  _MAINWIND_H
#include "mainwind.h"
#endif

#include "debug.h"

PeakList::PeakList(MainWindow *_Window, int _id)
 : Window(_Window)
 , id(_id)
 , baseline(0)
 , _baseline(0)
 , _baselineslope(0)
{
  TRACE1("PeakList=%p",this);
}

PeakList::~PeakList()
{
  TRACE1("~PeakList=%p",this);
}

#ifndef  _ID_H
#include "id.h"
#endif

void PeakList::append(const Peak &peak, int bNotNotify)
{
  PeakList::Node *pNode=new PeakList::Node(peak);
  Inherited::append(pNode);

//  Window->Refresh();

  if(!bNotNotify) {
  PTWindowsObject pPeakWindow=Window->FindPeakWindow(id);
  if(pPeakWindow) ::SendMessage(pPeakWindow->HWindow,WM_NEW_PEAK,id,(LPARAM)(Peak *)pNode);

  PTWindowsObject pGraphWindow=Window->FindGraphWindow(id);
  if(pGraphWindow) ::SendMessage(pGraphWindow->HWindow,WM_NEW_PEAK,id,(LPARAM)(Peak *)pNode);

  PTWindowsObject pGraphWindowAll=Window->FindGraphWindow(-1);
  if(pGraphWindowAll) ::SendMessage(pGraphWindow->HWindow,WM_NEW_PEAK,id,(LPARAM)(Peak *)pNode);
  }
}


