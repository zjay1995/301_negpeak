//#include "includes.h"
#pragma hdrstop

#include "internal.h"

#if CBW()

#include "MyCBW.h"

#ifndef __WIN32__
#include "wow.h"

CBWDLL::CBWDLL(const char *path)
 : WOWDLL(path)
{
  bOK = WOWDLL::Ok()

    && (cbAIn=GetProcAddress32W("cbAIn"))!=NULL
    && (cbAInScan=GetProcAddress32W("cbAInScan"))!=NULL
    && (cbAOut=GetProcAddress32W("cbAOut"))!=NULL
    && (cbDBitIn=GetProcAddress32W("cbDBitIn"))!=NULL
    && (cbDBitOut=GetProcAddress32W("cbDBitOut"))!=NULL
    && (cbDConfigPort=GetProcAddress32W("cbDConfigPort"))!=NULL
    && (cbDConfigBit=GetProcAddress32W("cbDConfigBit"))!=NULL
    && (cbDIn=GetProcAddress32W("cbDIn"))!=NULL
    && (cbDOut=GetProcAddress32W("cbDOut"))!=NULL
    && (cbInByte=GetProcAddress32W("cbInByte"))!=NULL
    && (cbOutByte=GetProcAddress32W("cbOutByte"))!=NULL
    && (cbInWord=GetProcAddress32W("cbInWord"))!=NULL
    && (cbOutWord=GetProcAddress32W("cbOutWord"))!=NULL
    && (cbWinBufToArray=GetProcAddress32W("cbWinBufToArray"))!=NULL
    && (cbWinArrayToBuf=GetProcAddress32W("cbWinArrayToBuf"))!=NULL
    && (cbWinBufAlloc=GetProcAddress32W("cbWinBufAlloc"))!=NULL
    && (cbWinBufFree=GetProcAddress32W("cbWinBufFree"))!=NULL
    && (cbGetConfig=GetProcAddress32W("cbGetConfig"))!=NULL           // 4 lines added. P).P. 03/13/17
    && (cbSetConfig=GetProcAddress32W("cbSetConfig"))!=NULL           //
    && (cbToEngUnits=GetProcAddress32W("cbToEngUnits"))!=NULL         //
//    && (cbFromEngUnits=GetProcAddress32W("FromEngUnits"))!=NULL       // something is wrong with this function. Do they still have it? P.P. 03/15/17
    && (cbGetBoardName=GetProcAddress32W("cbGetBoardName"))!=NULL     //
    && (cbGetConfigString=GetProcAddress32W("cbGetConfigString"))!=NULL  // added. P.P. 03/18/17
    ;
}

CBWDLL::~CBWDLL()
{
}

CBWDLL CBW_Dll("CBW32.dll");

///////////////////////////////

class Cbw
{
 public:
  Cbw();
  ~Cbw();
} cbw;

Cbw::Cbw()
{
  if(!CBW_Dll.Ok()) {
    #if !DEMO()
    MessageBox(NULL,"CBW32.dll not found", "wPeak", MB_OK);
    #endif
  }
  else {
  }
}
Cbw::~Cbw()
{
}

//////////////////////////////

int My_cbAIn (int BoardNum, int Chan, int Gain, USHORT *DataValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x08, CBW_Dll.cbAIn, (DWORD)BoardNum, (DWORD)Chan, (DWORD)Gain, (DWORD)DataValue);
  }
  return ret;
}

int My_cbAInScan (int BoardNum, int LowChan, int HighChan, long Count,
                            long *Rate, int Gain, HGLOBAL MemHandle, int Options)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 8|CPEX_DEST_STDCALL, 0x10, CBW_Dll.cbAInScan, (DWORD)BoardNum, (DWORD)LowChan, (DWORD)HighChan, (DWORD)Count, (DWORD)Rate, (DWORD)Gain, (DWORD)MemHandle, (DWORD)Options);
  }
  return ret;
}


int My_cbAOut (int BoardNum, int Chan, int Gain, USHORT DataValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbAOut, (DWORD)BoardNum, (DWORD)Chan, (DWORD)Gain, (DWORD)DataValue);
  }
  return ret;
}


int My_cbDBitIn (int BoardNum, int PortType, int BitNum, USHORT *BitValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x08, CBW_Dll.cbDBitIn, (DWORD)BoardNum, (DWORD)PortType, (DWORD)BitNum, (DWORD)BitValue);
  }
  return ret;
}

int My_cbDBitOut (int BoardNum, int PortType, int BitNum, USHORT BitValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbDBitOut, (DWORD)BoardNum, (DWORD)PortType, (DWORD)BitNum, (DWORD)BitValue);
  }
  return ret;
}

int My_cbDConfigPort (int BoardNum, int PortNum, int Direction)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbDConfigPort, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)Direction);
  }
  return ret;
}

int My_cbDConfigBit (int BoardNum, int PortNum, int BitNum, int Direction)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbDConfigBit, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)BitNum, (DWORD)Direction);
  }
  return ret;
}


int My_cbDIn (int BoardNum, int PortNum, USHORT *DataValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x04, CBW_Dll.cbDIn, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)DataValue);
  }
  return ret;
}

