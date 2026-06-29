#include <owl.h>
#pragma hdrstop

#include <commdlg.h>
#include "id.h"
#include "helpids.h"
#include "comm.h"
#include "wcpplib\str.h"
#include "winutil\dlg_item.h"
#include "winutil\app.h"
#include "wcpplib\3dui.h"
#include "wcpplib\fwstream.h"
#include "winlib\font.h"

#ifndef OFN_LONGNAMES
#define OFN_LONGNAMES                0x00200000
#endif

#ifndef WM_SETICON
#define WM_SETICON                      0x0080
#endif


#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_STATICEDGE        0x00020000L

class TMyDialog : public TDialog
{
  Font *font;
  Comm *pComm;
  Str FileName;
public:
  TMyDialog(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL);

  ~TMyDialog();

  void ReadComm();

  virtual void SetupWindow();
  virtual BOOL CanClose();

  virtual void WMDestroy(RTMessage Msg) = [WM_FIRST + WM_DESTROY];
  virtual void WMTimer(RTMessage Msg) = [WM_FIRST + WM_TIMER];
  virtual void WMCommNotify(RTMessage Msg) = [WM_FIRST + WM_COMMNOTIFY];

  virtual void WMInitMenuPopup(RTMessage Msg) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void WMNcHitTest(RTMessage Msg) = [WM_FIRST + WM_NCHITTEST];

  virtual void HandleHelp(RTMessage) = [ID_FIRST+IDHELP];
  virtual void HandleBrowse(RTMessage) = [ID_FIRST+IDBROWSE];

  ApplicationProfile pref;

  owstream *OpenFile();

  void CloseComm();
  void SetStatus(const char *status);
  Str status;
};


TMyDialog::TMyDialog(PTWindowsObject AParent, int ResourceId, PTModule AModule)
: TDialog(AParent, ResourceId, AModule)
, pComm(0)
, font(0)
{
}
TMyDialog::~TMyDialog()
{
  CloseComm();
  delete font;
}

void TMyDialog::SetStatus(const char *_status)
{
  if(status!=_status) {
    if(HWindow) {
      status=_status;
      SetDlgItemText(HWindow, IDD_COM_STATUS,_status);
    }
  }
}
void TMyDialog::CloseComm()
{
  delete pComm; pComm=0;
  SetDlgItemText(HWindow, IDOK, "&Start");
}

void TMyDialog::SetupWindow()
{
  TDialog::SetupWindow();
  SendDlgItemMsg(IDD_COMPORT, CB_ADDSTRING, 0, (LPARAM)"COM1:");
  SendDlgItemMsg(IDD_COMPORT, CB_ADDSTRING, 0, (LPARAM)"COM2:");
  SendDlgItemMsg(IDD_COMPORT, CB_ADDSTRING, 0, (LPARAM)"COM3:");
  SendDlgItemMsg(IDD_COMPORT, CB_ADDSTRING, 0, (LPARAM)"COM4:");


  int iCom=pref.GetInt("Communucations", "iCom", 0);
  SendDlgItemMsg(IDD_COMPORT, CB_SETCURSEL, iCom, 0);

  CloseComm();
  SetTimer(HWindow, 99, 500, NULL);

  if(__Has3dUI()) {
    SendMessage(HWindow,WM_SETICON,1,(LPARAM)LoadIcon(GetModule()->hInstance,MAKEINTRESOURCE(ID_WPEAK_ICON)));
  }

  //SetWindowLong(HWindow,GWL_EXSTYLE,GetWindowLong(HWindow,GWL_EXSTYLE)|WS_EX_DLGMODALFRAME);
  //SetWindowLong(HWindow,GWL_EXSTYLE,GetWindowLong(HWindow,GWL_EXSTYLE)|WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE);

  FileName=pref.GetString("Options", "FileName", FileName);
  SetDlgItemText(HWindow,IDD_STARTUPFILE,FileName);


  {
  Font font1((HFONT)SendDlgItemMessage(HWindow,IDD_PEAKLIST,WM_GETFONT,0,0L));
  LOGFONT lf;
  font1.Get(&lf);
  lf.lfWidth=0;
  lf.lfWeight=FW_DONTCARE;
  lf.lfQuality=DEFAULT_QUALITY;
  lf.lfOutPrecision=OUT_TT_PRECIS;
  lf.lfPitchAndFamily=FIXED_PITCH;
  lf.lfFaceName[0]='\0';

  font = new LogFont(&lf);
  }
  HFONT hfont=*font;
  SendDlgItemMessage(HWindow,IDD_PEAKLIST,WM_SETFONT,(WPARAM)hfont,0L);
/*
  SetDlgItemText(HWindow,IDD_PEAKLIST,
"\"Detector A\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:15\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector A\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:21\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector B\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:21\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector A\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:23\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector A\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:24\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector B\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:24\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector A\", \"no peaks\", -1.000, 0, 0, \"0:00\", \"none\", \"09.06.01\", \"2:26\", \"D:\\HNU\\WPEAK\2.run\", \"\"\r\n"
"\"Detector B\", \"unknown\", -1.000, 340, 91328, \"0:15\", \"none\", \"09.06.01\", \"2:26\", \"D:\\HNU\\WPEAK\\2.run\", \"\"\r\n"
"\"Detector B\", \"unknown\", -1.000, 343, 91282, \"0:39\", \"none\", \"09.06.01\", \"2:26\", \"D:\\HNU\\WPEAK\\2.run\", \"\"\r\n"
);
*/
}

