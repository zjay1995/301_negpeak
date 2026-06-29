#ifndef  _ID_H
#define  _ID_H

#include "helpids.h"


#define IDPRINT			996
#define IDBROWSE		997
#define IDHELP			998


#define WM_IDLE			(WM_USER)
#define WM_IS_IT_YOUR_NAME	(WM_USER+1)
#define WM_REFRESH		(WM_USER+2)
#define WM_GET_JOB		(WM_USER+3)
#define WM_GET_RUN		(WM_USER+4)
#define WM_GET_PEAKLIST		(WM_USER+5)
#define WM_GET_OUTPUT		(WM_USER+6)
#define WM_INTEGRATE_RUN	(WM_USER+7)
#define WM_NEW_PEAK		(WM_USER+8)
#define WM_NEW_RUN		(WM_USER+9)
#define WM_GET_PICTINFO		(WM_USER+10)
#define WM_GET_RUNPICTINFO	(WM_USER+11)


#define ID_MAIN_STATUS	1

#define ID_WPEAK_ICON 		1
#define ID_HNU_ICON 		2



#define ID_OUTPUT_EDIT	199
#define ID_STATUSLINE	200




#define IDD_ABOUT_APP_NAME	24
#define IDD_ABOUT_APP_DESCR     25


// Options Save dialog's IDs

#define IDD_ENVIROMENT	21
#define IDD_DESKTOP	22
#define IDD_METHOD	23
#define IDD_WINDOW	24
#define IDD_COM_STATUS 	25
#define IDD_COMPORT	26
#define IDD_CM		27
#define IDD_INCH	28
#define IDD_STARTUPABOUT 29
#define IDD_STARTUPWINDOW 30
#define IDD_STARTUPFILE 31
//#define IDD_GC311VERSION 32
#define IDD_3DDIALOGS 33
#define IDD_MODBUS    34
#define IDD_MODBUS_LABEL 35 // added. P.P. 08/05/05

// Components dialog's IDs

#define IDD_COMPONENT_LIST_A    40
#define IDD_COMPONENT_LIST_B    41

#define ID_EDIT_BUTTON		50

#define ID_CALIBRATION		60

// Edit Component dialog's IDs

#define ID_COMPONENT_NAME	100
#define ID_COMPONENT_ACTIVE	101
#define ID_COMPONENT_PEAK	102
#define ID_COMPONENT_WINDOW	103
#define ID_COMPONENT_RESP_FACT	104
#define ID_COMPONENT_LOW_ALARM	105
#define ID_COMPONENT_HIGH_ALARM	106

// Edit Standard dialog's IDs
#define ID_STAND_REF		150

#define ID_STAND_0		156
#define ID_STAND_1		157
#define ID_STAND_2		158
#define ID_STAND_3		159

// Method dialog's IDs

#define IDD_INJDET_TEMP_LABEL	198
#define IDD_INJDET_TEMP		199
#define IDD_OVEN_TEMP_LABEL	200
#define IDD_OVEN_TEMP		201
#define IDD_ANALYSIS_TIME	202
#define IDD_SAMPLE_TIME		203
#define IDD_INJECT_TIME		204
#define IDD_SYRINGE_INJ		205
#define IDD_ALARMS		206
#define IDD_PAPER_SAVE		207
#define IDD_CONT_PUMP		208
#define IDD_CALIBRATION_CYCLE	209
#define IDD_311_PLOTTER		210
#define IDD_311_TWA		211
#define IDD_311_STEL		212
#define IDD_TWA_CYCLE_LABEL	213
#define IDD_TWA_CYCLE		214
#define IDD_REPEAT_CYCLE	215
#define IDD_POINTS_ALARMS	216
#define IDD_EQUILIBRATE_TIME	217     // added. P.P. 04/06/02
//#define IDD_NEXT_TIME		218	// added P.P. 04/09/02
#define IDD_DESORB_TIME		218	// added instead. P.P. 09/13/02
#define IDD_2ND_PUMP            219     // added. P.P. 01/30/09

