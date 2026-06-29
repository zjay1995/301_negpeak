#include "includes.h"
#pragma hdrstop

#ifndef __LISTBOX_H
#include <listbox.h>
#endif


#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _CMPONENT_H
#include "cmponent.h"
#endif

#ifndef  _UTL_H
#include "utl.h"
#endif

#ifndef  _L_DLG_H
#include "l_dlg.h"
#endif


class ComponentsDlg : public ListDlg {
  typedef ListDlg Inherited;
 public:
   ComponentsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PTModule AModule = NULL);
 protected:
  virtual void SetupWindow();
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);
  virtual void Edit(int n);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  virtual void DeleteItem(DELETEITEMSTRUCT _FAR & DeleteInfo);
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];

  void InitTabPositions();

  int TabPositions[6];
};

ComponentsDlg::ComponentsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PTModule AModule)
 : ListDlg(AParent, ResourceId, pMethod, AModule)
{
  memset(&TabPositions[0],0,sizeof(TabPositions));
}

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
void ComponentsDlg::SetupWindow()
{
  Inherited::SetupWindow();
  Centralize(this);
  InitTabPositions();
}

void ComponentsDlg::InitTabPositions()
{
  TEXTMETRIC tm;
  HWND hwnd=list[0]? list[0]->HWindow: HWindow;
  ClientDC dc(hwnd);
  dc.GetTextMetrics(&tm);
  int dx=tm.tmMaxCharWidth;
  TabPositions[0]=0; // variable size defined later
  TabPositions[1]=TabPositions[0]+tm.tmAveCharWidth*4+dx; // on/off    width
  TabPositions[2]=TabPositions[1]+tm.tmAveCharWidth*6+dx; // RT
  TabPositions[3]=TabPositions[2]+tm.tmAveCharWidth*6+dx; // Window
  TabPositions[4]=TabPositions[3]+tm.tmAveCharWidth*6+dx; // Resp Fact
  TabPositions[5]=TabPositions[4]+tm.tmAveCharWidth*6+dx; // Low Alarm

  WRect rClient; ::GetClientRect(hwnd,&rClient);
  rClient.right-=GetSystemMetrics(SM_CXVSCROLL);
  int delta=rClient.Width()-(TabPositions[6-1]+tm.tmAveCharWidth*6/*dc.GetTextExtent("1234:00").X()*/);
  int wName=tm.tmAveCharWidth*10; //dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=0; i<6; ++i) {
    TabPositions[i]+=delta;
  }
}

void ComponentsDlg::WMSize(RTMessage Msg)
{
  InitTabPositions();
  Inherited::WMSize(Msg);
}

#ifndef  _WINLIB_BITMAP_H
#include "winlib\bitmap.h"
#endif

#ifndef __STDIO_H
#include <stdio.h>
#endif

void ComponentsDlg::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
{
  const WRect &r=(const WRect &)DrawInfo.rcItem;
  DC dc(DrawInfo.hDC);

  int l=DrawInfo.CtlType==ODT_LISTBOX;
  Color bkColor=GetSysColor(DrawInfo.itemState&ODS_SELECTED? COLOR_HIGHLIGHT: COLOR_WINDOW);
  if(l) {
    dc.FillRect(r,LogBrush(bkColor));
  }
  else {
    HBRUSH hBrush=(HBRUSH)SendMessage(HWindow,WM_CTLCOLOR,(WPARAM)DrawInfo.hDC,MAKELPARAM(DrawInfo.hwndItem,
      l? CTLCOLOR_LISTBOX: CTLCOLOR_STATIC));
    if(hBrush) dc.FillRect(r,Brush(hBrush));
  }


  Component *pComponent=(Component *)DrawInfo.itemData;
  if(pComponent || !l) {
    SelTextColor oldTextColor(dc,GetSysColor(DrawInfo.itemState&ODS_SELECTED? COLOR_HIGHLIGHTTEXT: COLOR_WINDOWTEXT));
    SelBkMode oldBkMode(dc,bkmTransparent);
    SelTextAlign oldTextAlign(dc,taLeft|taTop);
    #if 0
    dc.TextOut(r.A(),l? pComponent->name.Get(): "Component Name");
    #else
    {
    const char *name=l? pComponent->name.Get(): "Component Name";
    WRect _rc=r;
    _rc.Right()=_rc.Left()+TabPositions[0];
    dc.ExtTextOut(_rc.A(), ETO_CLIPPED, _rc, name, lstrlen(name), (LPINT)NULL);
    }
    #endif
    if(l) {
      if(pComponent->active_yn) {
	SelBkColor oldBkColor(dc,bkColor);
	StockBitmap bmpCheck(OBM_CHECK);
	WPoint pt=bmpCheck.Size( dc );
	bmpCheck.BitBlt(dc,r.Left()+TabPositions[0],r.Top(), pt.x, pt.y, 0, 0, SRCCOPY);
      }
    }
    else
      dc.TextOut(r.Left()+TabPositions[0],r.Top(),"on/off");
    char str[40];
    dc.TextOut(r.Left()+TabPositions[1],r.Top(),l? PrintSeconds(pComponent->peak_rt): "Peak RT");
    dc.TextOut(r.Left()+TabPositions[2],r.Top(),l? PrintSeconds(pComponent->window): "Window");
    if(l) sprintf(str,"%.2f",pComponent->resp_fact);
    else  sprintf(str,"Resp Fact");
    dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
    if(l) sprintf(str,"%.2f",pComponent->low_alarm[0]);
    else  sprintf(str,"Low alarm");
    dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
    if(l) sprintf(str,"%.2f",pComponent->high_alarm[0]);
    else  sprintf(str,"High alarm");
    dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  }
  if(DrawInfo.itemState&ODS_FOCUS) dc.DrawFocusRect(r);
}

