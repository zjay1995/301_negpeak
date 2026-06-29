#include "includes.h"
#pragma hdrstop

#ifndef __DIALOG_H
#include <dialog.h>
#endif
#ifndef __LISTBOX_H
#include <listbox.h>
#endif

#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif
#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _METHOD_H
#include "method.h"
#endif
/*
#ifndef  _WINUTIL_DLG_ITEM_H
#include "winutil\dlg_item.h"
#endif
*/

#ifndef  _L_DLG_H
#include "l_dlg.h"
#endif
#include  "peaklist.h"
class PeakList;

class StandardsDlg : public ListDlg {
  typedef ListDlg Inherited;
 public:
   StandardsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PeakList **pPeakList, PTModule AModule = NULL);
 protected:
  virtual void SetupWindow();
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);
  virtual void Edit(int n);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  virtual void DeleteItem(DELETEITEMSTRUCT _FAR & DeleteInfo);
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];

  virtual void CalibrationButton(RTMessage Msg) = [ID_FIRST + ID_CALIBRATION];

  void InitTabPositions();

  int TabPositions[6];
  PeakList **pPeakList;
};

StandardsDlg::StandardsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PeakList **_pPeakList, PTModule AModule)
 : ListDlg(AParent, ResourceId, pMethod, AModule)
 , pPeakList(_pPeakList)
{
  memset(&TabPositions[0],0,sizeof(TabPositions));
}

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif

void StandardsDlg::SetupWindow()
{
  Inherited::SetupWindow();
  InitTabPositions();
  Centralize(this);
}


void StandardsDlg::InitTabPositions()
{
  TEXTMETRIC tm;
  HWND hwnd=list[0]? list[0]->HWindow: HWindow;
  ClientDC dc(hwnd);
  dc.GetTextMetrics(&tm);
  int dx=tm.tmMaxCharWidth;
  TabPositions[0]=0; // variable size defined later
  TabPositions[1]=TabPositions[0]+tm.tmAveCharWidth*4+dx; // ref wodt
  TabPositions[2]=TabPositions[1]+tm.tmAveCharWidth*6+dx; // resp factr
  TabPositions[3]=TabPositions[2]+tm.tmAveCharWidth*6+dx; // stand[0]
  TabPositions[4]=TabPositions[3]+tm.tmAveCharWidth*6+dx; // stand[1]
  TabPositions[5]=TabPositions[4]+tm.tmAveCharWidth*6+dx; // stand[2]

  WRect rClient; ::GetClientRect(hwnd,&rClient);
  rClient.right-=GetSystemMetrics(SM_CXVSCROLL);
  int delta=rClient.Width()-(TabPositions[6-1]+tm.tmAveCharWidth*6/*dc.GetTextExtent("1234:00").X()*/);
  int wName=tm.tmAveCharWidth*10;//dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=0; i<6; ++i) {
    TabPositions[i]+=delta;
  }
}

void StandardsDlg::WMSize(RTMessage Msg)
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

void StandardsDlg::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
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
      if(pComponent->ref_peak) {
	SelBkColor oldBkColor(dc,bkColor);
	StockBitmap bmpCheck(OBM_CHECK);
	WPoint pt=bmpCheck.Size( dc );
	bmpCheck.BitBlt(dc,r.Left()+TabPositions[0],r.Top(), pt.x, pt.y, 0, 0, SRCCOPY);
      }
    }
    else
      dc.TextOut(r.Left()+TabPositions[0],r.Top(),"Ref");
    char str[40];
    if(l) sprintf(str,"%.2f",pComponent->resp_fact);
    else  sprintf(str,"RespFactor");
    dc.TextOut(r.Left()+TabPositions[1],r.Top(),str);
    for(int k=0; k<STAND_NUM; k++) {
//      if(l) sprintf(str,"%.2f",pComponent->stand[k]);
      if(l) sprintf(str,"%.4f",pComponent->stand[k]);   // higher resolution. P.P. 08/04/17
      else  sprintf(str,"Stand. %d",k);
      #if (2+STAND_NUM) > (sizeof(TabPositions)/sizeof(TabPositions[0]))
	#error Too few TabPositions!!
      #endif
      dc.TextOut(r.Left()+TabPositions[2+k],r.Top(),str);
    }
  }
  if(DrawInfo.itemState&ODS_FOCUS) dc.DrawFocusRect(r);
}