// if INTERNAL
#define IDD_TP			220
#define IDD_RAMPING		221
#define IDD_HOLD_TIME_1_LABEL   222
#define IDD_HOLD_TIME_1         223
#define IDD_RAMP_1_LABEL        224
#define IDD_RAMP_1              225
#define IDD_HOLD_TEMP_2_LABEL   226
#define IDD_HOLD_TEMP_2         227
#define IDD_HOLD_TIME_2_LABEL   228
#define IDD_HOLD_TIME_2         229
#define IDD_RAMP_2_LABEL        230
#define IDD_RAMP_2              231
#define IDD_HOLD_TEMP_3_LABEL   232
#define IDD_HOLD_TEMP_3         233
#define IDD_HOLD_TIME_3_LABEL   234
#define IDD_HOLD_TIME_3         235
// added for 301b with concentrator. P.P. 08/02/02
#define IDD_DESORB_TEMP_LABEL   236
#define IDD_DESORB_TEMP         237
#define IDD_PURGE_TIME          238
#define IDD_PURGE_TIME_LABEL    239
// added  to make different variants. 08/02/02
#define IDD_REPEAT_CYCLE_LABEL  240
#define IDD_CALIBRATION_CYCLE_LABEL 241
#define IDD_20MA_RANGE_LABEL    242
#define IDD_SAMPLE_TIME_LABEL   243
#define IDD_AUTOZERO_TIME_LABEL 244
#define IDD_INJECT_TIME_LABEL   245
#define IDD_DESORB_TIME_LABEL   246   // added. P.P. 09/13/02
#define IDD_EQUILIBRATE_TIME_LABEL	247  // added. P.P. 09/13/02 to pre-inject purge.
#define IDD_CALIBRATE_TIME      248   // added P.P. 12/19/02
#define IDD_CALIBRATE_TIME_LABEL 249  //
// end of added.

//#define IDD_X1_TIME       250         // same as detectors??? P.P. 09/17/05
//#define IDD_X1_TIME_LABEL 251         // 
#define IDD_DOOR_MANUAL     284   // P.P. 09/06/12 to enable/disable door to open/close automatically
#define IDD_X1_TIME       285         // 
#define IDD_X1_TIME_LABEL 286         // changed. P.P. 10/01/05
#define IDD_X1_OFF_TIME       287         //
#define IDD_X1_OFF_TIME_LABEL 288        // added. P.P. 06/18/06
#define IDD_TEMP_SLOPE_LABEL  289        // 4 lines added. P.P. 07/20/06
#define IDD_TEMP_SLOPE	      290
#define IDD_TEMP_INTERCEPT_LABEL  291
#define IDD_TEMP_INTERCEPT	  292
#define IDD_TEMP_SETTING      293     // P.P. 07/21/06	
// endif


#define IDD_DETECTOR_A		250
#define IDD_DETECTOR_B		251

#define IDD_DET_LABEL		252
#define IDD_NANDB_TIME		253
#define IDD_NANDB_LEN		254
#define IDD_MIN_AREA		255
#define IDD_MIN_HEIGHT		256
#define IDD_SEGMENT_WIDTH	257
#define IDD_BASELINE_PROJECTION	258
#define IDD_TANGENT_SKIMING	259
#define IDD_OPTIMIZATION	260
#define IDD_HEIGHT		261
#define IDD_AREA		262

#define IDD_RANGE_LABEL		263
#define IDD_RANGE		264
#define IDD_PLOT_ATTEN_LABEL	265
#define IDD_PLOT_ATTEN		266

#define IDD_DETECTOR_ON_OFF	267
#define IDD_UNITS		268

#define IDD_METHOD_NOTES	269

#define IDD_RUN_NOTES		270
#define IDD_RUN_DATE		271

