#include "includes.h"
#pragma hdrstop

#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif
#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif

#ifndef  _DETECTOR_H
#include "detector.h"
#endif


static ControlInfo far DetectorControlInfo[]={
  { citStr,	IDD_DET_LABEL,         FIELDOFFSET(Detector,Label) },
//  { citTIME,	IDD_NANDB_TIME,         FIELDOFFSET(Detector,NandBtime),  1, 10*60},
  { citTIME,	IDD_NANDB_TIME,         FIELDOFFSET(Detector,NandBtime),  1, 119*60},
  { citTIME,	IDD_NANDB_LEN,          FIELDOFFSET(Detector,NandBlen),  1, 30},
  { citRadio,   IDD_HEIGHT,         	FIELDOFFSET(Detector,detect_meth),	2	},
  { citRadio,   IDD_BASELINE_PROJECTION, FIELDOFFSET(Detector,peak_alg), 3		},
//  { citComboList, IDD_RANGE, 		FIELDOFFSET(Detector,gain), (LPARAM)"1\00010\000100\000"		}, // restored, only up to 100. P.P. 05/05/17
  { citComboList, IDD_RANGE, 		FIELDOFFSET(Detector,gain), (LPARAM)"1\00010\000100\0001000\000"		},  // range 1000. P.P. 12/11/09  06/20/11 restored 05/15/17
//  { citComboList, IDD_RANGE_1, 		FIELDOFFSET(Detector,gain_1), (LPARAM)"1\00010\000100\0001000\000"		},  // 2nd set. P.P. 11/09/12
//  { citComboList, IDD_RANGE, 		FIELDOFFSET(Detector,gain), (LPARAM)"1\00010\000100\0001000\00010000\000"	},  // range 10000 added. P.P. 02/15/11. here 04/01/11
  { citONOFF,   IDD_RANGEAUTO, 		FIELDOFFSET(Detector,auto_gain_on_off)		},
  { citComboList, IDD_PLOT_ATTEN, 	FIELDOFFSET(Detector,plot_atten), (LPARAM)"1\00010\000100\000"		},
  { citONOFF,	IDD_DETECTOR_ON_OFF,    FIELDOFFSET(Detector,detect_stat),		},
  { citLONG,	IDD_MIN_AREA,         	FIELDOFFSET(Detector,MinArea),0,9999999L	}, // changed. P.P. 03/21/05
  { citLONG,	IDD_MIN_HEIGHT,        	FIELDOFFSET(Detector,MinHeight),0,999999L}, // changed. P.P. 03/21/05
  { citINT,	IDD_SEGMENT_WIDTH,      FIELDOFFSET(Detector,segment_width),1,40},
  { citStr,	IDD_UNITS,		FIELDOFFSET(Detector,units),		},
  { citLONG,	IDD_20MA_RANGE,        	FIELDOFFSET(Detector,AnalogRange),0,99999L},
// added 07/27/07
  { citLONG, 	IDD_20MA_RANGE2,        FIELDOFFSET(Detector,AnalogRange[1]),0,99999L}, // added. P.P. 04/22/07
  { citLONG, 	IDD_20MA_RANGE3,        FIELDOFFSET(Detector,AnalogRange[2]),0,99999L}, // added. P.P. 04/22/07
  { citLONG, 	IDD_20MA_RANGE4,        FIELDOFFSET(Detector,AnalogRange[3]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE5,        FIELDOFFSET(Detector,AnalogRange[4]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE6,        FIELDOFFSET(Detector,AnalogRange[5]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE7,        FIELDOFFSET(Detector,AnalogRange[6]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE8,        FIELDOFFSET(Detector,AnalogRange[7]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE9,        FIELDOFFSET(Detector,AnalogRange[8]),0,99999L}, // 
  { citLONG, 	IDD_20MA_RANGE10,        FIELDOFFSET(Detector,AnalogRange[9]),0,99999L}, //
// end. 07/27/07
  { citONOFF,	IDD_LAMP_SAVE,    	FIELDOFFSET(Detector,lamp_save),		},
  { citONOFF,	IDD_UV,    		FIELDOFFSET(Detector,uv_type),		},
  { citONOFF,	IDD_EXTERNAL,    	FIELDOFFSET(Detector,ext_det),		},   // P.P. 11/14/08
  { citONOFF,	IDD_INVERT,    		FIELDOFFSET(Detector,invert),		},   // P.P. 03/25/09
  { citRadio,   IDD_ALL_PEAKS,         	FIELDOFFSET(Detector,known_peaks),	2	}, // 0-all, 1-known peaks only. P.P. 09/17/05
  { citONOFF,   IDD_DATA_EXCEL, 	FIELDOFFSET(Detector,data_to_excel)		}, // P.P. 02/24/12
  { citComboList, IDD_RANGE_1, 		FIELDOFFSET(Detector,gain_1), (LPARAM)"1\00010\000100\0001000\000"  		},
//  { citComboList, IDD_RANGE_1, 		FIELDOFFSET(Detector,gain_1), (LPARAM)"1\00010\000100\000"		}, // up to 100. P.P. 05/05/17

  { citNULL }	// End of array marker
};

class DetectorDlg : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  DetectorDlg(PTWindowsObject AParent, int ResourceId, char *Name, Detector *pDetector, int _num, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, pDetector, DetectorControlInfo, AModule)
   , num(_num)
  {
    SetCaption(Name);
  }
 protected:
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
  virtual void SetupControls(void *data);

  int num;
};


#include "internal.h" // INTERNAL()
void DetectorDlg::SetupControls(void *data)
{
  Inherited::SetupControls(data);
#if INTERNAL()
  ShowWindow(GetDlgItem(HWindow,IDD_PLOT_ATTEN_LABEL),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_PLOT_ATTEN),SW_HIDE);
  if(num==1) {  // Detector B
#if !GC321()
//    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),FALSE);  // rem. P.P. 10/27/02
//    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),FALSE);        //
#if !ANALOG_OUTPUT() 					     // added. P.P. 06/21/03
//      EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),FALSE);//
//      EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),FALSE);// added. P.P. 08/15/04
// added. 07/27/07
    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),SW_HIDE); // P.P. 04/16/07
//    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),SW_HIDE);       //
    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
#else // case added to show only needed number of analog outputs. P.P. 04/16/07
 // placeholder for restricting number of analog outputs
    int id_n = IDD_20MA_RANGE5;   			// P.P. 04/49/07  - for OLIN, only 4 components + flow
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
// end. 07/27/07
#endif                                                     //
#if STAND_ALONE() // 6 lines added. P.P. 08/17/03
    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),FALSE);  // removed in GC321. P.P. 06/11/02
    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),FALSE);
    EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),FALSE);
    EnableWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),FALSE);
