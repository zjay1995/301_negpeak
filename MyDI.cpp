#pragma hdrstop

#include "internal.h"

#if DI()

#include "mydi.h"

#ifndef __WIN32__

#include "wow.h"

class DIDLL : public WOWDLL {
 public:
  DIDLL(const char *path);
  ~DIDLL();


  BOOL OK() { return bOK; }

  BOOL bOK;


  DWORD di_com;

  DWORD di_info32;


  DWORD di_open;
  DWORD di_set_port_mode;
  DWORD di_set_data_mode;
  DWORD di_get_data_mode;

  //DWORD di_reset;// not exists in DI700nt.DLL
  DWORD di_close;

  DWORD di_buffer_status;
  DWORD di_buffer_alloc;
  DWORD di_buffer_free;

  DWORD di_digin;
  DWORD di_anin;
  DWORD di_digout;
  DWORD di_anout;
  DWORD di_mode;

  DWORD di_start_scan;
  DWORD di_stop_scan;

  DWORD di_inlist;
  DWORD di_outlist;
  DWORD di_list_length;

  DWORD di_burst_rate;
  DWORD di_strerr;

  DWORD di_buffer_size;
  DWORD di_copy_array;
  DWORD di_copy_buffer;
};


DIDLL::DIDLL(const char *path)
 : WOWDLL(path)
{
  bOK = WOWDLL::Ok()

    && (di_info32=GetProcAddress32W("di_info32"))!=NULL
    && (di_open=GetProcAddress32W("di_open"))!=NULL
    && (di_set_data_mode=GetProcAddress32W("di_set_data_mode"))!=NULL
    //&& (di_reset=GetProcAddress32W("di_reset"))!=NULL
    && (di_close=GetProcAddress32W("di_close"))!=NULL
    && (di_buffer_status=GetProcAddress32W("di_buffer_status"))!=NULL
    && (di_buffer_alloc=GetProcAddress32W("di_buffer_alloc"))!=NULL
    && (di_buffer_free=GetProcAddress32W("di_buffer_free"))!=NULL
    && (di_digin=GetProcAddress32W("di_digin"))!=NULL
    && (di_anin=GetProcAddress32W("di_anin"))!=NULL
    && (di_digout=GetProcAddress32W("di_digout"))!=NULL
    && (di_anout=GetProcAddress32W("di_anout"))!=NULL
    && (di_mode=GetProcAddress32W("di_mode"))!=NULL
    && (di_start_scan=GetProcAddress32W("di_start_scan"))!=NULL
    && (di_stop_scan=GetProcAddress32W("di_stop_scan"))!=NULL
    && (di_inlist=GetProcAddress32W("di_inlist"))!=NULL
    && (di_outlist=GetProcAddress32W("di_outlist"))!=NULL
    && (di_list_length=GetProcAddress32W("di_list_length"))!=NULL
    && (di_burst_rate=GetProcAddress32W("di_burst_rate"))!=NULL
    && (di_strerr=GetProcAddress32W("di_strerr"))!=NULL
    && (di_buffer_size=GetProcAddress32W("di_buffer_size"))!=NULL
    && (di_copy_array=GetProcAddress32W("di_copy_array"))!=NULL
    && (di_copy_buffer=GetProcAddress32W("di_copy_buffer"))!=NULL
    ;

  if(!bOK) {
    #if !DEMO()
    MessageBox(NULL,"DI700NT.DLL not found", "wPeak", MB_OK);
    #endif
  }
}

DIDLL::~DIDLL()
{
}

DIDLL didll("di700nt.dll");

///////////////////////////////

int my_di_com(unsigned di_command,unsigned di_data)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_com(%d,%d)\r\n", di_command,di_data);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0, didll.di_com, (DWORD)di_command, (DWORD)di_data);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}


int my_di_info32(struct di_info_struct32 *info)
{
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x1, didll.di_info32, (DWORD)info);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
    */
  }
  return ret;
}

int my_di_open(unsigned devno)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_open(%d)\r\n", devno);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_open, (DWORD)devno);
    /*
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
    */
  }
  return ret;
}

int my_di_set_port_mode(unsigned mode)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_set_port_mode(%d)\r\n", mode);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_set_port_mode, (DWORD)mode);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}

void my_di_set_data_mode(unsigned datamode)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_set_data_mode(%d)\r\n", datamode);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_set_data_mode, (DWORD)datamode);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  //return ret;
}