#define IDD_LAMP_SAVE		272

#define IDD_RANGEAUTO_LABEL	273
#define IDD_RANGEAUTO		274
//#define IDD_20MA_RANGE // moved down and changed. 07/27/07		275  //added. P.P. 03/19/02
#define IDD_UV                  277  // added 10/11/02. 276 - for FLAME.
#define IDD_ALL_PEAKS           278  // P.P. 09/17/05
#define IDD_KNOWN_PEAKS         279  // P.P. 09/17/05
#define IDD_EXTERNAL            280  // P.P. 11/14/08 - external detector
#define IDD_INVERT              281  // P.P. 03/25/09 - invert signal P.P. 11/06/09
#define IDD_DATA_EXCEL          282  // P.P. 02/24/12 - to write data to Excel file
#define IDD_RANGE_1_LABEL       283  // P.P. 11/09/12
#define IDD_RANGE_1             284  // P.P. 11/09/12, for setting gain in specified time intervals

#define IDD_TOP_MARGIN		301
#define IDD_TOP_MARGIN_UNITS	302
#define IDD_LEFT_MARGIN		303
#define IDD_LEFT_MARGIN_UNITS	304
#define IDD_RIGHT_MARGIN	305
#define IDD_RIGHT_MARGIN_UNITS	306
#define IDD_BOTTOM_MARGIN	307
#define IDD_BOTTOM_MARGIN_UNITS	308


#define IDD_RUNLIST             310
#define IDD_RUNLIST_LABEL       311
#define IDD_SELECT_ALL		312
#define IDD_UNSELECT_ALL	313
#define IDD_PEAKLIST		314

#define IDD_XMIN		320
#define IDD_XMAX		321
#define IDD_KX			322
#define IDD_XAUTO		333
#define IDD_YMIN		334
#define IDD_YMAX		335
#define IDD_KY			336
#define IDD_YAUTO		337
#define IDD_SCALE		338 // P.P. 01/22/13

#define IDD_PRINT_METHOD	350
#define IDD_PRINT_DETECTORS	351
#define IDD_PRINT_COMPONENTS	352
#define IDD_PRINT_STANDARDS	353
#define IDD_PRINT_RUN_LIST	354
#define IDD_PRINT_GRAPH		355
#define IDD_PRINT_TABLE		356
#define IDD_PRINT_STL		357
#define IDD_PRINT_TWA		358
#define IDD_PRINT_DETECTOR	359

#define IDD_ACQUIRE_MODE	370
#define IDD_ACQUIRE_MODE_1	371
#define IDD_ACQUIRE_MODE_2	372
#define IDD_ACQUIRE_MODE_3	373
#define IDD_SAVERUN_MODE	374
#define IDD_SAVERUN_MODE_1	375
#define IDD_SAVERUN_MODE_2	376
#define IDD_TWA_MODE		377
#define IDD_STEL_MODE		378

#ifdef FILTER
#define IDD_FILTER_DETECTOR	380
#define IDD_FILTER_NUM		381
#define IDD_FILTER_FLAG		382
#endif//FILTER

#define IDD_COLORA		391
#define IDD_COLORB		392
#define IDD_COLORALABEL		393
#define IDD_COLORBLABEL		394


#define IDD_STANDARDLIST             400
#define IDD_STANDARDLIST_LABEL       401

#define IDD_CALIBRATIONGRAPH         410


// added for flow mode dialog. P.P. 12/30/03
#define IDD_FLOW_MODE		420
#define IDD_FLOW_MODE_1		421
#define IDD_FLOW_MODE_2		422  // back - without flowInj mode. P.P. 01/09/04
#define IDD_FLOW_MODE_3		423
//#define IDD_FLOWINJ_MODE	422
//#define IDD_FLOWINJ_MODE_1	423
#define IDD_SETPOINT		424
#define IDD_SETPOINT_1		425
#define IDD_SETPOINT_2		426
#define IDD_SETPOINT_3		427
#define IDD_FLOW_VALUE		428
// end of added for flow mode dialog. P.P. 12/30/03
#define IDD_LOCK_SETPOINT	429   // P.P. 01/05/04

