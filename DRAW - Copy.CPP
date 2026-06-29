#include "includes.h"
#pragma hdrstop

//#pragma option -3 // 386 code

#ifndef __MATH_H
#include <math.h> // log10,
#endif
#ifndef __STDIO_H
#include <stdio.h> // sprintf,
#endif
#ifndef __STRING_H
#include <string.h>
#endif

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#ifndef  _WCPPLIB_WRECT_H
#include "wcpplib\wrect.h"
#endif
#ifndef  _WINLIB_FONT_H
#include "winlib\font.h"
#endif
#ifndef  _WINLIB_PEN_H
#include "winlib\pen.h"
#endif
#ifndef  _WINLIB_COLOR_H
#include "winlib\color.h"
#endif
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
#ifndef  _WINLIB_SELPEN_H
#include "winlib\selpen.h"
#endif
#ifndef  _WINLIB_SELFONT_H
#include "winlib\selfont.h"
#endif
#ifndef  _WINLIB_SELBRUSH_H
#include "winlib\selbrush.h"
#endif


#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif
#ifndef  _OZ_H
#include "oz.h"
#endif
#ifndef  _UTL_H
#include "utl.h"
#endif
#ifndef  _DRAW_H
#include "draw.h"
#endif
#ifndef  _JOB_H     // added. P.P. 01/24/13
#include "job.h"
#endif

void read(Str &str, Ripstream is)
{
  str.Delete();
  size_t len;
  is>>len;
  if(len) {
    str.Alloc( len + 1 );
    is.readBytes(str.GetPtr(),len*sizeof(char));
    str.GetPtr()[len]='\0';
  }
}
void write(const Str &str, Ropstream os)
{
  size_t len=str.Length();
  os<<len;
  if(len) {
    os.writeBytes(str,len*sizeof(char));
  }
}


AxisInfo::AxisInfo()
// : bAuto(TRUE), mi(0), ma(0), k(5)                  
 : bAuto(TRUE), mi(0), ma(0), k(5), bScale(FALSE)     // P.P. 01/24/13
{
}

void AxisInfo::write (Ropstream os)
{
  ::write(Title,os);
//  os<<bAuto<<mi<<ma<<k;
  os<<bAuto<<mi<<ma<<k<<bScale;
}

Pvoid AxisInfo::read (Ripstream is)
{
  ::read(Title,is);
  if(job_file_version >=37) {     // P.P. 01/24/13
    is>>bAuto>>mi>>ma>>k>>bScale;
  }
  else {
    is>>bAuto>>mi>>ma>>k;
  }
  return this;
}


PictInfo::PictInfo()
 : TextColor(0)
{
  lfUser.lfHeight=16;
  lfUser.lfWidth=0;
  lfUser.lfEscapement=0;
  lfUser.lfOrientation=0;
  lfUser.lfWeight=FW_NORMAL;//FW_DONTCARE;
  lfUser.lfItalic=0;
  lfUser.lfUnderline=0;
  lfUser.lfStrikeOut=0;
  lfUser.lfCharSet=DEFAULT_CHARSET; // not ANSI_CHARSET for International support
  lfUser.lfOutPrecision=OUT_TT_PRECIS;//OUT_DEFAULT_PRECIS;
  lfUser.lfClipPrecision=CLIP_DEFAULT_PRECIS;
  lfUser.lfQuality=PROOF_QUALITY;//DEFAULT_QUALITY;
  lfUser.lfPitchAndFamily=FF_DONTCARE|VARIABLE_PITCH;//DEFAULT_PITCH;
  lstrcpy(lfUser.lfFaceName,"Arial");
  { for(int i=0; i<2; i++) Color[i]=0; }
}

void PictInfo::write (Ropstream os)
{
  os<<TextColor;
  os.writeBytes(&lfUser,sizeof(lfUser));
  { for(int i=0; i<2; i++) ai[i].write(os); }
  { for(int i=0; i<2; i++) os<<Color[i]; }
}

Pvoid PictInfo::read (Ripstream is)
{
  is>>TextColor;
  is.readBytes(&lfUser,sizeof(lfUser));
  { for(int i=0; i<2; i++) ai[i].read(is); }
  { for(int i=0; i<2; i++) is>>Color[i]; }
  return this;
}




extern char data_rate;           // rate data received in pts/sec.

#ifndef  _RUN_H
#include "run.h"
#endif

#include "debug.h"

#define USE_GLOBALALLOC() 1

POINT FAR *AllocPoints(uint cPoints)
{
  #if USE_GLOBALALLOC()
  POINT FAR *lppt=(POINT FAR *)GlobalLock(GlobalAlloc(GMEM_MOVEABLE,cPoints*sizeof(*lppt)));
  #else
  POINT FAR *lppt=new POINT FAR[cPoints];
  #endif
  //TRACE3("AllocPoints(%d*%d)=%p)",cPoints,sizeof(*lppt),lppt);
  return lppt;
}
void DeletePoints(POINT FAR *lppt)
{
  //TRACE1("DeletePoints %p)",lppt);
  #if USE_GLOBALALLOC()
  if(lppt) GlobalFree(((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lppt)))));
  #else
  delete []lppt;
  #endif
}

//#include "job.h"  // moved up. P.P. 01/24/13
#include "method.h"
#include "calc.h"

#if OPTICS()
double _xmin_=400;
double _dx_=0.36;
#else
const double _xmin_=0;
#endif

extern int mult_num[2][30][4]; // added. P.P. 01/11/13
long start_autoscale[2] = {1,1};  // to show multiplier only if range changes. P.P. 05/14/17 moved here 06//24/17
int start_gain[2] = {2,2};  // declare. P.P. 05/14/17 moved here 06/24/17
int cur_mult[2] = {11}; // to display Range the data was taken on. P.P. 06/24/17
double cur_indicator[2] = {1,1};  // P.P. 06/24/17