#endif // stand_alone

#else // GC321
//    ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),SW_HIDE);  // removed in GC321. P.P. 06/11/02
//    ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),SW_HIDE);        // and commented 04/01/08 - from now we leave these.
//    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),SW_HIDE);
#if !ANALOG_OUTPUT() // cond added, GC322 can have analog ouputs from now on. 04/01/08
    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),SW_HIDE); // P.P. 04/01/08
// added 07/27/07
    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
// end 07/27/07
#endif // !analog_output
#endif //GC321
// P.P. 11/01/02. Next line removed - Lamp save check box is active in both A and B from now on.
//    EnableWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),FALSE); // Not active in B. P.P. 10/27/02 -
  }                                                        // - or needs extra contact closure.
  else {  // Detector A
#if GC321()
//    ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),SW_HIDE);  // removed in GC321. P.P. 06/11/02
//    ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),SW_HIDE);        // and commented 04/01/08 - from now we leave these.
//    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),SW_HIDE);
#if !ANALOG_OUTPUT() // cond added, GC322 can have analog ouputs from now on. 04/01/08
    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),SW_HIDE); // P.P. 04/01/08
// added. 07/27/07
    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
// end 07/27/07
#if STAND_ALONE() // hide in Det A also for stand_alone case. P.P. 08/16/02
    EnableWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),FALSE);