// block of defines added for additional X-valves. P.P. 06/20/10
#define IDD_X2_TIME       	440         
#define IDD_X2_TIME_LABEL 	441         
#define IDD_X2_OFF_TIME       	442         
#define IDD_X2_OFF_TIME_LABEL 	443        
#define IDD_X3_TIME       	444         
#define IDD_X3_TIME_LABEL 	445         
#define IDD_X3_OFF_TIME       	446        
#define IDD_X3_OFF_TIME_LABEL 	447        
#define IDD_X4_TIME       	448         
#define IDD_X4_TIME_LABEL 	449         
#define IDD_X4_OFF_TIME       	450        
#define IDD_X4_OFF_TIME_LABEL 	451        
#define IDD_X5_TIME       	452   // x5 added. P.P. 07/01/10
#define IDD_X5_TIME_LABEL 	453         
#define IDD_X5_OFF_TIME       	454        
#define IDD_X5_OFF_TIME_LABEL 	455        
// end
#define IDD_LOW_ALARMS		500
#define IDD_LA1			501
#define IDD_LA2			502
#define IDD_LA3			503
#define IDD_LA4			504
#define IDD_LA5			505
#define IDD_LA6			506
#define IDD_LA7			507
#define IDD_LA8			508
#define IDD_LA9			509
#define IDD_LA10		510
#define IDD_LA11		511
#define IDD_LA12		512
#define IDD_LA13		513
#define IDD_LA14		514
#define IDD_LA15		515
#define IDD_LA16		516
#define IDD_LA17		517
#define IDD_LA18		518
#define IDD_LA19		519
#define IDD_LA20		520
#define IDD_LA21		521
#define IDD_LA22		522
#define IDD_LA23		523
#define IDD_LA24		524
#define IDD_LA25		525
#define IDD_LA26		526
#define IDD_LA27		527
#define IDD_LA28		528
#define IDD_LA29		529
#define IDD_LA30		530
#define IDD_LA31		531
#define IDD_LA32		532
#define IDD_LA_ALL		533  // P.P. 12/22/10
#define IDD_LA_CLEAR		534

#define IDD_HIGH_ALARMS		540
#define IDD_HA1			541
#define IDD_HA2			542
#define IDD_HA3			543
#define IDD_HA4			544
#define IDD_HA5			545
#define IDD_HA6			546
#define IDD_HA7			547
#define IDD_HA8			548
#define IDD_HA9			549
#define IDD_HA10		550
#define IDD_HA11		551
#define IDD_HA12		552
#define IDD_HA13		553
#define IDD_HA14		554
#define IDD_HA15		555
#define IDD_HA16		556
#define IDD_HA17		557
#define IDD_HA18		558
#define IDD_HA19		559
#define IDD_HA20		560
#define IDD_HA21		561
#define IDD_HA22		562
#define IDD_HA23		563
#define IDD_HA24		564
#define IDD_HA25		565
#define IDD_HA26		566
#define IDD_HA27		567
#define IDD_HA28		568
#define IDD_HA29		569
#define IDD_HA30		570
#define IDD_HA31		571
#define IDD_HA32		572
#define IDD_HA_ALL		573  // P.P. 12/22/10
#define IDD_HA_CLEAR		574