void Draw(Job *pJob, Run *pRun, PictInfo **_pPictInfo,const DC &dc, const DC &ic, const WRect &rc, const RunRawData &runrawdata,
  #ifdef FILTER
  FiltData *pFiltData,
  #endif//FILTER
  int num, PeakList **_pPeakList, HWND *hwnd)
{
  Method *pMethod=pJob;
  static const char far _y_fmt[]="%.0f";//"%.3g";
  const char far *y_fmt=_y_fmt;

double range=1;

double factor=1;
TextAlign taY=taRight;

  PictInfo *pPictInfo=_pPictInfo[num];
  double mi0Old=pPictInfo->ai[0].mi;
  double ma0Old=pPictInfo->ai[0].ma;
  double mi1Old=pPictInfo->ai[1].mi;
  double ma1Old=pPictInfo->ai[1].ma;
  Color color=Black;
  #if OPTICS()
  const char *xname="nm";
  #else
  const char *xname="min";
  #endif // optics
  const char *yname="";
  int kx=7;
  int ky=7;
  int i,j,nnx=0,nny;
//  int x,y;
  char buffer[80];

  WRect rcClient=rc;
  rcClient.Left()+=rc.Width()/8;
  rcClient.Right()-=rc.Width()/12;
  rcClient.Top()+=rc.Height()/8;
  rcClient.Bottom()-=rc.Height()/8;

  double ymin=0x7FFFFFFFL,ymax=-0x7FFFFFFFL,ystep;
  double xmin=0;//_xmin_;
  double xmax=xmin;
  if(!pPictInfo->ai[0].bAuto) {
    xmin=pPictInfo->ai[0].mi;
    xmax=pPictInfo->ai[0].ma;
    xmin*=data_rate;
    xmax*=data_rate;
  }
  long scale[NUMDETECTORS]={1,1};
  long lmax=0;
//  long start_autoscale = 1;  // to show multiplier only if range changes. P.P. 01/10/13
//  long start_autoscale[2] = {1,1};  // to show multiplier only if range changes. P.P. 05/14/17  moved up 06/24/17
//  Detector *pDetector=&pMethod->det[0]; // rem here, done below for both detectors, separetely. P.P. 05/14/17 
//  int start_gain = pDetector->gain;  // initiate. P.P. 12/28/12
//  int start_gain[2] = {2,2};  // declare. P.P. 05/14/17  moved up 06/24/17
//  start_autoscale = (long) pow10(start_gain);
//  start_autoscale[2] = {(long) pow10(2),(long) pow10(2)]};  // declare and initialize. P.P. 05/14/17
  {
    for(int _num=num==NUMDETECTORS? 0: num; _num<( num==NUMDETECTORS? NUMDETECTORS: num+1); ++_num) {
      Detector *pDetector=&pMethod->det[_num];
      start_gain[_num] = pDetector->gain;  // read from det dialog. P.P. 05/14/17
      double yminl=0x7FFFFFFFL,ymaxl=-0x7FFFFFFFL;
      start_autoscale[_num] = (long) pow10(start_gain[_num]);  // read from det dialog. P.P. 05/14/17

      #if FLAME()
      //if(num==1) { yminl=0; ymaxl=-0x7FFFFFFFL; }
      #endif // FLAME()
      double x=0;//_xmin_;
      long ll=0;
//      DataIterator *pit;
      RawDataIterator *pit; // replaced to use gain. P.P. 12/28/12
      #ifdef FILTER
      if(pFiltData && pFiltData->p) pit=new FiltDataIterator(pFiltData, _num);
      else
      #endif//FILTER
      pit=new RawDataIterator(runrawdata, _num);
      RawData rawdata(pDetector->segment_width, pMethod->method_table.analysis_time);
      for( ; pit->Ok(); pit->GetNext(), x++) {
	int32 y=pit->Get();
	rawdata.ProcessData(y);
//	scale[_num]=rawdata.cur_autoscale; // P.P. 12/27/12
	scale[_num]=1; // P.P. 01/18/13

	if(rawdata.acquire_finished)
	  break;

	if(!rawdata.dq.IsSampleFinished()) { // if sample finished,...
	  continue;
	}
	y=rawdata.dq.GetScaledSample();
#if 1  //  peak scaling disabled. P.P. 01/20/13  enabled 01/24/13
/*
	if(rawdata.cur_autoscale>0) {         //
          y = y*(1000/rawdata.cur_autoscale); // added. P.P. 12/27/12
        }                                     //
*/
	if(pPictInfo->ai[1].bScale) {   // cond added. P.P. 01/24/13
//	  if((rawdata.cur_autoscale>0)&&(start_autoscale>0)) {  // now all related to the start range. P.P. 01/18/13
	  if((rawdata.cur_autoscale>0)&&(start_autoscale[_num]>0)) {  // now all related to the start range. P.P. 05/14/17
/*
	    if(rawdata.cur_autoscale<=start_autoscale) {         //
	      y = y*(start_autoscale/rawdata.cur_autoscale); // replacement. P.P. 01/18/13
	    }
	    else {
	      y = y/(rawdata.cur_autoscale/start_autoscale); // decrease peaks but show multipliers >1. P.P. 01/18/13
            }
*/
// individually for each detector now. P.P. 05/14/17
	    if(rawdata.cur_autoscale<=start_autoscale[num]) {         //
	      y = y*(start_autoscale[_num]/rawdata.cur_autoscale); // replacement. P.P. 05/14/17
	    }
	    else {
	      y = y/(rawdata.cur_autoscale/start_autoscale[_num]); // decrease peaks but show multipliers >1. P.P. 05/14/17
            }
// end of individually 
	  }
	}                                         //
#endif // end of peack scaling. P.P. 01/20/13
	//long y=pit->Get();
	if(pPictInfo->ai[0].bAuto || (xmin<=x && x<=xmax)) {
	  if(ymaxl<y) ymaxl=y;
	  if(yminl>y) yminl=y;
	}
	if(pPictInfo->ai[0].bAuto) {
	  xmax=x;
	}
	ll++;
      }
//      int gain=pit->Gain();// ??*pDetector->gain??  moved up. P.P. 12/26/12
      //if(scale[_num]<pow10(gain))
//      scale[_num]=pow10(gain); //////////////??    moved up
      if(scale[_num]<=0) scale[_num]=1;
      ymaxl*=scale[_num];
      if(ymax<ymaxl) ymax=ymaxl;
      yminl*=scale[_num];
      if(ymin>yminl) ymin=yminl;

      delete pit;
      if(lmax<ll) lmax=ll;
    }
  }
//if(!lmax) lmax=300; // DEBUG

  if(xmax<=xmin) return;


#if FLAME()
  if(num==1) {
    Detector *pDetector=&pMethod->det[num];
    CalResults *pCalResults=pJob->GetCalResults(pRun);

    CalcConcVars(pDetector, pCalResults->cal_results[num]);

    Component *pComponent=&pDetector->components[0];
    double conc_max;
    double conc_min;
    int flags1=pDetector->Concentration(pMethod, pRun, pComponent, ymax, num, &conc_max);
    int flags2=pDetector->Concentration(pMethod, pRun, pComponent, ymin, num, &conc_min);

    if(flags1&&flags2) {
      if((ymax-ymin) && (conc_max-conc_min))
	factor=(ymax-ymin)/(conc_max-conc_min);

      static const char far _y_fmt1[]="%.3g";
      y_fmt=_y_fmt1;

      if(fabs(conc_max)<1.0 && fabs(conc_max)<1.0)
	taY=taLeft;
    }
  }
#endif // flame

  double xstep;
  #if OPTICS()
  const double sec_per_min=1.;
  #else
  const double sec_per_min=60.;
  #endif
  double data_rate_per_min=data_rate*sec_per_min;

#if OPTICS() // cond added. P.P. 12/09/03
  xmin=_xmin_+xmin*_dx_;
  xmax=_xmin_+xmax*_dx_;
#endif

  if(pPictInfo->ai[0].bAuto) {
    xmax=(int)((xmax+data_rate_per_min-1)/data_rate_per_min);  // in min
    if(xmax==1)      { kx=6; xstep=xmax/kx; nnx=2; } // special seconds treatment
    else if(xmax==2) { kx=6; xstep=xmax/kx; nnx=4; }
    else if(xmax==3) { kx=6; xstep=xmax/kx; nnx=3; }
    else if(xmax==4) { kx=8; xstep=xmax/kx; nnx=3; }
    else if(xmax==5) { kx=5; xstep=xmax/kx; nnx=3; }
    else if(xmax==6) { kx=6; xstep=xmax/kx; nnx=3; }
    else if(xmax==7) { kx=7; xstep=xmax/kx; nnx=2; }
    else if(xmax==8) { kx=8; xstep=xmax/kx; nnx=2; }
    else if(xmax==9) { kx=9; xstep=xmax/kx; nnx=2; }
    else if(xmax==10) { kx=5; xstep=xmax/kx; nnx=4; }
    else {
      xmax*=0.9999;
      if(xmax<xmin) return;
      oz(&xmin,&xmax,5,10,0,&kx,&xstep);
    }
    pPictInfo->ai[0].mi=xmin*sec_per_min;
    pPictInfo->ai[0].ma=xmax*sec_per_min;
    pPictInfo->ai[0].k=kx;
    xmin*=data_rate_per_min;
    xmax*=data_rate_per_min;
    xstep*=data_rate_per_min;
  }
  else {
    kx=pPictInfo->ai[0].k;
    if(kx<1) kx=1;
    xstep = (xmax-xmin)/kx;
    if(xstep<=0) xstep=1;
  }
  if(!nnx) {
    int i=log10(xstep);
    if(xstep>1.) i++;
    if((int)(xstep/pow10(i-1)+.5)==2) nnx=4;
    else nnx=5;
  }
  if(pPictInfo->ai[1].bAuto) {
    if(ymax<=ymin) ymax=ymin+1;

    ymin/=factor; ymax/=factor;

    oz(&ymin,&ymax,5,10,0,&ky,&ystep);
    pPictInfo->ai[1].mi=ymin;
    pPictInfo->ai[1].ma=ymax;
    pPictInfo->ai[1].k=ky;
  }
  else {
    ymin=pPictInfo->ai[1].mi;
    ymax=pPictInfo->ai[1].ma;
    ky=pPictInfo->ai[1].k;
    if(ky<1) ky=1;
    ystep = (ymax-ymin)/ky;

  }
  ymin*=factor; ymax*=factor; ystep*=factor;
  if(ystep<=0) ystep=1;

  {
    int i=log10(ystep);
    if(ystep>1.) i++;
    if((int)(ystep/pow10(i-1)+.5)==2) nny=4;
    else nny=5;
  }

  if(ystep/factor) {
    int i=log10(ystep/factor);
    if(i>=2) {
      range=pow10(i);
      static char yrange[20];
      if(i>4)
	wsprintf(yrange,"*10e%d",i);
      else
	wsprintf(yrange,"*%d",(int)range);
      yname=yrange;
    }
  }

  if(hwnd) {
    int bChanged=
      mi0Old!=pPictInfo->ai[0].mi ||
      ma0Old!=pPictInfo->ai[0].ma ||
      mi1Old!=pPictInfo->ai[1].mi ||
      ma1Old!=pPictInfo->ai[1].ma;
    if(bChanged) {
      InvalidateRect(*hwnd,0,TRUE); // ??
      return; //???
    }
  }
#if OPTICS() // cond added. P.P. 12/09/03
  xmin=(xmin-_xmin_)/_dx_;
  xmax=(xmax-_xmin_)/_dx_;
  xstep=xstep/_dx_;
#endif

  double dxx=xmax-xmin; if(!dxx) dxx=1;
  double dyy=ymax-ymin; if(!dyy) dyy=1;

  int logpixelsy=dc.IsMetafile()? 2540: ic.GetCapability(LOGPIXELSY);

  SelBkMode OldBkMode(dc,bkmTransparent);
  int penwidth=dc.IsRasterDisplay()? 0: MulDiv(50,logpixelsy,2540);	// 0.5 mm = 50 * 0.01 mm
  LogPen pen(psSolid, penwidth, color);
  SelPen oldPen(dc,pen);

  LOGFONT lf=pPictInfo->lfUser;

  if(ic.IsRasterDisplay()) lf.lfHeight=MulDiv(16,logpixelsy,72); // 16 pts (1pt == 1"/72)
  else lf.lfHeight=MulDiv(pPictInfo->lfUser.lfHeight,logpixelsy,72); // user pts (1pt == 1"/72)
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
	sprintf(buffer,y_fmt,(ymin+ystep*i)/factor/range);
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

    SelTextColor selColor(dc, pPictInfo->TextColor);
    SelFont selFont(dc,*pFont);
    int x=rcClient.Left()-(2*linewidth/5+linewidth/4);
    if(x<rc.Left()) x=rc.Left();
    {
      SelFont selFont(dc,*pFontItal);
      SelTextAlign taOld(dc,taLeft|taBaseline);
      int y=(rc.Top()+rcClient.Top())/2;
      dc.TextOut(x+linewidth/2,y,yname);
    }
    SelTextAlign taOld(dc,taY|taBaseline);
    if(taY==taLeft)
      x-=wid-2*linewidth/5-linewidth/4;
    for(i=0; i<=ky; i++) {
      if(i!=ky) for(j=1; j<nny; j++) {
	int y=rcClient.Bottom()-1-(int)((ystep*i+(ystep*j)/nny)/dyy*dy+0.5);
	dc.MoveTo(rcClient.Left()-linewidth/5,y);
	dc.LineTo(rcClient.Left()            ,y);
      }
      int y=rcClient.Bottom()-1-(int)((ystep*i)/dyy*dy+0.5);
      dc.MoveTo(rcClient.Left()-2*linewidth/5,y);
      dc.LineTo(rcClient.Left()            ,y);
      sprintf(buffer,y_fmt,(ymin+ystep*i)/factor/range);
      dc.TextOut(x,y+lineheight/4,buffer);
    }
    dc.SetTextAlign(taCenter|taTop);
    int y=rcClient.Bottom()+lineheight/3;
    for(i=0; i<=kx; i++) {
      if(i!=kx) for(j=1; j<nnx; j++) {
	x=rcClient.Left()+(int)((xstep*i+(xstep*j)/nnx)/dxx*dx+0.5);
	dc.MoveTo(x,rcClient.Bottom());
	dc.LineTo(x,rcClient.Bottom()+lineheight/5);
      }
      x=rcClient.Left()+(int)((xstep*i)/dxx*dx+0.5);
      dc.MoveTo(x,rcClient.Bottom());
      dc.LineTo(x,rcClient.Bottom()+lineheight/3);
      strcpy(buffer,PrintSeconds((xmin+xstep*i)/data_rate+0.5));
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

    int h=rcClient.Height();
    int w=rcClient.Width();

    dc.Save(); // to restore ClipRegion later
    dc.IntersectClipRect(rcClient);
    //long threshold=0;
    for(int s=/*0*/1; s<2; s++) { // raw or sampled data
      for(int _num=num==NUMDETECTORS? 0: num; _num<( num==NUMDETECTORS? NUMDETECTORS: num+1); ++_num) {
	Detector *pDetector=&pMethod->det[_num];
	Color color=pPictInfo->Color[_num];//Black;
	LogPen pen(s==0? psDot: psSolid, penwidth, color);
	SelPen oldPen(dc,pen);

	double yminl=ymin/scale[_num];
	double dyyl=dyy/scale[_num];
	double x=0;//_xmin_;
	const int buffSize=1024;
	int cPoints=lmax>buffSize? buffSize: (int)lmax;
	POINT FAR * const lpptArray=AllocPoints(cPoints);
	if(lpptArray) {
	  POINT FAR* lppt=lpptArray;
	  int c=0;
//	  DataIterator *pit;
	  RawDataIterator *pit;  // replacement. P.P. 01/04/13
	  #ifdef FILTER
	  if(pFiltData && pFiltData->p) pit=new FiltDataIterator(pFiltData, _num);
	  else
	  #endif//FILTER
	  pit=new RawDataIterator(runrawdata, _num);
	  if(pit) {
            int32 yprev=0x7FFFFFFFL;
	    RawData rawdata(pDetector->segment_width, pMethod->method_table.analysis_time);
	    for(long ll=0; pit->Ok() && cPoints; pit->GetNext(), x++) {
	      int32 y=pit->Get();
	      #if !OPTICS()
	      //if(ll==0) threshold=y;
	      #endif

	      if(s==1) {
	        rawdata.ProcessData(y);
	        if(!rawdata.dq.IsSampleFinished()) { // if sample finished,...
		  continue;
	        }
	        y=rawdata.dq.GetScaledSample();
#if 1 // to disable peak scaling. P.P. 01/20/13  enabled back. 01/24/13
/*
		if(rawdata.cur_autoscale>0) {         //
                  y = y*(1000/rawdata.cur_autoscale); // added. P.P. 12/27/12
                }                                     //
*/
		if(pPictInfo->ai[1].bScale) {   // cond added. P.P. 01/24/13
/*
		  if((rawdata.cur_autoscale>0)&&(start_autoscale>0)) {  // now all related to the start range. P.P. 01/18/13
	  	    if(rawdata.cur_autoscale<=start_autoscale) {         //
	    	      y = y*(start_autoscale/rawdata.cur_autoscale); // replacement. P.P. 01/18/13
	  	    }
	  	    else {
	    	      y = y/(rawdata.cur_autoscale/start_autoscale); // decrease peaks but show multipliers >1. P.P. 01/18/13
          	    }   
		  }
*/
// new code - individually for detA and DetB. P.P. 05/14/17
		  if((rawdata.cur_autoscale>0)&&(start_autoscale[_num]>0)) {  // now all related to the start range. P.P. 05/14/17
	  	    if(rawdata.cur_autoscale<=start_autoscale[_num]) {         //
	    	      y = y*(start_autoscale[_num]/rawdata.cur_autoscale); // replacement. P.P. 05/14/17
	  	    }
	  	    else {
	    	      y = y/(rawdata.cur_autoscale/start_autoscale[_num]); // decrease peaks but show multipliers >1. P.P. 05/14/17
          	    }   
		  }
// end of new code. P.P. 05/14/17
		}                                         //
#endif // 0, to disable peak scaling. P.P. 01/20/13
	      }

              {
                int b=0;
                if(labs(yprev-y)>labs(y)/1000) {
                  yprev=y;
                  b=1;
                }
                extern double form(long x, double Time, double HalfWidth);
                PeakList *pPeakList=_pPeakList[_num];
                if(pPeakList) {
	          int _x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
                  double baseline=pPeakList->_baseline+pPeakList->_baselineslope*x;
                  double F=baseline;
                  for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
                    //if(pNode==pPeakList->first()) continue;
                    double a=pNode->_Height*form(x,pNode->_Time,pNode->_HalfWidth);
#if 1  //disable peak scaling. P.P. 01/20/13. eNABLED BACK 01/24/13
// new multiplies applied to calculated form as well. P.P. 01/18/13
	            if(pPictInfo->ai[1].bScale) {   // condd added. P.P. 01/24/13
/*
		      if((rawdata.cur_autoscale>0)&&(start_autoscale>0)) {  // now all related to the start range. P.P. 01/18/13
	  	        if(rawdata.cur_autoscale<=start_autoscale) {         //
	    	          a = a*(start_autoscale/rawdata.cur_autoscale); // replacement. P.P. 01/18/13
	  	        }
	  	        else {
	    	          a = a/(rawdata.cur_autoscale/start_autoscale); // decrease peaks but show multipliers >1. P.P. 01/18/13
          	        }   
		      }
*/
// new code, individually for Det A ans B. P.P. 05/14/17
		      if((rawdata.cur_autoscale>0)&&(start_autoscale[_num]>0)) {  // now all related to the start range. P.P. 05/14/17
	  	        if(rawdata.cur_autoscale<=start_autoscale[_num]) {         //
	    	          a = a*(start_autoscale[_num]/rawdata.cur_autoscale); // replacement. P.P. 05/14/17
	  	        }
	  	        else {
	    	          a = a/(rawdata.cur_autoscale/start_autoscale[_num]); // decrease peaks but show multipliers >1. P.P. 05/14/17
          	        }   
		      }
// end of new code.
		    }                                         //
// end. P.P. 01/18/13
#endif // disable peak scaling. P.P. 01/20/13
                    if(b) {
	              int _y=rcClient.Bottom()-1-(int)(double(a+baseline-yminl)*h/dyyl);
                      LogPen penRed(psSolid, penwidth/2, LightRed);
                      SelPen oldPen(dc,penRed);
                      dc.MoveTo(_x,_y-1-penwidth/2);
                      dc.LineTo(_x,_y+2);
                      dc.MoveTo(_x-1-penwidth/2,_y);
                      dc.LineTo(_x+2,_y);
                    }
                    F+=a;
                  }
                  //y=F;  
	          int _y=rcClient.Bottom()-1-(int)(double(F-yminl)*h/dyyl);
                  if(1){
                    LogPen penRed(psSolid, penwidth/2, LightGreen);
                    SelPen oldPen(dc,penRed);
                    dc.MoveTo(_x-1-penwidth/2,_y-1-penwidth/2);
                    dc.LineTo(_x+2,_y+2);
                    dc.MoveTo(_x-1-penwidth/2,_y+1+penwidth/2);
                    dc.LineTo(_x+2,_y-2);
                  }
                }
              }
	      lppt->x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
	      lppt->y=rcClient.Bottom()-1-(int)(double(y-yminl)*h/dyyl);
	      lppt++;
	      c++;
              ll++;
	      if(c==cPoints) {
		dc.Polyline(lpptArray, cPoints);
		cPoints=(lmax-ll)>buffSize? buffSize: (int)(lmax-ll);

		lppt=lpptArray;
		c=0;
		/*
		*lpptArray=lppt[-1]; // copy last point to the first for next line
		lppt=lpptArray+1;
		c=1;
		cPoints+=c; // add the first point (duplicated from previous line)
		*/
	      }
	    }
	    if(c)
	      dc.Polyline(lpptArray, c);
	  }
	  delete pit;
	}
	DeletePoints(lpptArray);
      }
    }
    dc.Restore();  // Restore ClipRegion. May be needed later?
    {
      #if 0//OPTICS()
      xmin-=0;//_xmin_;
      ///xmin/=_dx_;
      //xmin/=data_rate;
      dxx/=_dx_;
      #endif
      for(int _num=num==NUMDETECTORS? 0: num; _num<( num==NUMDETECTORS? NUMDETECTORS: num+1); ++_num) {
	double yminl=ymin/scale[_num];
	double dyyl=dyy/scale[_num];
	PeakList *pPeakList=_pPeakList[_num];
	if(pPeakList) {
	  SelFont selFont(dc,*pFont);
	  SelTextAlign taOld(dc,taLeft|taBottom);
	  for(PeakList::Node *ppln=pPeakList->first(); ppln; ppln=ppln->nextNode()) {
	    int x=rcClient.Left()+(int)(double(ppln->Time-xmin)*w/dxx);
	    if(rcClient.left>x || x>=rcClient.right) continue; // skip if out of range
	    {
	      int xFrom=rcClient.Left()+(int)(double(ppln->From-xmin)*w/dxx);
	      int xTo=rcClient.Left()+(int)(double(ppln->To-xmin)*w/dxx);
	      dc.MoveTo(xFrom,rcClient.Bottom()-1);
	      dc.LineTo(xFrom,rcClient.Bottom()-1-lineheight/3);
	      dc.MoveTo(xTo,rcClient.Bottom()-1);
	      dc.LineTo(xTo,rcClient.Bottom()-1-lineheight/3);
	    }
	    long peakHeight=ppln->Height;
#if 1 //disable peak scaling. P.P. 01/20/13  enabled back. 01/24/13
//	    long peakHeight=(ppln->Height)*(1000/(ppln->multiplier));  // P.P. 01/12/13
	    if(pPictInfo->ai[1].bScale) {   // condd added. P.P. 01/24/13
/*
	      if((ppln->multiplier)<=start_autoscale) {
	        peakHeight*=(start_autoscale/(ppln->multiplier));  // P.P. 01/18/13
	      }
	      else {
	        peakHeight/=((ppln->multiplier)/start_autoscale);  // P.P. 01/18/13
	      }
*/
// new code foe individually working with detectors A and B. P.P. 05/14/17
	      if((ppln->multiplier)<=start_autoscale[_num]) {
	        peakHeight*=(start_autoscale[_num]/(ppln->multiplier));  // P.P. 05/14/17
	      }
	      else {
	        peakHeight/=((ppln->multiplier)/start_autoscale[_num]);  // P.P. 05/14/17
	      }
// end of new code
	    }
#endif // 0, to disable peak scaling. P.P. 01/20/13
//	    peakHeight+=pPeakList->_baseline+pPeakList->_baselineslope*ppln->Time;
	    peakHeight = peakHeight + pPeakList->_baseline+pPeakList->_baselineslope*ppln->Time;
//	    int y=rcClient.Bottom()-1-(int)(double(peakHeight /* ?? *scale[_num]*/ -yminl)*h/dyyl);
	    int y=rcClient.Bottom()-1-(int)(double(peakHeight -yminl)*h/dyyl);
//	    y = y*(1000/scale[num]);
//	    int y=rcClient.Bottom()-1-(int)(double(peakHeight *scale[_num] -yminl)*h/dyyl); // P.P. 12/27/12 and restored.
	    if(y<rcClient.Top()+lineheight) y=rcClient.Top()+lineheight; //clip peak number
	    if(y>=rcClient.Bottom()) y=rcClient.Bottom();
	    char str[12];
#if 1 // disable peak scaling. P.P. 01/20/13 ENABLED BACK. 01/24/13
	    if(pPictInfo->ai[1].bScale) {   // cond added. P.P. 01/24/13
//	      int cur_mult =  ppln->multiplier; // to display Range the data was taken on. P.P. 01/11/13  moved up, made an array. 06/24/71
              cur_mult[_num] = ppln->multiplier; // to display Range the data was taken on. P.P. 06/24/17
//	      double cur_indicator = 1;  // moved up, made an array. P.P. 06/24/17
//              if(start_autoscale>0) {
	      if(start_autoscale[_num]>0) {
//	        cur_indicator = ((double)(ppln->multiplier))/start_autoscale;
//		cur_indicator = ((double)(ppln->multiplier))/start_autoscale[_num];  // P.P. 05/14/17
		cur_indicator[_num] = ((double)(ppln->multiplier))/start_autoscale[_num];  // P.P. 06/24/17
	      }
//	      if(cur_mult != (int) start_autoscale[_num])  {  // temp. P.P. 05/14/17
//		sprintf(str,"%d (X%g)",ppln->Num, cur_indicator); // P.P.  01/17/13
	      if(cur_mult[_num] != (int) start_autoscale[_num])  {  //  P.P. 06/24/17
		sprintf(str,"%d (X%g)",ppln->Num, cur_indicator[_num]); // 
	      }
	      else {
	        wsprintf(str,"%d",ppln->Num); // P.P. 12/28/12>
	      }
	    }
	    else {
		wsprintf(str,"%d",ppln->Num); // P.P. 01/24/13>
	    }
#else  // show the peak number, as usual. P.P. 01/20/13
	      wsprintf(str,"%d",ppln->Num); // P.P. 12/28/12>
#endif   // disable peak scaling. P.P. 01/20/13
	    dc.TextOut(x,y,str);
	  }
          if(1) {
            //int penwidth=dc.IsRasterDisplay()? 0: MulDiv(50,logpixelsy,2540);	// 0.5 mm = 50 * 0.01 mm
	    LogPen pen(psDot, penwidth/2, LightBlue);
	    SelPen oldPen(dc,pen);

	    int y=rcClient.Bottom()-1-(int)(double(pPeakList->baseline-yminl)*h/dyyl);
            dc.MoveTo(rcClient.Left(),y);
            dc.LineTo(rcClient.Right(),y);
          }
	}
      }
    }
  }
  delete pFont;
}