void ComponentsDlg::DeleteItem(DELETEITEMSTRUCT _FAR & DeleteInfo)
{
  delete (Component *)DeleteInfo.itemData;
  DeleteInfo.itemData=0;
}

void ComponentsDlg::SetupControls(void *data)
{
  Method *pMethod=(Method *)data;
  for(int n=0; n<NUMDETECTORS; n++) {
    ::SendMessage(list[n]->HWindow,WM_SETREDRAW,FALSE,0L);
    Component *pComponent=pMethod->det[n].components;
    for(int i=0; i<COMPONENT_ROWS; i++,pComponent++) {
      Component *pComponentNew=new Component;
      *pComponentNew=*pComponent;
      list[n]->AddString((LPSTR)pComponentNew);
    }
    ::SendMessage(list[n]->HWindow,WM_SETREDRAW,TRUE,0L);
    ::InvalidateRect(list[n]->HWindow,NULL,FALSE);
  }
}

void ComponentsDlg::AcceptControls(void *data)
{
  Method *pMethod=(Method *)data;
  for(int n=0; n<NUMDETECTORS; n++) {
    Component *pComponent=pMethod->det[n].components;
    for(int i=0; i<COMPONENT_ROWS; i++,pComponent++) {
      LRESULT res=::SendMessage(list[n]->HWindow,LB_GETITEMDATA,i,0L);
      if(res!=LB_ERR) { *pComponent=*(Component *)res; }
      //else memset(ci,0,sizeof(*ci));
    }
  }
}

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
static ControlInfo far ci[]={
  { citStr,	ID_COMPONENT_NAME,	FIELDOFFSET(Component,name), },
  { citTIME,	ID_COMPONENT_PEAK,	FIELDOFFSET(Component,peak_rt), 0, 0x7FFFFFFFL},
  { citTIME,	ID_COMPONENT_WINDOW,	FIELDOFFSET(Component,window), 0, 0x7FFFFFFFL},
  { citDouble,	ID_COMPONENT_RESP_FACT,	FIELDOFFSET(Component,resp_fact), },
  #if POINTS()<2
  { citDouble,  ID_COMPONENT_LOW_ALARM,	FIELDOFFSET(Component,low_alarm[0]), },
  { citDouble,  ID_COMPONENT_HIGH_ALARM,FIELDOFFSET(Component,high_alarm[0]), },
  #endif
  { citONOFF,	ID_COMPONENT_ACTIVE,	FIELDOFFSET(Component,active_yn), },
  { citNULL }	// End of array marker
};



class ComponentDlg : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
   ComponentDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, PTModule AModule = NULL);
 protected:
  virtual void SetupWindow();
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);
#if POINTS()>1
  virtual void CMComponentAlarms(RTMessage Msg) = [ID_FIRST + ID_COMPONENT_LOW_ALARM];
  Component component;
#endif //POINTS()
};

ComponentDlg::ComponentDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, PTModule AModule)
 : TransferPlusDlg(AParent, ResourceId, pComponent, ci, AModule)
{
}

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
void ComponentDlg::SetupWindow()
{
  Inherited::SetupWindow();
  Centralize(this);
}


#if POINTS()>1

class ComponentAlarmsDlg : public HelpDlg {
  typedef HelpDlg Inherited;
 public:
   ComponentAlarmsDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, PTModule AModule = NULL)
    : HelpDlg(AParent, ResourceId, pComponent, AModule) {}
 protected:
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);

  virtual void CMEqual(RTMessage Msg) = [ID_FIRST + IDD_EQUAL];
};