#endif // stand_alone
#endif // !analog_output
#else // !GC321
#if !ANALOG_OUTPUT() 					     // added. P.P. 06/22/03
//      EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),FALSE);//
//      EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),FALSE);// added. P.P. 08/15/04
// added 07/27/07
    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),SW_HIDE); // P.P. 04/16/07
//    ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),SW_HIDE);
    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
#else // case added to show only needed number of analog outputs. P.P. 04/16/07
 // placeholder for restricting number of analog outputs
//#if OLIN()    // cond rem P.P. 10/24/14
    int id_n = IDD_20MA_RANGE5;   			// P.P. 04/49/07  - for OLIN, only 4 components + flow
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
//#endif // olin

// end 07/27/07
#endif // !analog_output                                                     //
#if STAND_ALONE() // 6 lines added. P.P. 08/17/03
//#if !DI() // added and removed. P.P. 10/12/03
    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),FALSE);  // removed in GC321. P.P. 06/11/02
    EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),FALSE);
//#endif // di()
//    EnableWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),FALSE);
// added 07/27/07
//    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
// end 07/27/07
    EnableWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),FALSE);
#endif // stand_alone
#if FID() // case added. P.P. 08/12/05
  ShowWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),SW_HIDE);
#endif // fid
#endif // GC321
  }
//  EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),FALSE);  // disabled in version 36. P.P. 10/17/14 enabled 05/10/17
//  EnableWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),FALSE);
#if !NOISE_REDUCT()     // added to hide variant - disable) radio buttons. P.P. 09/17/05                                   //
  ShowWindow(GetDlgItem(HWindow,IDD_ALL_PEAKS),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_KNOWN_PEAKS),SW_HIDE);
//  EnableWindow(GetDlgItem(HWindow,IDD_KNOWN_PEAKS),FALSE); //
#endif // !noise_reduct                                    //
#else  // !INTERNAL
  ShowWindow(GetDlgItem(HWindow,IDD_LAMP_SAVE),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO_LABEL),SW_HIDE);  // 5 lines added P.P. 09/25/04
  ShowWindow(GetDlgItem(HWindow,IDD_RANGEAUTO),SW_HIDE);        //
//  ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE),SW_HIDE);       //
  ShowWindow(GetDlgItem(HWindow,IDD_20MA_RANGE_LABEL),SW_HIDE); //
// added 07/27/07
    int id_n = IDD_20MA_RANGE;   			// P.P. 04/16/07
    for(; id_n<=IDD_20MA_RANGE_LABEL10; id_n++) {       //
      ShowWindow(GetDlgItem(HWindow,id_n),SW_HIDE);     //
    }                                                   //
// end 07/27/07
  EnableWindow(GetDlgItem(HWindow,IDD_UV),FALSE);               //
#if !NOISE_REDUCT()    // added to hide variant - disable) radio buttons. P.P. 09/17/05                                   //
  ShowWindow(GetDlgItem(HWindow,IDD_ALL_PEAKS),SW_HIDE);
  ShowWindow(GetDlgItem(HWindow,IDD_KNOWN_PEAKS),SW_HIDE);
//  EnableWindow(GetDlgItem(HWindow,IDD_KNOWN_PEAKS),FALSE); //
#endif // !noise_reduct                                    //
#endif // internal
}

/* ----------------------------------------------------------------- */

#ifndef  _DET_DLG_H
#include "det_dlg.h"
#endif

PTWindowsObject newDetectorDlg(PTWindowsObject AParent, int ResourceId, char *Name, Detector *pDetector, int num, PTModule AModule)
{
  return new DetectorDlg(AParent, ResourceId, Name, pDetector, num, AModule);
}