#define IDD_POINTS		580
#define IDD_POINT1		581
#define IDD_POINT2		582
#define IDD_POINT3		583
#define IDD_POINT4		584
#define IDD_POINT5		585
#define IDD_POINT6		586
#define IDD_POINT7		587
#define IDD_POINT8		588
#define IDD_POINT9		589
#define IDD_POINT10		590
#define IDD_POINT11		591
#define IDD_POINT12		592
#define IDD_POINT13		593
#define IDD_POINT14		594
#define IDD_POINT15		595
#define IDD_POINT16		596
#define IDD_POINT17		597
#define IDD_POINT18		598
#define IDD_POINT19		599
#define IDD_POINT20		600
#define IDD_POINT21		601
#define IDD_POINT22		602
#define IDD_POINT23		603
#define IDD_POINT24		604
#define IDD_POINT25		605
#define IDD_POINT26		606
#define IDD_POINT27		607
#define IDD_POINT28		608
#define IDD_POINT29		609
#define IDD_POINT30		610
#define IDD_POINT31		611
#define IDD_POINT32		612
#define IDD_POINTS_ALL		613  // P.P. 12/22/10
#define IDD_POINTS_CLEAR	614

// added to show 1st component concentration in 10 points. P.P. 12/28/02
#define IDD_CONC_1_1            621
#define IDD_CONC_2_1            622
#define IDD_CONC_3_1            623
#define IDD_CONC_4_1            624
#define IDD_CONC_5_1            625
#define IDD_CONC_6_1            626
#define IDD_CONC_7_1            627
#define IDD_CONC_8_1            628
#define IDD_CONC_9_1            629
#define IDD_CONC_10_1           630
// end of added. P.P. 12/28/02
#define IDD_CHECK_ALARM         640  // added 04/06/03


#define IDD_EQUAL		645


#define IDD_SAVESHIFT_MODE	670
#define IDD_SAVESHIFT_MODE_1	671
#define IDD_SAVESHIFT_MODE_2	672
#define IDD_SAVESHIFT_MODE_3	673
#define IDD_PRINTSHIFT_MODE	674
#define IDD_PRINTSHIFT_MODE_1	675
#define IDD_PRINTSHIFT_MODE_2	676
#define IDD_PRINTSHIFT_MODE_3	677
#define IDD_PRINTSHIFTONALARM	678

#define IDD_SHIFTTIME	 	680

// added 07/27/07
#define IDD_20MA_RANGE		700  //added. P.P. 03/19/02
#define IDD_20MA_RANGE2		701  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL2   702  //
#define IDD_20MA_RANGE3		703  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL3   704  //
#define IDD_20MA_RANGE4		705  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL4   706  //
#define IDD_20MA_RANGE5		707  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL5   708  //
#define IDD_20MA_RANGE6		709  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL6   710  //
#define IDD_20MA_RANGE7		711  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL7   712  //
#define IDD_20MA_RANGE8		713  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL8   714  //
#define IDD_20MA_RANGE9		715  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL9   716  //
#define IDD_20MA_RANGE10	717  //added. P.P. 04/14/07
#define IDD_20MA_RANGE_LABEL10  718  //
// end 07/27/07

#define IDD_QUICK_SCAN_GROUP_BOX        728  //
#define IDD_DESORBER_GROUP_BOX          729  // P.P. 07/22/18
// added P.P. 03/07/10 for GC with concentrator
#define IDD_CONC_PREHEAT_TIME 		730  // P.P. 03/07/10
#define IDD_CONC_PREHEAT_TIME_LABEL 	731  //
#define IDD_QUICK_SCAN		        732 //
#define IDD_QUICK_SCAN_TIME       	733 // 3 lines added for quick scan. P.P. 12/14/10
#define IDD_QUICK_SCAN_TIME_LABEL       734 //
#define IDD_ZERO_GAS_TIME		735 // added. P.P. 03/14/11
#define IDD_ZERO_GAS_TIME_LABEL		736
#define IDD_ZERO_GAS_CYCLE		737
#define IDD_ZERO_GAS_CYCLE_LABEL	738
#define IDD_CONC_WAIT_TIME 		739  // P.P. 03/26/18
#define IDD_CONC_WAIT_TIME_LABEL 	740  //