void StandardsDlg::DeleteItem(DELETEITEMSTRUCT _FAR & DeleteInfo)
{
  delete (Component *)DeleteInfo.itemData;
  DeleteInfo.itemData=0;
}

void StandardsDlg::SetupControls(void *data)
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

void StandardsDlg::AcceptControls(void *data)
{
  Method *pMethod=(Method *)data;
  for(int n=0; n<NUMDETECTORS; n++) {
    Component *pComponent=pMethod->det[n].components;
    for(int i=0; i<COMPONENT_ROWS; i++,pComponent++) {
      LRESULT res=::SendMessage(list[n]->HWindow,LB_GETITEMDATA,i,0L);
      if(res!=LB_ERR) { *pComponent=*(Component *)res; }
      //else { memset(ci,0,sizeof(*ci)); memset(si,0,sizeof(*si)); }
    }
  }
}

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif

static ControlInfo far ci[]={
  { citStr,	ID_COMPONENT_NAME,	FIELDOFFSET(Component,name), },
  { citONOFF,	ID_STAND_REF,	FIELDOFFSET(Component,ref_peak), },
  { citDouble,	ID_COMPONENT_RESP_FACT,	FIELDOFFSET(Component,resp_fact), },
  { citDouble,	ID_STAND_0,	FIELDOFFSET(Component,stand[0]), },
  { citDouble,	ID_STAND_1,	FIELDOFFSET(Component,stand[1]), },
  { citDouble,  ID_STAND_2,	FIELDOFFSET(Component,stand[2]), },
  { citDouble,  ID_STAND_3,	FIELDOFFSET(Component,stand[3]), },
  { citNULL }	// End of array marker
};

void StandardsDlg::Edit(int n)
{
  int Index=list[n]->GetSelIndex();
  if(Index<0) return;
  LRESULT res=::SendMessage(list[n]->HWindow,LB_GETITEMDATA,Index,0L);
  Component *pComponent=(Component *)res;
  int i=GetModule()->ExecDialog(new TransferPlusDlg(this,ID_EDITSTANDARDDIALOG,pComponent,ci));
  if(i!=IDCANCEL) {
    if(pComponent->ref_peak) {
      for(int i=0; i<COMPONENT_ROWS; i++) {
	Component *pComponent1=(Component *)::SendMessage(list[n]->HWindow,LB_GETITEMDATA,i,0L);
	if(pComponent!=pComponent1) { pComponent1->ref_peak=0; }
      }
    }
    else {
      for(int i=0; i<COMPONENT_ROWS; i++) {
	Component *pComponent1=(Component *)::SendMessage(list[n]->HWindow,LB_GETITEMDATA,i,0L);
	if(pComponent1->ref_peak) break;
      }
      if(!(i<COMPONENT_ROWS)) pComponent->ref_peak=1;
    }
    InvalidateRect(list[n]->HWindow,NULL,FALSE);
  }
}

PTWindowsObject newCalibrationDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, Detector *pDetector, Peak *pPeak, PTModule AModule=NULL);


void StandardsDlg::CalibrationButton(RTMessage)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    int Index=list[n]->GetSelIndex();
    if(Index<0) continue;
    LRESULT res=::SendMessage(list[n]->HWindow,LB_GETITEMDATA,Index,0L);
    Component *pComponent=(Component *)res;
    Peak *pPeak=0;
    if(pPeakList[n] && pComponent) {
      for(PeakList::Node *pNode=pPeakList[n]->first(); pNode; pNode=pNode->nextNode()) {
	//if(pNode->pComponent==pComponent) {
	if(pNode->pComponent && pNode->pComponent->name==pComponent->name) {
	  pPeak=pNode;
	  break;
	}
      }
    }
    Method *pMethod=(Method *)TransferBuffer;
    Detector *pDetector=&pMethod->det[n];
    GetModule()->ExecDialog(newCalibrationDlg(this,ID_CALIBRATIONDIALOG,pComponent,pDetector,pPeak));
    break;
  }
}