#include "country.h"
#include "method.h"

class _N : public List::Node
{
 public:
  double datetime;
  struct {
    struct {
      Component *pComponent;
      RunInfo *p;
    } components[COMPONENT_ROWS];
  } det[NUMDETECTORS];
};




/////////////////////////////////////////////////



void Draw1(Method *pMethod, RunPictInfo *pRunPictInfo,const DC &dc, const DC &ic, const WRect &rc, const List &list, HWND *hwnd)
{
hwnd=0;

  double mi0Old=pRunPictInfo->ai[0].mi;
  double ma0Old=pRunPictInfo->ai[0].ma;
  double mi1Old=pRunPictInfo->ai[1].mi;
  double ma1Old=pRunPictInfo->ai[1].ma;
  static const char far y_fmt[]="%.3g";//"%.0f";
  Color color=Black;
  const char *xnameT="time";
  const char *xnameD="date";
  const char *yname="";
  int kx=7;
  int ky=7;
  int i,j,nnx,nny;
  int x,y;
  char buffer[80];

  WRect rcClient=rc;
  rcClient.Left()+=rc.Width()/8;
  rcClient.Right()-=rc.Width()/12;
  rcClient.Top()+=rc.Height()/8;
  rcClient.Bottom()-=rc.Height()/8;

  double ymin=0x7FFFFFFFL,ymax=0,ystep;
  double xmin=0;
  double xmax=0;


  List elemList;

  {
    double curdatetime=0;
    _N *pN=0;
    for(RunInfo *p=(RunInfo *)list.first(); p; p=(RunInfo *)p->nextNode() ) {
      if(!pN || (curdatetime!=p->datetime)) {
	pN=new _N;
	pN->datetime=p->datetime;
	memset(&pN->det, 0, sizeof(pN->det));
	elemList.append(pN);
	curdatetime=p->datetime;
      }
      for(int n=0; n<NUMDETECTORS; n++) {
	Detector *pDetector=&pMethod->det[n];
	if(pDetector->Label[0]!='?') // old file has no valid names
	  if(pDetector->Label!=p->det_name) continue;
	int bFound=0;
	for(int i=0; i<COMPONENT_ROWS; i++) {
	  Component *pComponent=&pDetector->components[i];
	  if(pComponent->name==p->peak_name) {
	    pN->det[n].components[i].p=p;
	    pN->det[n].components[i].pComponent=pComponent;
	    bFound=1;
	    break;
	  }
	}
	if(bFound) break;
      }
    }
  }

  for(int _num=0; _num<NUMDETECTORS; _num++)
  for(int i=0; i<COMPONENT_ROWS; i++)
  for(_N *node=(_N *)elemList.first(); node; node=(_N *)node->nextNode()) {
    RunInfo *p=node->det[_num].components[i].p;
    double conc=0;
    if(p) conc=p->conc;

    if(conc<ymin) ymin=conc;
    if(conc>ymax) ymax=conc;

    if(!xmin) xmin=node->datetime;
    else if(xmin>node->datetime) xmin=node->datetime;
    if(!xmax) xmax=node->datetime;
    else if(xmax<node->datetime) xmax=node->datetime;
  }

  double xstep;

  int bPrintHours=0;
  if(pRunPictInfo->ai[0].bAuto) {
    double dx=xmax-xmin;
    if(!dx) dx=1;
    xmin+=dx/2;
    dx*=0.9999;
    xmin-=dx/2;
    xmax=xmin+dx;
    unsigned long coeff=1;
    if(dx<60UL*60) { // < 1 hour
      coeff=60UL; // in minutes
    }
    else if(dx<60UL*60*24) { // < 1 day
      coeff=60UL*60; // in hours
    }
    else {
      coeff=60UL*60*24;// in days
    }
    xmin/=coeff;
    xmax/=coeff;
    oz(&xmin,&xmax,4,7,0,&kx,&xstep);
    if(xstep<1) {
      xmin*=coeff;
      xmax*=coeff;
      if(coeff==60UL*60*24) { // in days
	coeff=60UL*60; // in hours
      }
      else {
	coeff/=60;
      }
      xmin/=coeff;
      xmax/=coeff;
      oz(&xmin,&xmax,4,7,0,&kx,&xstep);
    }
    xmin*=coeff;
    xmax*=coeff;
    xstep*=coeff;

    pRunPictInfo->ai[0].mi=xmin;
    pRunPictInfo->ai[0].ma=xmax;
    pRunPictInfo->ai[0].k=kx;
  }
  else {
    xmin=pRunPictInfo->ai[0].mi;
    xmax=pRunPictInfo->ai[0].ma;
    kx=pRunPictInfo->ai[0].k;
    if(kx<1) kx=1;
    xstep = (xmax-xmin)/kx;
    if(xstep<=0) xstep=1;
  }
  bPrintHours=xstep<60UL*60*24;
  {
    unsigned long coeff=1;
    if(xstep<60UL*60) { // < 1 hour
      coeff=60UL; // in minutes
    }
    else if(xstep<60UL*60*24) { // < 1 day
      coeff=60UL*60; // in hours
    }
    else {
      coeff=60UL*60*24;// in days
    }
    if(xstep/coeff<1) {
      if(coeff==60UL*60*24) { // in days
	coeff=60UL*60; // in hours
      }
      else {
	coeff/=60;
      }
    }
    int i=log10(xstep/coeff);
    if(xstep/coeff>1.) i++;
    if((int)(xstep/coeff/pow10(i-1)+.5)==2) nnx=4;
    else {
      if((coeff==60UL*60*24)) {// in days
	if(xstep/coeff<8) { //??
	  nnx=xstep/coeff;
	}
	else {
	  nnx=5;
	}
      }
      else
	nnx=6;
    }

  }
  if(pRunPictInfo->ai[1].bAuto) {
    if(ymax<=ymin) ymax=ymin+1;
    oz(&ymin,&ymax,5,10,0,&ky,&ystep);
    pRunPictInfo->ai[1].mi=ymin;
    pRunPictInfo->ai[1].ma=ymax;
    pRunPictInfo->ai[1].k=ky;
  }
  else {
    ymin=pRunPictInfo->ai[1].mi;
    ymax=pRunPictInfo->ai[1].ma;
    ky=pRunPictInfo->ai[1].k;
    if(ky<1) ky=1;
    ystep = (ymax-ymin)/ky;
    if(ystep<=0) ystep=1;
  }
  {
    int i=log10(ystep);
    if(ystep>1.) i++;
    if((int)(ystep/pow10(i-1)+.5)==2) nny=4;
    else nny=5;
  }

  if(hwnd) {
    int bChanged=
      mi0Old!=pRunPictInfo->ai[0].mi ||
      ma0Old!=pRunPictInfo->ai[0].ma ||
      mi1Old!=pRunPictInfo->ai[1].mi ||
      ma1Old!=pRunPictInfo->ai[1].ma;
    if(bChanged) {
      InvalidateRect(*hwnd,0,TRUE); // ??
      return; //???
    }
  }

  double dxx=xmax-xmin; if(!dxx) dxx=1;
  double dyy=ymax-ymin; if(!dyy) dyy=1;

  int logpixelsy=dc.IsMetafile()? 2540: ic.GetCapability(LOGPIXELSY);

  SelBkMode OldBkMode(dc,bkmTransparent);
  int penwidth=dc.IsRasterDisplay()? 0: MulDiv(50,logpixelsy,2540);	// 0.5 mm = 50 * 0.01 mm
  LogPen pen(psSolid, penwidth, color);
  SelPen oldPen(dc,pen);

  LOGFONT lf=pRunPictInfo->lfUser;

  if(ic.IsRasterDisplay()) lf.lfHeight=MulDiv(16,logpixelsy,72); // 16 pts (1pt == 1"/72)
  else lf.lfHeight=MulDiv(pRunPictInfo->lfUser.lfHeight,logpixelsy,72); // user pts (1pt == 1"/72)
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

    SelTextColor selColor(dc, pRunPictInfo->TextColor);
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

      if(bPrintHours) {
	dc.TextOut(x,y,PrintTime(xmin+xstep*i, 1));
	if(i==0) {
	  dc.TextOut(x,y+lineheight,PrintDate(xmin+xstep*i, 1, 1));
	}
	else if(i==kx) {
	  dc.TextOut(x,y+lineheight,PrintDate(xmin+xstep*i, 1, 1));
	}
      }
      else {
	dc.TextOut(x,y,PrintDate(xmin+xstep*i, 0, 1));
      }
    }
    {
      SelFont selFont(dc,*pFontItal);
      SelTextAlign oldTexAlign(dc,taLeft|taBaseline);
      x=rcClient.Right()+linewidth/2;
      dc.TextOut(x,y-linewidth/3,bPrintHours? xnameT: xnameD);
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

    int h=rcClient.Height();
    int w=rcClient.Width();
    double yminl=ymin;
    double dyyl=dyy;

    dc.Save(); // to restore ClipRegion later
    dc.IntersectClipRect(rcClient);
    {
      for(int _num=0; _num<NUMDETECTORS; _num++)
      for(int i=0; i<COMPONENT_ROWS; i++) {
	static const Color a[12]={Blue, Green, Cyan, Red, Magenta, Brown,
	     LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow};
	Color color=a[i%12];//pPictInfo->Color[_num];//Black;
	for(_N *node=(_N *)elemList.first(); node; node=(_N *)node->nextNode()) {
	  Component *pComponent=node->det[_num].components[i].pComponent;
	  RunInfo *p=node->det[_num].components[i].p;
	  double conc=0;
	  if(p) conc=p->conc;

	  if(pComponent) { // draw Alarms
	    #if POINTS()<2
	    int pointnumer=0;
	    {
	    #else
	    for(int pointnumer=0; pointnumer<POINTS(); pointnumer++) {
	      if((pMethod->method_table.points & (1<<pointnumer))==0) continue;
	    #endif
	    {
	      LogPen pen(psDot, penwidth/2, color);
	      SelPen oldPen(dc,pen);
	      int x=rcClient.Left();
	      int y=rcClient.Bottom()-1-(int)(double(pComponent->low_alarm[pointnumer]-yminl)*h/dyyl);
	      dc.MoveTo(x,y);
	      x=rcClient.Right();
	      dc.LineTo(x,y);
	    }
	    {
	      LogPen pen(psDash, penwidth/2, color);
	      SelPen oldPen(dc,pen);
	      int x=rcClient.Left();
	      int y=rcClient.Bottom()-1-(int)(double(pComponent->high_alarm[pointnumer]-yminl)*h/dyyl);
	      dc.MoveTo(x,y);
	      x=rcClient.Right();
	      dc.LineTo(x,y);
	    }
	    }
	  }
	}


	LogPen pen(psSolid, penwidth, color);
	SelPen oldPen(dc,pen);

	long lmax=0;
	{
	  for(_N *node=(_N *)elemList.first(); node; node=(_N *)node->nextNode()) {
	    RunInfo *p=node->det[_num].components[i].p;
	    double conc=0;
	    if(p) conc=p->conc;

	    lmax++;
	  }
	}

	long ll=0;

	const int buffSize=1024;
	int cPoints=lmax>buffSize? buffSize: (int)lmax;
	POINT FAR * const lpptArray=AllocPoints(cPoints);
	if(lpptArray) {
	  {
	    POINT FAR* lppt=lpptArray;
	    int c=0;
	    for(_N *node=(_N *)elemList.first(); node; node=(_N *)node->nextNode()) {
	      RunInfo *p=node->det[_num].components[i].p;
	      double conc=0;
	      if(p) conc=p->conc;

	      {
		double y=conc;
		double x=node->datetime;
		lppt->x=rcClient.Left()+(int)(double(x-xmin)*w/dxx);
		lppt->y=rcClient.Bottom()-1-(int)(double(y-yminl)*h/dyyl);
		lppt++;
		c++;
		if(c==cPoints) {
		  dc.Polyline(lpptArray, cPoints);
		  cPoints=(lmax-ll)>buffSize? buffSize: (int)(lmax-ll);

		  lppt=lpptArray;
		  c=0;
		  /*
		  *lpptArray=lppt[-1]; // copy last point to the first for next line
		  lppt=lpptArray+1;
		  c=1;
		  cPoints+=c; // add the first point (duplicated from previous line)
		  */
		}
	      }
	      ll++;
	    }
	  }
	}
	DeletePoints(lpptArray);
      }
    }
    dc.Restore();
  }
  elemList.shutDown();
  delete pFont;
}



RunInfo::RunInfo()
 : conc(-1)
 , height(0)
 , area(0)
 , datetime(0)
{
}