#define IDD_HEAT_ON			750  // added as LED indicator. P.P. 04/30/13
#define IDD_HEAT_OFF			751  //
#define IDD_SAMPLE			752  // added as LED indicator. P.P. 03/14/17
#define IDD_EQUILIBRATE			753  // added as LED indicator. P.P. 03/14/17
#define IDD_INJECT			754  // added as LED indicator. P.P. 03/14/17
#define IDD_DESORB			755  // added as LED indicator. P.P. 03/14/17
#define IDD_PURGE			756  // added as LED indicator. P.P. 03/18/17
#define IDD_FAN				757  // added as LED indicator. P.P. 03/18/17
#define IDD_INJ_V4			758  // added as LED indicator. P.P. 03/18/17
#define IDD_LINE_TEMP			759  // added as edit box. P.P. 03/28/17
#define IDD_LINE_TEMP_LABEL		760  // added as label at edit box. P.P. 03/28/17
#define IDD_LINE_HEAT                   761  // added as LED indicator. P.P. 03/28/17
#define IDD_NEXT_POINT_TIME		762  // added as edit box. P.P. 06/05/17  here 10/16/17
#define IDD_NEXT_POINT_TIME_LABEL	763  // added as label for edit box. P.P. 06/05/17
#define IDD_FLOW_HIGH			764  // added as high flow indicator. P.P. 05/13/18
#define IDD_FLOW_LOW			765  // added as low flow indicator. P.P. 05/18/18
#define IDD_FID_TEMP                    766  // P.P. 07/16/18 added for the case of 3 or 4 temp zones
#define IDD_FID_TEMP_LABEL              767  // P.P. 07/16/18 added for the case of 3 or 4 temp zones
#define IDD_FAN_DSB		        768  // added as LED indicator. P.P. 07/16/18
#define IDD_HEAT_INJ_ON                 769  // P.P. 12/12/18

#define IDS_MDISYSTEM	30900
#define IDS_MDIRESTORE	30901
#define IDS_MDIMOVE	30902
#define IDS_MDISIZE	30903
#define IDS_MDIMINIMIZE	30904
#define IDS_MDIMAXIMIZE	30905
#define IDS_MDICLOSE	30906
#define IDS_MDINEXT	30907
#define IDS_MDIPREV	30908

#define IDS_SYSTEM	30910
#define IDS_FILE	30911
#define IDS_EDIT	30912
#define IDS_VIEW	30913
#define IDS_METHOD      30914
#define IDS_RUN		30915

#define IDS_OPTIONS     30917
#define IDS_WINDOW	30918
#define IDS_HELP	30919


#define IDS_SAVE_JOB_AS         31000
#define IDS_LOAD_JOB_FROM	31001
#define IDS_METHOD_FILTER	31002
#define IDS_JOB_FILTER		31003
#define IDS_METHOD_JOB_FILTER	31004
#define IDS_SAVE_PEAKS_AS	31005
#define IDS_PEAKS_FILTER	31006
#define IDS_SAVEREPORT		31007
#define IDS_RTF_FILTER		31008
#define IDS_SAVEPICTURE		31009
#define IDS_WMF_FILTER		31010
#define IDS_SAVEPEAKLIST	31011
#define IDS_RTF_TXT_FILTER		31012

#define CM_AUTOEXEC     1100


#define SystemFirst	SC_RESTORE
#define FileFirst	CM_FILENEW
#define EditFirst	CM_EDITPASTE//CM_EDITUNDO
#define ViewFirst	CM_CHANGEFONT
#define MethodFirst	CM_METHODEDIT
#define RunFirst	CM_ACQUIRE
#define IntegrateFirst	CM_INTEGRATE
#define OptionsFirst	CM_OPTIONS_PREFERENCES
#define WindowFirst	CM_CASCADECHILDREN
#define HelpFirst	CM_HELPCONTENTS


#endif