/* ----------------------------------------------------------------- */

#ifndef  _STND_DLG_H
#include "stnd_dlg.h"
#endif

PTWindowsObject newStandardsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PeakList **pPeakList, PTModule AModule)
{
  return new StandardsDlg(AParent, ResourceId, pMethod, pPeakList, AModule);
}





/* ----------------------------------------------------------------- */




class CalibrationDlg : public HelpDlg {
  typedef HelpDlg Inherited;
 public:
   CalibrationDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, Detector *pDetector, Peak *pPeak, PTModule AModule = NULL);
 protected:
  virtual void SetupWindow();
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);

  Detector *pDetector;
  Peak *pPeak;
};

CalibrationDlg::CalibrationDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, Detector *_pDetector, Peak *_pPeak, PTModule AModule)
 : HelpDlg(AParent, ResourceId, pComponent, AModule)
 , pDetector(_pDetector)
 , pPeak(_pPeak)
{
}

void CalibrationDlg::SetupWindow()
{
  Inherited::SetupWindow();
  Centralize(this);
}

void CalibrationDlg::SetupControls(void *)
{
}

void CalibrationDlg::AcceptControls(void *)
{
}

#include "oz.h"
#include "winlib\pen.h"
#include "winlib\selpen.h"
#include "winlib\font.h"
#include "winlib\selfont.h"
#include "winlib\brush.h"
#include "winlib\selbrush.h"
#include <math.h>
void CalibrationDlg::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
{
  Component *pComponent=(Component *)TransferBuffer;


  static const char far x_fmt[]="%.0f";//"%.3g";
  static const char far y_fmt[]="%.2f";//"%.3g";
  Color color=Black;
  const char *xname="";
  const char *yname="";
  int kx=7;
  int ky=7;
  int i,j,nnx,nny;
  int x,y;
  char buffer[80];

  DC dc(DrawInfo.hDC);
  DC ic(DrawInfo.hDC);

  const WRect &rc=(const WRect &)DrawInfo.rcItem;
  WRect rcClient=rc;
//  rcClient.Left()+=rc.Width()/8;
//  rcClient.Right()-=rc.Width()/12;
//  rcClient.Top()+=rc.Height()/8;
//  rcClient.Bottom()-=rc.Height()/8;

  double ymin=0, ymax=0, ystep;
  double xmin=0, xmax=0, xstep;

  {
    for(int j1=1; j1<=STAND_NUM; j1++) {
      if(pComponent->lstart[j1]) {
	double y=pComponent->lvalue[j1];
	if(y>ymax) ymax=y;
	double x=pComponent->lstart[j1];
	if(x>xmax) xmax=x;
      }
      else {
	j1--;
	xmax*=1.1;
	double y=pComponent->lvalue[j1]+(xmax-pComponent->lstart[j1])*pComponent->lslope[j1];
	if(y>ymax) ymax=y;
	break;
      }
    }
  }


  if(xmax<=xmin) xmax=xmin+1;
  if(ymax<=ymin) ymax=ymin+1;
  oz(&xmin,&xmax,5,10,0,&kx,&xstep);
  oz(&ymin,&ymax,7,12,0,&ky,&ystep);

  double dxx=xmax-xmin; if(!dxx) dxx=1;
  double dyy=ymax-ymin; if(!dyy) dyy=1;

  {
    int i=log10(xstep);
    if(xstep>1.) i++;
    if((int)(xstep/pow10(i-1)+.5)==2) nnx=4;
    else nnx=5;
  }
  {
    int i=log10(ystep);
    if(ystep>1.) i++;
    if((int)(ystep/pow10(i-1)+.5)==2) nny=4;
    else nny=5;
  }

  int logpixelsy=dc.IsMetafile()? 2540: ic.GetCapability(LOGPIXELSY);

  SelBkMode OldBkMode(dc,bkmTransparent);
  int penwidth=dc.IsRasterDisplay()? 0: MulDiv(50,logpixelsy,2540);	// 0.5 mm = 50 * 0.01 mm
  LogPen pen(psSolid, penwidth, color);
  SelPen oldPen(dc,pen);

  LOGFONT lf;//=pPictInfo->lfUser;
  memset(&lf,0,sizeof(lf));

  lf.lfHeight=MulDiv(8,logpixelsy,72); // 8 pts (1pt == 1"/72)
  int lineheight=1,linewidth=1;
  LogFont *pFont=NULL;
  int wid=0;
  int ht=0;
  int m=MulDiv(8,logpixelsy,72);

  while( abs(lf.lfHeight) >= m ) {
    LogFont *p=Validate(new LogFont(&lf));
    if(!p) break;
    {
      SelFont selFont(ic,*p);
      linewidth = ic.GetTextExtent("W").X();
      wid=0;
      ht=ic.GetTextExtent("H").X();
      for(i=0; i<=ky; i++) {
	sprintf(buffer,y_fmt,ymin+ystep*i);
	int w=ic.GetTextExtent(buffer).X();
	if(wid<w) wid=w;
      }
      wid+=2*linewidth/5+linewidth/4;
      ht+=lineheight/3;
      if(!ic.IsRasterDisplay() || abs(lf.lfHeight)==m ||
	(rc.Left()<=rcClient.Left()-wid && rc.Bottom()>=rcClient.Bottom()+ht ) ) {
	pFont=p;
	lineheight=ic.GetTextExtent("H").Y();
	break;
      }
    }
    delete p;
    if(lf.lfHeight>0) lf.lfHeight--;
    else lf.lfHeight++;
  }
  if(pFont) {
    LOGFONT lfItal=lf;
    lfItal.lfItalic=1;
    LogFont *pFontItal=Validate(new LogFont(&lfItal));

    int height=lineheight*2;
    if(ic.IsRasterDisplay() && rc.Left()+wid>rcClient.Left() && rcClient.Width()/3>wid && rcClient.Height()/3>height ) {
      rcClient.Left()=rc.Left()+wid;
      rcClient.Right()=rc.Right()-wid;

      rcClient.Top()=rc.Top()+height;
      rcClient.Bottom()=rc.Bottom()-height;
    }
    if(!ic.IsRasterDisplay() && rc.Left()+wid>rcClient.Left() ) {
      //MessageBox(NULL, "Wid", "Internal Error", MB_ICONINFORMATION|MB_OK|MB_TASKMODAL);
      rcClient.Left()=rc.Left()+wid;
      //rcClient.Right()=rc.Right()-wid;
    }

    int dx=rcClient.Width();
    int dy=rcClient.Height();

    //SelTextColor selColor(dc, pPictInfo->TextColor);
    SelFont selFont(dc,*pFont);
    x=rcClient.Left()-(2*linewidth/5+linewidth/4);
    if(x<rc.Left()) x=rc.Left();
    {
      SelFont selFont(dc,*pFontItal);
      SelTextAlign taOld(dc,taLeft|taBaseline);
      y=(rc.Top()+rcClient.Top())/2;
      dc.TextOut(x+linewidth/2,y,yname);
    }
    SelTextAlign taOld(dc,taRight|taBaseline);
    for(i=0; i<=ky; i++) {
      if(i!=ky) for(j=1; j<nny; j++) {
	y=rcClient.Bottom()-1-(int)((ystep*i+(ystep*j)/nny)/dyy*dy+0.5);
	dc.MoveTo(rcClient.Left()-linewidth/5,y);
	dc.LineTo(rcClient.Left()            ,y);
      }
      y=rcClient.Bottom()-1-(int)((ystep*i)/dyy*dy+0.5);
      dc.MoveTo(rcClient.Left()-2*linewidth/5,y);
      dc.LineTo(rcClient.Left()            ,y);
      sprintf(buffer,y_fmt,(ymin+ystep*i));
      dc.TextOut(x,y+lineheight/4,buffer);
    }
    dc.SetTextAlign(taCenter|taTop);
    y=rcClient.Bottom()+lineheight/3;
    for(i=0; i<=kx; i++) {
      if(i!=kx) for(j=1; j<nnx; j++) {
	x=rcClient.Left()+(int)((xstep*i+(xstep*j)/nnx)/dxx*dx+0.5);
	dc.MoveTo(x,rcClient.Bottom());
	dc.LineTo(x,rcClient.Bottom()+lineheight/5);
      }
      x=rcClient.Left()+(int)((xstep*i)/dxx*dx+0.5);
      dc.MoveTo(x,rcClient.Bottom());
      dc.LineTo(x,rcClient.Bottom()+lineheight/3);
      sprintf(buffer,x_fmt,(xmin+xstep*i)/*data_rate_per_min*/);
      dc.TextOut(x,y,strtok(buffer," "));
    }
    {
      SelFont selFont(dc,*pFontItal);
      SelTextAlign oldTexAlign(dc,taLeft|taBaseline);
      x=rcClient.Right()+linewidth/2;
      dc.TextOut(x,y-linewidth/3,xname);
    }
    delete pFontItal;
    ///delete pFont;
  }

  if(!rcClient.IsEmpty()) {
    {
      StockBrush brush(sbiNull);
      SelBrush oldBrush(dc,brush);
      dc.Rectangle(rcClient);
    }

    double h=rcClient.Height();
    double w=rcClient.Width();

    dc.Save(); // to restore ClipRegion later
    dc.IntersectClipRect(rcClient);
    {
      {
	Color color=Black;//pPictInfo->Color[_num];//Black;
	LogPen pen(psSolid, penwidth, color);
	SelPen oldPen(dc,pen);

	POINT ptArray[STAND_NUM+2];
	{
	  int cPoints=0;
	  POINT *lppt=ptArray;
	  double x=0;
	  lppt->x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
	  double y=0;
	  lppt->y=rcClient.Bottom()-1-(int)(double(y-ymin)*h/dyy);
	  lppt++;
	  cPoints++;
	  {
	    for(int j1=1; j1<=STAND_NUM; j1++) {
	      if(pComponent->lstart[j1]) {
		double x=pComponent->lstart[j1];
		lppt->x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
		double y=pComponent->lvalue[j1];
		lppt->y=rcClient.Bottom()-1-(int)(double(y-ymin)*h/dyy);
		{
		  dc.MoveTo(lppt->x-linewidth/5,lppt->y);
		  dc.LineTo(lppt->x+linewidth/5,lppt->y);
		  dc.MoveTo(lppt->x,lppt->y-lineheight/5);
		  dc.LineTo(lppt->x,lppt->y+lineheight/5);
		}
		lppt++;
		cPoints++;
	      }
	      else {
		j1--;
		double x=xmax;
		lppt->x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
		double y=pComponent->lvalue[j1]+(xmax-pComponent->lstart[j1])*pComponent->lslope[j1];
		lppt->y=rcClient.Bottom()-1-(int)(double(y-ymin)*h/dyy);
		cPoints++;
		break;
	      }
	    }
	  }
	  dc.Polyline(ptArray, cPoints);
	}
      }
      if(pPeak) {
	Color color=Red;
	LogPen pen(psSolid, penwidth, color);
	SelPen oldPen(dc,pen);
	int x=rcClient.Left()+(int)(double((pDetector->detect_meth? pPeak->Area: pPeak->Height)-xmin)*w/dxx);
	int y=rcClient.Bottom()-1/*-(int)(double(y-ymin)*h/dyy)*/;
	dc.MoveTo(x,y);
	dc.LineTo(x,y-lineheight);
      }
    }
    dc.Restore();  // Restore ClipRegion. May be needed later?
  }
  delete pFont;
}



PTWindowsObject newCalibrationDlg(PTWindowsObject AParent, int ResourceId, Component *pComponent, Detector *pDetector, Peak *pPeak, PTModule AModule)
{
  return new CalibrationDlg(AParent, ResourceId, pComponent, pDetector, pPeak, AModule);
}