int My_cbDOut(int BoardNum, int PortNum, USHORT DataValue)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbDOut, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)DataValue);
  }
  return ret;
}

int My_cbInByte (int BoardNum, int PortNum)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbInByte, (DWORD)BoardNum, (DWORD)PortNum);
  }
  return ret;
}

int My_cbOutByte (int BoardNum, int PortNum, int PortVal)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbOutByte, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)PortVal);
  }
  return ret;
}

int My_cbInWord (int BoardNum, int PortNum)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbInWord, (DWORD)BoardNum, (DWORD)PortNum);
  }
  return ret;
}

int My_cbOutWord (int BoardNum, int PortNum, int PortVal)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbOutWord, (DWORD)BoardNum, (DWORD)PortNum, (DWORD)PortVal);
  }
  return ret;
}

int My_cbWinBufToArray (HGLOBAL MemHandle, USHORT *DataArray, long StartPt, long Count)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x02, CBW_Dll.cbWinBufToArray, (DWORD)MemHandle, (DWORD)DataArray, (DWORD)StartPt, (DWORD)Count);
  }
  return ret;
}

int My_cbWinArrayToBuf (USHORT *DataArray, HGLOBAL MemHandle, long StartPt, long Count)
{
  int ret=0;
  if(CBW_Dll.bOK) {
//    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x01, CBW_Dll.cbWinBufToArray, (DWORD)DataArray, (DWORD)MemHandle, (DWORD)StartPt, (DWORD)Count);
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x01, CBW_Dll.cbWinArrayToBuf, (DWORD)DataArray, (DWORD)MemHandle, (DWORD)StartPt, (DWORD)Count);  // P.P. 03/13/17. Check! 
  }
  return ret;
}

HGLOBAL My_cbWinBufAlloc (long NumPoints)
{
  HGLOBAL ret=0;
  if(CBW_Dll.bOK) {
    ret=(HGLOBAL)CBW_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbWinBufAlloc, (DWORD)NumPoints);
  }
  return ret;
}

int My_cbWinBufFree (HGLOBAL MemHandle)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbWinBufFree, (DWORD)MemHandle);
  }
  return ret;
}


//int My_cbGetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, int *ConfigVal)
int My_cbGetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, DWORD *ConfigVal)   // store 32-bit number. P.P. 03/16/17
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 5|CPEX_DEST_STDCALL, 0x10, CBW_Dll.cbGetConfig, (DWORD)InfoType, (DWORD)BoardNum, (DWORD)DevNum, (DWORD)ConfigItem, (DWORD)ConfigVal);
  }
  return ret;
}
int My_cbSetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, int ConfigVal)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 5|CPEX_DEST_STDCALL, 0x0, CBW_Dll.cbSetConfig , (DWORD)InfoType, (DWORD)BoardNum, (DWORD)DevNum, (DWORD)ConfigItem, (DWORD)ConfigVal);
  }
  return ret;
}
int My_cbToEngUnits (int BoardNum, int Range, USHORT DataVal, float *EngUnits)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x08, CBW_Dll.cbToEngUnits , (DWORD)BoardNum, (DWORD)Range, (DWORD)DataVal, (DWORD)EngUnits);
  }
  return ret;
}
int My_cbFromEngUnits (int BoardNum, int Range, float EngUnits, USHORT *DataVal)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x08, CBW_Dll.cbFromEngUnits, (DWORD)BoardNum, (DWORD)Range, (DWORD)EngUnits, (DWORD)DataVal);
  }
  return ret;
}
int My_cbGetBoardName (int BoardNum, char *BoardName)
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, CBW_Dll.cbGetBoardName, (DWORD)BoardNum, (DWORD)BoardName);
  }
  return ret;
}

int My_cbGetConfigString (int InfoType, int BoardNum, int DevNum, int ConfigItem, char* ConfigVal, int* maxConfigLen) // added. P.P. 03/18/17
{
  int ret=0;
  if(CBW_Dll.bOK) {
    ret=(int)CBW_Dll.CallProcEx32W( 6|CPEX_DEST_STDCALL, 0x20, CBW_Dll.cbGetConfigString, (DWORD)InfoType, (DWORD)BoardNum, (DWORD)DevNum, (DWORD)ConfigItem, (DWORD)ConfigVal, (DWORD) maxConfigLen);
  }
  return ret;

}


#else // __WIN32__

#endif __WIN32__

extern HINSTANCE _hInstance;
void GetCBWErrorString(unsigned long error, char *errorstring)
{
    char buff[100];
    const char *str=0;
    if(DOS_ERR_OFFSET<=error /*??? && error<=DOS_ERR_OFFSET+100*/) {
      // DOS errors are remapped by adding DOS_ERR_OFFSET to them
      wsprintf(buff,"DOS error=%d",error);
      // FormatError(...)
      str=buff;
    }
    switch(error) {
      case 0                  : str="No error occurred"; break;
      default:
	if(!LoadString(_hInstance,CBW_ERR+error,buff,sizeof(buff))) {
	  wsprintf(buff,"CBW error=%d",error);
	}
	str=buff;
      break;
    }
    lstrcpy(errorstring,str);

}


#endif // CBW()