void TMyDialog::WMDestroy(RTMessage Msg)
{
  CloseComm();
  KillTimer(HWindow, 99);
  TDialog::WMDestroy(Msg);
}


BOOL TMyDialog::CanClose()
{
  if(pComm) {
    CloseComm();
  }
  else {
    FileName=GetDlgItemText(HWindow,IDD_STARTUPFILE);
    if(FileName.IsEmpty()) {
      MessageBox(HWindow, "No file name is given", "wPeakC", MB_OK|MB_ICONSTOP);
    }
    else {
      {
	wstream *pos=OpenFile();
	if(!pos) return FALSE;
	delete pos;
      }
      pref.WriteString("Options", "FileName", FileName);

      int iCom=SendDlgItemMsg(IDD_COMPORT, CB_GETCURSEL, 0, 0L);
      pref.WriteInt("Communucations", "iCom", iCom);

      SetDlgItemText(HWindow, IDOK, "&Stop");
      char str[40];
      wsprintf(str,"COM%d",iCom+1);
      //TRACE1("new Comm=%s",str);
      pComm=new Comm(str, 1024, 1024, WPEAKC_ID, WPEAK_ID);
      if(*pComm==-1) { // Open OK
        CloseComm();
      }
      else {
        wsprintf(str,"COM%d:9600,n,8,1",iCom+1);
        pComm->SetParam(str);
        pComm->EnableNotification(HWindow, 1, -1);

      }
    }
  }

  return FALSE;
}


void TMyDialog::WMTimer(RTMessage /*Msg*/)
{
  ReadComm();
}

void TMyDialog::WMCommNotify(RTMessage /*Msg*/)
{
  ReadComm();
}