#include "winutil\dlg_item.h"
void ComponentAlarmsDlg::SetupControls(void *data)
{
  const Component *pComponent=(const Component *)data;
  //Inherited::SetupControls(data);
  for(int p=0; p<MAX_POINTS; p++) {
    SendDlgItemMsg(IDD_LA1+p, EM_LIMITTEXT, 15, 0L);
    SetDlgItemDouble(HWindow, IDD_LA1+p, pComponent->low_alarm[p], "%f");

    SendDlgItemMsg(IDD_HA1+p, EM_LIMITTEXT, 15, 0L);
    SetDlgItemDouble(HWindow, IDD_HA1+p, pComponent->high_alarm[p], "%f");

    if(p>=POINTS()) {
      EnableWindow(GetDlgItem(HWindow,IDD_POINT1+p),FALSE);
      EnableWindow(GetDlgItem(HWindow,IDD_LA1+p),FALSE);
      EnableWindow(GetDlgItem(HWindow,IDD_HA1+p),FALSE);
      ShowWindow(GetDlgItem(HWindow,IDD_POINT1+p),SW_HIDE);
      ShowWindow(GetDlgItem(HWindow,IDD_LA1+p),SW_HIDE);
      ShowWindow(GetDlgItem(HWindow,IDD_HA1+p),SW_HIDE);
    }
  }
  #if POINTS()<=16
  EnableWindow(GetDlgItem(HWindow,IDD_POINTS),FALSE);
  EnableWindow(GetDlgItem(HWindow,IDD_LOW_ALARMS),FALSE);
  EnableWindow(GetDlgItem(HWindow,IDD_HIGH_ALARMS),FALSE);
  ShowWindow(GetDlgItem(HWindow,IDD_POINTS),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_LOW_ALARMS),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_HIGH_ALARMS),SW_HIDE);
  #endif
}
void ComponentAlarmsDlg::AcceptControls(void *data)
{
  Component *pComponent=(Component *)data;
  //Inherited::AcceptControls(data);
  for(int p=0; p<MAX_POINTS; p++) {
    pComponent->low_alarm[p]=GetDlgItemDouble(HWindow, IDD_LA1+p, NULL);
    pComponent->high_alarm[p]=GetDlgItemDouble(HWindow, IDD_HA1+p, NULL);
  }
}
void ComponentAlarmsDlg::CMEqual(RTMessage)
{
  Str low=GetDlgItemText(HWindow,IDD_LA1);
  Str high=GetDlgItemText(HWindow,IDD_HA1);
  for(int p=1; p<MAX_POINTS; p++) {
    SetDlgItemText(HWindow, IDD_LA1+p, low);
    SetDlgItemText(HWindow, IDD_HA1+p, high);
  }
}
///////

void ComponentDlg::CMComponentAlarms(RTMessage)
{
  GetModule()->ExecDialog(new ComponentAlarmsDlg(this,ID_COMPONENTALARMSDIALOG,&component));
}
static HWND ChangeToButton(HWND HWindow, int id, HINSTANCE hInstance)
{
  HWND hl=GetDlgItem(HWindow,id);
  HWND hlText=GetWindow(hl, GW_HWNDPREV);
  char szText[80];
  GetWindowText(hlText,szText, sizeof(szText));

  WRect rc;
  GetWindowRect(hl,&rc);
  MapWindowPoints(NULL,HWindow,(LPPOINT)&rc,2);
  rc.left-=2;
  rc.bottom+=2;

  HWND h=CreateWindowEx(0,"BUTTON",szText,BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP,rc.Left(),rc.Top(),rc.Width(),rc.Height(),HWindow,(HMENU)id,hInstance,NULL);
  SetWindowPos(h,hl,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  SendMessage(h,WM_SETFONT,SendMessage(hl,WM_GETFONT,0,0),0);

  DestroyWindow(hlText);
  DestroyWindow(hl);

  return h;
}
#endif // POINTS()

void ComponentDlg::SetupControls(void *data)
{
  const Component *pComponent=(const Component *)data;
  #if POINTS()>1
  SetWindowText(ChangeToButton(HWindow, ID_COMPONENT_LOW_ALARM, GetModule()->hInstance),
    "&Alarms...");
  DestroyWindow(ChangeToButton(HWindow, ID_COMPONENT_HIGH_ALARM, GetModule()->hInstance));

  component=*pComponent;
  #endif
  Inherited::SetupControls(data);
}

void ComponentDlg::AcceptControls(void *data)
{
  Component *pComponent=(Component *)data;
  #if POINTS()>1
  *pComponent=component; // Get alarms
  #endif
  Inherited::AcceptControls(data);
}



void ComponentsDlg::Edit(int n)
{
  int Index=list[n]->GetSelIndex();
  if(Index<0) return;
  LRESULT res=::SendMessage(list[n]->HWindow,LB_GETITEMDATA,Index,0L);
  Component *pComponent=(Component *)res;
  int i=GetModule()->ExecDialog(new ComponentDlg(this,ID_EDITCOMPONENTDIALOG,pComponent));
  if(i!=IDCANCEL) {
    InvalidateRect(list[n]->HWindow,NULL,FALSE);
  }
}

/* ----------------------------------------------------------------- */


#ifndef  _CMPN_DLG_H
#include "cmpn_dlg.h"
#endif

PTWindowsObject newComponentsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PTModule AModule)
{
  return new ComponentsDlg(AParent, ResourceId, pMethod, AModule);
}

