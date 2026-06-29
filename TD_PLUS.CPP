#include "includes.h"
#pragma hdrstop

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif
#ifndef  _WINUTIL_DLG_ITEM_H
#include "winutil\dlg_item.h"
#endif

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
#ifndef  _UTL_H
#include "utl.h"
#endif

#include "mainwind.h"

void TransferPlusDlg::SetupControl(ControlInfo *cip, void *data)
{
  int idControl=cip->idControl;
  switch(cip->type) {
    case citINT:
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 6, 0L);
      SetDlgItemInt(HWindow, idControl, *(int *)data, TRUE);
    break;
    case citLONG:
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 11, 0L);
      SetDlgItemLong(HWindow, idControl, *(long *)data, TRUE);
    break;
    case citDouble:
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 15, 0L);
//      SetDlgItemDouble(HWindow, idControl, *(double *)data, "%f");
      SetDlgItemDouble(HWindow, idControl, *(double *)data, "%4.2f");// changed. P.P. 07/21/06
    break;
    case citTEMP:
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 6, 0L);
      SetDlgItemInt(HWindow, idControl, *(unsigned *)data, FALSE);
    break;
    case citTIME:
    {
#if OPTICS()
      SetDlgItemLong(HWindow, idControl, *(long *)data, TRUE);
      //SetDlgItemDouble(HWindow, idControl, *(double *)data, "%f");
#else
      long l=*(long *)data;
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 9, 0L);
      SetDlgItemText(HWindow, idControl, PrintSeconds(l));
#endif
    }
    break;
    case citONOFF:
      CheckDlgButton(HWindow, idControl, *(int *)data ? 1: 0);
    break;
    case citOFFON:
      CheckDlgButton(HWindow, idControl, *(int *)data ? 0: 1);
    break;
    case citRadio:
    {
      for(int i=0; i<(int)cip->info1; i++) {
	CheckDlgButton(HWindow, idControl+i, (*(int *)data)==i ? 1: 0);
      }
    }
    break;
    case citComboList:
    {
      for(char *cp=(char *)cip->info1; *cp; cp+=lstrlen(cp)+1) {
	SendDlgItemMsg( idControl, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)cp);
      }
      SendDlgItemMsg( idControl, CB_SETCURSEL, *(int *)data, 0L);
    }
    break;
    case citCYCLE:
      SendDlgItemMsg(idControl, EM_LIMITTEXT, 6, 0L);
      SetDlgItemInt(HWindow, idControl, *(int *)data, TRUE);
    break;
    case citStr:
      SendDlgItemMsg(idControl, WM_SETTEXT, 0, (LPARAM)(LPSTR)((Str *)data)->Get() );
    break;
    default:
      //CheckRadioButton(HWindow, idFirstButton, idLastButton, (UINT)*data);
    break;
  }
}

BOOL TransferPlusDlg::ValidateControl(ControlInfo *cip, void */*data*/)
{
  int idControl=cip->idControl;
  BOOL fTranslated;
  switch(cip->type) {
    case citINT:
    {
      int i=GetDlgItemInt(HWindow, idControl, &fTranslated, TRUE); if(!fTranslated) goto error_label;
      if(i<cip->info1 || i>cip->info2) goto error_label;
    }
    break;
    case citLONG:
    {
      long i=GetDlgItemLong(HWindow, idControl, &fTranslated, TRUE); if(!fTranslated) goto error_label;
      if(i<cip->info1 || i>cip->info2) goto error_label;
    }
    break;
    case citDouble:
      GetDlgItemDouble(HWindow, idControl, &fTranslated); if(!fTranslated) goto error_label;
    break;
    case citTEMP:
    {
      unsigned i=GetDlgItemInt(HWindow, idControl, &fTranslated, FALSE); if(!fTranslated) goto error_label;
      if(i<cip->info1 || i>cip->info2) goto error_label;
    }
    break;
    case citTIME:
    {
#if OPTICS()
      long i=GetDlgItemLong(HWindow, idControl, &fTranslated, TRUE); if(!fTranslated) goto error_label;
      //GetDlgItemDouble(HWindow, idControl, &fTranslated); if(!fTranslated) goto error_label;
#else
      char str[12];
      GetDlgItemText(HWindow, idControl, str, sizeof(str));
      long i=ConvertTime( str );
      if(i<cip->info1 || i>cip->info2) goto error_label;
#endif
    }
    break;
    case citCYCLE:
    {
      int i=GetDlgItemInt(HWindow, idControl, &fTranslated, TRUE); if(!fTranslated) goto error_label;
      if(i<cip->info1 || i>cip->info2) goto error_label;
    }
    break;
    default:
      //??CheckRadioButton(HWindow, idFirstButton, idLastButton, (UINT)*data);
    break;
  }
  return TRUE;
 error_label:
  Str str=GetDlgItemText(HWindow, cip->idControl);
  char errorMessage[80];
  if(!LoadString(GetModule()->hInstance,idControl,errorMessage,sizeof(errorMessage))) {
    *errorMessage=0;
  }
  char buff[256];
  wsprintf(buff,"%s\n%s",str.Get(),errorMessage);
  MessageBeep(MB_ICONHAND);
  MessageBox(HWindow, buff, "Wrong value in dialog box", MB_ICONHAND|MB_OK|MB_TASKMODAL);
  HWND hwndControl=GetDlgItem(HWindow,cip->idControl);
  if(hwndControl) {
    SetFocus(hwndControl);
    SendMessage(hwndControl, EM_SETSEL, 0, MAKELONG(0, -1));
  }
  return FALSE;
}