#include <time.h>
owstream *TMyDialog::OpenFile()
{
  {
    struct find_t ffblk;
    unsigned err=_dos_findfirst(FileName,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH, &ffblk);
    if(err==0) { // file exists
      int Year= ((ffblk.wr_date >> 9) & 127) + 1980;
      int Month= (ffblk.wr_date >> 5) & 15;
      int Day= ffblk.wr_date & 31;

      time_t cur_time=time(NULL);
      struct tm *t=localtime(&cur_time);
      if(Day!=t->tm_mday || Month!=1+t->tm_mon || Year!=1900+t->tm_year) {
	Str new_name=FileName;
	const char *cp=strrchr(new_name,'\\');
	if(cp) {
	  Str ext=strrchr(cp+1,'.');
	  *((char *)(cp+1))=0; // cut name
	  // DDMMYY -  for better file name sorting
	  char buff[15];
	  // The file name in format mm/dd/yy or dd/mm/yy - dependd on country settings
	  int dateformat=0; // 0 - MMDDYY(USA); 1 - DDMMYY(Europe); 2 - YYMMDD(Japan)
	  {
	    char fmt[20];
	    GetProfileString("Intl","sShortDate","m/d/yy",fmt,sizeof(fmt));

	    if(fmt[0]=='d'||fmt[0]=='D') dateformat=1;
	    else if(fmt[0]=='y'||fmt[0]=='Y') dateformat=2;
	  }

	  switch(dateformat) {
	    case 1: // DDMMYY
	      wsprintf(buff,"%02d%02d%02d",t->tm_mday,1+t->tm_mon,t->tm_year%100);
	    break;
	    case 2: // YYMMDD
	      wsprintf(buff,"%02d%02d%02d",t->tm_year%100,1+t->tm_mon,t->tm_mday);
	    break;
	    default: // MMDDYY
	      wsprintf(buff,"%02d%02d%02d",1+t->tm_mon,t->tm_mday,t->tm_year%100);
	  }
	  new_name+=buff; // add new
	  new_name+=ext; // ext
	  FileName=new_name;
	}
      }
    }
  }
  char buff[300];
  owstream *pos = new ofwstream(FileName,FALSE); // open existing
  if(!pos->good()) { delete pos; pos=new ofwstream(FileName,TRUE); } // open new
  if(!pos->good()) {
    wsprintf(buff,"Error opening file:\n%s", FileName.Get());
    MessageBox(HWindow,buff,"wPeak Communication Agent", MB_OK|MB_TASKMODAL);
    delete pos;
    pos=0;
  }
  else {
    pos->seek(0,wstream::end); // go to EOF
    if(!pos->good()) {
      wsprintf(buff,"Error going to the end of file:\n%s", FileName.Get());
      MessageBox(HWindow,buff,"wPeak Communication Agent", MB_OK|MB_TASKMODAL);
      delete pos;
      pos=0;
    }
  }
  return pos;
}

//#define MYDEBUG() 1 //  Debug
#define MYDEBUG() 0 // No Debug

void TMyDialog::ReadComm()
{
  if(pComm || MYDEBUG()) {
    uint8 mess_buf[256+2];
    uint8 nRead;
    int bAck=0;
    #if MYDEBUG()
    long err=0; bAck=0; strcpy(mess_buf,"qqqqq"); nRead=5; // DEBUG
    #else
    long err=pComm->recv_msg(mess_buf,&nRead,&bAck);
    #endif
    if(bAck) {
      SetStatus("Active");
    }
    else if(err) {
      SetStatus("Error");
    }
    else if(nRead) {  // if message received OK,...
      SetStatus("Recieving");
      owstream *pos=OpenFile();
      if(pos) {
	pos->write(mess_buf,nRead);
	//if(!pos->good()) message("Error writting file:\n%s", FileName.Get());
      }
      delete pos;
      {
	mess_buf[nRead]='\0';
	if(SendDlgItemMessage(HWindow, IDD_PEAKLIST,WM_GETTEXTLENGTH,0,0L)<32000) { // Check edit-control buffer overflow
	  SendDlgItemMessage(HWindow, IDD_PEAKLIST, EM_SETSEL,0,MAKELPARAM(-1,-1));
	  SendDlgItemMessage(HWindow, IDD_PEAKLIST, EM_REPLACESEL,0,(LPARAM)(LPCSTR)mess_buf);
	}
	else {
	  SetDlgItemText(HWindow,IDD_PEAKLIST,mess_buf);
        }
      }

    }
    else {
      SetStatus("Waiting");
    }
  }
  else {
    SetStatus("Stopped");
  }
}