unsigned my_di_get_data_mode(void)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_get_data_mode()\r\n");
  unsigned int ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_get_data_mode);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}


/*
int my_di_reset(void)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_reset()\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_reset);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}
*/

int my_di_close(void)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_close()\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_close);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}



unsigned my_di_buffer_status(unsigned chn)
{
//  _pMainWindow->print("  my_di_buffer_status(%d)\r\n", chn);
  int ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_buffer_status, (DWORD)chn);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}

short * my_di_buffer_alloc(unsigned chn,unsigned size)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_buffer_alloc(%d,%d)\r\n", chn, size);
  short * ret=0;
  if(didll.bOK) {
    ret=(short *)didll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0, didll.di_buffer_alloc, (DWORD)chn, (DWORD)size);
    if(!ret) {
      //char sErrMsg[256];
      //my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error allocating buffer\r\n");
    }
  }
  return ret;
}


unsigned my_di_buffer_free(unsigned chn)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_buffer_free(%d)\r\n", chn);
  unsigned ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_buffer_free, (DWORD)chn);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}


unsigned my_di_digin(void)
{
//  _pMainWindow->print("  my_di_digin()\r\n");
  unsigned ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_digin);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}
int my_di_anin(struct di_anin_struct *analog_input)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_anin(...)\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, didll.di_anin, (DWORD)analog_input);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}
unsigned my_di_digout(unsigned i)
{
//  _pMainWindow->print("  my_di_digout(%d)\r\n",i);
  unsigned ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_digout, (DWORD)i);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}
int my_di_anout(unsigned dac_data,unsigned range)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_anout(%d,%d)\r\n", dac_data, range);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0, didll.di_anin, (DWORD)dac_data,(DWORD)range);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}
unsigned my_di_mode(struct di_mode_struct *mode)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_mode(...)\r\n");
  unsigned ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, didll.di_mode, (DWORD)mode);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

int my_di_start_scan(void)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_start_scan()\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_start_scan);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}
int my_di_stop_scan(void)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_stop_scan()\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_stop_scan);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}


int my_di_inlist(struct di_inlist_struct *input_list)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_inlist(...)\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 1, didll.di_inlist, (DWORD)input_list);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

int my_di_outlist(struct di_outlist_struct *output_list)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_outlist(...)\r\n");
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 1, didll.di_outlist, (DWORD)output_list);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

int my_di_list_length(unsigned in_length,unsigned out_length)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_list_length(%d, %d)\r\n", in_length, out_length);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0, didll.di_list_length, (DWORD)in_length, (DWORD)out_length);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}



int my_di_burst_rate(unsigned rate)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_burst_rate(%d)\r\n", rate);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0, didll.di_burst_rate, (DWORD)rate);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

int my_di_strerr(unsigned err_code,char *err_str)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_strerr(%d)\r\n", err_code);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 2, didll.di_strerr, (DWORD)err_code, (DWORD)err_str);
  }
  return ret;
}


unsigned my_di_buffer_size(void)
{
//  _pMainWindow->print("  my_di_buffer_size()\r\n");
  unsigned ret=0;
  if(didll.bOK) {
    ret=(unsigned)didll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0, didll.di_buffer_size);
    /*if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }*/
  }
  return ret;
}

int my_di_copy_array(unsigned buf_index,const short *source,unsigned word_count)
{
  //if(_pMainWindow) _pMainWindow->print("  my_di_copy_array(%d, count=%d)\r\n", buf_index, word_count);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 2, didll.di_copy_array, (DWORD)buf_index, (DWORD)source, (DWORD)word_count);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

int my_di_copy_buffer(unsigned buf_index,short *dest, unsigned word_count)
{
//  _pMainWindow->print("  my_di_copy_buffer(%d, count=%d)\r\n", buf_index, word_count);
  int ret=0;
  if(didll.bOK) {
    ret=(int)didll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 2, didll.di_copy_buffer, (DWORD)buf_index, (DWORD)dest, (DWORD)word_count);
    if(ret) {
      char sErrMsg[256];
      my_di_strerr(ret, sErrMsg);
      //if(_pMainWindow) _pMainWindow->print("    error=%d \'%s\'\r\n", ret, sErrMsg);
    }
  }
  return ret;
}

#else //  __WIN32__

#endif // __WIN32__


#endif // DI()