void TransferPlusDlg::AcceptControl(ControlInfo *cip, void *data)
{
  int idControl=cip->idControl;
  switch(cip->type) {
    case citINT:
      *(int *)data=GetDlgItemInt(HWindow, idControl, NULL, TRUE);
    break;
    case citLONG:
      *(long *)data=GetDlgItemLong(HWindow, idControl, NULL, TRUE);
    break;
    case citDouble:
      *(double *)data=GetDlgItemDouble(HWindow, idControl, NULL);
    break;
    case citTEMP:
      *(unsigned *)data=GetDlgItemInt(HWindow, idControl, NULL, FALSE);
    break;
    case citTIME:
    {
#if OPTICS()
      *(long *)data=GetDlgItemLong(HWindow, idControl, NULL, TRUE);
      //*(double *)data=GetDlgItemDouble(HWindow, idControl, NULL);
#else
      char str[12];
      GetDlgItemText(HWindow, idControl, str, sizeof(str));
      *(long *)data=ConvertTime( str );
#endif
    }
    break;
    case citONOFF:
      *(int *)data=IsDlgButtonChecked(HWindow, idControl)? 1: 0;
    break;
    case citOFFON:
      *(int *)data=IsDlgButtonChecked(HWindow, idControl)? 0: 1;
    break;
    case citRadio:
    {
      *(int *)data=0;
      for(int i=0; i<(int)cip->info1; i++) {
	if(IsDlgButtonChecked(HWindow, idControl+i)) {
	  *(int *)data=i; break;
	}
      }
    }
    break;
    case citComboList:
      *(int *)data=(int)SendDlgItemMsg( idControl, CB_GETCURSEL, 0, 0L);
    break;
    case citCYCLE:
      *(int *)data=GetDlgItemInt(HWindow, idControl, NULL, TRUE);
    break;
    case citStr:
    {
      size_t l = (size_t)SendDlgItemMsg(idControl, WM_GETTEXTLENGTH, 0, 0);
      ((Str *)data)->Alloc(l+1);
      SendDlgItemMsg(idControl, WM_GETTEXT, l+1, (LPARAM)(LPSTR)((Str *)data)->GetPtr() );
    }
    break;
    default:
      //??CheckRadioButton(HWindow, idFirstButton, idLastButton, (UINT)*data);
    break;
  }
}

void TransferPlusDlg::SetupControls(void *data)
{
  for(ControlInfo *cip=ciArray; cip->type!=citNULL; cip++) {
    SetupControl(cip, (char *)data + cip->offset);
  }
}

BOOL TransferPlusDlg::ValidateControls(void *data)
{
  for(ControlInfo *cip=ciArray; cip->type!=citNULL; cip++) {
    if(!ValidateControl(cip, (char *)data + cip->offset)) {
      return FALSE;
    }
  }
  return TRUE;
}

void TransferPlusDlg::AcceptControls(void *data)
{
  for(ControlInfo *cip=ciArray; cip->type!=citNULL; cip++) {
    AcceptControl(cip, (char *)data + cip->offset);
  }
}