static BOOL OpenSaveDialog(HWND hwnd, const char *title, const char *filter, Str *FileName, BOOL fSave)
{
  HINSTANCE hInstance=GetApplicationObject()->hInstance;
  BOOL retValue;
  /*static */char szFile[256];
  static char _far szFileTitle[256];
  {
    strcpy(szFile,*FileName);

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(OPENFILENAME));	// Set all structure members to zero.
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hInstance = hInstance;
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = NULL; //szDirName;
    ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    //ofn.Flags |= fSave? OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN: OFN_FILEMUSTEXIST;
    ofn.Flags |= OFN_NOREADONLYRETURN;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = 0;

    if(__Has3dUI()) {
      ofn.Flags |= OFN_LONGNAMES;
    }




    /*
    if(fSave) {
      ofn.Flags |= OFN_ENABLETEMPLATE;
      ofn.lpTemplateName=MAKEINTRESOURCE(ID_OPENDLG);
    }
    */

    //commdlg_id=fSave? ID_SAVEDIALOG : ID_OPENDIALOG;
    //retValue=fSave? GetSaveFileName(&ofn): GetOpenFileName(&ofn);
    //commdlg_id=0;
    retValue=GetSaveFileName(&ofn);
    if(retValue) {
      if(ofn.nFileExtension && !szFile[ofn.nFileExtension]) {
	const char *cp=filter;
	cp+=lstrlen(cp)+1;
	for(int i=0; i<(ofn.nFilterIndex-1); i++) {
	  cp+=lstrlen(cp)+1;
	  cp+=lstrlen(cp)+1;
	}
	cp=strchr(cp,'.');
	if(cp) {
	  lstrcat(szFile,cp);
	}
      }
    }
  }
  if(!retValue) {
    DWORD dwError=CommDlgExtendedError();
    if(dwError) {
      //message("CommDlg error %lu",dwError);
    }
    return FALSE;
  }

  if(FileName) *FileName=szFile;

  return TRUE;
}

static BOOL GetFileName(HWND hwnd, Str *FileName)
{
  static const char _far strFilter[]="All files(*.*)\0*.*\0";
  static const char _far strTitle[]="Select File Path";

  static Str staticFileName;
  //if(!fSave) staticFileName="";
  //else { const char *cp=strrchr(staticFileName,'\\'); if(cp) staticFileName.Set(cp+1); }
  BOOL retValue=OpenSaveDialog(hwnd, strTitle, strFilter, &staticFileName, TRUE/*fSave*/);
  if(retValue) {
    *FileName=staticFileName;
  }
  return retValue;
}

void TMyDialog::HandleBrowse(RTMessage)
{
  Str FileName;
  if(GetFileName(HWindow, &FileName)) {
    SetDlgItemText(HWindow,IDD_STARTUPFILE,FileName);
  }
}


void TMyDialog::WMInitMenuPopup(RTMessage msg)
{
  HMENU hmenuPopup = (HMENU)msg.WParam;         /* handle of pop-up menu */
  BOOL fSystemMenu = (BOOL)msg.LP.Hi; /* System-menu flag      */
  if(fSystemMenu) {
    EnableMenuItem(hmenuPopup, SC_SIZE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
  }
}
void TMyDialog::WMNcHitTest(RTMessage Msg)
{
  static int b=0; // In SendMessage
  if(!b) {
    b=1;
    Msg.Result=DefWindowProc/*SendMessage*/(Msg.Receiver, Msg.Message, Msg.WParam, Msg.LParam);
    switch(Msg.Result) {
      case HTBOTTOM:
      case HTBOTTOMLEFT:
      case HTBOTTOMRIGHT:
      case HTLEFT:
      case HTRIGHT:
      case HTTOP:
      case HTTOPLEFT:
      case HTTOPRIGHT:
        Msg.Result=HTBORDER; // Inhibit dialog sizing
      break;
      default:
        Msg.Result=0; // go to default processing
    }
    b=0;
  }
}

#include "about.h"
void TMyDialog::HandleHelp(RTMessage)
{
  GetModule()->ExecDialog( newAboutDialog(this,ID_ABOUTDIALOG) );
}

//////////////////////////



class TMyApp : public TApplication
{
public:
  TMyApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
      LPSTR lpCmdLine, int nCmdShow)
  : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
  virtual void InitMainWindow();
  virtual void IdleAction();
};

void TMyApp::InitMainWindow()
{
  MainWindow = new TMyDialog(NULL, ID_PREFERENCESDIALOG);
}

void TMyApp::IdleAction()
{
  if(MainWindow) SendMessage(MainWindow->HWindow,WM_USER,0,0L);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  TMyApp MyApp("wPeakC", hInstance, hPrevInstance,
		   lpCmdLine, nCmdShow);
  MyApp.Run();
  return MyApp.Status;
}