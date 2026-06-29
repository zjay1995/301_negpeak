
#include "CBW.h"

#ifndef RC_INVOKED

#ifndef __WIN32__

#include "wow.h"
class CBWDLL : public WOWDLL {
 public:
  CBWDLL(const char *path);
  ~CBWDLL();


  BOOL OK() { return bOK; }
  BOOL bOK;


  DWORD cbAIn;
  DWORD cbAInScan;
  DWORD cbAOut;

  DWORD cbDBitIn;
  DWORD cbDBitOut;
  DWORD cbDConfigPort;
  DWORD cbDConfigBit;

  DWORD cbDIn;
  DWORD cbDOut;

  DWORD cbInByte;
  DWORD cbOutByte;
  DWORD cbInWord;
  DWORD cbOutWord;

  DWORD cbWinBufToArray;
  DWORD cbWinArrayToBuf;
  DWORD cbWinBufAlloc;
  DWORD cbWinBufFree;

  DWORD cbGetConfig;
  DWORD cbSetConfig;
  DWORD cbToEngUnits;
  DWORD cbFromEngUnits;
  DWORD cbGetBoardName;
  DWORD cbGetConfigString; // added. P.P. 03/18/17; 
//  DWORD bSetConfigString;    // added. P.P. 03/18/17; not needed at this time. P.P. 03/18/17

};

int My_cbAIn (int BoardNum, int Chan, int Gain, USHORT *DataValue);
int My_cbAInScan (int BoardNum, int LowChan, int HighChan, long Count,
                            long *Rate, int Gain, HGLOBAL MemHandle,
                                                        int Options);
int My_cbAOut (int BoardNum, int Chan, int Gain, USHORT DataValue);


int My_cbDBitIn (int BoardNum, int PortType, int BitNum, USHORT *BitValue);
int My_cbDBitOut (int BoardNum, int PortType, int BitNum, USHORT BitValue);
int My_cbDConfigPort (int BoardNum, int PortNum, int Direction);
int My_cbDConfigBit (int BoardNum, int PortNum, int BitNum, int Direction);

int My_cbDIn (int BoardNum, int PortNum, USHORT *DataValue);
int My_cbDOut(int BoardNum, int PortNum, USHORT DataValue);

int My_cbInByte (int BoardNum, int PortNum);
int My_cbOutByte (int BoardNum, int PortNum, int PortVal);
int My_cbInWord (int BoardNum, int PortNum);
int My_cbOutWord (int BoardNum, int PortNum, int PortVal);

int My_cbWinBufToArray (HGLOBAL MemHandle, USHORT *DataArray,
                                          long StartPt, long Count);
int My_cbWinArrayToBuf (USHORT *DataArray, HGLOBAL MemHandle,
                                          long StartPt, long Count);
HGLOBAL My_cbWinBufAlloc (long NumPoints);
int My_cbWinBufFree (HGLOBAL MemHandle);


//int My_cbGetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, int *ConfigVal);
int My_cbGetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem,  DWORD *ConfigVal);
int My_cbSetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, int ConfigVal);
int My_cbToEngUnits (int BoardNum, int Range, USHORT DataVal, float *EngUnits);
int My_cbFromEngUnits (int BoardNum, int Range, float EngUnits, USHORT *DataVal);
int My_cbGetBoardName (int BoardNum, char *BoardName);
int My_cbGetConfigString (int InfoType, int BoardNum, int DevNum,
                  int ConfigItem, char* ConfigVal, int* maxConfigLen); // added. P.P. 03/18/17
//int My_cbSetConfigString (int InfoType, int BoardNum, int DevNum,
//                  int ConfigItem, char* ConfigVal, int* configLen);   // added. P.P. 03/18/17 not needed at this time


#else // __WIN32__


#define My_cbAIn         cbAIn
#define My_cbAInScan     cbAInScan
#define My_cbAOut        cbAOut
#define My_cbDBitIn      cbDBitIn
#define My_cbDBitOut     cbDBitOut
#define My_cbDConfigPort cbDConfigPort
#define My_cbDConfigBit  cbDConfigBit

#define My_cbDIn         cbDIn
#define My_cbDOut        cbDOut

#define My_cbInByte      cbInByte
#define My_cbOutByte     cbOutByte
#define My_cbInWord      cbInWord
#define My_cbOutWord     cbOutWord

#define My_cbWinBufToArray cbWinBufToArray
#define My_cbWinArrayToBuf cbWinArrayToBuf

#define My_cbWinBufAlloc cbWinBufAlloc
#define My_cbWinBufFree  cbWinBufFree


#define My_cbGetConfig   cbGetConfig
#define My_cbSetConfig   cbSetConfig
#define My_cbToEngUnits  cbToEngUnits
#define My_cbFromEngUnits cbFromEngUnits
#define My_cbGetBoardName cbGetBoardName
#define My_cbGetConfigString cbGetConfigString    //P.P. 03/18/17
//#define My_cbSetConfigString cbSetConfigString  // P.P. 03/18/17 but rem, not needed

#pragma comment(lib, "cbw32.lib")

#endif // __WIN32__



void GetCBWErrorString(unsigned long error, char *errorstring);


#endif // RC_INVOKED

#define CBW_ERR 12000
