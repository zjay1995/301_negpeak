#include "includes.h"
#pragma hdrstop

#ifndef __DIALOG_H
#include <dialog.h>
#endif
#ifndef __LISTBOX_H
#include <listbox.h>
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _L_DLG_H
#include "l_dlg.h"
#endif

// Base class for Components and Standards List Dialogs

ListDlg::ListDlg(PTWindowsObject AParent, int ResourceId, void *data, PTModule AModule)
 : HelpDlg(AParent, ResourceId, data, AModule)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    list[n]=new TListBox(this, IDD_COMPONENT_LIST_A+n, AModule);
    //if(list[n]) list[n]=(PTListBox)GetModule()->ValidWindow(list[n]);
    if(!list[n]) Status=EM_INVALIDCHILD;
  }
}

void ListDlg::WMDeleteItem(RTMessage Msg)
{
  DeleteItem(*(DELETEITEMSTRUCT _FAR *)Msg.LParam);
}

void ListDlg::DeleteItem(DELETEITEMSTRUCT _FAR & /*DeleteInfo*/)
{
}

void ListDlg::WMCommand(RTMessage Msg)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    if(Msg.WParam==IDD_COMPONENT_LIST_A+n) {
      switch ( Msg.LP.Hi ) {
	case LBN_DBLCLK:
	  Edit(n);
	return;
	case LBN_SETFOCUS:
        {
	  for(int n1=0; n1<NUMDETECTORS; n1++) {
	    if(n1!=n) list[n1]->SetSelIndex(-1);
	  }
        }
	break;
	case LBN_KILLFOCUS:
	break;
	//return;
      }
    }
  }
  Inherited::WMCommand(Msg);
}

void ListDlg::EditButton(RTMessage)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    if(list[n]->GetSelIndex()>=0) {
      Edit(n);
      return;
    }
  }
}

void ListDlg::Edit(int /*n*/)
{
}


