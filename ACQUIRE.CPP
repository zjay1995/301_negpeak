#include "includes.h"
#pragma hdrstop

#ifndef __TIME_H
#include <time.h>
#endif
#ifndef __STDIO_H
#include <stdio.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif

#ifndef  _ACQUIRE_H
#include "acquire.h"
#endif
#ifndef  _MAINWIND_H
#include "mainwind.h"
#endif
#ifndef  _WINUTIL_STATLINE_H
#include "winutil\statline.h"
#endif


#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif

#ifndef  _COMM_H
#include "comm.h"
#endif
#ifndef  _HNU_H
#include "hnu.h"
#endif

#include "debug.h"


//#define MYDEBUG() 1 //  Debug
#define MYDEBUG() 0 // No Debug

#if MYDEBUG()
static long _i_demo=0;
static long _J_demo=0;
static int _ran=0;
#undef DIAGNOSTICS
#define DIAGNOSTICS() 0 // diagnistics_port missed
#endif

//extern long pow10_array[3];
extern long pow10_array[4]; // adding range 1000. P.P. 12/11/09  back. 06/20/11
//extern long pow10_array[5]; // adding range 10000. P.P. 06/20/11

static MainWindow *_pMainWindow;

#if INTERNAL()
static int _cur_gain = 0;
BOOL Lamp_error = 0;  // bit 5 in alarm port, added for diagnostics. P.P. 11/16/02
BOOL Oven_error = 0;  // bit 1 in alarm port
BOOL Oven_high_error = 0; // added. P.P. 02/19/03
BOOL Flow_error = 0;  // bit 2 in alarm port
BOOL N2_error = 0;  // changed. P.P. 02/19/03
BOOL Cal_error  = 0;  // bit 3 in alarm port
BOOL Air_error = 0;         // added. P.P. 02/19/03
BOOL CalGas_error = 0;      //
BOOL Diagnostics_error = 0; //
BOOL HL_alarm   = 0;  // added P.P. 11/18/02
BOOL End_run    = 0;  // bit 7 in alarm port, added P.P. 12/06/02  Restored 08/08/04
//static BOOL End_run    = 0;  // bit 7 in alarm port or portC2. P.P. 05/31/04
BOOL Board_error = 0; // added. P.P. 04/25/04
BOOL Diagn_error = 0; // added. P.P. 04/25/04
//#if 1//DIAGNOSTICS() // cond added. P.P. 08/15/04 rem 09/01/18
BOOL Fatal_error = 0; // added. P.P. 02/22/03
Str  Fatal_error_Str;
#if FID()
BOOL FID_error = 0; // added. P.P. 07/03/05
BOOL FID_error_event = 0; // P.P. 08/07/05
//static double t_fid = 50.0; // added to store fid temp from Method. P.P. 08/28/05
//static double t_fid = 100.0; // changed. P.P. 09/24/05
static double t_fid = 60.0; // changed. P.P. 01/16/18
#endif // fid
#if HEATED_LINE()
static long t_line = 0.0;  //  P.P. 07/18/18
#endif
static double t_oven_stored = -1000.0;  // changed. P.P. 06/23/06
static double t_inj_stored = -1000.0;  // changed. P.P. 06/26/06
static double t_line_stored = 0;  // added. P.P. 03/31/17
static double t_dsb_stored = 0;  //  P.P. 07/27/18
static double t_fid_stored = 0;  //
//#endif // rem. P.P. 05/04/07
static double m = 100.0;      // added for T=mV+b. P.P. 07/21/06
static double b = 0;          //
//#if CONCENTRATOR()              // added. P.P. 02/20/06 cond removed. P.P. 03/24/17
static double temp_conc = 0;    //
//BOOL fan_on = 0; // added. P.P. 02/23/10.
BOOL purge_on = 0; // added. P.P. 03/16/17.
BOOL inj_V4_on = 0;           // added. P.P. 03/18/17
BOOL line_heat_on = 0; // P.P. 03/28/17
BOOL X5_valve_on = 0;  // P.P. 05/13/18
BOOL fan_dsb_on = 0; // added. P.P. 07/26/18.
BOOL dsb_on = 0; // added. P.P. 07/26/18.
//#endif                          //
static int Diagnostics(int diagnostics_port);
#endif // diagnostics replaced by 1
//BOOL bAlarm = 0;  // added. P.P. 08/17/04  and removed
static int current_state_alarm[8] = {-1,-1,-1,-1,-1,-1,-1,-1}; // -1 - unknown state. Added for alarms. P.P. 11/27/02
static int NextPointNumber = 0; // added. P.P. 04/25/03
static int Lamp_error_count = 0; // added. P.P. 04/25/04
BOOL oven_on = 0;           // added. P.P. 12/30/04
BOOL sample_on = 0;           // added. P.P. 03/14/17
BOOL equilibrate_on = 0;           // added. P.P. 03/14/17
BOOL fan_on = 0; // added. P.P. 02/23/10. moved here 03/18/17
BOOL inject_on = 0;           // added. P.P. 03/14/17
BOOL desorb_on = 0;           // added. P.P. 03/14/17
BOOL oven_enabled = 0; // added. P.P. 07/02/06
BOOL det_on = 0;           // added. P.P. 06/11/06
BOOL oven_door_on = 0;   // added. P.P. 06/23/06
BOOL lamp_on = 0;        // P.P. 07/02/06
BOOL lampA_on = 0;        // P.P. 03/21/08
BOOL lampB_on = 0;        // P.P. 03/21/08
static int lamp_state[2]={0,0}; // P.P. 03/21/08
//#if PMD_12LS()||PMD_16FS()  // added to measure once and use later. P.P. 11/28/04,
static int ConfigVal = 0;  // added. P.P. 01/01/07
static int ConfigVal2 = 0;  // added. P.P. 03/15/17

static int32 data_A = 0;    //   and the cond removed to use in all cases. P.P. 09/03/05
static int32 data_B = 0;    //
static double temp_oven = 0;//
static double temp_oven_raw = 0; // added. P.P. 04/16/06
static double temp_det = 0;//  added. P.P. 07/04/05
static int temp_oven_event = 0; // added. P.P. 04/16/06
static double temp_det_raw = 0; // added. P.P. 06/11/06
static double temp_line = 0; // added. P.P. 03/29/17
static int temp_det_event = 0; // added. P.P. 06/11/06
static int number_of_beeps = 3; // P.P. 06/25/06
static int safety_warning = 0; // added. P.P. 04/17/09
static int first_start = 1; // P.P. 04/20/09
static double temp_oven_safety = 0; // P.P. 04/20/09
static int temp_oven_count = 0; // added. P.P. 10/30/09
const int temp_oven_count_max = 12; // added. P.P. 10/30/09
static int count_step = 0;   // P.P. 10/30/09
static int check_count = 0;  //
BOOL show_message = 0; //  10/31/09
static double temp_fid = 0;//    // P.P. 11/22/17
static double temp_fid_raw = 0;  //
char time_str[200];  // P.P. 03/13/18
#if SCAN_CHANNELS() // cond added. P.P. 03/18/18
//unsigned short far *data_buffer = (unsigned short far*) my_cbWinBufAlloc(512); // P.P 03/17/18
HGLOBAL My_cbWinBufAlloc (long NumPoints);             // function prototype. P.P 03/17/18
HGLOBAL data_buffer = (HGLOBAL) My_cbWinBufAlloc(512); // to replace each ch reading for scan through all 8 channels. P.P 03/17/18
#endif
//static int time_delay = 20; // trying to start reading usb boards after some delay P.P. 03/29/18
static int time_delay = 1; // start reading usb boards after 1s delay P.P. 12/09/18
//#endif // pmd               //
#if GC312() // P.P. 04/07/06
static double battery_voltage = 0; // added. P.P. 04/07/06
int battery_channel = 4;
static int battery_low_event = 0;  // added. P.P. 04/09/06
static int battery_count = 0; // added. P.P. 10/30/09
const int battery_count_max = 50; // added. P.P. 10/30/09
int lamp_channel = 5; // added. P.P. 05/19/06
int lamp_channel_B = 6; // lamp B added P.P. 06/10/06
//const double max_temp_bat = 60.0; // P.P. 02/13/09
const double max_temp_bat = 100.0; // with new oven. P.P. 10/14/09
//#elif GC321() // case added. P.P. 06/09/06
#elif GC321()||GC326() //  P.P. 12/07/18
int lamp_channel = 5;   // lamp A
int lamp_channel_B = 6; // lamp B
#endif // gc312
int pressure_channel = 7; // added. P.P. 10/02/09
static double pressure_cylinder = 0; // added. P.P. 10/02/09
#if HUMIDITY_SENSOR()    // cond added. P.P. 03/18/18
int humidity_channel = 6;
static double humidity_value = 0; // P.P. 06/26/17
#endif
#if H2_PRESSURE_SENSOR()  // cond added. P.P. 03/18/18
int H2_channel = 5; // added. P.P. 11/24/17
static double H2_pressure = 0; // added. P.P. 11/24/17
#endif
BOOL battery_low = 0; // 3 parameters added. P.P. 04/08/06
BOOL battery_run = 0; //
BOOL oven_high = 0;   //
BOOL after_cal = 0; // P.P. changing sampling after cal run. P.P. 04/29/05
BOOL start_fid = 0; // P.P. 07/03/05
BOOL fid_on = 0; // added. P.P. 08/05/05
#if PULSING_OVEN() // cond added. P.P. 03/18/18
static int temp_delay4 = 4; // P.P. 07/08/06   for all cases, for pulsing power near working point
static int temp_delay8 = 8; // P.P. 07/09/06
#endif
//#if GC321() // added. P.P. 06/26/06
#if GC321()||GC326() //  P.P. 12/07/18
//static int temp_delay4 = 4; // P.P. 06/26/06. 07/08/06 - moved out of gc321
//static int temp_delay8 = 8; // P.P. 06/26/06  07/09/06 - moved out of gc321
static int temp_delay16 = 16; // P.P. 06/26/06
static int temp_delay32=32;   // P.P. 06/26/06
#endif // gc321 and manual
BOOL alarm_cal_off = 0; // added 10/12/08 in us070107    // 12/12/08 here
static int high_alarm_state[POINTS()] = {0}; // 10/13/08
static int low_alarm_state[POINTS()] = {0}; // 10/13/08
//static int quick_scan_active = 0; // added P.P. 12/17/10
//static int quick_scan_active = 1; // added P.P. 12/17/10  rem 01/02/11
#if QUICK_SCAN()
static int quick_scan_next_point = 1; // added P.P. 12/25/10
static int run_info_state = 0;  // P.P. 05/13/11
static int cal_info_state = 0;  // P.P. 05/13/11
#endif // quick scan
#if QUICK_SCAN()&&(!GC312())
int quick_scan_channel = 4; // P.P. 01/01/11
#endif
static int zero_gas_info_state = 0;  // P.P. 05/13/11
#if FLOW() // cond added. P.P. 03/18/18
static double flow_value = 0; // added. P.P. 06/14/11
#endif
static int motor_on = 0; // P.P. 10/04/11
static int door_manual = 1; // to allow manual, before 1st run, added. P.P. 09/07/12
static int cur_gain[2] ={2,2}; // added. P.P. 01/04/13 not needed, we already have this. restored. P.P. 05/12/17
#if SERIAL_NUMBER() // cond added. P.P. 03/18/18
//static int ser_num = 2; // added to read the boards serial number. P.P. 03/12/17
//static uint32 ser_num = 2; // added to read the boards serial number. P.P. 03/12/17
static DWORD ser_num = 2; // added to read the boards serial number. P.P. 03/12/17
static uint32 mfg_num = 3; // added to read 8-bit boards MFG number. P.P. 03/15/17
//Str serialNum = "23324554"; // P.P. 03/17/17 to show in ShowTemperatures
char serialNum[9] = "1234F6A8"; // P.P. 04/01/17 to show in ShowTemperatures
static  char serNum[9];
#endif // serial number
static int gain_stored[2] = {2,2};   // added to store gain and gain_1 fron the range and range_1 fron detector dialog. P.P. 05/13/17
static int gain_1_stored[2] = {2,2};

#include "tctrl.h"

#include <mmsystem.h>
#include <math.h>


//#include "wow.h"

#if CBW()
#include "MyCBW.h"
#endif





typedef unsigned char byte; // 2 lines moved here. P.P.
const byte BIT[] = {1, 2, 4, 8, 16, 32, 64, 128};
#if (PMD_12LS()||CBWUSB_IO())&&(!(USB_1024LS()||USB_DIO24())) //  P.P. 03/09/18
const int fluidics_port_offset = FIRSTPORTA;    // P.P. 02/17/07
#if (POINTS()>1)||CONCENTRATOR() // 1-point case with concentrator - port B is for output. P.P. 12/30/09
const int point_port_offset = FIRSTPORTB;       //
#elif FID() // case added. P.P. 03/24/10
const int point_port_offset = FIRSTPORTB;       //
#else
const int diagnostics_port = FIRSTPORTB;  // mostly lamps. P.P. 02/20/09
#endif // points>1 or concentrator
#if PMD_16FS() //
const int control_port_offset = AUXPORT;        //
#endif // pmd_16fs
#else //
#if (PMD_12LS()||CBWUSB_IO())&&(USB_1024LS()||USB_DIO24()) // cond changed to use portB for diagnostics. P.P. 11/02/08
const int diagnostics_port = FIRSTPORTB; // in pmd_12LS
#if PMD_16FS() // added. P.P. 11/25/06
const int control_port_offset = AUXPORT;
const int fluidics_port_offset = FIRSTPORTA;    // P.P. 11/25/06  in PMD_12LS
const int point_port_offset = FIRSTPORTA;       // in usb_1024 or usb_dio24. P.P. 11/02/08
const int alarm_port_offset = FIRSTPORTB;       // in usb_1024 or usb_dio24. P.P. 11/02/08
const int high_alarm_port_offset = FIRSTPORTCL; // in usb_1024 or usb_dio24. P.P. 11/02/08
#else // only pmd_12LS - fluidics somewhere else. P.P. 11/25/06
const int control_port_offset = FIRSTPORTA;
#endif // 1608fs
#elif PMD_16FS() // without 12LS board - restored. P.P. 03/10/06
const int diagnostics_port = AUXPORT;
const int control_port_offset = AUXPORT;
const int fluidics_port_offset = AUXPORT;
#endif // pmd_12ls or 16FS
#endif // usb digital IO boards. Moved here from above. P.P. 03/18/06
// definitions added. P.P. 11/28/04
//  Fluidics bit_numbers:
const byte sample_bit_number = 0;
const byte inject_bit_number = 1;
const byte cal_bit_number    = 2;
const byte alarm_bit_number  = 3;   // internal alarm
#if CBWUSB_IO()
const byte autozeroB_bit_number = 1; // in AUXPORT 1608 board, separate autozero for det B. P.P. 08/03/18
#else
const byte autozeroB_bit_number = 7; // in port A 1208 board, separate autozero for det B. P.P. 06/21/17
#endif
const byte dsb_temperature_bit_number = 4; // in PortB P.P. 07/26/18
const byte dsb_fan_bit_number = 0; // in PortB of USB24IO or 1208, P.P. 08/03/18
const byte fan_bit_number    = 2; // P.P. 02/23/10. in port B of USB-1208LS for oven, same as oven_door
#if CBWUSB_IO()
const byte oven_temperature_bit_number = 0; // using Dig output in PortC to set oven temperature. P.P. 03/09/18
const byte det_temperature_bit_number = 1; // in PortC
#endif
//const byte diagnostics_bit_number = 3; // in place of int alarm, from now on. 01/05/09
const byte diagnostics_bit_number = 7; // bit 3 is GainA2 from now on. 12/30/09
const byte cal_error_bit_number = 7;   // in place of hydrogen bit. 01/05/09
//const byte lamp_bit_number   = 4;
#if GC312()&&RAMPING() // case added to use fan as oven door. P.P. 11/12/10
const byte oven_door_bit_number   = 2; // fan instead of oven door, in PortB of 1208 board. P.P. 06/20/10
//#elif GC321() // 09/30/11
#elif GC321()||GC326() //  P.P. 12/07/18
const byte oven_door_bit_number   = 2; // motor for oven door, in PortB of 1208 board. P.P. 09/30/11 restored 08/03/12
//const byte oven_door_bit_number   = 0; // changed - bad bit 2 today. P.P. 11/23/11. motor for oven door, in PortB of 1208 board. P.P. 11/23/11
const byte motor_polarity_bit_number = 1; // 09/30/11
#else
#if USB_DIO24()||USB_1024LS() // added. P.P. 06/20/11
#if POINTS()<=20
const byte oven_door_bit_number   = 4; // oven door moved to 4 leaving 7 for H2. P.P. 10/23/05
#elif POINTS()<=25
const byte oven_door_bit_number   = 7; // last bit in portB. P.P. 06/20/11
#else
//const byte oven_door_bit_number   = 4; // oven door moved to 4 leaving 7 for H2. P.P. 10/23/05
const byte oven_door_bit_number   = 7; // oven door moved to 7 . P.P. 06/21/11
#endif // points
#else // without additional DIO boards
//const byte oven_door_bit_number   = 4; // oven door moved to 4 leaving 7 for H2. P.P. 10/23/05 restored for GC312+FID. 03/09/12
const byte oven_door_bit_number   = 2; // same standard location, bit 2 in USB24IO or 1208. P.P. 08/03/18
//const byte oven_door_bit_number   = 7; //  P.P. 06/21/11
#endif  // dio24 or 1024 boards
#endif // gc312 and ramping
const byte filter_bit_number = 5;
const byte equilibrate_bit_number = 5; // Filter valve not used. P.P. 10/30/02
const byte temperature_bit_number = 6; // added. P.P. 07/06/02
//const byte oven_door_bit_number = 7; // added. P.P. 11/29/02
const byte hydrogen_bit_number = 7; // added. P.P. 06/28/05
// Diagnostics alarm bit = 7 in case of DIO24 and POINTS()<12. P.P. 02/19/03
//const byte diagnostics_bitnumber = 7; //
const byte cal_info_bit = 4; // in port C of DIO24 or DDA_2. added. P.P. 07/25/04
const byte zero_gas_bit_number = 4; // in port B of 1208 board. P.P. 03/17/11
const byte manifold_bit_number = 5; // in port B of 1208 board. P.P. 03/17/11
const byte zero_gas_autozero_bit_number = 6; // in port B of 1208 board. P.P. 03/17/11
const byte run_info_bit_number = 4; // in port C of USB-DIO24 board, info for QS. P.P. 05/13/11
const byte cal_info_bit_number = 5; // in port C of USB-DIO24 board, info for QS. P.P. 05/13/11
const byte zero_gas_info_bit_number = 6; // in port C of USB-DIO24 board, info for QS. P.P. 05/13/11
#if CONCENTRATOR() // any GC with USB-1608 and (USB-1208 or USBDIO_24),  P.P. 07/26/18
const byte desorber_bit_number = 6; // P.P. 07/26/18. in port B of USB-1208LS or USBDIO_24, enable desorber
const int conc_channel = 6; // in 1608 board, analog ch 6. P.P. 07/26/18
const byte injectV4_bit_number = 5; // in portB of 1208LS.  P.P. 12/30/09
//const byte injectV5_bit_number = 6; // P.P. 12/30/09  rem - not needed
const byte purge_bit_number    = 3; // P.P. 12/30/09. in port B of USB-1208LS or USBDIO_24
#endif // concentrator
#if FID()
//const byte flame_bit_number = 4; // added. P.P. 06/28/05
const byte flame_bit_number = 7; // in port B, USBDIO24 0r 1208, ignite flame if needed. P.P. 08/03/18
const int fid_channel = 5; // in 1608 board, analog ch 5. P.P. 07/26/18
#elif HEATED_LINE()
const byte line_heat_bit_number = 7; // in port B, USBDIO24 or 1208, power to heated line. P.P. 08/03/18
//const byte line_heat_bit_number = 4; // added. P.P. 03/29/17
const int line_channel = 5; // in 1608 board, analog ch 5. P.P. 07/26/18
#endif //fid

//const int TCD_bit_number = 0; // added to control TCD. P.P. 04/25/04
#if !GC312() // cond added - TCD as lamp B for gc312. P.P. 02/20/09
const byte TCD_bit_number = 0; // replaced. P.P. 05/31/04
#else
const byte TCD_bit_number = 4; // replaced. P.P. 02/20/09.  with 2 detectors, TCD is B.
#endif // !gc312
const byte N2_error_bit = 2;     // in PORTB, P.P. 04/25/04
//const byte Air_error_bit = 4;    //
//const byte CalGas_error_bit = 5; //
const byte Air_error_bit = 5;         //   3 lines moved here from 0509. P.P. 05/21/04
const byte CalGas_error_bit = 6;      //
const byte check_remote_cal_bit = 7;  // in portB, added. P.P. 05/09/04
const byte remote_cal_bit = 3;  // in portB, added. P.P. 08/15/04
const byte common_high_bit = 6;  // in portC, common high alarm, added. P.P. 01/11/05

//#if !GC321()                      //
#if !(GC321()||GC326()) //  P.P. 12/07/18
const byte makeup_bit_number = 7; // added for makeup valve. P.P. 01/05/04
const byte x1_bit_number = 7;   // these bits - in port B of 12LS board. Added P.P. 06/20/10 here 11/12/10
const byte x2_bit_number = 3;   //
const byte x3_bit_number = 4;
const byte x4_bit_number = 5;
const byte x5_bit_number = 6;   // added P.P. 07/01/10
#else // added for gc322. P.P. 06/20/10
const int x1_bit_number = 7;   // these bits - in port B of 12LS board. Added P.P. 06/20/10
const int x2_bit_number = 3;   //
const int x3_bit_number = 4;
const int x4_bit_number = 5;
const int x5_bit_number = 6;   // added P.P. 07/01/10, changed to int.
#endif                            //

// Digital outputs and inputs of CIO-DAS802/16 board // these can be removed if not used anywhere else. P.P. 08/28/18
const byte gain10_bit_number        = 5; // output
const byte gain100_bit_number       = 6; // output
const byte set_autozero_bit_number  = 7; // output, to set autozero on/off
const byte read_autozero_bit_number = 5; // input, to read autozero status
const byte remote_start_bit_number  = 6; // input, for remote start
// analog channels for flow sensors: // added. P.P. 05/23/04
//const short column_flow_chan = 1;    // set in Get_Flow()
const short sample_flow_chan = 4;    //
#if WATER_PUMP() // P.P. 01/04/17
int water_pump_number = 0;
#endif //water_pump
// Function Set_valve was changed to work with REL-16 board. P.P. 11/10/01
void Set_valve(int port_offset, byte bit_number, BOOL on)  // modified 06/06/03
{                                                          // changed 07/19/03
//  #if MYDEBUG()
#if MYDEBUG()||DEMO() // demo added. P.P. 10/05/04
    //#if MYDEBUG()||DEMO()||MANUAL_INJ() // no valves if manual only. P.P. 05/13/06
return;
#endif
// moved here - fluidics via 1208ls, alarms - via usberb08. P.P. 02/18/07
#if PMD_12LS()
    #if CBW() // the must for PMD
if((bit_number>=0)&&(bit_number<16)) {
My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, (int) bit_number, on? 1: 0); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
}
#endif // cbw
// code added to use USB_ERB24/08 to control valves, without DDA. P.P. 10/14/05
#elif CBWUSB_IO()
    My_cbDBitOut (USB_IO, FIRSTPORTA, (int) bit_number, on? 1:0); // changed here and below.  P.P. 03/13/18
#elif PMD_12FS() // added here 11/05/05
    #if CBW() // the must for PMD
if((bit_number>=0)&&(bit_number<16)) {
My_cbDBitOut (CBWDEVICE_12FS, FIRSTPORTA, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
}
#endif // cbw
#elif PMD_16FS()
    #if CBW() // the must for PMD
if((bit_number>=0)&&(bit_number<8)) {
My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1608FS, added. P.P. 11/28/04
}
#endif // cbw
// end of added code to set valves via PMD devices. P.P. 11/19/04
#endif //  PMD. P.P. 08/28/18
}

///New function added to set alarm bits to DDA in case REL_16 present. P.P. 11/27/02
// and modified to work with dio24 board as well. P.P. 05/01/04
void Set_alarm_bit(int port_offset, byte bit_number, BOOL on)
{
//#if DEMO() // added. P.P. 10/05/04
#if !STAND_ALONE() // 07/15/08
    #if DEMO()||STAND_ALONE() //  P.P. 07/15/08
    //#if DEMO()||GC312() // changed - no alarms in portable GC312c. P.P. 03/18/06
   return;
#endif
    byte port_value;
#if (PMD_12LS()||PMD_12FS()||PMD_16FS()||CBWUSB_IO()) // added for all USB boards. P.P. 03/13/18
#if CBW() // the must for PMD
    if(port_offset == control_port_offset) {
//#if PMD_12LS()
#if PMD_12LS()||CBWUSB_IO() // P.P. 03/13/18
#if PMD_16FS() // added, 2 boards +relays case, control via 16_fs. P.P. 11/25/06
        if((bit_number>=0)&&(bit_number<8)) {
            My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1608FS, added. P.P. 11/28/04
        }
#else // 12_ls or cbwusb_io board only
        #if PMD_12LS()
     if((bit_number>=0)&&(bit_number<16)) {
       My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
#elif CBWUSB_IO()
       My_cbDBitOut (USB_IO, FIRSTPORTA, (int) bit_number, on? TRUE: FALSE); // out bit_number in USB-DIO24, added. P.P. 03/13/18
#endif     }
#endif // pmd_16fs
#elif PMD_12FS()
        if((bit_number>=0)&&(bit_number<16)) {

       My_cbDBitOut (CBWDEVICE_12FS, FIRSTPORTA, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
     }
#elif PMD_16FS()
     if((bit_number>=0)&&(bit_number<8)) {
       My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, (int) bit_number, on? TRUE: FALSE); // out bit_number in PMD_1608FS, added. P.P. 11/28/04
     }
#endif // pmd boards
    } //control port
#endif // cbw
#endif // usb boards. 11/05/05
#endif // !stand_alone. 07/15/08
}
/// end of Set_alarm_bit.

////// The code to measure  Temperatures
/// work with DAC-802/16

//const int BASE=0x300; // base address of ADC card   // moved up. P.P. 02/26/02
static void Wait_loop_delay(int delay) // delays if needed somewhere
{
    int y = 0;
    for (int x = 0; x<= delay; x++)
        y =y++;

}

//// The function to read temperature with ADC card.
//// Voltage measured is 2.6 V at 100 degrees
//// and 0.085 V at 0 degrees C.
///////The channel is not defined till now,
///////we should make a choice. #0-oven, #7-INJ/DET.

static double Get_Temperature(int bInjDet)
{
#if MYDEBUG() || DEMO()  // added. P.P. 11/06/02
    //  #if MYDEBUG() || DEMO() || 1  // temp.. P.P. 12/30/10
return 25.0;
#endif
    short channel=bInjDet? 1: 0;  // from now on. P.P. 11/13/05
    uint16 data_in;
    uint32 sum_data_in=0;    //
#if CBW() // with cbw() code added. P.P. 07/19/03
    //#if (!PMD_16FS())&&(!PMD_12LS()) // cond added. P.P. 11/07/04
//  int max_meas_number = 4;
int max_meas_number = 1;  // P.P. 11/28/04
#if Get_Data_Bipolar()
int Gain = BIP10VOLTS;  // from now on - amplified thermocouple. P.P. 06/16/06
#else
int Gain = UNI10VOLTS;
#endif
//  for(int i=0; i<max_meas_number; i++) {
//#if !(PMD_16FS()||PMD_12LS())
#if PMD_16FS() // P.P. 07/03/05
for(int i=0; i<max_meas_number; i++) {  //  P.P. 11/07/04
My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
sum_data_in += data_in; // P.P. 11/07/04
}
#elif PMD_12LS()
for(int i=0; i<max_meas_number; i++) {  //  P.P. 11/07/04
My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data_in);
sum_data_in += (data_in&0x0FFF); // 12 bit data. P.P. 11/07/04
}
#elif PMD_12FS() // P.P. 07/05/05
for(int i=0; i<max_meas_number; i++) {  //  P.P. 11/07/04
My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&data_in);
sum_data_in += (data_in&0x0FFF); // 12 bit data. P.P. 11/07/04
}
#endif
#endif // cbw
#if PMD_16FS()||PMD_12LS()||PMD_12FS()// one of PMD. P.P. 07/03/05
    data_in = (uint16)(sum_data_in);// no averaging. P.P.
#endif
//_pMainWindow->print("  Get_Temperature(%d) results %u\r\n", channel, (unsigned short)data_in);
#if Get_Data_Bipolar()   //added. P.P. 06/29/02
    //  double voltage = (10.0*data_in)/32768.0;   // P.P 06/29/02
const double vmin=-10.00; // gain=1 From now on - amplified thermocouple. P.P. 06/16/06
const double vmax=+10.00;
#if PMD_16FS() // 16-bit boards. P.P. 07/03/05
double voltage = data_in*((vmax-vmin)/65536.0) +vmin; // P.P. 07/03/02. 0 in the middle.
#elif PMD_12LS()||PMD_12FS() // 12-bit case, and no DAS802. P.P. 07/03/05
double voltage = (data_in<<4)*((vmax-vmin)/65536.0) +vmin; // P.P. 11/07/04. 0 in the middle.
#endif
#endif // bipolar
//  const double R1 = 1470.00;   // R1 in series with RTD
    const double R1 = 1576.00;   // R1 in series with RTD adjusted. P.P. 04/23/06
    const double Vsource = 3.85; // voltage source
#if PMD_12LS()||PMD_16FS()||PMD_12FS()  // changed. P.P. 08/19/05.
    //    return (voltage*100.0); // 10 mV/degree. // replaced for line below. P.P. 07/21/06
return voltage*m + b;               //
#endif // PMD                                         //
}

////// New oven reading for safety. Only for USB-1208LS at this time. P.P. 09/07/07 added here 04/17/09
static double Check_Temp_Safety()
{
#if DEMO()||MYDEBUG()
    //#if 1  // for my tests without 1208 oard connected. P.P. 03/14/17
return 0;
#endif
//  short channel= 0;  //
    short channel= 2;  // for modified 1208LS board. P.P. 04/20/09
    uint16 data_temp = 0;
//  int Gain = BIP10VOLTS;
//  int Gain = BIP2PT5VOLTS;  // 04/20/09
    int Gain = BIP20VOLTS;  // 04/20/09
#if CBW() // with cbw() code added. P.P. 07/19/03
    #if PMD_12LS()&&PMD_16FS()
My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data_temp);
data_temp &= 0x0FFF; // 12 bit data. P.P. 11/07/04
#endif // pmd_12
#endif // cbw
#if Get_Data_Bipolar()   //added. P.P. 06/29/02
    const double vmin=-10.00; // gain=1 From now on - amplified thermocouple. P.P. 06/16/06
const double vmax=+10.00;
//  const double vmin=-2.50; // for modified 1208LS board. P.P. 04/20/09
//  const double vmax=+2.50;
//  const double vmin=-1.00; // gain = bip1volts
//  const double vmax=+1.00;
#if PMD_12LS() // 12-bit case
//  double voltage = (double) (data_temp)*((vmax-vmin)/4096.0) +vmin; // P.P. 11/07/04. 0 in the middle.
#endif
#endif // bipolar
//     return voltage*m + b;               //
    double temp_check = 0.1217*data_temp - 249.56; // for modified 1208LS board. P.P. 04/20/09
    return temp_check;
//        return (double) data_temp;
}
////// end of new oven reading for safety. 09/07/07


// new function to measure FID temperature using 1608 board. P.P. 07/27/18
static double Get_Temperature_FID(int fid_channel)
{
#if MYDEBUG() || DEMO()  //
    //  #if MYDEBUG() || DEMO() || 1  // temp.to test. P.P. 02/25/18
return 25.0;
#endif
    uint16 fid_data_in;
    uint32 fid_sum_data_in=0;    //
    int max_meas_number = 1;
    int Gain = BIP10VOLTS;  // amplified thermocouple.
#if PMD_16FS() // P.P. 11/22/17 using 1208 board with the 1608 present, for other data
    for(int i=0; i<max_meas_number; i++) {
My_cbAIn(CBWDEVICE_16FS, (int) fid_channel, Gain, (USHORT *)&fid_data_in);
fid_sum_data_in += (fid_data_in); // 16-bit data.
}
#endif //pmd1608
    fid_data_in = (uint16)(fid_sum_data_in/max_meas_number);// averaging. P.P.
    const double vmin=-10.00; // gain=1 amplified thermocouple.
    const double vmax=+10.00;
#if PMD_16FS() // 16-bit boards. P.P. 07/03/05
    double voltage = (fid_data_in)*((vmax-vmin)/65536.0) +vmin; // 0 in the middle.
#endif //pmd1608
    return voltage*m + b;               //
}

// measure line temp with 1608. P.P. 07/27/18
static double Get_Temperature_Line(int line_channel)
{
#if MYDEBUG() || DEMO()  //
    //  #if MYDEBUG() || DEMO() || 1  // temp.to test. P.P. 02/25/18
return 25.0;
#endif
    uint16 line_data_in;
    uint32 line_sum_data_in=0;    //
    int max_meas_number = 1;
    int Gain = BIP10VOLTS;  // amplified thermocouple.
#if PMD_16FS() // P.P. 11/22/17 using 1208 board with the 1608 present, for other data
    for(int i=0; i<max_meas_number; i++) {
My_cbAIn(CBWDEVICE_16FS, (int) line_channel, Gain, (USHORT *)&line_data_in);
line_sum_data_in += (line_data_in); // 16-bit data.
}
#endif //pmd1608
    line_data_in = (uint16)(line_sum_data_in/max_meas_number);// averaging. P.P.
    const double vmin=-10.00; // gain=1 amplified thermocouple.
    const double vmax=+10.00;
#if PMD_16FS() // 16-bit boards. P.P. 07/03/05
    double voltage = (line_data_in)*((vmax-vmin)/65536.0) +vmin; // 0 in the middle.
#endif //pmd1608
    return voltage*m + b;               //
}
// end of FID and line temp measurements. P.P. 07/27/18

static double Get_Temp_Concentrator() // to be changed, amplified thermocouple from 06/16/06 and on. P.P. 06/16/06
{
#if CBW() // cond added. P.P. 05/05/07
    //  short channel = 1;
    short channel = 6;   // separate channel from now on for desorber Temp measurement. P.P. 07/27/18
#if MYDEBUG() || DEMO()
    return 30.0;
#endif
    uint16 data_in;
    uint32 sum_data_in=0;    //
    int max_meas_number = 1;
    double voltage = 0;
#if Get_Data_Bipolar()
    int Gain = BIP10VOLTS;
#else  // unipolar
    int Gain = UNI10VOLTS;
#endif // bipolar
#if PMD_16FS()
    for(int i=0; i<max_meas_number; i++) {
        My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
        sum_data_in += data_in; // P.P. 11/07/04
    }
#endif
    data_in = (uint16)(sum_data_in);// no averaging. P.P.

#if Get_Data_Bipolar()
    const double vmin=-10.00;
    const double vmax=+10.00;
#if PMD_16FS() // 16-bit boards.
    voltage = data_in*((vmax-vmin)/65536.0) +vmin;
#elif PMD_12LS()||PMD_12FS() // 12-bit case
    voltage = (data_in<<4)*((vmax-vmin)/65536.0) +vmin;
#endif  // pmd_16FS
#else // unipolar
#endif // bipolar
    return voltage*m + b; // P.P. 12/30/09
#else       // !cbw. Added 05/05/07
    return 0; //
#endif      //
}

//#if !STAND_ALONE() // added. P.P. 07/14/03  rem temp. P.P. 07/12/06
//*****************************************
// definition of function to convert
// temperature in volts and then into bits
#if PMD_12LS() // case added. P.P. 11/11/05

// heated line case added. P.P. 03/28/17
static void SetLineTemperature(double temperature)
{
#if DEMO() ||(!HEATED_LINE())
return;
#elif (!DEMO()) && HEATED_LINE()   // case added. P.P. 08/29/17
//#endif
uint data_value = 0;
int err_code = 0;
if(temperature<=temp_line) {
if(line_heat_on) { // cond added. P.P. 10/24/08
err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, line_heat_bit_number+8, FALSE); // changed here and below.  P.P. 03/29/17
line_heat_on = 0;
}
}
else if(temperature>(temp_line+1.0)) { // P.P. 03/28/17, to work with mechanical relay
if(!line_heat_on) { //
err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, line_heat_bit_number+8, TRUE); // changed here and below.  P.P. 03/29/17
line_heat_on = 1;
}
}
else {
}
#else
return;
#endif
}
// end of heated line code

static void Write_to_DAC(int chan, uint data_value)
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if CBW() // added. P.P. 07/19/03
My_cbAOut(CBWDEVICE_12LS, chan, UNI5VOLTS, (USHORT) data_value); // replacement. P.P. 01/07/06
#endif
}

//static void SetOvenTemperature(long temperature)
static void SetOvenTemperature(double temperature)  // P.P. 10/26/08
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
_pMainWindow->print("SetTemperature %ld\r\n", temperature);
////// new. P.P. 11/11/05
// case added here for low-power oven, no pulsing. 04/13/08
#if GC312()// low power oven, pulsing is not needed. 02/23/08  // pulsing restored for 301-25 pnt. P.P. 06/13/11  restored 03/02/12
//#if 0  // P.P 03/02/12
#if !PULSING_OVEN()// no pulsing at all. P.P. 10/24/08  // temp, to exclude pulsing in GC301c-25points with Omron elay. P.P. 06/08/11  c0nd now P.P.03/07/17
//#if GC312()||GC321() // low power oven, pulsing is not needed. 02/23/08  It is need. 10/17/08
uint data_value = 0;
double temperature_delta = 0.5; // 02/23/08.  Restored 07/12/09 to avoid blinks resulting in Det signal ripple.
//  double temperature_delta = 0.0; // 02/23/08  // let it blink to overcome inertia  P.P. 11/22/11 for GC322 returned back to delta=0, with SSR, let it link. P.P. 10/05/17
if(((double) temperature<=0)||(temp_oven < 0)) {   // if board is bad, it shows  T<0. Turn heater OFF! P.P. 03/07/18
data_value = 0; // 0 V, OFF
Write_to_DAC(0, data_value);
oven_on = 0;
}
else if(((double) temperature)>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
data_value = 1023; // 5 V, ON/OFF
oven_on = 1;
Write_to_DAC(0, data_value);
}
else if(((double) temperature)<temp_oven) {
data_value = 0; // 0 V, ON/OFF
oven_on = 0;
Write_to_DAC(0, data_value);
}
else {  // do nothing
}
#else // pulsing for  GC312 with new oven. // P.P. 05/06/11
uint data_value = 0;
//  double temperature_delta = 2.0; // P.P. 05/06/11
double temperature_delta = 1.0; // P.P. 05/06/11
// code added for pulsing in GC312 with new oven. P.P. 05/06/11
if(temperature>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
data_value = 1023; // 5 V, ON/OFF
//    oven_on = 1;
if(!oven_on) {  // avoid extra writings. P.P. 10/24/08
Write_to_DAC(0, data_value);
oven_on=1;
}
}
// more pulsing variants added. P.P. 03/30/07
//.  else if(temperature>(temp_oven+2.0)) {
else if(temperature>(temp_oven+temperature_delta/1.5)) {  // P.P. 04/21/07
if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
if(temp_delay4>0) {
temp_delay4+=(-2);  // corrected. P.P. 10/26.08
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay4=4; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
// end of new code
}
// end of added code. P.P. 03/30/07
//  else if(temperature>(temp_oven+0.50)) {
else if(temperature>(temp_oven+temperature_delta/6)) {
if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
if(temp_delay4>0) {
temp_delay4--;
//	temp_delay4+=(-2);
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay4=4; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
// end of new code
/* // replaced for above
    data_value = 1023; // 5 V, ON/OFF
    oven_on = 1;
    Write_to_DAC(0, data_value);
*/
}
// code added 07/09/06
else if(temperature>(temp_oven)) {
if(!oven_on) {   // P.P. 06/25/06
if(temp_delay8>0) {
temp_delay8--;
//	temp_delay8+=(-2);  // P.P. 10/26/08
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay8=8; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay8=8; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
}
// end 07/09/06
//  else if(temperature<(temp_oven)) {
//  else if((temperature+5.0)<temp_oven) {  // P.P. 06/18/06
else if((temperature+1.0)<temp_oven) {  // P.P. 05/06/11
if(oven_on) {  // cond added. P.P. 10/24/08
data_value = 0; // 0 V
Write_to_DAC(0, data_value);
oven_on = 0;
temp_delay4=4; // restore
temp_delay8=8;
}
}
else if((temperature<=(temp_oven))&&oven_on) {  // P.P. 06/18/06
data_value = 0; // 0 V
oven_on = 0;
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
temp_delay8=8;
}
else {
}
// end of new code. P.P. 05/06/``
#endif // pulsing

#else // not 312 but a regular case - pulsing or not, on define.
uint data_value = 0;
#if !PULSING_OVEN()  // P.P. 05/01/13
//  uint data_value = 0;
double temperature_delta = 0.5; // 02/23/08.  Restored 07/12/09 to avoid blinks resulting in Det signal ripple.   03/15/17
//  double temperature_delta = 0.0; // 02/23/08  // let it blink to overcome inertia  P.P. 11/22/11 for GC322
if(((double) temperature)>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
data_value = 1023; // 5 V, ON/OFF
oven_on = 1;
Write_to_DAC(0, data_value);
}
else if(((double) temperature)<temp_oven) {
data_value = 0; // 0 V, ON/OFF
oven_on = 0;
Write_to_DAC(0, data_value);
}
else {  // do nothing
}
#else // with pulsing
//  double temperature_delta = 3.0; // added. P.P. 04/21/07
double temperature_delta = 4.0; // added. P.P. 04/21/07
if(temperature>=90.0) {
temperature_delta = 1.0;
}
else if(temperature>=80.0) {
//    temperature_delta = 1.5;
temperature_delta = 2.0;
}
else if(temperature>=70.0) {
//    temperature_delta = 1.0;
//    temperature_delta = 2.0;  // 08/26/07
temperature_delta = 3.0;  // 10/24/08
}
if(temperature>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
data_value = 1023; // 5 V, ON/OFF
//    oven_on = 1;
if(!oven_on) {  // avoid extra writings. P.P. 10/24/08
Write_to_DAC(0, data_value);
oven_on=1;
}
}
// more pulsing variants added. P.P. 03/30/07
//.  else if(temperature>(temp_oven+2.0)) {
else if(temperature>(temp_oven+temperature_delta/1.5)) {  // P.P. 04/21/07
if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
if(temp_delay4>0) {
temp_delay4+=(-2);  // corrected. P.P. 10/26.08
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay4=4; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
// end of new code
}
// end of added code. P.P. 03/30/07
//  else if(temperature>(temp_oven+0.50)) {
else if(temperature>(temp_oven+temperature_delta/6)) {
if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
if(temp_delay4>0) {
temp_delay4--;
//	temp_delay4+=(-2);
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay4=4; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
// end of new code
}
// code added 07/09/06
else if(temperature>(temp_oven)) {
if(!oven_on) {   // P.P. 06/25/06
if(temp_delay8>0) {
temp_delay8--;
//	temp_delay8+=(-2);  // P.P. 10/26/08
}
else {  // delay = 0 - turn on
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(0, data_value);
temp_delay8=8; // restore
oven_on = 1;
}
}
else { // oven on - turn it off. P.P. 07/08/06
data_value = 0; // 0 V,
temp_delay8=8; // restore
Write_to_DAC(0, data_value);
oven_on = 0;
}
}
// end 07/09/06
//  else if(temperature<(temp_oven)) {
//  else if((temperature+5.0)<temp_oven) {  // P.P. 06/18/06
else if((temperature+3.0)<temp_oven) {  // P.P. 04/21/07
if(oven_on) {  // cond added. P.P. 10/24/08
data_value = 0; // 0 V
Write_to_DAC(0, data_value);
oven_on = 0;
temp_delay4=4; // restore
temp_delay8=8;
}
}
else if((temperature<=(temp_oven))&&oven_on) {  // P.P. 06/18/06
data_value = 0; // 0 V
oven_on = 0;
Write_to_DAC(0, data_value);
temp_delay4=4; // restore
temp_delay8=8;
}
else {
}
// end of replacement. P.P. 07/07/06
#endif // no pulsing_oven. P.P. 05/01/13
#endif // gc312 or GC322// 04/13/08
}

// SetDetTemperature added for the case of PMD_12LS(). P.P. 07/12/06
static void SetDetTemperature(double temperature)
{
#if DEMO() // added. P.P. 10/05/04   disabled to test. P.P. 06/19/09
return;
#endif
uint data_value = 0;
//  if(temperature>(temp_det+1.0)) {
//  if(temperature<= 0) { // P.P. 03/07/18  added to definitely turn the heater OFF if themperature is set to 0.
if((temperature<= 0)||(temp_det < 0)) { // P.P. 03/07/18  added to turn the heater OFF if temperature is set to 0 or measured T <0, bad board!.
if(det_on) { // cond added. P.P. 10/24/08
data_value = 0; // 0 V, ON/OFF
Write_to_DAC(1, data_value);
det_on = 0;
}
}
if(temperature>(temp_det+1.0)) { // P.P. 09/07/12, to work with mechanical relay
if(!det_on) { // cond added. P.P. 10/24/08
data_value = 1023; // 5 V, ON/OFF
Write_to_DAC(1, data_value);
det_on = 1;
}
}
else if(temperature<temp_det) {
if(det_on) { // cond added. P.P. 10/24/08
data_value = 0; // 0 V, ON/OFF
Write_to_DAC(1, data_value);
det_on = 0;
}
}
else {
}
}
// end of added SetDetTemperature function for the case of pmd_12ls. P.P. 07/12/06

// separate (from injdet) desorber temp setting is added. P.P. 07/26/18
void SetDsbTemperature(double temperature)
{
#if DEMO() ||(!CONCENTRATOR())
return;
#endif
int err_code = 0;
if(temperature<=temp_conc) {
if(dsb_on) { // cond added. P.P. 10/24/08
err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, dsb_temperature_bit_number+8, FALSE); // changed here and below.  P.P. 03/29/17
dsb_on = 0;
}
}
else if(temperature>(temp_conc+1.0)) { // P.P. 03/28/17, to work with mechanical relay
if(!dsb_on) { //
err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, dsb_temperature_bit_number+8, TRUE); // changed here and below.  P.P. 03/29/17
dsb_on = 1;
}
}
else {
}
}
// end of desorber temperature setting code with 1208 board

// new code, to replace 1208 for USB-DIO24 in setting all tempratures. P.P. 03/09/18
#elif CBWUSB_IO()
// heated line case added. P.P. 3/28/17
static void SetLineTemperature(double temperature)
{
#if DEMO() ||(!HEATED_LINE())
   return;
#elif (!DEMO()) && HEATED_LINE()   // case added. P.P. 08/29/17
//#endif
  uint data_value = 0;
  int err_code = 0;
  if(temperature<=temp_line) {
    if(line_heat_on) { // cond added. P.P. 10/24/08
      err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, line_heat_bit_number+8, FALSE); // changed here and below.  P.P. 03/29/17
      line_heat_on = 0;
    }
  }
  else if(temperature>(temp_line+1.0)) { // P.P. 03/28/17, to work with mechanical relay
    if(!line_heat_on) { //
      err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, line_heat_bit_number+8, TRUE); // changed here and below.  P.P. 03/29/17
      line_heat_on = 1;
    }
  }
  else {
  }
#else
  return;
#endif
}
// end of heated line code
// added separate desorber Temp control. P.P. 07/26/18
static void SetDsbTemperature(double temperature)
{
#if DEMO() ||(!CONCENTRATOR())
   return;
#endif
  int err_code = 0;
  if(temperature<=temp_conc) {
    if(dsb_on) { // cond added. P.P. 10/24/08
      err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, dsb_temperature_bit_number+8, FALSE); // changed here and below.  P.P. 03/29/17
      dsb_on = 0;
    }
  }
  else if(temperature>(temp_conc+1.0)) { // P.P. 03/28/17, to work with mechanical relay
    if(!dsb_on) { //
      err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, dsb_temperature_bit_number+8, TRUE); // changed here and below.  P.P. 03/29/17
      dsb_on = 1;
    }
  }
  else {
  }
}

//static void SetOvenTemperature(long temperature)
static void SetOvenTemperature(double temperature)  // P.P. 07/27/18 for the case of USB_IO, 24-bit DIO board, no 1208 board.
{
#if DEMO() // added. P.P. 10/05/04
   return;
#endif
  _pMainWindow->print("SetTemperature %ld\r\n", temperature);
////// new. P.P. 11/11/05
#if !PULSING_OVEN()// no pulsing at all. P.P. 10/24/08  // temp, to exclude pulsing in GC301c-25points with Omron elay. P.P. 06/08/11  c0nd now P.P.03/07/17
  uint data_value = 0;
  double temperature_delta = 0.5; // 02/23/08.  Restored 07/12/09 to avoid blinks resulting in Det signal ripple.
//  double temperature_delta = 0.0; // 02/23/08  // let it blink to overcome inertia  P.P. 11/22/11 for GC322 returned back to delta=0, with SSR, let it link. P.P. 10/05/17
  if(((double) temperature<=0)||(temp_oven < 0)) {   // if board is bad, it shows  T<0. Turn heater OFF! P.P. 03/07/18
    My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
    oven_on = 0;
  }
  else if(((double) temperature)>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
    oven_on = 1;
     My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
  }
  else if(((double) temperature)<temp_oven) {
    oven_on = 0;
    My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
  }
  else {  // do nothing
  }
#else // pulsing
  uint data_value = 0;
//  double temperature_delta = 2.0; // P.P. 05/06/11
  double temperature_delta = 1.0; // P.P. 05/06/11
  if(temperature>(temp_oven+temperature_delta)) {   // changed P.P. 04/21/07
    if(!oven_on) {  // avoid extra writings. P.P. 10/24/08
      My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
      oven_on=1;
    }
  }
// more pulsing variants added. P.P. 03/30/07
  else if(temperature>(temp_oven+temperature_delta/1.5)) {  // P.P. 04/21/07
    if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
      if(temp_delay4>0) {
	temp_delay4+=(-2);  // corrected. P.P. 10/26.08
      }
      else {  // delay = 0 - turn on
	My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
	temp_delay4=4; // restore
	oven_on = 1;
      }
    }
    else { // oven on - turn it off. P.P. 07/08/06
      temp_delay4=4; // restore
      My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      oven_on = 0;
    }
// end of new code
  }
// end of added code. P.P. 03/30/07
//  else if(temperature>(temp_oven+0.50)) {
  else if(temperature>(temp_oven+temperature_delta/6)) {
    if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
      if(temp_delay4>0) {
	temp_delay4--;
//	temp_delay4+=(-2);
      }
      else {  // delay = 0 - turn on
	My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
	temp_delay4=4; // restore
	oven_on = 1;
      }
    }
    else { // oven on - turn it off. P.P. 07/08/06
      temp_delay4=4; // restore
      My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      oven_on = 0;
    }
// end of new code
  }
// code added 07/09/06
  else if(temperature>(temp_oven)) {
    if(!oven_on) {   // P.P. 06/25/06
      if(temp_delay8>0) {
	temp_delay8--;
//	temp_delay8+=(-2);  // P.P. 10/26/08
      }
      else {  // delay = 0 - turn on
	My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
	temp_delay8=8; // restore
	oven_on = 1;
      }
    }
    else { // oven on - turn it off. P.P. 07/08/06
      temp_delay8=8; // restore
      My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      oven_on = 0;
    }
  }
// end 07/09/06
//  else if(temperature<(temp_oven)) {
//  else if((temperature+5.0)<temp_oven) {  // P.P. 06/18/06
  else if((temperature+1.0)<temp_oven) {  // P.P. 05/06/11
    if(oven_on) {  // cond added. P.P. 10/24/08
     My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      oven_on = 0;
      temp_delay4=4; // restore
      temp_delay8=8;
    }
  }
  else if((temperature<=(temp_oven))&&oven_on) {  // P.P. 06/18/06
    oven_on = 0;
    My_cbDBitOut (USB_IO, FIRSTPORTA, oven_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
    temp_delay4=4; // restore
    temp_delay8=8;
  }
  else {
  }
// end of new code. P.P. 05/06/``
#endif // pulsing
///// end of new
}

// SetDetTemperature added for the case of USB_IO. P.P. 07/27/18
static void SetDetTemperature(double temperature)
{
#if DEMO() // added. P.P. 10/05/04   disabled to test. P.P. 06/19/09
   return;
#endif
//  temp_det=Get_Temperature(1); // this is measured somewhere else.
  uint data_value = 0;
//  if(temperature>(temp_det+1.0)) {
//  if(temperature<= 0) { // P.P. 03/07/18  added to definitely turn the heater OFF if themperature is set to 0.
  if((temperature<= 0)||(temp_det < 0)) { // P.P. 03/07/18  added to turn the heater OFF if temperature is set to 0 or measured T <0, bad board!.
    if(det_on) { // cond added. P.P. 10/24/08
      My_cbDBitOut (USB_IO, FIRSTPORTA, det_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      det_on = 0;
    }
  }
//  if(temperature>(temp_det+0.5)) {  // gap decreased to maintain Tdet better. P.P. 10/24/08
//  if(temperature>(temp_det)) {  // gap removed to maintain Tdet better. P.P. 11/02/11
  else if(temperature>(temp_det+1.0)) { // P.P. 09/07/12, to work with mechanical relay
    if(!det_on) { // cond added. P.P. 10/24/08
      My_cbDBitOut (USB_IO, FIRSTPORTA, det_temperature_bit_number+16, TRUE); // changed here and below.  P.P. 03/29/17
      det_on = 1;
    }
  }
  else if(temperature<temp_det) {
    if(det_on) { // cond added. P.P. 10/24/08
      My_cbDBitOut (USB_IO, FIRSTPORTA, det_temperature_bit_number+16, FALSE); // changed here and below.  P.P. 03/29/17
      det_on = 0;
    }
  }
  else {
  }
}
// end of added SetDetTemperature function for the case of usb-io. P.P. 03/11/18
// end of new code for USB-dio24 in setting temperatures. P.P. 03/09/18

#elif MANUAL_INJ()&&STAND_ALONE() // stand_alone with temperature control. P.P. 01/04/16
static void SetOvenTemperature(double temperature)
{
#if DEMO() // added. P.P. 10/05/04
   return;
#endif
//  if(temperature>(temp_oven+1.0)) {   // P.P. 06/16/06  // rem 01/05/16
#if PMD_16FS()
    int err_message = 0;
#if !PULSING_OVEN()  // P.P 01/05/16
    double temperature_delta = 0.5; // to avoid blinks resulting in Det signal ripple.
//    double temperature_delta = 1.0; // to avoid blinks resulting in Det signal ripple.
    if(temperature>(temp_oven+temperature_delta)) {
      if(!oven_on) {  // turn on
        err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 1); // out bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          oven_on = 1;
        }
      }
      else { // aready on - do nothing
      }
    }
    else if(oven_on&&(temperature<temp_oven)) { // turn off
      if(oven_on) {
        err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          oven_on = 0;
        }
      }
      else {  // already off - do nothing
      }
    }
    else {
    }
#else // pulsing for  stand alone with Toven control. // P.P. 01/05/16
//    double temperature_delta = 1.0; // P.P. 01/05/16
//    double temperature_delta = 4.0; // P.P. 01/05/16
    double temperature_delta = 2.0; // P.P. 01/05/16
    if(temperature>(temp_oven+temperature_delta)) {
      if(!oven_on) {  // turn on
        err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 1); // out bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          oven_on = 1;
        }
      }
      else {  // already on - do nothing
      }
    }
    else if(temperature>(temp_oven+temperature_delta/1.5)) {  // P.P. 04/21/07
      if(!oven_on) {   // P.P. 06/25/06
// new code to pulse power. P.P. 07/08/06
        if(temp_delay4>0) {
	  temp_delay4+=(-2);  // corrected. P.P. 10/26.08
        }
        else {  // delay = 0 - turn on
          err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 1); // out bit 5 in PMD_1608FS, P.P. 05/13/06
          if(err_message) { // DEBUG
            Str str="USB_1608FS error";
//          MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
          }
          else {
	    temp_delay4=4; // restore
	    oven_on = 1;
          }
	}
      }
      else { // oven on - turn it off. P.P. 07/08/06
        err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          temp_delay4=4; // restore
	  oven_on = 0;
        }
      }
    }
    else if(temperature>(temp_oven+temperature_delta/6)) {
      if(!oven_on) {   // P.P. 06/25/06
	if(temp_delay4>0) {
	  temp_delay4--;
//	  temp_delay4+=(-2);
        }
        else {  // delay = 0 - turn on
	  err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 1); // out bit 5 in PMD_1608FS, P.P. 05/13/06
          if(err_message) { // DEBUG
            Str str="USB_1608FS error";
//          MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
          }
          else {
	    temp_delay4=4; // restore
	    oven_on = 1;
          }
	}
      }
      else { // oven on - turn it off. P.P. 07/08/06
	err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          temp_delay4=4; // restore
	  oven_on = 0;
        }
      }
    }
// code added 07/09/06
    else if(temperature>(temp_oven)) {
      if(!oven_on) {   // P.P. 06/25/06
        if(temp_delay8>0) {
	  temp_delay8--;
//	  temp_delay8+=(-2);  // P.P. 10/26/08
	  temp_delay4=4; // restore
	}
        else {  // delay = 0 - turn on
	  err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 1); // out bit 5 in PMD_1608FS, P.P. 05/13/06
          if(err_message) { // DEBUG
            Str str="USB_1608FS error";
//          MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
          }
          else {
	    temp_delay8=8; // restore
	    temp_delay4=4; // restore
	    oven_on = 1;
          }
	}
      }
      else { // oven on - turn it off. P.P. 07/08/06
	err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
          temp_delay8=8; // restore
	  temp_delay4=4; // restore
	  oven_on = 0;
        }
      }
    }
    else if((temperature+1.0)<temp_oven) {  // P.P. 05/06/11
      if(oven_on) {  // cond added. P.P. 10/24/08
	err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
        if(err_message) { // DEBUG
          Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
        }
        else {
	  oven_on = 0;
	  temp_delay8=8; // restore
	  temp_delay4=4; // restore
	}
      }
    }
    else if((temperature<=(temp_oven))&&oven_on) {  // P.P. 06/18/06
      err_message = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 5, 0); // off bit 5 in PMD_1608FS, P.P. 05/13/06
      if(err_message) { // DEBUG
        Str str="USB_1608FS error";
//        MessageBox(NULL,str, "wPeak: USB_1608FS DIO error", MB_OK); rem P.P. 11/25/06
      }
      else {
	oven_on = 0;
	temp_delay8=8; // restore
	temp_delay4=4; // restore
      }
    }
    else {
    }
#endif // !pulsing_oven. P.P. 01/05/16
#endif // PMD_16 P.P. 05/13/06
//  } // added. 07/27/08  // rem 1/05/16
}

#else // !PMD_12LS and !CBWUSB_IO. P.P. 08/29/18
#endif // PMD_12LS
//#endif // !stand_alone, added. P.P. 07/14/03  rem temp 07/12/06
void SetTemperatures(double t_oven, double t_inj) // for TCTRL.CPP   // modified. P.P. 05/02/02
{
#if DEMO() // added. P.P. 10/05/04
    return;
#endif
    _pMainWindow->print("SetTemperatures %f %f\r\n", t_oven, t_inj);
#if !STAND_ALONE() // added. P.P. 07/14/03
    //#if PMD_12FS()||PMD_12LS() // set temp via analog output. P.P. 03/10/06
#if PMD_12LS()||CBWUSB_IO() // set temp via analog or digital output. P.P. 08/29/18
#if !OVEN_EQ_INJ()
    SetOvenTemperature((long) t_oven);
   SetDetTemperature((long) t_inj);
#else // oven = inj
    SetOvenTemperature((long) t_oven); // P.P. 03/10/06
#endif // oven!=inj
#if HEATED_LINE()
    void SetLineTemperature(double temperature); // prototype   P.P. 07/18/18
   Method *pMethod;
   double t_line = pMethod->method_table.line_temp;
   SetLineTemperature((long) t_line);  // P.P. 03/29/17
#endif // heated line
#else  // do n1208 0r cbwusb_io. P.P. 08/29/18
#endif // !stand_alone. added P.P. 07/14/03
}

static void SetCalValve(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
My_cbDBitOut (4, FIRSTPORTA, 2, on? FALSE:TRUE); // changed here and below.  P.P. 03/13/18
Set_valve(fluidics_port_offset, cal_bit_number, on? FALSE:TRUE); // opposite to other fluidics

#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if !(STAND_ALONE()||GC312()) // changed - no cal valve in portable. P.P. 03/18/06
#if PMD_12LS()||CBWUSB_IO() // P.P. 03/13/18
//Set_valve(fluidics_port_offset, cal_bit_number, on? FALSE: TRUE); // opposite to other fluidics
#endif //usb boards
//  #else
#endif // !STAND_ALONE(). added. P.P. 08/11/03
}


#if CBW() // condition added. P.P. 07/21/03  (only with DAS802. 05/11/07)
#if (PMD_16FS()||PMD_12LS()||PMD_12FS()) // one of PMD boards, without DAS_802. P.P. 06/17/05
//static int32 Get_Data(int nDetector, int gain) //
static int16 Get_Data(int nDetector, int gain) //    For transfer with gain bits in a 32-bit number. P.P. 11/09/14
{
#if DEMO() // added. P.P. 10/05/04
    return 5000;
#elif MYDEBUG()  // replacement. P.P. 10/05/04
    //  #if MYDEBUG()
  {
  _i_demo++;
  int32 data_in=nDetector==0?(7750+(-cos((_i_demo-350)*0.06)/8+cos((_i_demo-350)*0.04)/3-cos((_i_demo-350)*0.02)/4+cos((_i_demo-350)*0.01))*4000*_ran):
                       (2000-cos(_J_demo++*0.02)*200*_ran);
  return data_in/pow10(gain);
  }
#endif // demo or mydebug

#if TEMP_GRAPH() //GC321() // temp, to show oven temperature in graph B. P.P. 06/25/06  disabled. P.P. 08/29/12
    //  if(!nDetector) {
  if(nDetector) {    // P.P. 12/02/11 Det B graph for oven temp.
    return (int32) (temp_oven*100.00);
  }
#endif // temp_graph

    short channel=2+nDetector; // ??????
    uint16 data_in=0;   //
    uint32 sum_data_in=0;   // P.P.
#if PMD_16FS() // cond added. P.P. 05/28/06
//  int max_meas_number = 4;   //  P.P. 01/02/06
    int max_meas_number = 1;   //  P.P. 10/24/08
#else
    int max_meas_number = 1;   // P.P. 02/28/06
#endif
#if Get_Data_Bipolar()
    int Gain = BIP10VOLTS; // default.
/* // rem. Later can be changed to get x1000, with previous x100. P.P. 01/02/06
  if(gain==1) {  // added to switch gain of the PMD. Only Range 100 and 10. P.P. 11/28/04
      Gain = BIP1VOLTS;  // no other gain setting - good for external signals. P.P. 11/28/04
  }
*/
#else  // not available - only bipolar for these PMDs. P.P. 11/28/04
#endif
// to be changed, gain can be set by dig outputs from the same PMD. P.P. 11/06/04
// can be done in Set_alarm_bit or directly with MyDOut.
    // rem temp. P.P. 11/07/04 Restored 01/02/06
// void Set_alarm_bit(int port_offset, byte bit_number, BOOL on);
    int err_code=0;
// code added to control both A and B detectors. P.P. 04/02/06
    int gain_bit_1 = 2 + 4*nDetector;
    int gain_bit_2 = 3 + 4*nDetector;
    int gain_bit_3 = 3 + nDetector; // P.P. 12/11/09, for range 1000.
#if QUICK_SCAN() // re-define gain bits. P.P. 01/01/11
    if((nDetector==2)||(nDetector==3)) { // det quick scan detector, nDetector = 2, data written into det A // P.P. 01/08/11
   gain_bit_1 = 2 + 4*(nDetector-2);
   gain_bit_2 = 3 + 4*(nDetector-2);
   gain_bit_3 = 3 + (nDetector-2); // P.P. 12/11/09, for range 1000.
 }
#endif
    static int gain_old[2] = {-1, -1};  //moved here from below. P.P. 08/15/03
    if(gain !=gain_old[nDetector]) { // added. P.P. 04/02/06
        if(gain == 0) { // max amplification = 100
#if PMD_16FS() // then via aux port. P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_2, TRUE); // direct.  P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_1, FALSE); //  P.P. 03/10/06
#if PMD_12LS()
            err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 12/11/09
#elif CBWUSB_IO()
            err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 03/11/18
#endif // 12LS or usb-io
#else  // nothing to do
#endif
        }
        else if(gain == 1) {  // amplification 10
#if PMD_16FS() // then via aux port. P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_2, FALSE); // direct.  P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_1, TRUE); //  P.P. 03/10/06
#if PMD_12LS()
            err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 12/11/09
#elif CBWUSB_IO()
            err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 03/11/18
#endif // 12LS
#else  // nothing to do
#endif
        }
        else if(gain == 2) {  // amplification 1
#if PMD_16FS() // then via aux port. P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_2, FALSE); // direct.  P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_1, FALSE); //  P.P. 03/10/06
#if PMD_12LS()
            err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 12/11/09
#elif CBWUSB_IO()
            err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, gain_bit_3, FALSE); //  P.P. 03/11/18
#endif // 12LS
#else  // nothing to do
#endif
        }
        else if(gain == 3) {  // amplification 1/10, for new range 1000
//#if PMD_12LS() // cond added. P.P. 03/10/06
#if PMD_16FS() // then via aux port. P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_2, FALSE); // direct.  P.P. 03/10/06
            err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_1, FALSE); //  P.P. 03/10/06
//     err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, gain_bit_1, TRUE); //  P.P. 06/17/11 fixed P.P. 10/31/12
#if PMD_12LS()
            err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, gain_bit_3, TRUE); //  P.P. 12/11/09 divide by 100
#elif CBWUSB_IO()
            err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, gain_bit_3, TRUE); //  P.P. 03/11/18
#endif // 12LS
#else  // nothing to do
#endif
        }
        gain_old[nDetector] = gain;
    } // if gain != gain_old. P.P. 04/02/06
///// End of additional gain setting. P.P. 09/07/02

//  for(int i=0; i<max_meas_number; i++) {
#if STAND_ALONE() // case added. P.P. 06/03/06
    if(gain==2) Gain= BIP10VOLTS;
  else if(gain==1) Gain= BIP1VOLTS;
  else if(gain==0) Gain= BIP1VOLTS;
  else {}
#endif // stand_alone
    for(int i=0; i<max_meas_number; i++) { // P.P. 11/07/04
#if PMD_16FS()
        My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
        sum_data_in += (uint32) data_in;  //  P.P. 06/20/11
#elif PMD_12FS()
        My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&data_in);
//    sum_data_in += (data_in&0x0FFF);  // 12-bit A/D. P.P. 11/07/04
    sum_data_in += (data_in&0x0FFF)*16;  // variant. P.P. 07/27/08
#elif PMD_12LS()
    My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data_in);
//    sum_data_in += (data_in&0x0FFF);  // 12-bit A/D. P.P. 11/07/04
    sum_data_in += (data_in&0x0FFF)*16;  // variant. P.P. 11/28/04
#endif // pmd_16
    }
    data_in = (uint16) (sum_data_in/max_meas_number); // averaging. P.P. 05/28/06
#if Get_Data_Bipolar() // P.P. 07/05/02
    int16 data_out = 0;
    if(data_in > 32767) {
        data_out = data_in - 32768U;  // for +-10V , 0 is in the middle. P.P. 07/05/02
    }
    else {
        data_out = data_in - 32767U; // P.P. 07/05/02
    }
//  if(nDetector) {              // added to save and use until updated. P.P. 11/28/04
    if((nDetector==1)||(nDetector==3)) { // DetB or quick_scan det B. P.P. 01/08/11
#if IN_4_20() // new cond to use 4-20 mA input shifted to zero. P.P. 05/22/09
        data_out = data_out - 5636U; // 4mA*430 Ohm = 1.72 V-> 5636 counts. P.P. 05/22/09
    if(data_out<-32767) data_out = -32767; // to prevent < than -10V
#endif // 4_20_IN()
        data_B = (int32)data_out;  //
    }                            //
//  else {                       //
    else if((nDetector==0)||(nDetector==2)) { // Der A or quick_scan det A. P.P. 01/08/11
        data_A = (int32)data_out;  //
    }                            //
    return (int32)data_out;
#else // unipolar, added 05/22/09
    return 0;
#endif // P.P. bipolar. added here 06/25/06
}

// New code for battery voltage monitoring. P.P. 04/07/06
#if GC312() // cond added. P.P. 04/07/06
static double GetBatteryVoltage(int channel) //
{
#if DEMO()||MYDEBUG()
//   return 12.0;
   return 6.0;     // replaced - mult by 2 later. P.P. 04/10/06
#endif // demo or mydebug
  uint16 data_in=0;
  uint32 sum_data_in=0;
#if PMD_16FS()
//  int max_meas_number = 4;   //  for PMD_16FS board
  int max_meas_number = 1;   //  for PMD_16FS board   // P.P. 04/08/09
#elif PMD_12FS()||PMD_12LS()
  int max_meas_number = 1;   //  for PMD_12 boards
#endif // pmd boards
  int Gain = BIP10VOLTS; // default.
  int err_code=0;
  for(int i=0; i<max_meas_number; i++) { // P.P. 11/07/04
#if PMD_16FS()
    My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
    sum_data_in += data_in;  //
#elif PMD_12FS()
    My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&data_in);
    sum_data_in += data_in&0x0FFF)*16;  // variant. P.P. 11/28/04
#elif PMD_12LS()
    My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data_in);
    sum_data_in += (data_in&0x0FFF)*16;  // variant. P.P. 11/28/04
#endif // pmd_16
  }
#if PMD_16FS()
//  data_in = (uint16) (sum_data_in/4); // averaging. P.P. 04/07/06
  data_in = (uint16) (sum_data_in); // no averaging. P.P. 04/08/09
#elif PMD_12FS()
  data_in = (uint16) (sum_data_in); //
#endif
#if Get_Data_Bipolar()
  int16 data_out = 0;
  if(data_in > 32767) {
    data_out = data_in - 32768U;  // for +-10V , 0 is in the middle.
  }
  else {
    data_out = data_in - 32767U; //
  }
  const double vmin=-10.0; // gain=1
  const double vmax=+10.0;
  double voltage = (((int32) data_out) *(vmax-vmin))/65536.0;
  battery_voltage = voltage;
  return voltage;
#else  // unipolar signal
  double voltage = (10.0*data_in)/65536.0;
  battery_voltage = voltage;
  return voltage;
#endif // bipolar
}
#endif // GC312. P.P. 04/07/06
// end of new code for battery voltage monitoring. P.P. 04/07/06

// new function to measure gas pressure. P.P. 10/02/09
static double Get_Pressure(int channel) //
{
//#if DEMO()||MYDEBUG()
#if DEMO()||MYDEBUG()||(!PRESSURE_SENSOR())  // P.P. 10/23/09
    return 500;     //
#endif // demo or mydebug
    uint16 data_in=0;
    uint32 sum_data_in=0;
    int max_meas_number = 1;   //  for PMD_16FS board   // P.P. 04/08/09
    int Gain = BIP10VOLTS; // default.
    int err_code=0;
#if PMD_16FS()
    My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
#elif PMD_12FS()
    My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&data_in);
    sum_data_in += (uint32) ((data_in&0x0FFF)*16);
#elif PMD_12LS()
    My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data);
    sum_data_in += (uint32) ((data_in&0x0FFF)*16);
#endif // pmd_16
#if PMD_16FS() // added. P.P. 10/15/09
    // just use data_in. P.P. 10/15/09
#elif (PMD_12LS()||PMD_12FS())
    data_in = (uint16) (sum_data_in); // no averaging. P.P. 04/08/09
#endif // 12LS or 12FS
    double pressure=0;
#if Get_Data_Bipolar()
    int16 data_out = 0;
    if(data_in > 32767) {
        data_out = data_in - 32768U;  // for +-10V , 0 is in the middle.
    }
    else {
        data_out = data_in - 32767U; //
    }
    const double vmin=-10.0; // gain=1
    const double vmax=+10.0;
    double voltage = (((int32) data_out) *(vmax-vmin))/65536.0;
#if WATER_PUMP()
    pressure = voltage*25; // 0-100 psi range gives 0-4 V output. P.P. 01/30/17
#elif TWO_COLUMNS()
    pressure = voltage*25; // 0-100 psi range gives 0-4 V output. P.P. 03/24/17
#elif !GC312() // for 301 with some other pressure sensor. P.P. 06/26/17
//  pressure = voltage*2.275; // cal from Bret measurements. P.P. 06/28/17 added here from 2r062617. PP.P. 12/21/18
    pressure = (voltage-0.5)*25.0; //new cal from Bret. P.P. 12/21/18
#else
    pressure = (voltage-1)*625;
#endif
#else  // unipolar signal
    double voltage = (10.0*data_in)/65536.0;
  double pressure = (voltage-1)*625;
#endif // bipolar
    return pressure*1.5; // pressure for MSP 300-2k5-4-N-1 transducer. P.P. 10/02/09
}
// end of new function for measuring gas pressure. P.p. 10/02/09

// new function to measure H2 pressure. P.P. 11/24/17
static double Get_Pressure_H2(int channel) //
{
//#if DEMO()||MYDEBUG()
#if DEMO()||MYDEBUG()||(!H2_PRESSURE_SENSOR())  // P.P. 10/23/09
    return 500;     //
#endif // demo or mydebug
    uint16 H2_data_in=0;
    uint32 H2_sum_data_in=0;
    int max_meas_number = 1;   //  for PMD_16FS board   // P.P. 04/08/09
    int Gain = BIP10VOLTS; // default.
    int err_code=0;
#if PMD_16FS()
    My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&H2_data_in);
#elif PMD_12FS()
    My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&H2_data_in);
    H2_sum_data_in += (uint32) ((H2_data_in&0x0FFF)*16);
#elif PMD_12LS()
    My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&H2_data);
    H2_sum_data_in += (uint32) ((H2_data_in&0x0FFF)*16);
#endif // pmd_16
#if PMD_16FS() // added. P.P. 10/15/09
    // just use data_in. P.P. 10/15/09
#elif (PMD_12LS()||PMD_12FS())
    H2_data_in = (uint16) (H2_sum_data_in/max_meas_number); // P.P. 11/24/17
#endif // 12LS or 12FS
    double H2_pressure=0;
#if Get_Data_Bipolar()
    int16 data_out = 0;
    if(H2_data_in > 32767) {
        data_out = H2_data_in - 32768U;  // for +-10V , 0 is in the middle.
    }
    else {
        data_out = H2_data_in - 32767U; //
    }
    const double vmin=-10.0; // gain=1
    const double vmax=+10.0;
    double voltage = (((int32) data_out) *(vmax-vmin))/65536.0;
    H2_pressure = (voltage-1)*625;
#else  // unipolar signal
    double voltage = (10.0*data_in)/65536.0;
  double pressure = (voltage-1)*625;
#endif // bipolar
    return H2_pressure; // pressure for MSP 300-2k5-4-N-1 transducer. P.P. 10/02/09
}
// end of new function for measuring H2 pressure. P.p. 11/24/17

// new function to measure relative humidity. P.P. 06/26/17 here 07/27/17
static double Get_Humidity(int channel) //
{
#if DEMO()||MYDEBUG()||(!HUMIDITY_SENSOR())  // P.P. 06/26/17
    return 20;     //
#endif // demo or mydebug
    uint16 data_in=0;
    uint32 sum_data_in=0;
    int max_meas_number = 1;   //  for PMD_16FS board   // P.P. 04/08/09
    int Gain = BIP10VOLTS; // default.
    int err_code=0;
#if PMD_16FS()
    My_cbAIn(CBWDEVICE_16FS, (int) channel, Gain, (USHORT *)&data_in);
#elif PMD_12FS()
    My_cbAIn(CBWDEVICE_12FS, (int) channel, Gain, (USHORT *)&data_in);
    sum_data_in += (uint32) ((data_in&0x0FFF)*16);
#elif PMD_12LS()
    My_cbAIn(CBWDEVICE_12LS, (int) channel, Gain, (USHORT *)&data);
    sum_data_in += (uint32) ((data_in&0x0FFF)*16);
#endif // pmd_16
#if PMD_16FS() // added. P.P. 10/15/09
    // just use data_in. P.P. 10/15/09
#elif (PMD_12LS()||PMD_12FS())
    data_in = (uint16) (sum_data_in); // no averaging. P.P. 04/08/09
#endif // 12LS or 12FS
    double humidity=0;
#if Get_Data_Bipolar()
    int16 data_out = 0;
    if(data_in > 32767) {
        data_out = data_in - 32768U;  // for +-10V , 0 is in the middle.
    }
    else {
        data_out = data_in - 32767U; //
    }
    const double vmin=-10.0; // gain=1
    const double vmax=+10.0;
    double voltage = (((int32) data_out) *(vmax-vmin))/65536.0;
//  humidity = voltage*100; // 0-100 % range gives 0-2 V output. P.P. 06/26/17
    humidity = voltage*3.145 - 2.382; // Bret's calibration. P.P. 06/28/17
#else  // unipolar signal
    double voltage = (10.0*data_in)/65536.0;
//  double humidity = voltage*100;
  humidity = voltage*3.145 - 2.382; // Bret's calibration. P.P. 06/28/17
#endif // bipolar
    return humidity; // humidity 0-100 % 0-1 or 0-2 V. P.P. 06/26/17
}
// end of relative humidity function. P.P. 06/26/17

#endif // any of PMD. P.P. 11/06/04
#endif // cbw(). End of added code for Get_Data(). P.P. 07/21/03

void Acquire::SendEndAcquire()
{
    MethodTable &method_table=pMethod->method_table;
#if !STAND_ALONE() // added. P.P. 07/18/03
#if DEMO() // added. P.P. 10/05/04
    return;
#endif
/* // removed - lamp stays ON and can be turned OFF by a button. P.P. 03/31/07
  // Next line uncommented to leave Lamp ON in continuous mode (if it was ON). P.P. 12/21/01
  if(runMode.Acquire!=1) { // not Continuous mode
#if !(GC321()||GC312()) // cond added. P.P. 07/19/06
    SetLamps(FALSE); // removed temp. P.P. 10/03/03. Restored 12/14/03 restored 07/19/06
#endif // !gc321 or gc312
  }                                       // and back - P.P. 08/26/03. Rem 06/21/06 permanently
*/
    Inject(FALSE);
#if CONCENTRATOR()     // added 02/05/06. P.P.
    void SetDsbTemperature(double t);
  Purge(FALSE);        //
//  Concentrator(FALSE); // set temp, then disable. P.P. 03/07/18
//  SetDetTemperature(0); // added. P.P. 03/31/17
  SetDsbTemperature(0); // added. P.P. 07/27/18
  Concentrator(FALSE); //
//  Set_valve(point_port_offset, injectV4_bit_number, FALSE);  // added P.P. 02/20/06
  Set_valve(point_port_offset, injectV4_bit_number+8, FALSE);  // using portA notaton P.P. 03/08/17
  inj_V4_on = FALSE; // added. P.P. 03/18/17
//  Set_valve(point_port_offset, injectV5_bit_number, FALSE);  // rem as not needed. P.P. 03/04/06
  Fan(FALSE); // P.P. 02/23/10
#endif // Concentrator //

//  if((acquire_type != CAL)||(runMode.Acquire!=1)||!bRepeatCalSend) { // cond added. P.P. 03/23/05. Removed 06/14/05
    SetCalValve(TRUE); // P.P. 10/12/02. Added to turn it OFF on STOP button if it was ON.
//  }
//#if !CONCENTRATOR() // cond added. P.P. 03/04/06  cond rem 03/17/17
    Equilibrate(FALSE); // added. P.P. 10/30/02  This also flushes 1st column. P.P. 01/21/17
//#endif // !concentrator
//#if GC501()         // added. P.P. 04/25/04 // Case removed - standard GC! P.P. 07/10/05
//  SamplePump(FALSE);
    AutoZero(FALSE); // added. P.P. 07/30/04
    AutoZeroB(FALSE); // added. P.P. 06/21/17
//#else
    SamplePump((acquire_type != CAL) && method_table.cont_pump_on_off);
#if WATER_PUMP() // water pump and 2 columns use autozeroB DO bit - should be moved somewhere else!. P.P. 07/27/18
    WaterPump(pointnumber, FALSE); // P.P. 01/04/17
#if PMD_12LS()
  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, flush 2nd column (OFF). P.P. 01/07/17
#elif CBWUSB_IO()
  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 0); // bit 7, flush 2nd column (OFF). P.P. 03/11/18
#endif
#elif TWO_COLUMNS()  // case added. P.P. 03/24/17
    #if PMD_12LS()
  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, flush 2nd column (OFF). P.P. 03/24/17
#elif CBWUSB_IO()
  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 0); // bit 7, flush 2nd column (OFF). P.P. 03/11/18
#endif
#endif // water pump
//#endif
    ///added to open the oven. P.P. 04/02/02
    door_manual = method_table.oven_door_manual; // on start, read the check box. P.P. 09/07/12
#if RAMPING() // changed. P.P. 04/01/08
    if(!method_table.oven_door_manual) {
    if(pMethod->method_table.RampingEnabled)  {  // changed. P.P. 05/18/02
///   added to open oven only if > 3 degree than initial. P.P. 08/09/02
//      double meas_temperature=Get_Temperature(0); // for Oven Temp. replaced 11/18/05
      temp_oven=Get_Temperature(0); // for Oven Temp. P.P. 07/03/05. restored. 11/18/05
//      const double oven_door_delta = 3; //
      const double oven_door_delta = 1; // P.P. 10/24/08
//      if((meas_temperature - method_table.oven_temp) > oven_door_delta){ // Oven Door. P.P. 08/09/02
      if((temp_oven - method_table.oven_temp) > oven_door_delta){ // Oven Door. P.P. 07/03/05
        if(!oven_door_on) {
	  OvenDoor(TRUE);    // open to cool down the oven. P.P.
//         dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
	  dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
	  DoorMotor(TRUE);
        }
      }
    }
  }
#endif ///*GC321() or*/ Ramping
    /// end
#if X1_VALVE() // P.P. 06/18/06
    dwXvalveOffTime=0;
    dwXvalveTime=0;
#endif // X1_valve
#if FID() // added. P.P. 07/03/05
    //  FlameDetector(FALSE); rem - this done in ~acquire. P.P. 10/02/05
#endif // fid
    End_run = 1;
//#if DDA()   // point alarms usage for diagnostics in 1 point units P.P. 11/06/02
//  }                       // rem 12/08/02
// end. P.P. 12/06/02
#if QUICK_SCAN() // case added. P.P. 05/13/11
    void InfoRun(BOOL on);
    void InfoCal(BOOL on);
    InfoRun(FALSE);
    InfoCal(FALSE);
#endif // quick scan
#else //stand_alone case. P.P. 07/18/03
#endif // !stand_alone
    TRACE1("Acquire::SendEndAcquire =%p", this);
        My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, 1, DIGITALOUT);
    My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 1, 1);
}

void Acquire::Inject(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if !STAND_ALONE() // added. P.P. 08/11/03
#if PMD_16FS()||PMD_12LS()||CBWUSB_IO() // 2 lines added to work with PMDs. P.P. 03/13/18
Set_valve(fluidics_port_offset, inject_bit_number, on? TRUE: FALSE);
#endif //PMD board
#endif // !stand_alone. added P.P. 08/11/03
if(on) {          // added. P.P. 03/14/17
inject_on = TRUE;
}
else {
inject_on = FALSE;
}
}

// new function for Hydrogen valve. P.P. 06/28/05
void Acquire::HydrogenValve(BOOL on) // P.P. 06/28/05
{
//#if DEMO()||MYDEBUG()
#if DEMO()||MYDEBUG()||(!FID()) // P.P. 01/05/09 restored 03/24/10 for GC312 with FID
//#if DEMO()||MYDEBUG()||(!FID())||GC312() // P.P. 06/19/09 restore if manual ON/OFF hydrogen valve. P.P. 11/22/17
return;
#endif
#if !STAND_ALONE()
#if PMD_16FS()||PMD_12LS()||PMD_12FS()||CBWUSB_IO() //  PMD or USB_IO without DDA.P..P. 03/09/18
Set_valve(fluidics_port_offset, hydrogen_bit_number, on? TRUE: FALSE);
#endif // PMD  or cbwusb_io
#endif // !stand_alone. added P.P. 08/11/03
}
// end of Hydrogen valve. P.P. 06/28/05

/// new function for makeup valve. Copied from Inject(..). P.P. 01/05/04  Not done yet for cbwusb_io(). )3/13/18
void Acquire::Makeup(BOOL on)
{
//#if DEMO() // added. P.P. 10/05/04
#if DEMO()||GC312() // added. P.P. 03/18/06
return;
#endif
#if !STAND_ALONE() //
// #if !DDA()
// #if (!DDA() && !DAQ2000())
#if PMD_16FS()||PMD_12LS() // 2 lines added to work with PMDs. P.P. 11/28/04
//#if !GC312() // cond added - no extra bits in PMD1606FS. P.P. 04/02/06
//#if !(GC312()||GC321()) // changed P.P. 06/09/06
#if !(GC312||GC321()||GC326()) //  P.P. 12/07/18
#if CONCENTRATOR() // case added. P.P. 01/01/10
Set_valve(point_port_offset, makeup_bit_number, on? TRUE: FALSE); // in portB of 1208 board
#else
//  Set_valve(fluidics_port_offset, makeup_bit_number, on? TRUE: FALSE); // rem, used for diagn int alarm in GC301c. P.P. 01/01/10
#endif // concentrator
#endif // gc312
#endif //PMD
#endif // !stand_alone. added P.P. 08/11/03
}
/// end of makeup valve function. P.p. 01/05/04

// 4 new functions added for x1-4 valves. P.P. 06/20/10 here 11/12/10
void Acquire::X1_valve(BOOL on)
{
#if DEMO() //
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, x1_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, x1_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
}
void Acquire::X2_valve(BOOL on)
{
#if DEMO()
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, x2_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, x2_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
}

void Acquire::X3_valve(BOOL on)
{
#if DEMO()
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, x3_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, x3_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
}

void Acquire::X4_valve(BOOL on)
{
#if DEMO()
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, x4_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, x4_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
}

void Acquire::X5_valve(BOOL on)   // added P.P. 07/01/10
{
#if DEMO()
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, x5_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, x5_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
X5_valve_on = on;   // P.P. 05/13/18
}
// end of 4 new functions for x2-5 valves. P.P. 07/01/10

// new function, for zero gas valve. P.P. 03/17/11
void Acquire::ZeroGasValve(BOOL on)   // added P.P. 03/17/11
{
#if DEMO()
//#if DEMO()||(!QUICK_SCAN()) // P.P. 04/01/11 cond rem 05/13/11 - can be used without QS
return;
#endif
#if !STAND_ALONE() //
#if PMD_16FS()&&PMD_12LS() //
int err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, zero_gas_bit_number+8, on? 1: 0); // out bit in portB of PMD_1208LS, added. P.P. 06/20/10
#elif PMD_16FS()&&CBWUSB_IO() //
int err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, zero_gas_bit_number+8, on? 1: 0); // out bit in portB of USB-DIO24, added. P.P. 03/11/18
#endif // pmd_16 and pmd_12 together.
#endif // !stand_alone.
}
/// 2 new functions for quick scan case and one for zero gas. P.P. 05/13/11
#if QUICK_SCAN()
void InfoRun(BOOL on)
{
#if DEMO() //
   return;
#endif
#if USB_DIO24()||USB_1024LS() //
  int err_code = 0;
  if(((!run_info_state)&&on)||(run_info_state&&(!on))) {
    err_code = My_cbDBitOut (CBWDEVICE_1024, FIRSTPORTA, run_info_bit_number+16, on? 1: 0); // in portC of USB-DIO24, added. P.P. 05/13/11
  }
  run_info_state = on? 1: 0;
#endif //
}

void InfoCal(BOOL on)
{
#if DEMO() //
   return;
#endif
#if USB_DIO24()||USB_1024LS() //
  int err_code = 0;
  if(((!cal_info_state)&&on)||(cal_info_state&&(!on))) {
    err_code = My_cbDBitOut (CBWDEVICE_1024, FIRSTPORTA, cal_info_bit_number+16, on? 1: 0); // in portC of USB-DIO24, added. P.P. 05/13/11
  }
  cal_info_state = on? 1: 0;
#endif // pmd_16 and pmd_12 together.
}
#endif // quick_scan.

void InfoZeroGas(BOOL on)
{
//#if DEMO() //
#if DEMO()|1 // P.P. 06/18/11, disabled
return;
#endif
#if USB_DIO24()||USB_1024LS() //
int err_code = 0;
if(((!zero_gas_info_state)&&on)||(zero_gas_info_state&&(!on))) {
err_code = My_cbDBitOut (CBWDEVICE_1024, FIRSTPORTA, zero_gas_info_bit_number+16, on? 1: 0); // in portC of USB-DIO24, added. P.P. 05/13/11
}
zero_gas_info_state = on? 1: 0;
#endif // pmd_16 and pmd_12 together.
}
/// end of 3 new functions. P.P. 05/13/11

void Acquire::SetFlowMode(int flow_mode)
{
//#if DEMO() // added. P.P. 10/05/04
//#if DEMO()||CONCENTRATOR() // added. P.P. 02/15/10
//#if DEMO()||CONCENTRATOR()||1 // disabled as not used. P.P. 06/18/11
#if DEMO()||CONCENTRATOR()||!FLOW() // enabled in FLOW() case. P.P. 12/08/18
return;
#endif
switch(flow_mode) {    // Changed: Makeup energized only when it gows OFF. P.P. 03/05/04
case 0:
//      Makeup(TRUE);    //
Makeup(FALSE);     //
Inject(FALSE);
break;
case 1:
//      Makeup(TRUE);    //
Makeup(FALSE);     //
Inject(TRUE);
break;
case 2:
//    Makeup(FALSE);     //
Makeup(TRUE);      //
Inject(FALSE);
break;
case 3:
//      Makeup(FALSE);   //
Makeup(TRUE);      //
Inject(TRUE);
break;
}
}


void Acquire::SamplePump(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if !STAND_ALONE() // added. P.P. 08/11/03
#if PMD_16FS()||PMD_12LS()||CBWUSB_IO() // P.P. 03/13/18
Set_valve(fluidics_port_offset, sample_bit_number, on? TRUE: FALSE);
#endif //pmd
#endif // !stand_alone. P.P. 08/11/03
if(on) {          // added. P.P. 03/14/17
sample_on = TRUE;
}
else {
sample_on = FALSE;
}
}

///// New function to fire concentrator. P.P. 08/02/02. Added here and modified 02/05/06
void Acquire::Concentrator(BOOL on)  // since now it will serves to enable desorber heater. P.P. 01/02/10
{
//#if DEMO() // added. P.P. 10/05/04
#if DEMO()||(!CONCENTRATOR()) //  P.P. 01/29/17
return;
#endif
#if CONCENTRATOR() // P.P. 08/31/18
#if PMD_12LS()&&PMD_16FS()||CBWUSB_IO() // case added for GC301c with concentrator. P.P. 12/30/09
Set_valve(fluidics_port_offset, desorber_bit_number+8, on? TRUE: FALSE); // P.P. 02/18/10
#endif //pmd
#endif // concentrator
if(on) {          // added. P.P. 03/14/17
desorb_on = TRUE;
}
else {
desorb_on = FALSE;
}
}

//// P.P. 08/02/02. New function to purge the concentrator.
void Acquire::Purge(BOOL on)
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if CONCENTRATOR() // P.P. 08/31/18
#if PMD_12LS()&&PMD_16FS()||CBWUSB_IO() // P.P. 03/13/18
Set_valve(fluidics_port_offset, purge_bit_number+8, on? TRUE: FALSE); // P.P. 02/18/10
purge_on = (on? TRUE: FALSE);  // P.P. 03/16/17
#endif //pmd
#endif // concentrator
purge_on = (on? TRUE: FALSE); // added P.P. 03/18/17
}

// new function to control an additional fan to cool down the desorber. P.P. 02/23/10
void Acquire::Fan(BOOL on)      // note: fan can use dsb_fan_bit_number, leaving fan_bit_number for oven door. P.P. 08/31/18
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if CONCENTRATOR() // P.P. 08/31/18
#if PMD_12LS()&&PMD_16FS()||CBWUSB_IO() //  GC301c with concentrator.  P.P. 03/13/18
Set_valve(fluidics_port_offset, fan_bit_number+8, on? TRUE: FALSE); // P.P. 02/23/10  restored 08/31/18
//  Set_valve(fluidics_port_offset, dsb_fan_bit_number+8, on? TRUE: FALSE); // P.P. 07/27/18 use bit 1, leave bit 2 for oven door
fan_on = (on? TRUE: FALSE);  // P.P. 02/23/10
#endif //pmd-12ls and pmd-16fs
#endif // concentrator
}
// end of the Fan function. P.P. 02/23/10


void Acquire::TCDDetector(BOOL on)
{
TRACE1( "Acquire::TCDDetector(%d)", on );
//#if DEMO() // added. P.P. 10/05/04            restored - GC312 with TCD as Det B. P.P. 02/20/09
#if DEMO()||WATER_PUMP()||(!TCD()) // P.P. 01/21/17
return;
#endif
#if !STAND_ALONE() // added. P.P. 11/07/04
#if TCD() // cond added. P.P. 10/14/05
//  Set_alarm_bit(control_port_offset, TCD_bit_number, on? TRUE: FALSE); // P.P. 09/05/04, 09/21/04
Set_alarm_bit(control_port_offset, TCD_bit_number, on? FALSE: TRUE); // P.P. 02/20/09 fron now on
#endif // tcd
#endif
fan_on = (on? TRUE : FALSE); // added. P.P. 03/18/17
}

// new function to control temp enable relay contacts. P.P. 06/30/06
void Acquire::EnableOven(BOOL on)
{
//#if DEMO()||GC312()// added. P.P. 03/18/06
#if DEMO() // changed. P.P. 11/25/06
    return;
#endif
#if (PMD_12LS()||CBWUSB_IO())&&PMD_16FS() // case added. P.P. 11/25/06 03/13/18
    Set_valve(fluidics_port_offset, temperature_bit_number, on? TRUE: FALSE); // P.P. 11/25/06
    oven_enabled = (on? TRUE: FALSE); // added. P.P. 07/02/06
#else // gc312 without 2 boards together
    return;
#endif // 12_ls + 16_fs boards
}
// end of function to enable oven. P.P. 06/30/06

// new function to control flame detector. P.P. 06/28/05
void Acquire::FlameDetector(BOOL on)
{
TRACE1( "Acquire::FlameDetector(%d)", on );
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if !STAND_ALONE() // added. P.P. 11/07/04
#if FID() // cond added. P.P. 10/14/05
#if PMD_12LS()||CBWUSB_IO() // case added. P.P. 03/13/18
#if !GC312() // cond. added - we do not use this in GC312. P.P. 02/18/11  here 03/02/12
#if !WATER_PUMP()  // P.P. 01/29/17
Set_valve(fluidics_port_offset, flame_bit_number+8, on? TRUE: FALSE); // in port B. P.P. 03/24/10
if(on) {
start_fid = 1;
}
#endif
#endif // !gc312. P.P. 02/18/11
#endif // pmd
#endif // fid
#endif // !stand_alone
}
// end of control flame function. P.P. 06/28/05

// New function added to equilibrate. P.P. 10/30/02
void Acquire::Equilibrate(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
TRACE1( "Acquire::Equilibrate(%d)", on );
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if !STAND_ALONE() // added. P.P. 08/11/03
#if PMD_16FS()||PMD_12LS()||CBWUSB_IO() // 2 lines added to work with PMDs. P.P. 03/13/18
Set_valve(fluidics_port_offset, equilibrate_bit_number, on? TRUE: FALSE);
#endif //pmd
#endif // !stand_alone. P.P. 08/11/03
equilibrate_on =(on? TRUE: FALSE);
}

void Acquire::Alarm(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, alarm_bit_number, on? TRUE: FALSE); // out bit_number in PMD_1608FS, added. P.P. 11/28/04

//#if DEMO() // added. P.P. 03/18/06
#if DEMO()||1 // changed. P.P. 01/05/09, disabled from now on
return;
#endif
#if !(STAND_ALONE()||GC312())// changed. P.P. 03/18/06
#if PMD_16FS()||PMD_12LS()||CBWUSB_IO() // 2 lines added to work with PMDs. P.P. 11/28/04
//#if !GC321() // cond added. P.P. 07/12/06
#if !(GC321()||GC326()) //  P.P. 12/07/18
Set_valve(fluidics_port_offset, alarm_bit_number, on? TRUE: FALSE); // P.P. 02/18/07
#endif // !gc321
#endif //pmd
#endif // !stand_alone. P.P. 08/11/03
    My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 4, on? 1: 0);

}

//// New function to turn Filter_valve ON/OFF. P.P. 01/12/02
void Acquire::Filter_valve(BOOL on)
{
}
//// End Filter_valve. P.P.

//// New functions for High and Low alarms. P.P. 10/06/01
// New variant - P.P. 11/11/01
void Acquire::Low_Alarm(int pointnumber, BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
#if 1 // temp, for gc301c-12pnt without alarms. P.P. 12/26/07 also 301 1pnt with conc. P.P. 02/15/10
//#if DEMO()
//#if DEMO()||GC312()||GC321() // P.P. 10/17/08
return;
#endif
#if !(STAND_ALONE()||GC312()) // changed. P.P. 03/18/06
//pMainWindow->print("Acquire::Low_Alarm(%d,%d)", pointnumber, on);
// code added for the case of 12LS+usberb08, 4 point. P.P. 02/18/07
#if PMD_12LS()
#if (USB_1024LS()||USB_DIO24())&&(POINTS()<=8)  // case added. P.P. 07/13/07
//    int low_alarm_bit_number=pointnumber; // P.P. 07/07/07
int low_alarm_bit_number=pointnumber+8; // P.P. 07/13/07
int err_code=My_cbDBitOut (CBWDEVICE_1024, FIRSTPORTA, low_alarm_bit_number, on? 1: 0); // port B. P.P. 07/07/07
#elif (USB_1024LS_2()||USB_DIO24_2())&&(POINTS()<=12)  // case added. P.P. 11/06/07
int low_alarm_bit_number=pointnumber; // ports A and CL. P.P. 11/06/07
int err_code=0; // added. 11/06/07
if(low_alarm_bit_number<8) {
err_code=My_cbDBitOut (CBWDEVICE_1024_2, FIRSTPORTA, low_alarm_bit_number, on? 1: 0); // port A. P.P. 11/06/07
}
else if(low_alarm_bit_number<12) {
err_code=My_cbDBitOut (CBWDEVICE_1024_2, FIRSTPORTA, low_alarm_bit_number+8, on? 1: 0); // port CL. P.P. 11/06/07
}
else {
}
#endif // usb_1024. 08/31/18
#endif //pmd

#endif // !stand_alone. P.P. 08/11/03 and !gc312.
My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 4, on? 1: 0);
}

void Acquire::High_Alarm(int pointnumber, BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16. Not done ywt for CBWUSB_IO() case. P.P. 03/13/18
{
#if 1 // temp, for gc301c-12pnt without alarms. P.P. 12/26/07  restored 04/01/08 also for 301-1pnt with conc. P.P. 02/15/10
//#if DEMO() // added. P.P. 10/05/04
//#if DEMO()||GC312()||GC312() // P.P. 10/17/08
return;
#endif
#if !(STAND_ALONE()||GC312()) // changed. P.P. 03/18/06
//pMainWindow->print("Acquire::High_Alarm(%d,%d)", pointnumber, on);
#if PMD_12LS()
#if (USB_1024LS()||USB_DIO24())&&(POINTS()<=8) // P.P. 07/13/07
int high_alarm_bit_number = pointnumber; // added. P.P. 07/07/07
int err_code=My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, (int) high_alarm_bit_number+16, on? TRUE: FALSE); // P.P. 07/07/07
#elif USB_3103()&&(POINTS()<=8) // P.P. 07/01/07
int high_alarm_bit_number = pointnumber; // added. P.P. 07/01/07
int err_code=My_cbDBitOut (CBWDEVICE_3103, AUXPORT, (int) high_alarm_bit_number, on? TRUE: FALSE); // P.P. 07/01/07
#elif (USB_1024LS_2()||USB_DIO24_2())&&(POINTS()<=12)  // case added. P.P. 11/06/07
int err_code=0; // added. 11/06/07
if(pointnumber<8) {
err_code=My_cbDBitOut (CBWDEVICE_1024_2, FIRSTPORTA, pointnumber+8, on? 1: 0); // port B. P.P. 11/06/07
}
else if(pointnumber<12) {
err_code=My_cbDBitOut (CBWDEVICE_1024_2, FIRSTPORTA, pointnumber+12, on? 1: 0); // port CH. P.P. 11/06/07
}
else {
}
#endif // usb-3103 or usb_1024ls 0r usb_1024_2/usb_dio24_2
#endif //1208LS

#endif // !stand_alone. P.P. 08/11/03 and !gc312.
//    My_cbDBitOut (CBWDEVICE_12FS, FIRSTPORTA, (int) high_alarm_bit_number, on? TRUE: FALSE); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
    My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 6, on? 1: 0);
}

//// End of High/Low alarm functions. P.P.

///// New function: Diagnostics Alarm. P.P. 02/19/03 AQll alarms are disabled, and not done yet for CBWUSB_IO() case. P.P. 03/13/18
void Acquire::Diagnostics_Alarm(int diagn_bit_number, BOOL on)
{
#if 1 // temp, for gc301c-12pnt without alarms. P.P. 12/26/07, now 01/29/17
//#if DEMO() // added. P.P. 10/05/04  restored 01/05/09
return;
#endif
#if!(STAND_ALONE()||GC312()) // added. P.P. 03/19/06
// location of diagn alarm port modified. P.P. 08/13/03
#if POINTS()<4 // use portC for Hi/Low and diagn alarms
//#if (DDA_2()||DIO24())&&(POINTS()==4) // replacement. Change points later. P.P. 05/31/04
#else  // points >=4 - use portC2 in any additional DO board.
//#if (DDA_2()||DIO24()||DIO48()) // using 24 or 48-relay board with portC2. P.P. 08/13/04
#if (PMD_12LS()||CBWUSB_IO())&&PMD_16FS() // P.P. 08/31/18
#if 1 // diagn alarm is always sent via 12LS board in place of Alarm, from now on. P.P. 01/05/09
//#if 0 //
Set_valve(fluidics_port_offset, diagn_bit_number, on? 1: 0); // internal alarm
#if POINTS()<=20  // case added. P.P. 06/16/10
#if (USB_1024LS()||USB_DIO24()) // no alarms
int ret = My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, diagn_bit_number-2+16, on? 1: 0); // additional - to port CH bit 5. P.P. 06/16/10
#endif //usb 1024 or dio24, no alarms
#endif // points <20
#else // as it was always. 01/05/09
#if POINTS()<=4
#if!(USB_DIO24()||USB_1024LS()) // case added for 4-pnt with one of these boards. P.P. 11/02/08
//   int ret = My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, diagn_bit_number+8, on? 1: 0); // P.P. 03/03/07
USHORT DataValue = 0; // variant using portB not bits. P.P. 03/04/07
int err_code=0;
#if CBW_12LS()
err_code=My_cbDIn(CBWDEVICE_12LS, FIRSTPORTB, &DataValue);
#elif CBWUSB_IO()
err_code=My_cbDIn(USB_IO, FIRSTPORTB, &DataValue);
#endif
if(on) {
DataValue = DataValue|BIT[diagn_bit_number];
}
else {
DataValue = DataValue&(~BIT[diagn_bit_number]);
}
#if CBW_12LS()
err_code=My_cbDOut(CBWDEVICE_12LS, FIRSTPORTB, DataValue);  // P.P. 03/04/07
#elif CBWUSB_IO()
err_code=My_cbDOut(USB_IO, FIRSTPORTB, DataValue);  // P.P. 03/11/18
#endif
#else // one of these two boards - portB used for input
#if USB_3103()
int ret = My_cbDBitOut(CBWDEVICE_3103, AUXPORT, diagn_bit_number, on? 1: 0); // P.P. 11/02/08
#else
int ret = My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, diagn_bit_number+16, on? 1: 0); // P.P. 11/02/08
#endif // usb_3103
#endif // !usb_dio24
#elif POINTS()<=8 // case added to use 3103 board in the 2nd block for diagn alarms. P.P. 07/07/07
#if USB_3103()
int ret = My_cbDBitOut(CBWDEVICE_3103, AUXPORT, diagn_bit_number, on? 1: 0); // P.P. 07/07/07
#endif // usb_3103
#elif POINTS()<=12  // case added. P.P. 11/06/07
#if (USB_1024LS()||USB_DIO24())&&(USB_1024LS_2()||USB_DIO24_2()) // alarms - to _2 board
int ret = My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, diagn_bit_number+16, on? 1: 0); // in port CH. P.P. 11/06/07
#endif //usb 1024 or dio24 and _2 for l/h alarms
// case added - no alarms, 20 points. P.P. 06/16/10
#elif POINTS()<=20  // case added. P.P. 06/16/10
#if (USB_1024LS()||USB_DIO24()) // no alarms
int ret = My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, diagn_bit_number+16, on? 1: 0); // in port CH. P.P. 06/16/10
#endif //usb 1024 or dio24, no alarms
#endif // points<=4
#endif // 1. 01/05/09
#endif // dda_2 or DIO24 or dio48
#endif // points <4
#endif // !standalone or gc312. added P.P. 03/19/06
//  My_cbDBitOut (CBWDEVICE_12FS, FIRSTPORTA, (int) diagn_bit_number, on? TRUE: FALSE); // out bit_number in PMD_1208LS, added. P.P. 11/28/04
    My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 7, on? 1: 0);
}
//// end of Diagnostics alarm function. P.P. 02/19/02

void Acquire::Lamp(BOOL on)
{

}

/// new function added to control lamp B separately. P.P. 12/31/02
void Acquire::LampB(BOOL on)  // modified from Lamp() but only in case of DDA, REL16, and HV_501, to simplify. P.P. 12/31/02
{
    TRACE1( "Acquire::LampB(%d)", on );
#if DEMO() // added. P.P. 10/05/04
    return;
#endif
// P.P. 12/08/02. Check first if the lamp is ON or OFF by reading port.
#if !STAND_ALONE() // added. P.P. 07/14/03
#if PMD_16FS() // first, check if the lampB is on or off. P.P. 08/25/18
    USHORT Lamp_bit_value;
#if PMD_12LS()
    My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 4, &Lamp_bit_value); // read HVB output. P.P. 08/25/18
#elif CBWUSB_IO()
    My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 4, &Lamp_bit_value);   // read input bit 4 as lampB status. P.P. 08/25/18
#endif
    if(((!Lamp_bit_value) && (!on)) ||
       (Lamp_bit_value && on)) { //if state is changed.  P.P. 06/24/06
#else // no 16fs - use flags
        if((flagsLamp&0x8000) || ((flagsLamp&2) && !on) || (!(flagsLamp&2) && on) ) { //if state is unknown or changed
#endif // pmd_16FS

// now - turn the lampB ON or OFF if the state has changed
#if PMD_12LS()||CBWUSB_IO() // added. P.P. 10/14/05
#if PMD_12LS()&&PMD_16FS() // case added for 2 boards + not usb repays. P.P. 11/25/06
        My_cbDBitOut(CBWDEVICE_16FS, AUXPORT, 4, on? FALSE: TRUE); // 4th bit in control port. P.P. 11/05/05
#elif CBWUSB_IO() //  via CBWUSB_IO board. P.P. 08/24/18
        My_cbDBitOut(USB_IO, FIRSTPORTCH, 3, on? FALSE: TRUE); // bit 3 bit in port CH. Low to turn ON. P.P. 08/24/18
#endif // 12LS or usb_io
#else // no one of dio boards
        #if PMD_16FS() // added. control port in auxport of 16-fs board. P.P. 11/25/06
   Set_alarm_bit(control_port_offset, 4, on? FALSE: TRUE);       // changed to get lampB  ON at bit4 = low. P.P. 11/05/05
#endif // 16_fs
#endif // one of 12LS or usbdio24
        TRACE1("Acquire::Lamp =%d", on);
//    pMainWindow->print("Acquire::Lamp =%d\r\n", on); // removed. P.P. 06/24/06

//#if !PMD_16FS() // added; then use flags. P.P. 06/24/06
/* // rem P.P. 05/04/07             // restored but only 1608 considered. 08/25/18
    flagsLamp&=~0x8000; //clear unknown state
    if(on) flagsLamp|=BIT[1]; // store state in 2nd bit  // P.P. 12/31/02
    else flagsLamp&=~BIT[1];
*/
//#endif // pmd boards. P.P. 06/24/06
    }
#endif // !stand_alone. P.P. 07/14/03
#if !PMD_16FS()
    if(on) lampB_on = 1; // added. P.P. 07/23/06   // in fact, not HVA but lamp status must be used! change later. P.P. 08/26/18
  else  lampB_on = 0;
#else // use real reading from 1608 board. P.P. 08/26/18
    if(!Lamp_bit_value) lampB_on = 1;  // Lamp_bit_value read from the lamp is LOW when the lamp is ON. P.P. 08/26/18
    else lampB_on = 0;   //
#endif // !1608
}
/// end of new LampB function. P.P. 12/31/02


void Acquire::SetLamps(BOOL bOn)
{
        My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, 1, DIGITALOUT);  // Ensure pin is output
        My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 1, bOn? 0:1);
}

void Acquire::AutoZero(BOOL on) // P.P. modified 06/07/03 to work with DAQ + REL_16.
{
    TRACE1( "Acquire::AutoZero(%d)", on );
#if DEMO() // added. P.P. 10/05/04
    return;
#endif
#if !STAND_ALONE() // added. P.P. 08/02/03
#if CBW() // the must for PMD
    int err_code = 0; // added error handling. P.P. 10/16/05
#if PMD_12LS()||CBWUSB_IO()
#if PMD_16FS()  // case added, 2 boards + relays. P.P. 11/25/06
    err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 0, on? 1: 0); // out bit 7 in PMD_1608FS, P.P. 11/28/04
    if(err_code) { // DEBUG
        Str str="USB_1608FS error";
//    MessageBox(NULL,str, "wPeak: USB relay board error", MB_OK); // rem. P.P. 11/25/06
    }
#else // only pmd_12ls
    #if PMD_12LS()
  err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, 0, on? TRUE: FALSE); // out bit 0 in PMD_1208LS, P.P. 10/16/05
  if(err_code) { // DEBUG
    Str str="USB_1208LS error";
//    MessageBox(NULL,str, "wPeak: USB relay board error", MB_OK); // rem. P.P. 11/25/06
  }
#elif USB_IO()
  err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, 0, on? TRUE: FALSE); // out bit 0 in PMD_1208LS, P.P. 10/16/05
#endif
#endif // pmd_16fs
#elif PMD_16FS()
    err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 0, on? 1: 0); // out bit 0 in PMD_1608FS, P.P. 11/28/04
  if(err_code) { // DEBUG
    Str str="USB_1608FS error";
//    MessageBox(NULL,str, "wPeak: USB relay board error", MB_OK); // rem. P.P. 11/25/06
  }
#endif // PMD_16 vs 12. P.P. 11/07/04
#endif // cbw
#else //stand-alone case. P.P. 11/01/15
    #if PMD_16FS()
  int err_code = 0; // added error handling. P.P. 10/16/05
  err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 0, on? 1: 0); // out bit 0 in PMD_1608FS, P.P. 11/28/04
  if(err_code) { // DEBUG
    Str str="USB_1608FS error";
//    MessageBox(NULL,str, "wPeak: USB relay board error", MB_OK); // rem. P.P. 11/25/06
  }
#endif // 1608 board
#endif // !stand_alone. P.P. 08/02/03
}

// independent autozero for det B added. P.P. 06/21/17
void Acquire::AutoZeroB(BOOL on)
{
    TRACE1( "Acquire::AutoZeroB(%d)", on );
#if DEMO() // added. P.P. 10/05/04
    return;
#endif
#if !STAND_ALONE() // added. P.P. 08/02/03
#if CBW() // the must for PMD
    int err_code = 0; // added error handling. P.P. 10/16/05
        err_code = My_cbDBitOut (4, FIRSTPORTA, 7, on? TRUE: FALSE); // out bit 7 in PMD_1208LS, P.P. 06/21/17

#if PMD_12LS()&&PMD_16FS()
    err_code = My_cbDBitOut (CBWDEVICE_12LS, FIRSTPORTA, autozeroB_bit_number, on? TRUE: FALSE); // out bit 7 in PMD_1208LS, P.P. 06/21/17
  if(err_code) { // DEBUG
    Str str="USB_1208LS error";
//    MessageBox(NULL,str, "wPeak: USB relay board error", MB_OK); // rem. P.P. 11/25/06
  }
#elif CBWUSB_IO()&&PMD_16FS()
//  err_code = My_cbDBitOut (USB_IO, FIRSTPORTA, autozeroB_bit_number, on? TRUE: FALSE); // out bit 7 in USB_DIO24, P.P. 03/11/18
    //err_code = My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 1, on? 1: 0); // out bit 1 in PMD_1608FS, P.P. 08/24/18
#endif //pmd_12LS and pmd_16fs
#endif // cbw
#endif // !stand_alone. P.P. 08/02/03
}
// end of autozeroB function.

//// Pointnumber() replaced with a new Pointnumber function to control point of choice. P.P. 01/10/03
void Acquire::PointNumber(int Pointnumber)
{
TRACE1("Acquire::PointNumber =%d", Pointnumber);                 // changed. P.P. 05/01/04
pMainWindow->print("Acquire::PointNumber =%d\r\n", Pointnumber); //
//  #if MYDEBUG()
//#if MYDEBUG()||DEMO()  // demo added. P.P. 10/05/04
//#if MYDEBUG()||DEMO()||GC312()  // only 1-point in gc312. P.P. 04/02/06
//#if MYDEBUG()||DEMO()||GC312()||(POINTS()<=1)  // nothing to do if only 1-point. P.P. 02/14/10
#if MYDEBUG()||DEMO()||GC312()||(POINTS()<2)  // 2-point variant. P.P. 04/15/17
return;
#endif
#if !STAND_ALONE() // added. P.P. 08/11/03
#if PMD_12LS()&&PMD_16FS()&&ANALOG_OUTPUT()  // case added to have points in analog out board. P.P. 01/06/09
#if (POINTS()<=4)
#if USB_3101()
int err_code;
int bit_num = 0;
for(bit_num=0; bit_num<4; bit_num++) {
if(bit_num != Pointnumber) {
err_code=My_cbDBitOut(CBWDEVICE_3101, AUXPORT, bit_num, 0);  // P.P. 01/30/09
}
else {
err_code=My_cbDBitOut(CBWDEVICE_3101, AUXPORT, bit_num, 1);  // P.P. 01/30/09
}
}
//     int err_code=My_cbDOut(CBWDEVICE_3101, AUXPORT, BIT[Pointnumber]);  // P.P. 01/06/09
#elif USB_3103()
int err_code;
int bit_num = 0;
for(bit_num=0; bit_num<4; bit_num++) {
if(bit_num != Pointnumber) {
err_code=My_cbDBitOut(CBWDEVICE_3103, AUXPORT, bit_num, 0);  // P.P. 01/30/09
}
else {
err_code=My_cbDBitOut(CBWDEVICE_3103, AUXPORT, bit_num, 1);  // P.P. 01/30/09
}
}
//     int err_code=My_cbDOut(CBWDEVICE_3103, AUXPORT, BIT[Pointnumber]);  // P.P. 01/06/09  replaced for above 01/30/09
#elif USB_3106()
int err_code;
int bit_num = 0;
for(bit_num=0; bit_num<4; bit_num++) {
if(bit_num != Pointnumber) {
err_code=My_cbDBitOut(CBWDEVICE_3106, AUXPORT, bit_num, 0);  // P.P. 01/30/09
}
else {
err_code=My_cbDBitOut(CBWDEVICE_3106, AUXPORT, bit_num, 1);  // P.P. 01/30/09
}
}
//     int err_code=My_cbDOut(CBWDEVICE_3106, AUXPORT, BIT[Pointnumber]);  // P.P. 01/06/09
#elif USB_3112()
int err_code;
int bit_num = 0;
for(bit_num=0; bit_num<4; bit_num++) {
if(bit_num != Pointnumber) {
err_code=My_cbDBitOut(CBWDEVICE_3112, AUXPORT, bit_num, 0);  // P.P. 01/30/09
}
else {
err_code=My_cbDBitOut(CBWDEVICE_3112, AUXPORT, bit_num, 1);  // P.P. 01/30/09
}
}
//     int err_code=My_cbDOut(CBWDEVICE_3112, AUXPORT, BIT[Pointnumber]);  // P.P. 01/06/09
#endif // one of these 4 boards, they all have 8 dig lines
#endif // points<=4
#elif (PMD_12LS()||CBWUSB_IO())&&PMD_16FS() // case added. P.P. 03/11/18  no analog output (but othe dio might be
#if (POINTS()<=20)
#if USB_1024LS()||USB_DIO24()
int err_code=0;
if(Pointnumber < 8) {
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTA, BIT[Pointnumber]);
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTB, 0x00);
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTCL, 0x00);  // P.P. 06/16/10
}
else if(Pointnumber<16) {
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTA, 0x00);  //
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTB, BIT[Pointnumber-8]);
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTCL, 0x00);  // P.P. 06/16/10
}
else if(Pointnumber<20) {
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTA, 0x00);  //
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTB, 0x00);
err_code=My_cbDOut(CBWDEVICE_1024, FIRSTPORTCL, BIT[Pointnumber-16]);  // P.P. 06/16/10
}
else {
}
#elif CBWUSB_IO_2()  // case added P.P. 2/13/18
int err_code=0;
if(Pointnumber < 8) {
err_code=My_cbDOut(USB_IO_2, FIRSTPORTA, BIT[Pointnumber]);
err_code=My_cbDOut(USB_IO_2, FIRSTPORTB, 0x00);
err_code=My_cbDOut(USB_IO_2, FIRSTPORTCL, 0x00);  // P.P. 06/16/10
}
else if(Pointnumber<16) {
err_code=My_cbDOut(USB_IO_2, FIRSTPORTA, 0x00);  //
err_code=My_cbDOut(USB_IO_2, FIRSTPORTB, BIT[Pointnumber-8]);
err_code=My_cbDOut(USB_IO_2, FIRSTPORTCL, 0x00);  // P.P. 06/16/10
}
else if(Pointnumber<20) {
err_code=My_cbDOut(USB_IO_2, FIRSTPORTA, 0x00);  //
err_code=My_cbDOut(USB_IO_2, FIRSTPORTB, 0x00);
err_code=My_cbDOut(USB_IO_2, FIRSTPORTCL, BIT[Pointnumber-16]);  // P.P. 06/16/10
}
else {
}
#endif // usb-1024 or usb-dio24 or CBWUSB_IO_2
#endif // points<=20
#endif // basic pmd boards
#endif // !stand_alone. P.P. 08/11/03
}
//// end of new pointnumber(int) function as replacement for one below.

void Acquire::WaterPump(int water_pump_number, BOOL on)
{
//#if DEMO()
#if DEMO()||1    // disable this function in the basic case. P.P. 08/31/18. CChange code later if needed.
return;
#endif
#if WATER_PUMP()
//#if PMD_16FS()&&PMD_12LS()&&(POINTS()<=3) // P.P. 12/31/16
#if PMD_16FS()&&(POINTS()<=3) // P.P. 12/31/16
int err_code=0; //
USHORT WaterPumpOn = 1;
#if PMD_12LS()
My_cbDBitIn (CBWDEVICE_12LS, FIRSTPORTA, water_pump_number+12, &WaterPumpOn); // from port B
if((!WaterPumpOn) && on) {
err_code = My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, water_pump_number+12, 1);
}
else if(WaterPumpOn && (!on)) {
err_code = My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, water_pump_number+12, 0);
}
else {
}
#elif USB_IO()
My_cbDBitIn (USB_IO, FIRSTPORTA, water_pump_number+12, &WaterPumpOn); // from port B
if((!WaterPumpOn) && on) {
err_code = My_cbDBitOut(USB_IO, FIRSTPORTA, water_pump_number+12, 1);
}
else if(WaterPumpOn && (!on)) {
err_code = My_cbDBitOut(USB_IO, FIRSTPORTA, water_pump_number+12, 0);
}
else {
}
#endif //12LS or usb_io
#endif // 16fs and <= 3 pntspmd boards
#endif // water_pump
return;
}

/// New function to open oven door. P.P. 04/02/02
//void Acquire::OvenDoor(BOOL on)     // Made simple way: P.P. 08/09/02. Modified 06/07/03 to work with DAQ + REL_16.
void Acquire::OvenDoor(BOOL on)     //Changed: P.P. 06/18/06.
{
#if DEMO() // added. P.P. 10/05/04
//#if DEMO()||GC326() // P.P. 12/07/18
return;
#endif
//#if !STAND_ALONE() // added. P.P. 08/02/03
#if !(STAND_ALONE()||GC312()) // changed. P.P. 03/19/06
//#if GC321()&&PMD_16FS() // case added. P.P. 10/17/08
#if (GC321()||GC326())&&PMD_16FS() //  P.P. 12/07/18
#if PMD_12LS()
int err_code=My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, oven_door_bit_number+8, on? TRUE: FALSE);  // P.P. 10/06/11
#elif CBWUSB_IO()
int err_code=My_cbDBitOut(USB_IO, FIRSTPORTA, oven_door_bit_number+8, on? TRUE: FALSE);  // P.P. 03/11/18
#endif
if(on)  oven_door_on = 1; // 10/17/08
else oven_door_on = 0;    //
//#elif PMD_16FS()&&PMD_12LS()&&ANALOG_OUTPUT()&&(!RAMPING()) // case added for GC301c to run 2nd pump. P.P. 01/30/09
#elif PMD_16FS()&&(PMD_12LS()||CBWUSB_IO())&&ANALOG_OUTPUT()&&(!RAMPING()) // case added for GC301c to run 2nd pump. P.P. 01/30/09
#if (POINTS()<=4)
#if USB_3101()
int err_code=My_cbDBitOut(CBWDEVICE_3101, AUXPORT, oven_door_bit_number, on? TRUE: FALSE);  // P.P. 01/30/09
#elif USB_3103()
int err_code=My_cbDBitOut(CBWDEVICE_3103, AUXPORT, oven_door_bit_number, on? TRUE: FALSE);  // P.P. 01/30/09
#elif USB_3106()
int err_code=My_cbDBitOut(CBWDEVICE_3106, AUXPORT, oven_door_bit_number, on? TRUE: FALSE);  // P.P. 01/30/09
#elif USB_3112()
int err_code=My_cbDBitOut(CBWDEVICE_3112, AUXPORT, oven_door_bit_number, on? TRUE: FALSE);  // P.P. 01/30/09
#endif // one of these 4 boards, they all have 8 dig lines
#endif // points<=4
//#elif PMD_16FS()&&PMD_12LS()&&(USB_DIO24()) // for GC301 20-pnts, no alarms. P.P. 06/18/10
#elif PMD_16FS()&&(PMD_12LS()||CBWUSB_IO())&&(USB_DIO24()) // for GC301 20-pnts, no alarms. P.P. 06/18/10
#if POINTS()<=20 // cond added. P.P. 06/20/11
int err_code=My_cbDBitOut(CBWDEVICE_1024, FIRSTPORTA, oven_door_bit_number+16, on? TRUE: FALSE);  // P.P. 06/18/10, corr 08/11/10
#endif
//#elif GC312()&&RAMPING()&&PMD_12LS()&&PMD_16FS() // case added for GC312 with ramping. P.P. 11/12/10
#elif GC312()&&RAMPING()&&(PMD_12LS()||CBWUSB_IO())&&PMD_16FS() // case added for GC312 with ramping. P.P. 11/12/10
Set_valve(fluidics_port_offset, oven_door_bit_number+8, on? TRUE: FALSE);
if(on)  oven_door_on = 1;
else oven_door_on = 0;    //
//#endif // usb_erb 08 or 24 or GC322 or oven door bit for 2nd pump moved down. P.P. 08/31/18
// end of added code. P.P.
#elif GC312()&&FID() // for current GC312 with fid. P.P. 06/19/09
//  Set_valve(fluidics_port_offset, oven_door_bit_number, on? TRUE: FALSE);
Set_valve(fluidics_port_offset, oven_door_bit_number+8, on? TRUE: FALSE); // in port B now. P.P. 02/18/11 here 03/02/12
if(on)  oven_door_on = 1; // 06/19/09
else oven_door_on = 0;    //
#endif // different sets of boards or GC322 or oven door bit for 2nd pump in 312
#endif //!stand_alone. P.P. 08/02/03
}
///end

// added DoorMotor(BOOL on) to turn it ON when the door starts open or starts close. P.P. 10/04/11
void Acquire::DoorMotor(BOOL on)     // P.P. 10/04/11.
{
//#if DEMO() // added. P.P. 10/05/04
#if DEMO()||GC326() // P.P. 12/07/18
return;
#endif
#if GC321()&&PMD_16FS()
#if PMD_12LS()
int err_code=My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 9, on? TRUE: FALSE);  // bit 1 in port B. P.P. 10/04/11
#elif CBWUSB_IO()
int err_code=My_cbDBitOut(USB_IO, FIRSTPORTA, 9, on? TRUE: FALSE);  // bit 1 in port B. P.P. 10/04/11
#endif
motor_on = on? 1:0;
#endif
}
// end of the DoorMotor() function. P.P. 10/04/11

void Acquire::SecondPump(BOOL on)     // new function. P.P. 08/03/18.
{
#if DEMO() // added. P.P. 10/05/04
return;
#endif
#if PMD_12LS()
int err_code=My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 10, on? TRUE: FALSE);  // bit 2 in port B, same as oven door. P.P. 08/03/18
#elif CBWUSB_IO()
int err_code=My_cbDBitOut(USB_IO, FIRSTPORTA, 21, on? TRUE: FALSE);  // bit 5 in port C. P.P. 08/03/18
#if CBWUSB_IO_2()
err_code=My_cbDBitOut(USB_IO_2, FIRSTPORTA, 21, on? TRUE: FALSE);  // bit 5 in port C. 2nd pump - can be used in any of those boards. added P.P. 12/13/18
#endif
#endif
motor_on = on? 1:0;
}
// end of the 2nd door function. P.P. 08/03/18

static int CheckRemoteCalibration()  // New function to start cal remotely. P.P. 11/23/02
{
//#if MYDEBUG()
//  #if MYDEBUG()||DEMO() // P.P. 10/05/04
//#if MYDEBUG()||DEMO()||GC312() // P.P. 04/02/06
#if 1 // P.P. 08/31/18
    return 0;
#endif
#if REMOTE_CAL() // cond added. P.P. 08/14/04
    int Diagnostics_error_flag = Diagnostics(diagnostics_port);
  if(!(Diagnostics_error_flag & BIT[remote_cal_bit])) { // replacement. P.P. 09/05/04
    return 1;
  }
  else {
    return 0;
  }
#else // without rem cal - return 0
    return 0;
#endif //remote_cal. P.P. 08/14/04
}

static int CheckRemoteCheckCalibration()
{
//#if MYDEBUG()
//#if MYDEBUG()||DEMO() // P.P. 10/05/04
//#if MYDEBUG()||DEMO()||GC312() // P.P. 04/02/06
#if 1  // disabled. P.P. 08/31/18
return 0;
#endif
#if REMOTE_CAL() // cond added. P.P. 08/14/04
int Diagnostics_error_flag = Diagnostics(diagnostics_port);
if(!(Diagnostics_error_flag & BIT[check_remote_cal_bit])) { // bit 7 for remote cal start
return 1;
}
else {
return 0;
}
#else
return 0;
#endif // remote_cal. P.P. 08/14/04
}

static int CheckRemoteStart()
{
return 0;  // code removed as not needed. P.P. 08/31/18
}

static int CheckRemoteStop() // This function should not be used - now
// I use this input bit to read Autozero state. P.P. 11/11/01
{
return 0; // added. P.P. 08/11/03
}

// New functions added to control TRACE_GC via USB boards. P.P. 07/17/03
//#if TRACE_GC()&&STAND_ALONE()                                                  // rem this later. P.P. 08/31/18
#if 0 // disabled                                                  // rem this later. P.P. 08/31/18
static unsigned CheckTraceGCStatus() // we could replace this function with
{                               // separate functions to checks particular bits
  return 0;
}

static void StartTraceGC(BOOL on) // P.P. 07/17/03
{
  return;
}

static void ProhibitReadyTraceGC(BOOL on) // P.P. 07/17/03
{
  return;
}

#endif //TRACE_GC&&STAND_ALONE - zero.
// end of added functions. P.P. 07/17/03

void ShowTemperatures(MainWindow *pMainWindow)
{
#if SERIAL_NUMBER() // added P.P. 04/01/17
    char str[200];  // P.P. 04/01/17
// show ser_num. P.P. 03/12/17
  sprintf(str,"S/N: %s", serialNum);  // P.P. 03/17/17
//  for(int x=0; x<9;x++) {
//    sprintf(str,"S/N: %8c", serialNum[x]);  // P.P. 03/17/17
//  }
  pMainWindow->sl->SetStatus(str);
#else // regular case, as it always was
//  char str[50];
//  char str[100];  // Increased. P.P. 10/18/02
    char str[200];  // Increased. P.P. 03/10/09
    char str_error[100]; // added. P.P. 06/20/03
    double ShowVolts(int DetNum); // added. P.P. 10/25/02
    double detA = ShowVolts(0);  // added here, for all the cases. P.P. 07/27/18
    double detB = ShowVolts(1);  //
    double Check_Temp_Safety(); // 04/20/09
#if !STAND_ALONE()
//  double Get_Flow(); // moved down. P.P. 05/23/04
#if OVEN_EQ_INJ()
#if !FLOW()          // condition added. P.P. 11/17/02
#if (PMD_12LS()||PMD_16FS()||PMD_12FS()) // cond changed. P.P. 06/21/05
//#else // with PMD, use saved data to save time. P.P. 11/28/04
#if !CONCENTRATOR()
#if GC312() // added. P.P. 04/09/06
    //   double detA = ShowVolts(0);  // P.P. 03/10/09    moved up 07/27/18
//   double detB = ShowVolts(1);  //
   if(battery_voltage<0.5) { // running on external power, P.P. 04/09/06
//     sprintf(str,"Oven: % 3.0fC, Det A: % 6.3f V",
//	temp_oven, ShowVolts(0));
#if !FID() // cond added 06/05/09
#if !PRESSURE_SENSOR() // P.P. 10/30/09
     sprintf(str,"Oven: %4.0fC, Det A: %6.3fV, Det B: %6.3fV",
	temp_oven, detA, detB);                   // show A and B det. P.P. 03/10/09
#else  // with pressure sensor
#if !HUMIDITY_SENSOR()
//     sprintf(str,"Oven: %3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi", // 11/01/09
     sprintf(str,"Oven: %4.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
	temp_oven, detA, detB, pressure_cylinder); // show Oven, A and B det, and pressure. P.P. 10/02/09
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"Oven: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f%",
	temp_oven, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 07/27/17

#endif // !pressure sensor
#else // 312 with FID. 06/05/09
//     sprintf(str,"F:%3.0fC, O:%3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
//	Get_Temperature(1),temp_oven, detA, detB, pressure_cylinder);                   // show flame T, oven T, A and B det, and pressure. P.P. 03/21/11, here 05/06/11
// new code, for FID case in 312. P.P. 08/29/17
#if !PRESSURE_SENSOR() // P.P. 10/30/09
     sprintf(str,"F:%3.0fC, Oven: %4.0fC, Det A: %6.3fV, Det B: %6.3fV",
	Get_Temperature(1),temp_oven, detA, detB);                   // show A and B det. P.P. 03/10/09
#else  // with pressure sensor
#if !HUMIDITY_SENSOR()
//     sprintf(str,"Oven: %3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi", // 11/01/09
     sprintf(str,"F:%3.0fC, O: %4.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
	Get_Temperature(1),temp_oven, detA, detB, pressure_cylinder); // show FIID temp,  Oven, A and B det, and pressure. P.P. 10/02/09
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"F:%3.0fC, O: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f%",
	Get_Temperature(1), temp_oven, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 07/27/17
#endif // !pressure sensor
// end of FID code
#endif // !FID
   }
   else { // running from battery - show battery voltage. P.P. 04/09/06
      double bat_volt = battery_voltage*2;
#if !FID() // cond added 06/05/09
#if !PRESSURE_SENSOR() // P.P. 10/30/09
      sprintf(str,"Oven: %4.0fC, A: %6.3fV, B: %6.3fV, BAT: %3.1fV",
	temp_oven, detA, detB, bat_volt);                        // show A and B det. P.P. 03/10/09
#else // with pressure sensor
#if !HUMIDITY_SENSOR()
      sprintf(str,"Oven %4.0fC, A: %6.3fV, B: %6.3fV, BAT: %3.1fV, P: %4.0f psi",
	temp_oven, detA, detB, bat_volt, pressure_cylinder);                        // show A and B det. P.P. 03/10/09
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"Oven: % 3.0fC, A: % 6.3fV, B: % 6.3fV, BAT: %3.1fV, P: %4.0f psi, RH: %2.2f%",
	temp_oven, ShowVolts(0), ShowVolts(1), bat_volt, pressure_cylinder, humidity_value);  // P.P. 07/28/17
#endif // !humidity sensor. P.P. 07/27/17
#endif // !pressure sensor
#else // 312 with FID. 06/05/09
#if !PRESSURE_SENSOR() // P.P. 10/30/09
     sprintf(str,"F:%3.0fC, Oven: %4.0fC, Det A: %6.3fV, Det B: %6.3fV, BAT: %3.1fV",
	Get_Temperature(1),temp_oven, detA, detB, bat_volt);                   // show A and B det. P.P. 03/10/09
#else  // with pressure sensor
#if !HUMIDITY_SENSOR()
     sprintf(str,"F:%3.0fC, O: %4.0fC, A: %6.3fV, B: %6.3fV, BAT: %3.1fV, P: %4.0f psi",
	Get_Temperature(1),temp_oven, detA, detB, bat_volt, pressure_cylinder); // show FID temp,  Oven, A and B det, Battery, and pressure. P.P. 08/29/17
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"F:%3.0fC, O: % 3.0fC, A: % 6.3f V, B: % 6.3f V, BAT: %3.1fV, P: %4.0f psi, RH: %2.2f%",
	Get_Temperature(1), temp_oven, ShowVolts(0), ShowVolts(1), bat_volt, pressure_cylinder, humidity_value);  // P.P. 08/29/17
#endif // !humidity sensor. P.P. 07/27/17
#endif // !pressure sensor
// end of FID code
#endif // !FID
   }
#else // regular case, not 312. Add cases without or with FID later.
#if !FID()
#if !PRESSURE_SENSOR()
    #if !HUMIDITY_SENSOR()
   sprintf(str,"Oven: % 3.0fC, Det A: % 6.3f V, Det B: % 6.3f V",
        temp_oven, ShowVolts(0), ShowVolts(1));
#else // with RH sensor
      sprintf(str,"Oven %4.0fC, A: %6.3fV, B: %6.3fV, RH: %2.2f%",
	temp_oven, detA, detB, humidity_value);                        // show A and B det. P.P. 03/10/09
#endif // !RH sensor
#else // GC301 with pressure sensor, case added P.P. 01/30/17
#if !HUMIDITY_SENSOR()
    sprintf(str,"Oven %4.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
            temp_oven, ShowVolts(0), ShowVolts(1), pressure_cylinder);                        // show A and B det. P.P. 03/10/09
#else // with humidity sensor
    sprintf(str,"Oven %4.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi, RH: %2.2f%",
	temp_oven, detA, detB, pressure_cylinder, humidity_value);                        // P.P. 08/29/17
#endif // !RH sensor
#endif // !pressure sensor
#else // regular GC with FID
    // new code, 301 ith FID. P.P. 08/29/17
#if !PRESSURE_SENSOR() // P.P. 10/30/09
#if !HUMIDITY_SENSOR()
     sprintf(str,"F:%3.0fC, Oven: %4.0fC, Det A: %6.3fV, Det B: %6.3fV",
	Get_Temperature(1),temp_oven, detA, detB);                   // show A and B det. P.P. 03/10/09
#else
  sprintf(str,"F:%3.0fC, O: % 3.0fC, A: % 6.3f V, B: % 6.3f V, RH: %2.2f%",
	Get_Temperature(1), temp_oven, ShowVolts(0), ShowVolts(1), humidity_value);  // P.P. 08/29/17
#endif // !humidity sensor
#else  // with pressure sensor
#if !HUMIDITY_SENSOR()
     sprintf(str,"F:%3.0fC, O: %4.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
	Get_Temperature(1),temp_oven, detA, detB, pressure_cylinder); // show FIID temp,  Oven, A and B det, and pressure. P.P. 10/02/09
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"F:%3.0fC, O: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f%",
	Get_Temperature(1), temp_oven, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 07/27/17
#endif // !pressure sensor
// end of new code for 301 with FID
#endif // !FID()
#endif // gc312
#else  // added for concentrator. P.P. 02/20/06, still oven=det
    #if !HEATED_LINE() // cond added. P.P. 03/31/17
#if !PRESSURE_SENSOR()
#if !HUMIDITY_SENSOR()
  sprintf(str,"Oven: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1));  // P.P. 02/20/06
#else //with the humidity sensor
  sprintf(str,"Oven: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, RH: %2.2f %",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 06/26/17
#else // with pressure to be shown. P.P. 06/26/17
#if !HUMIDITY_SENSOR()
#if H2_PRESSURE_SENSOR()
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure);  // P.P. 11/24/17
#endif // fid
#else // without h2_pressure_sensor
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder);  // P.P. 11/24/17
#endif // fid
#endif // h2 pressure sensor
#else //with the humidity sensor
// new
#if H2_PRESSURE_SENSOR()
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure, humidity_value);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure, humidity_value);  // P.P. 11/24/17
#endif // fid
#else // withour h2_pressure_sensor
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 11/24/17
#endif // fid
#endif // h2 pressure sensor
// end of new
#endif // !humidity sensor. P.P. 06/26/17
#endif // !pressure_sensor
#else // with line temperature shown
  sprintf(str,"Oven: % 3.0fC, Dsb: % 3.0fC, Line: % 3.0fC, A: % 6.3f V, B: % 6.3f V",
	temp_oven, temp_conc, temp_line, ShowVolts(0), ShowVolts(1));  // P.P. 03/31/17  line temperature shown
#endif // !heated_line
// temp, to show ser_num. P.P. 03/12/17
//  sprintf(str,"S/N: %8x", mfg_num);  // P.P. 03/17/17
//  sprintf(str,"S/N: %x", serialNum);  // P.P. 03/17/17
#endif // !concentrator
#endif // pmd

#else // with flow to be shown
    double Get_Flow();
//#if GC501() // added. P.P. 05/23/04  moved down. P.P. 08/15/04
  double Get_Flow_S(short channel);
#if GC501() // moved from above. P.P. 08/15/04. Column and sample flows in all 501s? P.P. 07/10/05
  sprintf(str,"% 3.0fC,  Det A: % 6.3f V,  C: % 3.2f ccm,  S: % 4.1f ccm",
      Get_Temperature(0), ShowVolts(0), Get_Flow(), Get_Flow_S(sample_flow_chan));
#else // not a 501F
      sprintf(str,"Oven %4.0fC, A: %6.3fV, B: %6.3fV, S: % 4.1f ccm, P: %4.0f psi",//
	temp_oven, ShowVolts(0), ShowVolts(1), flow_value,pressure_cylinder);                          // changed, to display values measured somewere else. P.P. 06/14/11
#endif // GC501()
#endif // flow

// changes needed for general case of oven, det, dsb, fid, and line temps. P.P. 07/27/18
#else  // oven and inj to be shown
    #if !FLOW()          // condition added. P.P. 11/17/02
  sprintf(str,"Oven: % 3.0fC, Inj: % 3.0fC, Det A: % 6.3f V, B: % 6.3f V", // P.P. 06/21/06
       temp_oven, temp_det, ShowVolts(0), ShowVolts(1)); // P.P. 07/03/05, 09/03/05

// copy from oven=det - no flow, but conc or without, etc, to be changed
#if (PMD_12LS()||PMD_16FS()||PMD_12FS()) // cond changed. P.P. 06/21/05
#if !CONCENTRATOR()
#if GC312() // added. P.P. 04/09/06
#else // regular case, not 312. Add cases without or with FID later.
#if !FID()
#if !PRESSURE_SENSOR()
#if !HUMIDITY_SENSOR()
  sprintf(str,"Oven: % 3.0fC, Inj: % 3.0fC, Det A: % 6.3f V, B: % 6.3f V", // P.P. 06/21/06
       temp_oven, temp_det, detA, detB); // P.P. 07/03/05, 09/03/05
#else // with RH sensor
      sprintf(str,"Oven %4.0fC, Inj: % 3.0fC, A: %6.3fV, B: %6.3fV, RH: %2.2f%",
	temp_oven, temp_det, detA, detB, humidity_value);                        // show A and B det. P.P. 03/10/09
#endif // !RH sensor
#else // GC322 with pressure sensor, case added P.P. 01/30/17
#if !HUMIDITY_SENSOR()
      sprintf(str,"Oven %4.0fC, Inj: % 3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
	temp_oven, temp_det, detA, detB, pressure_cylinder);                        // show A and B det. P.P. 03/10/09
#else // with humidity sensor
      sprintf(str,"Oven %4.0fC, Inj: % 3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi, RH: %2.2f%",
	temp_oven, temp_det, detA, detB, pressure_cylinder, humidity_value);                        // P.P. 08/29/17
#endif // !RH sensor
#endif // !pressure sensor
#else // regular GC with FID
// new code, 301 ith FID. P.P. 08/29/17
#if !PRESSURE_SENSOR() // P.P. 10/30/09
#if !HUMIDITY_SENSOR()
     sprintf(str,"F:%3.0fC, Oven: %4.0fC, Inj: % 3.0fC, Det A: %6.3fV, Det B: %6.3fV",
	temp_fid, temp_oven, temp_det, detA, detB);                   // show A and B det. P.P. 03/10/09
#else
  sprintf(str,"F:%3.0fC, O: % 3.0fC, Inj: % 3.0fC, A: % 6.3f V, B: % 6.3f V, RH: %2.2f%",
	temp_fid, temp_oven, temp_det, detA, detB, humidity_value);  // P.P. 08/29/17
#endif // !humidity sensor
#else  // with pressure sensor
#if !HUMIDITY_SENSOR()
//     sprintf(str,"Oven: %3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi", // 11/01/09
     sprintf(str,"F:%3.0fC, O: %4.0fC, Inj: % 3.0fC, A: %6.3fV, B: %6.3fV, P: %4.0f psi",
	temp_fid, temp_oven, temp_det, detA, detB, pressure_cylinder); // show FID temp,  Oven, A and B det, and pressure. P.P. 10/02/09
#else //with the humidity sensor, added here P.P. 07/27/17
  sprintf(str,"F:%3.0fC, O: % 3.0fC, Inj: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f%",
	temp_fid, temp_oven, temp_det, detA, detB, pressure_cylinder, humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 07/27/17
#endif // !pressure sensor
// end of new code for 301 with FID
#endif // !FID()
#endif // gc312
#else  // added for concentrator. P.P. 02/20/06, oven!=det
#if !HEATED_LINE() // cond added. P.P. 03/31/17
#if !PRESSURE_SENSOR()
#if !HUMIDITY_SENSOR()
  sprintf(str,"Oven: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V",
	temp_oven, temp_det, temp_conc, detA, detB);  // P.P. 02/20/06
#else //with the humidity sensor
  sprintf(str,"Oven: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, RH: %2.2f %",
	temp_oven, temp_det, temp_conc, detA, detB, humidity_value);  // P.P. 06/26/17
#endif // !humidity sensor. P.P. 06/26/17
#else // with pressure to be shown. P.P. 06/26/17
#if !HUMIDITY_SENSOR()
#if H2_PRESSURE_SENSOR()
#if FID()
  sprintf(str,"O: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi",
	temp_oven, temp_det, temp_conc, temp_fid, detA, detB, pressure_cylinder, H2_pressure);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi",
	temp_oven, temp_det, temp_conc, detA, detB, pressure_cylinder, H2_pressure);  // P.P. 11/24/17
#endif // fid
#else // without h2_pressure_sensor
#if FID()
  sprintf(str,"O: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi",
	temp_oven, temp_det, temp_conc, temp_fid, detA, detB, pressure_cylinder);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Inj: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi",
	temp_oven, temp_det, temp_conc, detA, detB, pressure_cylinder);  // P.P. 11/24/17
#endif // fid
#endif // h2 pressure sensor
#else //with the humidity sensor
// new
#if H2_PRESSURE_SENSOR()
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure, humidity_value);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, H2: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder, H2_pressure, humidity_value);  // P.P. 11/24/17
#endif // fid
#else // withour h2_pressure_sensor
#if FID()
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, FID: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, temp_fid, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 11/24/17
#else // without fid
  sprintf(str,"O: % 3.0fC, Dsb: % 3.0fC, A: % 6.3f V, B: % 6.3f V, P: %4.0f psi, RH: %2.2f %",
	temp_oven, temp_conc, ShowVolts(0), ShowVolts(1), pressure_cylinder, humidity_value);  // P.P. 11/24/17
#endif // fid
#endif // h2 pressure sensor
// end of new
#endif // !humidity sensor. P.P. 06/26/17
#endif // !pressure_sensor
#else // with line temperature shown
  sprintf(str,"Oven: % 3.0fC, Dsb: % 3.0fC, Line: % 3.0fC, A: % 6.3f V, B: % 6.3f V",
	temp_oven, temp_conc, temp_line, ShowVolts(0), ShowVolts(1));  // P.P. 03/31/17  line temperature shown
#endif // !heated_line
// temp, to show ser_num. P.P. 03/12/17
//  sprintf(str,"S/N: %8x", mfg_num);  // P.P. 03/17/17
//  sprintf(str,"S/N: %x", serialNum);  // P.P. 03/17/17
#endif // !concentrator
#endif // pmd
// end of copy

#else // with flow to be shown - left as it was at this time. P.P. 07/27/18
  double Get_Flow();                 //
  double Get_Flow_S(short channel);  // added here. P.P. 08/02/05
#if FID() // case added. P.P. 08/05/05
  sprintf(str,"Oven:% 3.0fC,  FID:% 3.0fC,  DetA: % 6.3f V,  S: % 4.0f ccm",              // Column flow not shown. P.P. 09/16/05
      temp_oven, temp_det, ShowVolts(0), flow_value); // changed to show sample flow. P.P. 08/15/04. Changed 06/18/11
#else
#if GC326() // case added, show column flow. P.P. 12/08/18
  sprintf(str,"Oven: % 3.0fC, Det: % 3.0fC, A: % 6.3f V, B: % 6.3f V,  Flow: % 4.0f ccm",
       temp_oven, temp_det, ShowVolts(0), ShowVolts(1), flow_value); // changed to show column flow. P.P. 12/08/18.
#else
  sprintf(str,"Oven: % 3.0fC, Det: % 3.0fC, A: % 6.3f V, B: % 6.3f V,  S: % 4.0f ccm",
       temp_oven, temp_det, ShowVolts(0), ShowVolts(1), flow_value); // changed to show sample flow. P.P. 08/15/04. Changed 09/03/05
#endif // gc326
#endif // fid. 08/05/05
#endif // flow
#endif // oven=inj
    if(Board_error) {                       // added. P.P. 06/20/03
        sprintf(str_error, "A/D board error");//
        pMainWindow->sl->SetStatus(str_error);// safety_warning cond added to all set status->sl. P.P. 04/17/09
    }                                       //
    else {                                  //
        pMainWindow->sl->SetStatus(str);
    }                                       //
#elif STAND_ALONE()&&GC321() // case added to show temperatures. P.P. 07/14/06
    sprintf(str, "Oven: % 3.0fC, Det A: % 6.3f V,  Det B: % 6.3f V",
        temp_oven, ShowVolts(0), ShowVolts(1));
  pMainWindow->sl->SetStatus(str);
#elif STAND_ALONE()&&MANUAL_INJ() // case added for stand-alone unit with temperature control. P.P. 01/04/16
  sprintf(str, "Oven: % 3.0fC, Det A: % 6.3f V,  Det B: % 6.3f V",
        temp_oven, ShowVolts(0), ShowVolts(1));
  pMainWindow->sl->SetStatus(str);
#else // stand_alone - show only volts. added. P.P. 08/01/03
  sprintf(str, "Det A: % 6.3f V,  Det B: % 6.3f V",
        ShowVolts(0), ShowVolts(1));
  pMainWindow->sl->SetStatus(str);
#endif // !stand_alone
    pMainWindow->bHeat = oven_on;
    pMainWindow->ReflectHeat(pMainWindow->bHeat);

    pMainWindow->bSample = sample_on;  // added. P.P. 03/14/17
    pMainWindow->ReflectSample(pMainWindow->bSample);
    pMainWindow->bEquilibrate = equilibrate_on;  // added. P.P. 03/14/17
    pMainWindow->ReflectEquilibrate(pMainWindow->bEquilibrate);
    pMainWindow->bInject = inject_on;  // added. P.P. 03/14/17
    pMainWindow->ReflectInject(pMainWindow->bInject);
#if CONCENTRATOR() // cond added, fan cools desorber. P.P. 12/12/18
    pMainWindow->bFan = fan_on;  // added. P.P. 03/18/17
#else
    pMainWindow->bFan = oven_door_on;  // fan indicator related to oven door. P.P. 12/12/18
#endif  // concentrator
    pMainWindow->ReflectFan(pMainWindow->bFan);
#if !OVEN_EQ_INJ() // case added. P.P. 12/12/18
    pMainWindow->bHeatInj = det_on; // P.P. 12/12/18
  pMainWindow->ReflectHeatInj(pMainWindow->bHeatInj);
#endif
#if CONCENTRATOR()
    //  pMainWindow->bDesorb = desorb_on;  // added. P.P. 03/14/17
  pMainWindow->bDesorb = dsb_on;  // added. P.P. 03/15/17
//  pMainWindow->bDesorb = det_on;  // added. P.P. 03/15/17
  pMainWindow->ReflectDesorb(pMainWindow->bDesorb);
  pMainWindow->bPurge = purge_on;  // added. P.P. 03/18/17
  pMainWindow->ReflectPurge(pMainWindow->bPurge);
//  pMainWindow->bFan = fan_on;  // added. P.P. 03/18/17
//  pMainWindow->ReflectFan(pMainWindow->bFan);
  pMainWindow->bInj_V4 = inj_V4_on;  // added. P.P. 03/18/17
  pMainWindow->ReflectInj_V4(pMainWindow->bInj_V4);
#endif
#if HEATED_LINE()
    pMainWindow->bLineHeat = line_heat_on;  // added. P.P. 03/28/17
  pMainWindow->ReflectLineHeat(pMainWindow->bLineHeat);
#endif
#if FLOW_RATE()
    pMainWindow->bFlowLow = X5_valve_on;  // added. P.P. 05/13/17
  pMainWindow->ReflectFlow(pMainWindow->bFlowLow);  //
#endif
#endif // serial_number && demo
}

#endif // INTERNAL()

#define  NUM_NOTIFIES         7
#define  MD_REC_CNT  10                // # empty recs. in a row to be an error
#define  MD_HEAD     4                 // len of header part of "MD" message
#define K_NUM_ERR 255                       // length byte representing error

    void FindRefComp(Method *pMethod, int *ref_comp_idx)
    {
        for(int n=0; n<NUMDETECTORS; n++) {
            ref_comp_idx[n] = pMethod->det[n].FindRefCompIndex();
        }
    }

//#if POINTS()>1
#if SAMPLE_AHEAD() // cond replacedfor this one- 1 point and sampling ahead is OK now. P.P. 11/07/17
    int get_available_point(ulong points, int pointnumber)
{
int p;
for(p=pointnumber; p<POINTS(); p++) {
if( (1L<<p) & points )
goto end;
}
for(p=0; p<pointnumber; p++) {
if( (1L<<p) & points )
goto end;
}
end:
return p;
}
#endif

    static int acq_runs=0; // 1 runs per acquire
// 4 runs per cal. acquire
//static uint32 huge *pZERO[NUMDETECTORS]={0};
//static uint32 huge *pCurZERO[NUMDETECTORS]={0};
//static int cur_gain[2] = {0};                        // added P.P. 03/02/02
// static int cur_gain[2] = {2,2};                        // replaced P.P. 04/22/06 rem here as restored earlier. P.P. 05/12/17
    BOOL bForceCalibrateNextRun = FALSE;  // added W.K. 11/23/02 for remote cal start. P.P.
    BOOL bForceCheckCalibrateNextRun = FALSE;

// acq_mode - old name cur_mode == MANUAL,...
    Acquire::Acquire(Comm *_pComm, int _acquire_type, int /*acq_mode*/, int _nStandard, Method *_pMethod, PeakList **_pPeakList, Run *_pRun, Job *_pJob, int _pointnumber, MainWindow *_pMainWindow)
    : acquire_type(_acquire_type), nStandard( _nStandard)
            , pMethod(_pMethod), pRun(_pRun),
            waiting_for_response(FALSE),
            comm_state(GET_STATUS)
            , ndata(0)
            ,Integration(_pMethod, _pPeakList, _pRun, _acquire_type, _nStandard)
#if INTERNAL()
            ,WTimer(_pMainWindow->HWindow, 1, 100, NULL) // ZLP
        , flagsAlarm(0)
        , flagsLamp(0x8000) // unknown state
        , dwLastTime(0)
        , tempcontrol(0)
        , dwUV_0_Time(0)
        , dwUV_Max_Time(0) // added. P.P. 09/26/03
#if X1_VALVE()  // cond changed. P.P. 07/10/05
        , dwXvalveTime(0)      //
        , dwXvalveOffTime(0) // added. P.P. 06/18/06
        // valves 2 - 5 added here. P.P. 11/12/10
        , dwXvalve2Time(0)      //  3 more valves. P.P. 06/20/10
        , dwXvalve2OffTime(0)
        , dwXvalve3Time(0)      //  3 more valves. P.P. 06/20/10
        , dwXvalve3OffTime(0)
        , dwXvalve4Time(0)      //  3 more valves. P.P. 06/20/10
        , dwXvalve4OffTime(0)
        , dwXvalve5Time(0)      //  3 more valves. P.P. 06/20/10
        , dwXvalve5OffTime(0)   // added. P.P. 07/03/10
        , dwAutoZero1(0)       //
        , dwAutoZero1_off(0)       //
        , dwRange_1(0)   // added. P.P. 09/18/15
        , dwRange_1_off(0)   // added. P.P. 09/18/15
        , dwAutoZero2(0)       //
        , dwAutoZero2_off(0)       //
        , dwRange_1_2(0)   // added. P.P. 06/20/17
        , dwRange_1_2_off(0)   // added. P.P. 09/18/15
        , dwAutoZero3(0)       //
        , dwAutoZero3_off(0)       //
        , dwAutoZero4(0)       //
        , dwAutoZero4_off(0)       //
//, dwRange_1_3_off(0)   // added. P.P. 09/18/15 rem P.P. 06/20/17

#endif // x1_valve       //
//not used , dwTemperatureDelayTime(0)   // added. P.P. 12/30/04
#if FID()           // added. P.P. 07/03/05
        , dwFidDelayTime(0) //
, dwFidTempTimeout(0)
, dwHydrogenDelayTime(0) // added. P.P. 09/20/05
#endif // fid       //
#if RAMPING() // case added. P.P. 06/23/06
        , dwOvenHoldTime1(0)      // 8 lines added. P.P. 06/23/06
, dwOvenHoldTime2(0)
, dwOvenHoldTime3(0)
, dwOvenRamp1Time(0)
, dwOvenRamp2Time(0)
#endif // ramping
#else // not internal
#endif // internal
            ,pMainWindow(_pMainWindow)
            ,dwRepeatTime(0)
            , pointnumber(_pointnumber)
            , bRepeatCalSend(FALSE)
            , dwShowMessageTime(0) // added. P.P. 11/01/09
#if QUICK_SCAN() // added. P.. 12/31/10
            //, quick_scan_active(1)   // P.P. 01/02/11
, quick_scan_active(0)   // P.P. 01/17/11
//, NextPointNumber(pointnumber) // P.P. 01/15/11 moved down. P.P. 09/01/18
#endif
            , NextPointNumber(pointnumber) // P.P. 01/15/11
    {

        ::_pMainWindow=pMainWindow; // DEBUG
        ::bForceCalibrateNextRun=FALSE;  // added W.K. 11/23/02 for remote cal start. P.P.
        ::bForceCheckCalibrateNextRun=FALSE;

#if INTERNAL()
        #if DIAGNOSTICS()
    Fatal_error=0;
#endif
#if FID()        // added. P.P. 07/03/05
    start_fid = 0; //
  FID_error=0; // added. P.P. 08/05/05
  FID_error_event = 0; // P.P. 08/07/05
#endif // fid    //
    {
        for(int n=0; n<NUMDETECTORS; n++) {
            uv_0[n]=-32000;
//       uv_0[n]=0;        // temp P.P. 11/22/09
            uv_Max[n]=32000;  // added. P.P. 09/26/03
//       uv_Max[n]=0;  // temp. P.P. 11/22/09
        }
    }

#endif //INTERNAL()

        _pJob->GetRawData(pRun);
        pCurRawData=pRun->runrawdata.pRawData32;

        pCurRawDataLastPainted=0;
        pRun->runrawdata.RawDataSize=0;
        pRun->bSkipTWA = 0; // added. P.P. 10/01/05
        if(pMainWindow->sl) pMainWindow->sl->SetStatus("Inactive");

#if SAMPLE_AHEAD() // cond replaced, 1 point and sampling ahead is OK now. P.P. 11/07/17
        ulong points=pMethod->method_table.points;
#if !QUICK_SCAN()  // added. P.P. 01/15/11
    pointnumber=get_available_point(points, pointnumber);
#else
    pointnumber=get_available_point(points, NextPointNumber);  // P.P. 01/15/11
#endif // !quick scan
#else
        pointnumber=0;
#endif

        pRun->pointnumber=pointnumber;

#if POINTS()>1
        pMainWindow->ReflectRun();
#endif

        if(acquire_type == METHOD) {
            pRun->comment="Method Download";
        }
        else if(acquire_type == CAL) {
            pRun->comment="Calibration";
        }
        else if(acquire_type == CHECKCAL) {
            pRun->comment="Check Calibration";
        }
        else { // JOB
#if POINTS()>1
            Str buff(48);
#if !QUICK_SCAN() // added. P.P. 01/17/11
        wsprintf(buff.GetPtr(), "pt#%d", 1+pRun->pointnumber);
#else // with quick scan
        if(quick_scan_active) {  // case added. P.P. 01/13/11
      wsprintf(buff.GetPtr(), "quick scan pt#%d", 1+pRun->pointnumber);
    }
    else { // as it was
      wsprintf(buff.GetPtr(), "pt#%d", 1+pRun->pointnumber);
    }
#endif // !quick scan
        pRun->comment=buff;
#endif
        }

#if INTERNAL()
        pRun->runrawdata.nRawDataFormat=2;
    for(int n=0; n<NUMDETECTORS; n++) {
        Detector *pDetector=&pMethod->det[n];
//    pRun->runrawdata.gain[n]=0;// now gain is in 32bit DATA pDetector->gain;
        pRun->runrawdata.gain[n]=pDetector->gain; //  replaced, to have the initial range as in Dee\tector dialog. P.P. 05/19/17
#if 0 /*HIGH_CONC_POINTS()*/  // cond added. P.P. 01/22/11  disabled, to set range from dialod. P.P. 06/24/11
        if(pointnumber < 4) {
      gain[n]=3; // decrease the signal for 4 points, #1-#4. P.P. 01/30/11
    }
    else {
      gain[n]=pDetector->gain; // initalize current gain from method
    }
#else // as it was
        gain[n]=pDetector->gain; // initalize current gain from method
        gain_1[n]=pDetector->gain_1; // added. P.P. 11/09/12
        cur_gain[n] = gain[n]; // added. P.P. 05/19/17
#endif // high concentration points
    }
#else //not INTERNAL
        // code removed. P.P. 09/01/18
#endif  // internal

        TRACE1("Acquire::Acquire =%p", this);

#if MYDEBUG()
        _i_demo=0;
  _J_demo=0;
  _ran= _ran==1? 2: 1;
#endif

        if(_acquire_type==CAL) {
            switch(runMode.Acquire) {
                case 1: // Continuous mode
                case 2: // Repeat mode
                case 3: // Remote mode. Added P.P. 12/07/02
                    // Added to turn filter_valve OFF if not the 1st run in calibration. P.P. 01/08/02
                    if(acq_runs<1) { // first time (not after failed cal)
                        acq_runs=3; //   restored. P.P. 04/12/05
//	acq_runs=5; //   increased to skip 1st and 2nd cal run anyway. P.P. 03/17/05
                    }
                    break;
            }      // Removed and
        }
        else {
#if GC501() // moved. P.P. 12/24/10
            if(pMethod->method_table.check_alarm) {
      switch(runMode.Acquire) {
	case 1: // Continuous mode
	case 2: // Repeat mode
	case 3: // Remote mode.
	  if(acq_runs<1) { // first time (not after alarm)
	   acq_runs=2; //  repeat on alarms
	  }
	break;
	default:
 	  acq_runs=1;
      }
    }
    else {
      acq_runs=1;
    }
#elif QUICK_SCAN() // case added. P.P. 12/24/10
            if(pMethod->method_table.check_alarm) {
      switch(runMode.Acquire) {
	case 1: // Continuous mode
	case 2: // Repeat mode
	case 3: // Remote mode.
	  if(acq_runs<1) { // first time (not after alarm)
	   acq_runs=2; //  repeat on alarms
	  }
	break;
	default:
 	  acq_runs=1;
      }
    }
    else {
      acq_runs=1;
    }
#else // not 501 moved. P.P. 12/24/10
            acq_runs=1;
#endif
        }

//// Check the time for new TWA cycle.
        DWORD dwTime=GetTime();
        switch(runMode.Acquire) {
            case 1: // Continuous mode
            case 2: // Repeat mode
                if(pMethod->method_table.TWA_cycle) {
                    if(TWA_cycle_start_time+pMethod->method_table.TWA_cycle*3600000L<=dwTime) {
#if GC501()||SHIFT_REPORT() // cond and code below added. P.P. 08/18/04
                        // added from uv101303. P.P. 08/18/04
	  {
//	    void CheckShiftReportTime(MainWindow *pMainWindow, BOOL bAlarm);
	    void CheckShiftReportTime(MainWindow *pMainWindow, int nAlarm); // P.P. 11/14/03
//	    CheckShiftReportTime(pMainWindow, FALSE);
	    CheckShiftReportTime(pMainWindow, 2); // P.P. 11/14/03
	  }
// end of added code.
#endif // gc501 or shift_report. P.P. 08/18/04
                        InitTWA();
                    }
                }
                break;
        }
//// End of TWA cycle check.
    }

    Acquire::~Acquire()
    {
        if(acq_runs>0) acq_runs--;

        TRACE1("Acquire::~Acquire =%p", this);

        if(pMainWindow->sl) {
#if FID()
            if((!Fatal_error)&&(!FID_error)) { // cond added to leave fatal exit message. P.P. 08/07/05
#else
            if(!Fatal_error) { // cond added to leave fatal exit message. P.P. 08/07/05
#endif
                pMainWindow->sl->SetStatus(0);
                pMainWindow->sl->Replace(0); // not work when press on toolbar "stop"
            }
        }

///?????
        switch(comm_state) {
            case RECEIVING_DATA:
            case START_ACQUIRE:
            case START_CAL_ACQUIRE:
            case WAIT_SAMPLE_TIME:   // added 2 lines. P.P. 10/12/02
            case WAIT_START_INJECT:
            case WAIT_TEMPERATURES:  // added. P.P. 08/12/03
#if FID() // added. P.P. 07/03/05
                #if !GC312() // cond added. P.P. 06/05/09
FlameDetector(FALSE);
start_fid=0; // added. P.P. 07/26/05
#endif //!gc312
#endif // fid
                if(dwMotorOffTime && (dwMotorOffTime<GetTickCount())) {   // added to turn motor OFF in 3 s. P.P. 10/04/11
                    DoorMotor(FALSE);
                    dwMotorOffTime = 0;
                }
                comm_state = END_ACQUIRE;
//Continue(); call SendEndAcquire(); to prevent PostMessage CM_XXX
                SendEndAcquire();
                if(ndata) {
                    WriteRawData(rdata, (ndata+1)*sizeof(rdata[0]));  // write raw data to the end of...
                    ndata=0;
                }
                break;
        }
#if INTERNAL()
        //// added to stop scan in case of end run. P.P. 08/16/02
//// end of added 08/16/02
#if !STAND_ALONE() // added. P.P. 08/11/03
/// moved here from below. P.P. 11/06/02
// "IF" condition added to leave oven temperature as it is on end acquire if not ramping. P.P. 07/11/02
if(tempcontrol) {  // added in this variant 07/23/02
// if(pMethod->method_table.RampingEnabled) {    // P.P. replaced. P.P. 09/29/02
delete tempcontrol;
}
// added. P.P. 09/29/02
#if !GC312() // added. P.P. 03/19/06
if(!Fatal_error) {  // cond. added, to leave it ON if not on emergency stop. P.P. 08/10/04
}
#endif // P.P. 03/19/06
#endif // !stand_alone. added. P.P. 08/11/03
////end of moved code.
/*                 // replaced to left diagn alarm as it was. P.P. 12/04/04
  if(flagsAlarm) {
    Alarm(FALSE);
  }
*/
#if !GC312() // added. P.P. 03/19/06
if(!(pMethod->method_table.alarms_on_off)) {  // replacement P.P. 12/04/04
Alarm(FALSE);                              //
}                                            //
else {                                       //
Alarm(Diagn_error||Diagnostics_error);     // ignore H_L alarms
}                                            //
#endif // !gc312

//(pRun->comment+=", Fatal:")+=Fatal_error_Str;
#if FID()
if(Fatal_error||FID_error) { // added. P.P. 08/07/05
#else
if(Fatal_error) { // cond added. P.P. 09/20/04
#endif // fid
//    (pRun->comment+=", ")+=Fatal_error_Str;
(pRun->comment+=" ")+=Fatal_error_Str; // P.P. 09/20/04
}
#if FID()
else if(FID_error_event) { // added. P.P. 08/07/05
pRun->comment+=" FID error";
}
#endif // fid

#if DIAGNOSTICS()
#if FID()                 // fid case added. P.P. 08/07/05
if(Fatal_error||FID_error) {
#else
if(Fatal_error) {
#endif // fid
MessageBeep(MB_ICONHAND);
MessageBox(pMainWindow->HWindow,Fatal_error_Str,"WPeak Fatal Error", MB_OK|MB_ICONHAND);
}
#endif //DIAGNOSTICS()

#endif // INTERNAL()

        TRACE1("Acquire::~Acquire end=%p", this);
    }


    int Acquire::CheckCalibration()
    {
#if !STAND_ALONE() // added. P.P. 07/14/03
        if(acquire_type == CAL) {
        int bFailed=0;
        for(int n=0; n<NUMDETECTORS; n++) {
            if(pIntegrator[n]) {
                // Check if the detector failed to calibrate.
                if(!pIntegrator[n]->cal_passed) {
                    static char far str[]=", X FAILED";
                    str[2]='A'+n;
                    pRun->comment+=str;
                    bFailed=1;
                }
            }
        }
        switch(runMode.Acquire) {
            case 1: // Continuous mode
            case 2: // Repeat mode
            case 3: // Remote mode - added 12/06/02. P.P.
                if(bFailed) { // restored. P.P. 03/19/06
//      if(bFailed||(acq_runs>3)) { // changed to ignore 1st and 2nd cal runs anyway. P.P. 03/17/05
//        if(acq_runs>0) { // Something to retry// P.P. 11/24/01. Does not stop.
                    if(acq_runs>1) { // Stops after 3 failed cal runs. P.P.
                        bRepeatCalSend=TRUE;
                        PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_CALIBRATE1, 0L);
                        TRACE("PostMessage CM_CALIBRATE1 -  retry calibration");
                        return -1; // STOP
                    }
                    else { // nothing to retry
                        TRACE("Calibration failed in continiuos mode");
                        pMainWindow->sl->Replace("Calibration failed in continuous mode");
#if INTERNAL()
                        if(pMethod->method_table.alarms_on_off) {
#if !GC312() // cond added. P.P. 03/19/06
                            Alarm(TRUE); // comments: internal alarm ON if cal failed, inependently on diagn. P.P. 12/04/04
#endif // gc312
                            Set_alarm_bit(control_port_offset, 5, TRUE); // external diagn alarm added. P.P. 06/28/03
                            Diagnostics_Alarm(diagnostics_bit_number, TRUE); // replaced. P.P. 05/31/04 moved down.
                            //PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_STOPINTEGRATION, 0L); //??
//P.P. 12/15/01. Removed until error fixed.            ::MessageBox(pMainWindow->HWindow,"Calibration failed in continiuos mode\n Operation aborted!","Error",MB_OK|MB_ICONSTOP|MB_TASKMODAL);
//            return -1; // STOP    // P.P. 01/08/02. Moved to stop without alarm also.
                        }
                        Cal_error = 1; // added
#if !GC312() // cond added. P.P. 03/19/06
                        SetCalValve(TRUE); // close cal gas. added. P.P. 03/23/05
#endif // gc312
// cal failed - diagn alarm, does't depend on Alarm box. P.P. 05/31/04
#if PMD_12LS()&&PMD_16FS() // case aded for 4-pnt GS301 with non-usb relays. P.P. 03/03/07
#if POINTS()<=4
                        if(pMethod->method_table.alarms_on_off) {
	    Diagnostics_Alarm(cal_error_bit_number, Cal_error); //
	  }
	  else {
	    Diagnostics_Alarm(cal_error_bit_number, FALSE); //
	  }
#endif // points<=4
#else // previous setting. P.P. 11/06/05
                        End_run = 1; // added. P.P. 12/07/02 - run finished but with cal_error.
#endif // pmd. P.P. 11/06/05
                        SetLamps(FALSE);
#endif //INTERNAL()
                        comm_state = END_ACQUIRE; // also restored as of 01/29/02
                        return -1; // STOP
                    }
                }
// removed. P.P. 12/07/02  and restored 12/12/02
                else { // bfailed = 0 - cal passed. Added 12/06/02 P.P.  But the same is in immediate stop on cal_passed.
#if !GC312() // cond added. P.P. 03/19/06
                    SetCalValve(TRUE); // added. P.P. 03/23/05
#endif // gc312
#if INTERNAL()
//        Cal_error = 0;  Removed - done on first detection cal_passed. P.p. 12/12/02
                    End_run = 1;
#endif //INTERNAL()
                }

        }
    }
#endif // !stand_alone. P.P. 07/14/03
        return 0;
    }

#if INTERNAL()
    static int IsInRange(Method *pMethod, long curr_rt, long window_delta_minus, long window_delta_plus)
{
int bInRange=0;
//  for(int n=0; n<1/*NUMDETECTORS*/; n++) { // only first now
for(int n=0; n<NUMDETECTORS; n++) { // Changed to turn lamp ON in any of windows. P.P. 11/01/02
Detector *pDetector=&pMethod->det[n];
if(pMethod->det[n].detect_stat != 0) { // active detector??
if(!pDetector->lamp_save) {
//        bInRange=1; // alway want lamp
bInRange += BIT[n]; // 1 or 2, for A or B det. P.P. 11/01/02
}
else {
Detector *pDetector=&pMethod->det[n];
for(int i=0; i!=COMPONENT_ROWS; i++) {
Component *pComponent=&pDetector->components[i];
if(pComponent->active_yn) { // If component ON,...
long lmin=pComponent->peak_rt-pComponent->window-window_delta_minus;
long lmax=pComponent->peak_rt+pComponent->window+window_delta_plus;
if((lmin<=curr_rt) && (curr_rt<=lmax)) {
//              bInRange=1;
bInRange += BIT[n]; // 1 or 2, for A or B det. P.P. 11/01/02
break;
}
}
}
}
}
//    if(bInRange)  // This should be removed when there will be 2 separate lamps. P.P. 11/06/02
//      break;      // Removed 2 lines. P.P. 01/02/03 - there are now 2 separate lamps.
}
return bInRange;
}

void Acquire::CheckLamp(long curr_rt)
{
//#if DEMO() // added. P.P. 10/05/04
//#if DEMO()||(TCD()&&FID()) // changed - no lamp present if tcd + fid. P.P. 02/18/11 here 03/02/12
#if DEMO()||(TCD()&&FID())||WATER_PUMP() // P.P. 01/30/17. changed - no lamp present if tcd + fid. P.P. 02/18/11 here 03/02/12
    return;
#endif
#if !STAND_ALONE() // added. P.P. 08/11/03
// added to get lamp error as fid error. P.P. 07/29/05
#if FID()
    #if !OVEN_EQ_INJ()
   Lamp_error = 0; // added P.P. 08/07/05
   BOOL Lamp_error_event = 0;
//   double meas_temperature=Get_Temperature(1); // channel for Injector Temp. Rem P.P. 10/01/05
   const double fid_delta=10.0;
//   if(meas_temperature <((pMethod->method_table.inj_det_temp)-fid_delta)) {  // replaced. P.P. 10/01/05
   if(temp_det <((pMethod->method_table.inj_det_temp)-fid_delta)) {            // 10/01/05
     Lamp_error_event = 1;
   }
//   else if(meas_temperature > ((pMethod->method_table.inj_det_temp) - 2.0)) { // replaced. P.P. 10/01/05
   else if(temp_det > ((pMethod->method_table.inj_det_temp) - 2.0)) {           // 10/01/05
     Lamp_error_event = 0;
   }
   else { // stays as it is
   }
// code added to set error flag if errors during >=1 s. P.P. 08/07/05
   if(Lamp_error_event) Lamp_error_count += 1;
   else Lamp_error_count = 0;
   if(Lamp_error_count > 20) {
     Lamp_error = 1;     //
     FID_error_event = 1; // if happened 20 times - mark this event in Run Comment. P.P. 08/07/05
     pRun->bSkipTWA = 1; // added. P.P. 08/27/05
   }
   else Lamp_error = 0;                         //
// end of added code. P.P. 08/07/05
#endif // oven_eq_inj
// end of changed code. P.P. 07/29/05
#else  //regular case - not FID. P.P. 02/20/09              //
//  int Diagnostics_Lamp();                      // added P.P. 11/17/02
    int Diagnostics(int diagn_port); // P.P. 09/20/04              // P.P. 09/19/04
    int Lamp_error_flag = 0;    // bit 0 and bit 1 of PORTB (configured for input). P.P. 11/27/02
    BOOL Lamp_error_event = 0; // added. P.P. 09/23/03
//  BOOL Lamp_error_event_B = 0; // added. P.P. 06/09/06 and removed
//  if(acquire_type == CAL) {  // moved down to separate lamps A and B. P.P. 12/31/02
//    Lamp(TRUE);               // still only common lamp ON - add later LampB and choice. P.P. 11/17/02
//  int Lamp_on = 0;
//  byte Lamp_on = 0;
    USHORT Lamp_on = 0;  // 04/12/08
    for(int n=0; n<NUMDETECTORS; n++) {
        Detector *pDetector=&pMethod->det[n];
        if(pMethod->det[n].detect_stat != 0) { // active detector??
            Lamp_on += BIT[n]; // 1 or 2, for A or B det. P.P. 11/01/02
            lamp_state[n]=1; // 03/21/08
        }
        else {
            lamp_state[n]=0; // 03/21/08
        }
    }
//    Lamp(TRUE);// rem. 12/31/02     // still only common lamp ON - add later LampB and choice. P.P. 11/17/02
    if(acquire_type == CAL) { // then the lamp is ON if the detector is active.
//    Lamp(1 & Lamp_on);  // should be this way instead. P.P. 11/17/02
//    LampB(2 & Lamp_on); // to turn lamp B if det B checked active. P.P. 12/31/02
        Lamp((BIT[0] & Lamp_on) > 0);  // changed. P.P. 01/02/03
        LampB((BIT[1] & Lamp_on) > 0); // P.P. 01/02/03
//    Lamp_error_flag = Diagnostics_Lamp();
//#if GC312() // case added. P.P. 03/19/06
#if CBW() // cond added. P.P. 05/05/07
//#if GC312()||GC321() // changed. P.P. 03/19/06
#if GC312()||GC321()||GC326() // P.P. 12/07/17
        #if MANUAL_INJ() // case added. P.P. 06/09/06
    int Gain = BIP10VOLTS; // default.
    uint16 data_lamp=0;
    uint16 data_lamp_B = 0;      // added. P.P. 06/09/06
#if PMD_16FS()
    if(pMethod->det[0].detect_stat != 0) { // active detector A?
      My_cbAIn(CBWDEVICE_16FS, lamp_channel, Gain, (USHORT *)&data_lamp);
    }
    if(pMethod->det[1].detect_stat != 0) { // active detector B?
      My_cbAIn(CBWDEVICE_16FS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
    }
#elif PMD_12FS()
    if(pMethod->det[0].detect_stat != 0) { // active detector A?
      My_cbAIn(CBWDEVICE_12FS, lamp_channel, Gain, (USHORT *)&data_lamp);
      data_lamp = data_lamp&0x0FFF)*16;  // variant. P.P. 11/28/04
    }
    if(pMethod->det[1].detect_stat != 0) { // active detector B?
      My_cbAIn(CBWDEVICE_12FS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
      data_lamp_B = data_lamp_B&0x0FFF)*16;  // variant. P.P. 11/28/04
    }
#elif PMD_12LS()
    if(pMethod->det[0].detect_stat != 0) { // active detector A?
      My_cbAIn(CBWDEVICE_12LS, lamp_channel, Gain, (USHORT *)&data_lamp);
      data_lamp = (data_lamp&0x0FFF)*16;  // variant. P.P. 11/28/04
    }
    if(pMethod->det[1].detect_stat != 0) { // active detector B?
      My_cbAIn(CBWDEVICE_12LS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
      data_lamp_B = (data_lamp_B&0x0FFF)*16;  // variant. P.P. 11/28/04
    }
#endif // pmd_16
// replacement for analyzing in binary form. P.P. 06/10/06
    if((data_lamp>40960)||(data_lamp_B>40960)) {
      Lamp_error_event = 1;
    }
    else if((data_lamp<35388)&&(data_lamp_B<35388)) {
      Lamp_error_event = 0;
    }
    else {
    }

// end of added code for manual_inject case. P.P. 05/19/06
#else // not manual, using relays for valves
//    USHORT BitValue;
    USHORT BitValue = 0; // P.P. 11/12/10
//#if PMD_16FS()&&PMD_12LS() // added 03/21/08 - use dig inputs of 1208ls board
#if PMD_16FS() // separated. P.P. 11/12/10
    My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
    if(BitValue) Lamp_error_event = 1;  // only one lamp
    else Lamp_error_event = 0;
#endif // pmd-1608FS. P.P. 11/12/10
#endif // manual_inj
#else  // regular case, not GC312 or GC322
#if USB_3112() //  cond changed temp, with pmd_12LS -
        //- use pmd_16FS. P.P. 02/18/07
    Lamp_error_flag = Diagnostics(diagnostics_port); // P.P. 09/19/04
//    Lamp_error = ((Lamp_on & Lamp_error_flag) != 0); // P.P. 11/17/02. Can give error if Det B checked but no lamp.
    Lamp_error_event = ((Lamp_on & Lamp_error_flag) != 0); // replaced. P.P. 09/23/03.
#else  // PP_TEMP: code added to use aux port, as in gc312. P.P. 04/30/06
//    USHORT BitValue;
        USHORT BitValue = 0; // P.P. 05/04/07
#if PMD_16FS() // cond added. P.P. 05/04/07
        My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
#endif // PMD16 or DDA
        if(BitValue) Lamp_error_event = 1;  // only one lamp
        else Lamp_error_event = 0;
#endif // usb_3112||pmd_12LS
#endif // gc312
#endif // cbw. added 05/05/07
        if(Lamp_error_event) Lamp_error_count += 1;  // Added code. P.P. 09/23/03
        else Lamp_error_count = 0;                    //
        if(Lamp_error_count > 20) Lamp_error = 1;     //
        else Lamp_error = 0;                         //
        //  High_Alarm(1, Lamp_error); // rem temp. P.P. 11/20/02                  //
//#if !(GC312()||GC321()) // cond added. P.P. 03/19/06
#if !(GC312()||GC321()||GC326()) // P.P. 12/07/18
        // code for internal alarm removed. P.P. 08/31/18
#endif // !gc312
        return;
    }
//  long window_delta_minus=12; // sec
//  long window_delta_plus=10; //sec
//  long window_delta_minus=6; // sec  Made them shorter. P.P. 06/12/01
//  long window_delta_plus=6; //sec
    long window_delta_minus=3; // sec  even shorter. P.P. 01/04/02. ret to 3s. P.P. 08/26/03
//  long window_delta_minus=7; // 7 sec  to test Jack's FUV approach in lamp_save. P.P. 08/22/03
    long window_delta_plus=0; //sec

    long between_delta=3; // sec - minimal time between On/Off

    int bLamp=IsInRange(pMethod, curr_rt, window_delta_minus, window_delta_plus);
    if(!bLamp) { // lamp not needed
//    if(flagsLamp&1) { // if lamp already On
        if(flagsLamp&3) { // if lamp already On. Changed for 2-det case. P.P. 11/01/02
            int bNext=IsInRange(pMethod, curr_rt, window_delta_minus+between_delta, window_delta_plus);
            if(bNext) { // next peak is close enough
//        bLamp=1; // lamp will be needed // change later if 2 lamps. P.P. 11/01/02
                bLamp = bNext; // lamp(s) will be needed //. P.P. 11/01/02
            }
        }
    }
//  Lamp(bLamp);

#if !HV_501() // added. P.P. 08/22/03
    if(bLamp) Lamp(TRUE);  // changed - bLamp can be 0 through 3 with 2 detectors. P.P. 11/01/02
  else Lamp(FALSE);
#else // 501 HV board. P.P. 08/22/03

// P.P. 12/31/02. Added to control lamps separately.
    if(bLamp & BIT[0]) Lamp(TRUE);
    else Lamp(FALSE);
    if(bLamp & BIT[1]) LampB(TRUE); // LampB(BOOL on) created. P.P. 12/31/02
    else LampB(FALSE);
#endif //HV_501 board. P.P. 08/22/03
#if CBW() // cond added. P.P. 05/05/07
//#if GC312()||GC321() // changed. P.P. 06/09/06
#if GC312()||GC321()||GC326() //  P.P. 12/07/18
    // code added to check lamp via analog input. P.P. 05/19/06
#if MANUAL_INJ() // case added. P.P. 05/19/06
  int Gain = BIP10VOLTS; // default.
  uint16 data_lamp=0;
  uint16 data_lamp_B = 0;     // added. P.P. 06/09/06
#if PMD_16FS() // changed to measure only if det is active
  if(pMethod->det[0].detect_stat != 0) { // active detector A?
    My_cbAIn(CBWDEVICE_16FS, lamp_channel, Gain, (USHORT *)&data_lamp);
  }
  if(pMethod->det[1].detect_stat != 0) { // active detector B? Added. P.P. 06/09/06
    My_cbAIn(CBWDEVICE_16FS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
  }
#elif PMD_12FS()
  if(pMethod->det[0].detect_stat != 0) { // active detector A?
    My_cbAIn(CBWDEVICE_12FS, lamp_channel, Gain, (USHORT *)&data_lamp);
    data_lamp = (data_lamp&0x0FFF)*16;  // variant. P.P. 11/28/04
  }
  if(pMethod->det[1].detect_stat != 0) { // active detector B?
    My_cbAIn(CBWDEVICE_12FS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
    data_lamp_B = (data_lamp_B&0x0FFF)*16;  // variant. P.P. 11/28/04
  }
#elif PMD_12LS()
  if(pMethod->det[0].detect_stat != 0) { // active detector A?
    My_cbAIn(CBWDEVICE_12LS, lamp_channel, Gain, (USHORT *)&data_lamp);
    data_lamp = (data_lamp&0x0FFF)*16;  // variant. P.P. 11/28/04
  }
  if(pMethod->det[1].detect_stat != 0) { // active detector B?
    My_cbAIn(CBWDEVICE_12LS, lamp_channel_B, Gain, (USHORT *)&data_lamp_B);
    data_lamp_B = (data_lamp_B&0x0FFF)*16;  // variant. P.P. 11/28/04
  }
#endif // pmd_16
// replacement for analyzing in binary form. P.P. 06/10/06
  if((data_lamp>40960)||(data_lamp_B>40960)) {
    Lamp_error_event = 1;
  }
  else if((data_lamp<35388)&&(data_lamp_B<35388)) {
    Lamp_error_event = 0;
  }
  else {
  }
#else // not manual, using valves and relays
// end of added code for manual_inject case. P.P. 05/19/06
  USHORT BitValue;
//#if PMD_16FS()&&PMD_12LS() // added 03/21/08 - use dig inputs of 1208ls board
#if PMD_16FS() // added 03/21/08 - use dig inputs of 1208ls board
#if PMD_12LS()
    My_cbDIn(CBWDEVICE_12LS, FIRSTPORTB, &BitValue);
    if(BitValue&Lamp_on) Lamp_error_event = 1;
    else Lamp_error_event = 0;
#elif CBWUSB_IO()
    My_cbDIn(USB_IO, FIRSTPORTB, &BitValue);
    if(BitValue&Lamp_on) Lamp_error_event = 1;
    else Lamp_error_event = 0;
#else  // no 1208ls board, only 1608fs
  My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
  if(BitValue) Lamp_error_event = 1;  // only one lamp
  else Lamp_error_event = 0;
#endif // 12_LS or usb_io
#endif // 16fs
#endif // manual_inj
#else // regular case - not a portable GC312
//#if USB_3112()||PMD_12LS() // PP_TEMP: cond added temp, if no dig usb board installed
#if USB_3112() // PP_TEMP: cond changed, with 12ls board -
    //- use pmd_16FS. P.P. 02/18/07
 // Lamp_error_flag = Diagnostics_Lamp();    //
  Lamp_error_flag = Diagnostics(diagnostics_port); // P.P. 09/19/04
  Lamp_error_event = ((bLamp & Lamp_error_flag) != 0); // error fixed. P.P. 02/07/04. copied here 03/14/04
#else  // PP_TEMP: code added to use aux port, as in gc312. P.P. 04/30/06
    USHORT BitValue = 0; // P.P. 05/04/07
#if PMD_16FS() // cond added. P.P. 05/04/07
    My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
#endif
    if(BitValue) Lamp_error_event = 1;  // only one lamp
    else Lamp_error_event = 0;
#endif // usb_3112||pmd_12LS
#endif // GC312
#endif // cbw. added 05/05/07
    if(Lamp_error_event) Lamp_error_count += 1;   // Added code. P.P. 09/23/03
    else Lamp_error_count = 0;                    //
//  if(Lamp_error_count > 20) Lamp_error = 1;     //
    if((Lamp_error_count > 20)&&bLamp) Lamp_error = 1;     // P.P. 04/20/09
    else Lamp_error = 0;                          //
//  High_Alarm(1, Lamp_error);  // rem temp. P.P. 11/20/02                 //
#endif //tcd. Added. P.P. 08/22/04 not tcd but fid. P.P. 12/30/09
#endif // !stand_alone. Added. P.P. 08/11/03
}

#endif // INTERNAL()

    void Acquire::Start()
    {
#if DEMO() // added. P.P. 10/09/05
        return;  //
#endif     //
#if INTERNAL()
        MethodTable &method_table=pMethod->method_table;
    m = method_table.TempSlope;      // added for T=mV+b. P.P. 07/21/06
    b = method_table.TempIntercept;  //
//  oven_door = FALSE;   // removed to allow closing the door. P.P. 06/11/02

// New code starts here. P.P. 06/04/01
// Added initialization to set Cal, Sample, Imject, Alarm, Lamp, Autozero,
// Range10, Range 100 to High which is OFF state. P.P. 06/04/01
#if !STAND_ALONE() // added - we do not control valves or lamps. P.P. 07/15/03
//#if !(GC312()||GC321()) // cond changed. P.P. 04/01/08
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
    SetCalValve(TRUE); //??
    PointNumber(pointnumber); // at start, 1st available point. P.P. 04/25/04
#endif // !gc312 and !gc322. 04/01/08
#if CONCENTRATOR()    // case added. P.P. 02/18/06
    SamplePump(FALSE);  //
#else
    SamplePump(method_table.cont_pump_on_off);
#endif
#if WATER_PUMP()
    for(int index=0; index<3; index++) {      // on start, all water pumps are OFF. P.P. 01/04/17
    WaterPump(index, FALSE); // P.P. 01/04/17
  }
  Equilibrate(FALSE); // flush 1st column. P.P. 01/21/17
#endif

    Inject(FALSE);
    Equilibrate(FALSE);  // added. P.P. 07/20/04. Rem 07/26/04   restored. P.P. 03/17/17
#if !GC312() // cond added. P.P. 03/19/06
    Alarm(FALSE);
#endif // !gc312
#if CONCENTRATOR()  // added here 02/05/06. P.P.
    Purge(FALSE);        // Valve opened for N2 always but the sample time. P.P. 02/18/06
  Concentrator(FALSE); // heater OFF
  Set_valve(point_port_offset, injectV4_bit_number+8, FALSE);  // added P.P. 03/05/06
  inj_V4_on = FALSE; // added. P.P. 03/18/17
  Fan(FALSE); // added P.P. 02/23/10
#endif // conc
//  SetLamps(FALSE);  // removed temp. P.P. 10/03/03
    AutoZero(FALSE);
    AutoZeroB(FALSE); // added. P.P. 06/21/17
    // P.P. 07/06/02. Added to allow temperature to be set only when the program starts.
// #if REL_16()&&(!DEMO())   // real contact closure is needed   // why DEMO? P.P. 09/28/02
#if PMD_12LS()&&PMD_16FS() // case added for the case with non-usb relays for points and fluidics via 12LS board. P.P. 03/03/07
    EnableOven(TRUE);
#endif // pmd
    // end of added code.
    Lamp_error = 0;  // added to initialize diagnostics as OK. P.P. 11/16/02
    Oven_error = 0;  //
    Flow_error = 0;  //
//   Cal_error  = 0;  // removed -should stay until Cal OK. P.P. 12/06/02
    HL_alarm   = 0;  // added. P.P. 11/18/02
    Diagn_error = 0; // added. P.P. 06/28/03
    Lamp_error_count = 0; // added. P.P. 09/23/03
#if FID()
    FID_error = 0; // added. P.P. 07/03/05
   FID_error_event = 0; // added. P.P. 08/07/05
#endif
#else // stand_alone
#endif // !stand_alone. P.P. 07/15/03

#endif //INTERNAL()

        StartMethTrans();
        //something wrong with method_needs_transmitting - ignore it???
        //if(pMethod->method_needs_transmitting) StartMethTrans();
        //else StartAcquire();
    }

#ifndef  _HNU_H
#include "hnu.h" // JOB
#endif
    void Acquire::StartAcquire()
    {
#if DEMO()||MYDEBUG() // added. P.P. 10/09/04
        return;  //
#endif     //

        SetLamps(TRUE);
// Find out how many detectors are currently active.
        num_det_active = 0;
        for(int n=0; n!=NUMDETECTORS; n++) {
            if(pMethod->det[n].detect_stat != 0) // active dectector?
                num_det_active++;
        }

        ndata=0;

        if(acquire_type == METHOD) { // no more to do
            comm_state = ABORT;
        }
        else if(acquire_type == JOB) {
            comm_state = START_ACQUIRE;      // trigger the start of comm. acquisition
        }
//  else if(acquire_type == CAL) {
        else if((acquire_type == CAL)||(acquire_type == CHECKCAL)) {  // changed. P.P. 07/25/04
            comm_state = START_CAL_ACQUIRE;  // trigger the start of calibration acquisition
        }
        else {
            comm_state = GET_STATUS;
        }
    }

    typedef Detector DT[NUMDETECTORS];

    void Acquire::StartMethTrans()
    {
#if INTERNAL()
        MethodTable &method_table=pMethod->method_table;  // added. P.P. 01/02/11
    Str buff(48);
#if QUICK_SCAN() // added. P.P. 01/17/11
    if(method_table.quick_scan_enabled) { // B only or A only, never both
    quick_scan_active = quick_scan_next_point; // added P.P. 01/02/11
  }
  else {
    quick_scan_active = 0; // added P.P. 01/02/11
  }
// added P.P. 01/13/11
  if(quick_scan_active) {  // case added. P.P. 01/13/11
    wsprintf(buff.GetPtr(), "quick scan pt#%d", 1+pRun->pointnumber);
  }
  else { // as it was
      wsprintf(buff.GetPtr(), "pt#%d", 1+pRun->pointnumber);
  }
  for(int n = 0; n<NUMDETECTORS; n++) {  // P.P.  01/05/11
    if(pIntegrator[n]) {
      pIntegrator[n]->quick_run = quick_scan_active;
    pRun->quick_scan_run = quick_scan_active? 1:0; // moved here. P.P. 01/17/11
    }
//    pRun->quick_scan_run = quick_scan_active; // moved here. P.P. 01/15/11
  }
//  pRun->quick_scan_run = quick_scan_active; // added. P.P. 01/05/11
  pRun->comment=buff; // moved here. P.P. 06/21/11
#else // as it was
//  wsprintf(buff.GetPtr(), "pt#%d", 1+pRun->pointnumber);   // removed. P.P. 06/21/11
#endif // quick scan
    comm_state=TRANS_METH;
#else                   // not internal
#endif // internal
    }


//   Description:   Check for unsolicited or response messages from
//                  the 311 GC, also poll the 311 for Raw Data Messages.
//

    class AlreadyIn
    {
    public:
        AlreadyIn();
        ~AlreadyIn();
        operator int() const { return already_in>1; }
        static int already_in;
    };
    AlreadyIn::AlreadyIn()
    {
        already_in++;
    }
    AlreadyIn::~AlreadyIn()
    {
        already_in--;
    }
    int AlreadyIn::already_in=0;

    int Acquire::Continue()
    {
        int retVal=0;
        static const char *notification[NUM_NOTIFIES] = {
                "GC has no detector selected!",            // references to 311 removed. P.P. 08/11/03
                "GC has not been calibrated!",
                "Oven not at temperature.",
                "Injector not at temperature.",
                "GC is in REPEAT mode!",
                "Waiting for GC to finish previous acquire.",
                "Calibration Table not valid on GC.",
        };

        AlreadyIn already_in;
        if(already_in) {
            //TRACE("Acquire::Continue() already_in");
            return 0; // ignore
        }

#if INTERNAL()
        _pMainWindow=pMainWindow; // DEBUG
    // 1. Send oven temperature
    // 2. Send INJ/DET temperature
    // 3. Wait for oven and INJ/DET temperature
    // 4. Get Data (and use ramping??)
#if !STAND_ALONE() // added. P.P. 07/15/03
    if(tempcontrol) tempcontrol->ControlTemperature(); // Moved down. P.P. 04/29/02 Restored. 05/03/02
#endif // !stand_alone. P.P. 07/15/03

#ifndef _NDEBUG
    {
    static int oldState=-1;
    if(oldState!=comm_state) {
      oldState=comm_state;
      TRACE1( "New comm_state: %d", comm_state );
    }
  }
#endif // _NDEBUG

    switch(comm_state) {
        case TRANS_METH:
        {
            pMainWindow->sl->SetStatus("Active");
#if !STAND_ALONE() // added. P.P. 08/23/02
// code added to turn End_run relay OFF. P.P. 09/06/04
            temp_oven=Get_Temperature(0); // for Oven Temp. P.P. added here 06/19/09
#if HEATED_LINE()
            temp_line = Get_Temperature_Line();  // P.P. 03/29/17
#endif
            dwShowMessageTime = GetTickCount() + 2*1000UL; // added for messages and safety measurements. P.P. 11/01/09
//     dwShowMessageTime = 0L; // will be initialized in other place. P.P. 07/31/10
            dwSampleTime = 0;      // aded here for clean start with messages. P.P. 11/01/09
            dwStartInjectTime = 0;  //
            dwEquilibrateTime = 0; //
            dwInjectTime = 0;      //
            dwInjectTime2 = 0;      // P.P. 01/29/17
            dwInjectTime2_off=0;               // P.P. 01/29/17
            dwNextPointTime = 0;   //
            dwDesorbTime = 0; // added P.P. 03/12/17
            dwCleanPowerOnTime;     // added. 10/26/17
            dwCleanPowerOffTime;     // added. 10/26/17
            dwMoistureOutTime = 0;       // P.P. 12/26/17
            dwDesorberBatteryCheckTime = 0; // 03/23/18
#if WATER_PUMP()
            dwWaterPumpTime = 0;
#endif
            pMainWindow->sl->Replace("Method Download in Progress..." );

            // setting initial temperatures
            delete tempcontrol; // must be zero     // moved here. P.P. 05/18/02
            if(pMethod->method_table.RampingEnabled) {    // P.P. 05/17/02.
//        delete tempcontrol; // must be zero

                tempcontrol=new TempControl(pMethod);
//        comm_state=WAIT_TEMPERATURES;
// stored temperatures added here. P.P. 06/18/06
                t_oven_stored = pMethod->method_table.oven_temp;
#if !OVEN_EQ_INJ()                                        // case added. P.P. 06/11/06
                t_inj_stored = pMethod->method_table.inj_det_temp;//
#endif // !ove=inj                                        //
#if CONCENTRATOR() // case added - now dsb and fid are separated fron jnj_det. P.P. 07/27/18
                t_dsb_stored = pMethod->method_table.desorb_temp;
#endif
#if FID()
                t_fid_stored = pMethod->method_table.fid_temp;
#endif
                first_start = 0; // added. P.P. 04/20/09
            }                                           // P.P.
            else {                                        // Bypass TempControl.
                double t_oven = pMethod->method_table.oven_temp;
#if !OVEN_EQ_INJ()                                        // case added. P.P. 06/11/06
                double t_inj = pMethod->method_table.inj_det_temp;//
#endif // !ove=inj                                        //
#if CONCENTRATOR() // case added - now dsb and fid are separated fron jnj_det. P.P. 07/27/18
                double t_dsb = pMethod->method_table.desorb_temp;
#endif
#if FID()
                double t_fid = pMethod->method_table.fid_temp;
#endif
#if HEATED_LINE()
                t_line = pMethod->method_table.line_temp;  // P.P. 03/29/17
#endif
/* // rem P.P. 08/01/18
#if OVEN_EQ_INJ()
//#if !GC312() // cond added. P.P. 06/05/09
#if FID() // P.P. 02/11/06
//        double t_inj = pMethod->method_table.inj_det_temp;
	double t_fid = pMethod->method_table.fid_temp;
#endif
#if CONCENTRATOR()
	double t_dsb = pMethod->method_table.desorb_temp; // added. P.P. 02/11/06
#endif
#if HEATED_LINE()
	t_line = pMethod->method_table.line_temp;  // P.P. 03/29/17
#endif

//#endif // fid or concentrator // rem P.P. 07/30/18
//#endif // !gc312
#endif // oven_eq_inj
*/
#if (!FID())&&(!CONCENTRATOR()) // regular case
                SetOvenTemperature((long) t_oven); // P.P. 01/06/06
                t_oven_stored = t_oven; // added. P.P. 01/06/06
                first_start = 0; // added. P.P. 04/20/09
#if !OVEN_EQ_INJ()
                SetDetTemperature((long) t_inj); // P.P. 01/06/06
        t_inj_stored = t_inj; // added. P.P. 01/06/06
#endif // !oven=inj_det
#if HEATED_LINE()
                static void SetLineTemperature(double temperature); // prototype
        SetLineTemperature ((long) t_line); // P.P. 03/31/17
        t_line_stored = t_line;
#endif // heated line
#else // with FID or concentrator
                //	SetOvenTemperature((long) t_oven); // P.P. 09/09/05
	SetOvenTemperature( t_oven); // P.P. 06/19/09   // it is double
	t_oven_stored = t_oven; // added. P.P. 06/19/09
#if HEATED_LINE()
        void SetLineTemperature(double temperature); // prototype
	SetLineTemperature ((long) t_line); // P.P. 03/31/17
        t_line_stored = t_line;
#endif // heated line

#if FID()
      t_fid = pMethod->method_table.fid_temp;
#endif
#if CONCENTRATOR()  // no actions yet
	t_oven_stored = t_oven; // added. P.P.
        t_dsb_stored = pMethod->method_table.desorb_temp;
#endif // concentrator
#endif // !fid and !concentrator
// oven and detector are separated if Flame detector. P.P. 07/01/05
                oven_on = 1;           // added. P.P. 12/30/04
            }                                             //
#endif // !stand_alone. P.P. 08/23/02
#if STAND_ALONE() // cases separated. P.P. 07/16/06
            //#if GC321() // case added. P.P. 07/16/06
#if GC321()||GC326() //  P.P. 12/07/18
      comm_state=WAIT_TEMPERATURES;
#elif MANUAL_INJ() // case added for stand alone with Toven control. P.P. 01/04/16
      double t_oven = pMethod->method_table.oven_temp;
      t_oven_stored = t_oven;
      comm_state=WAIT_TEMPERATURES;
#else // just old stand-alone
      StartAcquire();  // bypass Wait Temperatures, Sample, inject. P.P. 08/23/02
#endif //  gc321 ore none. P.P. 07/16/06
#else // not stand_alone
#if RAMPING() // cond added. P.P. 10/31/08  This is only for GC322, no ramping in others. P.P. 10/06/11
            if(!pMethod->method_table.oven_door_manual) {  // added. P.P. 09/07/12
	if(pMethod->method_table.RampingEnabled) {    // added. P.P. 10/24/08.
          if(!oven_door_on) {
	    OvenDoor(TRUE);
//	    dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
	    dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
	    DoorMotor(TRUE); // P.P. 10/04/11
//	    oven_door_on = 1;
          }
        }
      }
#elif PMD_16FS()&&PMD_12LS()&&ANALOG_OUTPUT()&&(POINTS()<=4) // added to turn 2nd pump on. P.P. 02/01/09
            OvenDoor(TRUE);
#elif PMD_16FS()&&PMD_12LS()&&GC312()&&FID() // case added for current GC312 with FID and 2nd pump. 06/19/09
            //	OvenDoor(TRUE); // rem to avoid auto start 2nd pump on start of run. P.P. 05/18/12
#endif // ramping
#if !QUICK_SCAN()
            comm_state=WAIT_TEMPERATURES; // added. P.P. 07/21/06
#else // with quick scan
            if(quick_scan_active) {  // case added. P.P. 01/15/11
	comm_state = RECEIVING_DATA;
      }
      else { // as it always been	  //
	comm_state=WAIT_TEMPERATURES; // added. P.P. 07/21/06
	dwAnalysisTime = 0; // P.P. 01/15/11
      }
#endif // !quick scan
#endif // stand-alone
#if CONCENTRATOR()
            Equilibrate(FALSE); // added. P.P. 03/17/17
#endif
// code added to transfer the range and range_1 values for both detectors. P.P. 05/13/17
            for(int n = 0; n<NUMDETECTORS; n++) {  // P.P.  05/13/17
                Detector *pDetector=&pMethod->det[n];
                if(pMethod->det[n].detect_stat == 0) continue; // active detector??
                gain_stored[n] = pMethod->det[n].gain;
                gain_1_stored[n] = pMethod->det[n].gain_1;
            }
// end of code for range and range_1 values
        }
            break;
        case WAIT_TEMPERATURES:
        {
#if !STAND_ALONE()     // added. P.P. 08/23/02
            //pMainWindow->sl->SetStatus("Active");
            MethodTable &method_table=pMethod->method_table;
//      dwShowMessageTime = GetTickCount() + 2*1000UL; // initialized. P.P. 07/31/10
// diagnostics added here. P.P. 05/02/04
#if HEATED_LINE()
            long t_line = method_table.line_temp;
        void SetLineTemperature(double temperature);// prototype
//	SetLineTemperature ((long) t_line_stored); // P.P. 03/31/17
	SetLineTemperature (t_line); // P.P. 03/31/17
#endif // heated line

#if DIAGNOSTICS()
            int Diagnostics_error_flag = Diagnostics(diagnostics_port);
        N2_error = ((Diagnostics_error_flag & BIT[N2_error_bit])!= 0);        // Changed. P.P. 02/21/03
#if TCD() // cond. added. P.P. 08/15/04
	if(N2_error) {
          Acquire::TCDDetector(FALSE);    // Turn ON manually after this. P.P. 06/15/04
        }
#endif // tcd
//        Air_error = ((Diagnostics_error_flag & BIT[Air_error_bit])!=0); // removed. P.P. 06/15/04
        CalGas_error = ((Diagnostics_error_flag & BIT[CalGas_error_bit])!=0);
//#endif // diagnostics // rem to get this under diagnostics. P.P. 06/19/09
#if FID() // added. P.P. 08/07/05
	if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error||FID_error) {  // FID error added, oven excluded during temperature set.  P.P. 04/27/04, 08/07/05
#else
        if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error) {  // oven excluded during temperature set.  P.P. 04/27/04
#endif // fid
          Diagnostics_error = 1;                              //
//	  Diagnostics_Alarm(diagnostics_bit_number, TRUE); // added. P.P. 08/08/04 - and rem 10/01/04
	}
	else {
	  Diagnostics_error = 0;
//	  Diagnostics_Alarm(diagnostics_bit_number, FALSE); // added. P.P. 08/08/04 - and rem 10/01/04
	}
//	Diagnostics_Alarm(diagnostics_bit_number, Diagn_error); // added. P.P. 08/08/04
//#if DIAGNOSTICS() // cond. added. P.P. 08/15/04 // rem to get this under diagnostics. P.P. 06/19/09
	if(N2_error || Air_error) {  // add later Oven and Oven_high errors. P.P. 02/23/03
	  Fatal_error = 1;
//	  Fatal_error_Str=N2_error? "N2_error": "Air_error";
	  Fatal_error_Str=N2_error? "Carrier gas error": "Air_error"; // replacement. P.P. 08/22/04
	}
	else {
	  Fatal_error = 0;
	}
	if(Fatal_error) {   // added to turn OFF the oven. P.P. 08/03/04
//          return -1; // stop
	}
        else { // added to restore oven enable. 05/18/07
	}
#endif // diagnostics
// end of added diagnostics
            // all initialization in Acquire::Start
            char buff[100]; buff[0]='\0';
            BOOL b_oven_OK=FALSE;
            {
                //short value=MyReadInputPort(strSerialNumber,inp_oven_port);
// code changed to average temp_oven. P.P. 04/16/06
                //          pMainWindow->print("  Oven temperature =%f\r\n", meas_temperature);
#if CONCENTRATOR()
                const double oven_delta=2;  // P.P. 02/05/06
#else
                const double oven_delta=1;
#endif
                const double oven_error_delta = 10; // added for diagnostics. P.P. 11/16/02
//        const double oven_error_delta = 20; // added for diagnostics. P.P. 11/16/02
#if RAMPING() // changed - Oven Door can be is now in GC301 as well. P.P. 11/11/05
                #if !GC312() // cond added 04/13/08
//	const double oven_door_delta = 3; //
//	const double oven_door_delta = 0.5; // P.P. 10/24/08
	const double oven_door_delta = 1.0; // P.P. 10/26/08 10/31/08
        if(!method_table.oven_door_manual) { // added. P.P. 09/07/12
//	  if(pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/17/06.
          if(1) { // now oven door will work in isothermal as well. P.P. 10/31/08
	    if((temp_oven - method_table.oven_temp) < oven_door_delta){ // P.P. 07/03/05 // restored P.P. 10/19/08
//	    if(oven_door_on&&((temp_oven - method_table.oven_temp) < (-5.0))) { // P.P. 06/23/06
	      if(oven_door_on) { // P.P. 06/25/06  // cond restored. P.P. 10/24/08
	        OvenDoor(FALSE);   // moved here from Start(). P.P. 05/11/02
//	        dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
	        dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
	        DoorMotor(TRUE); //  P.P. 10/04/11
	      }
            }
	    else if((temp_oven - method_table.oven_temp) >( oven_door_delta + 2)) { // P.P. 10/26/08
//	    else if((!oven_door_on)&&((temp_oven - method_table.oven_temp) > oven_door_delta + 1)) { // P.P. 10/24/08
              if(!oven_door_on) {  // P.P. 10/26/08
	        OvenDoor(TRUE); //
//	        dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
	        dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
	        DoorMotor(TRUE); // P.P. 10/04/11
	      }
            }
	    else {
	    } // nothing to do. P.P. 11/25/02
	  }
	  else {  // never happens, else = 0. P.P. 10/06/11
	    OvenDoor(FALSE);   // Added. P.P. 06/18/06
//	    dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
	    dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
	    DoorMotor(TRUE); // P.P. 10/04/11
	  }
        } // ! oven door manual
#endif // !gc312
#endif // GC321. P.P. 10/01/04 or Rumping
#if QUICK_SCAN()
                if(!quick_scan_active) { // regular run. P.P. 12/17/10
#else
                if(1) {
#endif // quick scan
//	  if(meas_temperature < (method_table.oven_temp - oven_error_delta)){ // Diagnostics Oven low. P.P. 11/16/02
                    if(temp_oven < (method_table.oven_temp - oven_error_delta)){ // Oven low. P.P. 07/03/05
                        Oven_error = 1;   //
                    }
//	  else if(meas_temperature > (method_table.oven_temp - 2*oven_delta)){  // 2 degrees close to norm
                    else if(temp_oven > (method_table.oven_temp - 2*oven_delta)){  // 2 degrees close to norm. P.P. 07/03/05
                        Oven_error = 0;
                    }
                    else { // stays as it is. P.P. 11/25/02
                    }
                }
//#if 0 // moved here. P.P. 10/01/04   // rem P.P. 12/08/18
                int Diagnostics_Flow();   // P.P. 11/18/02
//#if 0 /* FLOW()*/ // added to check flow. P.P. 03/16/04. rem for current 501. P.P. 07/25/04
#if FLOW() // added to check flow. P.P. 03/16/04. rem for current 501. P.P. 07/25/04 restored 12/08/18
                double Get_Flow();        //
        const double flow_error_delta = 2.0;
        const double flow_delta = 1.0;
        double check_point = method_table.set_point;
        double flow_data = Get_Flow();
	if(abs(flow_data-check_point) > flow_error_delta) {  // variant with flow window
          Flow_error = 1;
        }
	else if(abs(flow_data-check_point) < flow_delta) {  // variant with flow window
          Flow_error = 0;
        }
        else {}; // stays as it is.
#else
#endif // flow(). end of changes. P.P. 03/16/04

#if DIAGNOSTICS() // cond added to rem error handling. P.P. 12/08/18
                #if FLOW()  // cond added. P.P. 10/05/04
	Diagn_error=(Diagnostics_error || Flow_error);  // replaced. P.P. 05/02/04
#else
//	Diagn_error=(Diagnostics_error||Oven_error);  // replaced. P.P. 05/02/04
	Diagn_error=Diagnostics_error;  // oven error does not go into diagn alarm. P.P. 10/05/04
#endif
#if ALL_ERR_CAL() // condition added. P.P. 06/28/03
	if(Diagn_error) {
	  Cal_error = 1;                              // must follow. P.P. 12/12/02
	}
#endif
#if PMD_12LS()&&PMD_16FS() // case added for 4-point GC301 with non-usb relays. P.P. 03/04/07
//#if !(GC312()||GC321()) // cond added 03/25/08
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
#if !WATER_PUMP() // P.P. 01/29/17
	if(pMethod->method_table.alarms_on_off) {
	  if(Diagn_error||Oven_error) {
	    Diagnostics_Alarm(diagnostics_bit_number, TRUE);
	  }
	  else {
	    Diagnostics_Alarm(diagnostics_bit_number, FALSE);
	  }
	}
	else { // turn off if unchecked.
	  Diagnostics_Alarm(diagnostics_bit_number, FALSE);
        }
#endif // water_pump
#endif // gc322 or 312 or gc326
#endif // pmd_12ls
//#if !(GC312()||GC321())
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
#if !WATER_PUMP() // P.P. 01/29/17
// cond changed. No alarms in GC321. P.P. 06/17/06
	if(pMethod->method_table.alarms_on_off) {    // local diagnostics alarm added. P.P. 11/17/02
//	  Alarm(Oven_error||Flow_error||Lamp_error||Board_error); // Board_error added. P.P. 06/20/03
	  Alarm(Diagn_error||Oven_error); // Replacement. diagn includes all err but oven. Let oven err ssound. P.P. 12/04/04
        }                                            //
        else {          // added. Uncheck - and stop alarm during run. P.P. 11/17/02
          Alarm(FALSE);
//          Set_alarm_bit(control_port_offset, 5, FALSE); // added. P.P. 06/28/03
//          Diagnostics_Alarm(diagnostics_bit_number, FALSE); // removed. P.P. 05/31/04
        }
#endif // water_pump
#endif //!(gc312 or gc322)
#endif // diagnotics, added to finish removing diagnostics error handling P.P. 12/08/18
//        if(fabs(method_table.oven_temp-meas_temperature)<oven_delta)
                double temp_del = 0;  // added. P.P. 01/06/06
                temp_del = (method_table.oven_temp-temp_oven);
//    new code. P.P. 04/07/06
#if GC312() // added. P.P. 04/07/06
                if(battery_count >= battery_count_max) { // check not so often. P.P. 10/30/09
//	if(!show_message) {     // replaced. P.P. 11/01/09
          battery_count = 0; // rem
	  battery_voltage = GetBatteryVoltage(battery_channel); // P.P. 04/07/06
// code added to monitor battery and control temperature. P.P. 04/07/06
//	  double temp_del = 0;  // moved up. P.P. 04/07/06 removed. 04/08/06
	  if(battery_voltage>0.5) { // hence, on battery
            battery_run = 1; // added. P.P. 04/08/06
	    if(battery_voltage < 5.0) { // 12 V via 1/2 divider,  voltage < 10 V (low limit)
              battery_low = 1;
//	      battery_low_event +=1; // added. P.P. 04/09/06 rem 01/05/09
	      SetOvenTemperature(0);
	      oven_on = FALSE;
#if !OVEN_EQ_INJ()
              SetDetTemperature(0); // added. P.P. 06/16/06
#endif
            // cond if(!safety_warning) was added to all Replace( from here and down. P.p. 04/17/09
//	    if(!safety_warning) pMainWindow->sl->Replace("Battery Low !!! Stop and Turn OFF GC" ); // P.P. 04/07/06 Rem 10/22/09
	    }
	    else { // battery OK - hold temperature <= 50 C. P.P. 04/07/06
              battery_low = 0; // added. P.P. 04/08/06
//	      battery_low_event = 0;  // added. P.P. 04/09/06 rem 11/05/09
              Fatal_error = 0; // added. P.P. 04/09/06
              if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
//	        if(pMethod->method_table.oven_temp <= 50) { // regular work
	        if(pMethod->method_table.oven_temp <= max_temp_bat) { // regular work P.P. 02/13/09
	          oven_high = 0; // added. P.P. 04/08/06
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
		  if(!safety_warning) { // cond and action added. P.P. 04/24/09
	            SetTemperatures(t_oven_stored, t_inj_stored); // replacement. P.P. 06/16/06
                  }
#else
//		    SetOvenTemperature(pMethod->method_table.oven_temp); // replacement. P.P. 06/16/06
	          if(!safety_warning) { // cond and action added. P.P. 04/24/09    cond removed 06/19/09 restored 06/20/09
		    SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06
	            pMainWindow->sl->Replace("Waiting temperature" ); // added here P.P. 06/20/09
		  }
#endif // !oven_eq_inj
                      SetLamps(TRUE);

//	        pMainWindow->sl->Replace("Waiting temperature" ); // P.P. 04/08/06 Rem 04/08/06
	        }
	        else { // oven temp set > 50 C in Method - not allowed!  >100 C. P.P. 10/18/09
                  oven_high = 1;
// code added
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
		  SetTemperatures(0,0); // replacement. P.P. 06/16/06
#else
		  SetOvenTemperature(0); // replacement. P.P. 06/16/06 rem temp. 06/19/09  restored 06/20/09
#endif // !oven_eq_inj
//
		}
              }
	    }
	  }
	  else { // battery not connected - work from external power, no restrictions. P.P. 04/07/06
            battery_run = 0; // All OK. Added. P.P. 04/08/06
	    battery_low = 0; //
            oven_high = 0;   //
//	  temp_del = (pMethod->method_table.oven_temp-temp_oven); // rem. P.P. 04/16/06
// code added. P.P. 06/16/06
	    if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
//	    oven_high = 0; // added. P.P. 04/08/06
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
	      if(!safety_warning) { // cond and action added. P.P. 04/24/09
	        SetTemperatures(t_oven_stored, t_inj_stored); // replacement. P.P. 06/16/06
              }
#else
//	    SetOvenTemperature(pMethod->method_table.oven_temp); // replacement. P.P. 06/16/06
	      if(!safety_warning) { // cond and action added. P.P. 04/24/09        all cond removed. 06/19/09 restored 06/20/09
	        SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06
	      }
	      else {  // turn off
//	      SetOvenTemperature(0); // not needed here, done in other place
                EnableOven(FALSE);
	      }
#endif // !oven_eq_inj
	    }
	  }
	}
	else { // battery_count < battery_count_max - just wait about 2 s. P.P. 10/30/09
	  battery_count++;
	}
// end of added code to monitor battery and control temperature. P.P. 04/07/06
// ignition for FID added. 02/26/08
/* // rem because this is inside condition if GC312. P.P. 08/31/18
#if !GC312() // cond added. P.P. 06/05/09
#if FID()
	if(start_fid) { // beep warning. 02/16/08
	  MessageBeep(MB_ICONASTERISK); //cue(); // cue sound 02/26/08 testing
//	  MessageBeep(MB_ICONHAND);
	  if(!dwFidDelayTime) { // ignitor just turned on - count 5 s  and turn it OFF. 02/16/08
//	    dwFidDelayTime = GetTickCount() +5*1000UL;
	    dwFidDelayTime = GetTickCount() +8*1000UL;  // 02/20/08
	  }
	  else if(dwFidDelayTime<GetTickCount()) {
	    FlameDetector(FALSE);
	    start_fid=0;
	    dwFidDelayTime=0UL;
          }
	}
#endif // fid
#endif // !gc312
// end
*/
#else // regular GC
// end of new code. P.P. 04/07/06
// code replacement. P.P. 06/16/06
#if RAMPING() // cond added. 03/25/08
                if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
//	if(pMethod->method_table.RampingEnabled) {    // cond changed. P.P. 06/25/06. and restored. 06/27/06
#else // no ramping
                if(1) {
#endif // ramping
                    oven_high = 0; // added. P.P. 04/08/06
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
                    //        if(!oven_door_on) { // added. P.P. 06/23/06
#if RAMPING() // cond added. 03/25/08
	  if(oven_door_on) { // changed. P.P. 06/25/06
	    SetTemperatures((pMethod->method_table.oven_temp - 5.0),
	      pMethod->method_table.inj_det_temp); // replacement. P.P. 06/23/06
	  }
	  else {
#else // no ramping - no oven door
          if(1) {
#endif // ramping
	    if(!safety_warning) { // cond and action added. P.P. 04/24/09
	      SetTemperatures(pMethod->method_table.oven_temp,
	        pMethod->method_table.inj_det_temp); // replacement. P.P. 06/16/06
            }
	  }
//	  SetTemperatures(t_oven_stored, t_inj_stored); // replacement. P.P. 06/16/06
#else // oven=inj
                    if(!safety_warning) { // cond and action added. P.P. 04/24/09
                        SetOvenTemperature(pMethod->method_table.oven_temp); // replacement. P.P. 06/16/06
//	    SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06
                    }
#endif // !oven_eq_inj
                }
// end of code replacement. P.P. 06/16/06
#endif // GC312() P.P. 04/07/06
#if !QUICK_SCAN() // cond added. P.P. 01/17/11
                if(fabs(temp_del)<oven_delta) { // P.P. 07/03/05
                    b_oven_OK=TRUE;
                }
                else {              // added to wait if oven overshut while inj_det is reaching the temperature. P.P. 06/17/06
                    b_oven_OK=FALSE;  //
                }
#else
                if(quick_scan_active) { // cond added for quick scan to skip waiting temperature. P.P. 12/17/10
	  b_oven_OK=TRUE;
	}
        else {
	  if(fabs(temp_del)<oven_delta) { // P.P. 07/03/05
	    b_oven_OK=TRUE;
	  }
	  else {              // added to wait if oven overshut while inj_det is reaching the temperature. P.P. 06/17/06
	    b_oven_OK=FALSE;  //
	  }
	}                     //
#endif // !quick scan

            }
#if OVEN_EQ_INJ()
            BOOL b_inj_det_OK=b_oven_OK;
#else
            BOOL b_inj_det_OK=FALSE;
      {
        //short value=MyReadInputPort(strSerialNumber,inp_inj_det_port);
//        double meas_temperature=Get_Temperature(1); // channel for Injector Temp.
	temp_det=Get_Temperature(1); // channel for Injector Temp. P.P. 07/03/05 replacement. 09/03/05 Rem 06/11/06  Restored P.P. 10/24/08
      //  pMainWindow->print("  Inj temperature =%f\r\n", meas_temperature);
//        const double det_delta=1;
	const double det_delta=4;  // easier cond for starting the run. P.P. 12/02/11
#if !FID() // cond added. P.P. 06/28/05
	double temp_del_det = 0;  // added. P.P. 01/06/06
	temp_del_det = (method_table.inj_det_temp-temp_det);
//	if(fabs(temp_del_det)<det_delta) { // P.P. 06/11/06
	if(temp_del_det <det_delta) { // temp_det above (set temp - 4 degrees) P.P. 08/24/12
	  b_inj_det_OK=TRUE;
        }
	else {
//          sprintf(strchr(buff,'\0')," Injector not at temperature. %.2fC", meas_temperature);
	  sprintf(strchr(buff,'\0')," Injector not at temperature. %.2fC", temp_det); // P.P. 07/03/05, 09/03/05
//// temp_del averaging added to decrease clicks due to noise. P.P. 04/16/06
	  if(!safety_warning) { // cond and action added. P.P. 04/24/09
	    SetDetTemperature(method_table.inj_det_temp); // P.P. 06/16/06
	  }
	}
/// end of added code. P.P. 06/11/06

#else // with FID
#if !GC312() // cond added. P.P. 06/05/09
	if(1&&
	     (temp_det >(method_table.inj_det_temp-det_delta))) { // for future changes. P.P. 11/13/05
	  b_inj_det_OK=TRUE;
	  fid_on=1; // added. P.P. 08/05/05
	  start_fid = 0; // added - hence, during run can ignite if needed. P.P. 09/30/05
//          if(dwFidDelayTime) {  // cond. added - turn off only once. P.P. 07/24/05
//	  if(dwFidDelayTime && (dwFidDelayTime<GetTickCount())) { // changed. P.P. 09/23/05
	  if(dwFidDelayTime) { // changed. P.P. 09/30/05
	    FlameDetector(FALSE);
	    dwFidDelayTime = 0; // P.P. 07/24/05
            dwFidTempTimeout = 0; // P.P. 07/24/05
	  }
        }
	else { // temp low or hydrogen valve is OFF
	  b_inj_det_OK=FALSE; // added. P.P. 07/01/05
//	  sprintf(strchr(buff,'\0')," FID not at temperature. %.2fC", meas_temperature);
// added to turn ignite off. P.P. 07/24/05
	  if(dwHydrogenDelayTime && (dwHydrogenDelayTime<GetTickCount())) {  // code added. P.P. 09/20/05
	    HydrogenValve(TRUE);
	    dwHydrogenDelayTime = 0; // to turn ON only once. P.P. 09/20/05
          }
	  else if(dwFidDelayTime && (dwFidDelayTime<GetTickCount())) {
//	    Acquire::FlameDetector(FALSE);
	    FlameDetector(FALSE); // P.P. 07/26/05
	    dwFidDelayTime = 0;
//	    sprintf(strchr(buff,'\0')," FID not at temperature. %.2fC", meas_temperature);
	    sprintf(strchr(buff,'\0')," FID not at temperature. %.2fC", temp_det); // P.P. 09/03/05
	  }
	  else if(dwFidDelayTime) {
//	    sprintf(strchr(buff,'\0')," Ignite FID. %.2fC", meas_temperature);
	    sprintf(strchr(buff,'\0')," Ignite FID. %.2fC", temp_det);    // P.P. 09/03/05
	  }
	  else if(dwFidTempTimeout && (dwFidTempTimeout<GetTickCount())) {
//	    Acquire::HydrogenValve(FALSE);
	    HydrogenValve(FALSE); // P.P. 07/26/05
	    FlameDetector(FALSE); // P.P. 09/30/05
	    fid_on=0; // added. P.P. 08/05/05
// added. P.P. 08/05/05
	    FID_error=1;
	    dwFidTempTimeout = 0; // turn OFF only once. P.P. 07/24/05
	    Diagnostics_Alarm(diagnostics_bit_number, TRUE); // should be traced throughout first. P.P. 07/03/05.  enabled 08/05/05
//              SendEndAcquire();
//              comm_state = END_ACQUIRE;
//            Fatal_error = 1;
	    if(FID_error) {  // added. P.P. 08/05/05
	      Fatal_error_Str="FID failure";
            }
// end
//	    sprintf(strchr(buff,'\0')," FID error. Hydrogen shut off. %.2fC", meas_temperature);
	    sprintf(strchr(buff,'\0')," FID error. Hydrogen shut off. %.2fC", temp_det);  // P.P. 09/03/05
	    return -1; // Fatal error - stop.
          }
	  else if(dwFidTempTimeout) {
//	    sprintf(strchr(buff,'\0')," FID not at temperature. %.2fC", meas_temperature);
	    sprintf(strchr(buff,'\0')," FID not at temperature. %.2fC", temp_det); // P.P. 09/03/05
	  }
// end of added code. P.P. 07/24/05
	}
#endif // !gc12
#endif // !FID
      }
#endif // OVEN_EQ_INJ()

#if (MYDEBUG() || DEMO())  // work without A/D board. P.P. 10/23/01
#else
//      if((b_oven_OK)&&(b_inj_det_OK))  //brackets added P.P. 06/01/01
            if(!((b_oven_OK)&&(b_inj_det_OK))) { //changed. P.P. 05/14/02
                data_A = Get_Data(0, cur_gain[0]);  // added. P.P. 08/02/10
// more diagnostics added. P.P. 05/02/04
// all err messages - to err_str. P.P. 04/30/04
                char err_str[150] ={0};
                char str_temp[20] = {0};
// Warning messages added. P.P. 04/08/06
                if(battery_low) {
//	  sprintf(err_str, "Battery Low !!! Stop and Turn OFF GC");  // rem 10/22/09
                }
                else if(oven_high) {
//	  sprintf(err_str, "Running on battery! Set oven to <= 50 C");
//	  sprintf(err_str, "Running on battery! Set oven to <= 60 C");  // P.P. 02/13/09
//	  sprintf(err_str, "Running on battery! Set oven to <= 100 C");  // P.P. 10/18/09   rem 10/22/09
                }
                else { // all OK - check diagnostics
// end of changed messages. P.P. 04/08/06
                    if((!safety_warning)&&(!Diagn_error)) {  // cond added. P.P. 10/22/09
#if RAMPING()  // cond added. P.P. 07/22/06
                        if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/23/06.
	      sprintf(err_str, "Set Temperatures, Isothermal ");  // rem P.P. 04/08/06
	    }
	    else {
	      if(oven_door_on) {                                      //
	        sprintf(err_str, "Set Temperatures, cooling oven ");  //  P.P. 10/19/08
	      }
              else {
	        sprintf(err_str, "Set Temperatures ");  // rem P.P. 04/08/06
	      }
	    }
#else
#if !OVEN_EQ_INJ() // case added. P.P. 07/23/06
                        sprintf(err_str, "Set Temperatures ");  // P.P. 07/23/06
#else
                        sprintf(err_str, "Set Temperature ");  // P.P. 07/23/06
#endif
#endif // ramping. P.P. 07/22/06
                        if(!show_message) { // cond added. P.P. 11/01/09
                            pMainWindow->sl->Replace(err_str); // moved from above. P.P. 04/08/06
                        }
                    }
                }
//	if(!safety_warning) pMainWindow->sl->Replace(err_str); // moved from above. P.P. 04/08/06 rem 10/22/09
// end of additional diagnostics. P.P. 05/02/04
            }
//      else // start if temperature OK, independently on other errors
#if RAMPING() // cond added. P.P. 06/21/09
                else if(!oven_door_on)// start if temperature OK and door closed. P.P. 06/23/06
#else
            else // temp OK - can start. P.P. 06/21/09
#endif // ramping
#endif
            { // { added, to turn Equilibrate only when T oven == OK. P.P. 10/01/04
#if CONCENTRATOR()
                Equilibrate(FALSE); // added. P.P. 03/17/17
#endif
                StartAcquire();  //comm_state=START_ACQUIRE;
// added Equil temporarily, for aditional valve. P.P. 07/29/04
//#if GC501() // cond added. P.P. 08/26/04
            } // } added. P.P. 10/01/04
#else // stand_alone //added. P.P. 07/17/03
            //#if GC321() // case added. P.P. 07/16/06
#if GC321()||GC326() //  P.P. 12/07/18
	StartAcquire();  //comm_state=START_ACQUIRE;

#elif MANUAL_INJ() // case added for stand alone +Toven control. P.P. 01/04/16
        MethodTable &method_table=pMethod->method_table;
        char buff[100]; buff[0]='\0';
        BOOL b_oven_OK=FALSE;
        const double oven_delta=1;
        double temp_del = 0;  // added. P.P. 01/06/06
        temp_del = (method_table.oven_temp-temp_oven);
	char err_str[150] ={0};
	if(fabs(temp_del)<oven_delta) { // P.P. 07/03/05
	  b_oven_OK=TRUE;
        }
        else {              // added to wait if oven overshut while inj_det is reaching the temperature. P.P. 06/17/06
	  b_oven_OK=FALSE;  //
        }
	if (!b_oven_OK) { //changed. P.P. 05/14/02
          data_A = Get_Data(0, cur_gain[0]);  // added. P.P. 08/02/10
// more diagnostics added. P.P. 05/02/04
// all err messages - to err_str. P.P. 04/30/04
	  sprintf(err_str, "Set Temperature ");  // P.P. 07/23/06
        }
        else {
	  StartAcquire();  //comm_state=START_ACQUIRE;
        }
#endif // 321 or manual inj
#endif // !stand_alone. P.P. 07/14/03
        }
            break;
        case START_ACQUIRE:
        case START_CAL_ACQUIRE:
        {
#if STAND_ALONE()               // added. P.P. 08/23/02

            //#if GC321() // case added, stand_alone+gc321. P.P. 07/12/06
#if GC321()||GC326() //  P.P. 12/07/18
      dwSampleTime=GetTickCount()+5*1000UL; // 5 seconds. P.P. 05/19/06
      comm_state= WAIT_SAMPLE_TIME;
//      pMainWindow->sl->Replace("Waiting sample time"); // moved down. P.P. 04/28/06
//	if(pMethod->method_table.syringe_on_off) {         // added. P.P. 04/29/02
	  if(!safety_warning) pMainWindow->sl->Replace("Start");
//        }
#else
//      comm_state = RECEIVING_DATA;  // For Stand_alone, autozero is needed. P.P. 11/01/15
      AutoZero(TRUE);
      AutoZeroB(TRUE); // added. P.P. 06/21/17
      dwSampleTime=GetTickCount()+5*1000UL; // 5 seconds. // therefore this code added
      comm_state= WAIT_SAMPLE_TIME;
      pMainWindow->sl->Replace("Start");
              SetLamps(TRUE);

//        }
#endif // gc321
      dwLastTime=0;                 //
      dwInjectTime=0;               //
      dwInjectTime2=0;               // P.P. 01/29/17
      dwInjectTime2_off=0;               // P.P. 01/29/17
      dwEndRunTime = 0;  // added. P.P. 12/06/02
      dwNextPointTime = 0; // added. P.P. 04/27/04
      dwPurgeTime = 0; // added. P.P. 02/18/06
#else  // not stand_alone                         //
            MethodTable &method_table=pMethod->method_table;
            //SendStartAcquire();
            //StartAcquire();
// removed - was needed only for KOGAS. P.P. 07/10/05
/*
	if((acquire_type == CAL)||(acquire_type == CHECKCAL)) {
#if SAMPLE_AHEAD()                      // added to sample cal gas same time as sample. P.P. 04/23/05
	  if(method_table.purge_time) {
	    dwSampleTime=GetTickCount()+(method_table.sample_time +
            method_table.analysis_time - method_table.purge_time)*1000UL; // extra time added. P.P. 04/23/05
	  }
          else {
	    dwSampleTime=GetTickCount()+method_table.sample_time*1000UL; // reg case. P.P. 04/22/05
          }
#else
	  dwSampleTime=GetTickCount()+method_table.sample_time*1000UL;
//	  dwSampleTime=GetTickCount()+method_table.sample_time*1000UL + 120*1000UL;
#endif // sample ahead. End of added code. P.P. 04/23/05
	  after_cal = TRUE; // P.P. 05/04/05
	}
        else {
	  if(after_cal) {  // case added. P.P. 05/03/05
	    dwSampleTime=GetTickCount()+(method_table.sample_time +
              60)*1000UL; // extra 1 min time added. P.P. 05/03/05
            after_cal = FALSE;
	  }
          else {
	    dwSampleTime=GetTickCount()+method_table.sample_time*1000UL; // reg case. P.P. 04/22/05
	  }
	}
*/ // end of removed code. 1 line placed below: P.P. 07/10/05
#if MANUAL_INJ()
            dwSampleTime=GetTickCount()+5*1000UL; // 5 seconds. P.P. 05/19/06
#else
            dwSampleTime=GetTickCount()+method_table.sample_time*1000UL; // reg case. P.P. 07/10/05
#if GC321() // case added. P.P. 07/01/10  here 11/12/10
            //#if GC321()||GC326() //  P.P. 12/07/18 rem - in GC326, x2 and x3 times will be as in regular GCs. P.P. 12/09/18
#if X1_VALVE()
/* // moved to count from start of inject
      dwXvalveTime = GetTickCount()+method_table.x1_time*1000UL;
      dwXvalveOffTime = GetTickCount()+method_table.x1_off_time*1000UL;
*/

      if((method_table.x2_time)&&
	   ((method_table.x2_off_time - method_table.x2_time)>0)) { //  condition added to disable if x2_time=0. P.P. 08/19/10
	dwXvalve2Time = GetTickCount()+method_table.x2_time*1000UL;
	dwXvalve2OffTime = GetTickCount()+method_table.x2_off_time*1000UL;
      }
      if((method_table.x3_time)&&
	   ((method_table.x3_off_time - method_table.x3_time)>0)) { //  condition added. P.P. 08/19/10
        dwXvalve3Time = GetTickCount()+method_table.x3_time*1000UL;
        dwXvalve3OffTime = GetTickCount()+method_table.x3_off_time*1000UL;
      }
/* // moved to count from start of inject
      dwXvalve4Time = GetTickCount()+method_table.x4_time*1000UL;
      dwXvalve4OffTime = GetTickCount()+method_table.x4_off_time*1000UL;
      dwXvalve5Time = GetTickCount()+method_table.x5_time*1000UL;
      dwXvalve5OffTime = GetTickCount()+method_table.x5_off_time*1000UL;
*/
#endif // x1_valve
#endif // gc321
#endif // manual inj
            comm_state= WAIT_SAMPLE_TIME;
//      pMainWindow->sl->Replace("Waiting sample time"); // moved down. P.P. 04/28/06
            if(pMethod->method_table.syringe_on_off) {         // added. P.P. 04/29/02
//	if(!safety_warning) pMainWindow->sl->Replace("Start");   // rem P.P. 08/01/10 - messages are in another place
            }
            else {
//        if(!safety_warning) pMainWindow->sl->Replace("Waiting sample time"); // moved here. p.P. 04/28/06 // rem P.P. 08/01/10 - messages are in another place
            }
            pMainWindow->print("Waiting sample time...\r\n");   // enabled P.P. 03/12/17
            //  MySetOutput(strSerialNumber, 21, acquire_type == JOB? TRUE: FALSE);
//#if !(GC312()||GC321()) // cond. changed - no cal valve in 312 and 322. P.P. 04/01/08
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
            if(!(pMethod->method_table.syringe_on_off)) {         // cond added. P.P. 04/28/06
                SetCalValve(acquire_type == JOB);
            }
#endif // !gc312        }
#if QUICK_SCAN()  // case added. P.P. 05/13/11
            InfoRun(TRUE);
      if((acquire_type == CAL)||(acquire_type == CHECKCAL)) {
	InfoCal(TRUE);
      }
      else {
	InfoCal(FALSE);
      }
#endif

            //       SamplePump(method_table.cont_pump_on_off); //P.P.
// Line above replaced - Pump is ON, cont or not. P.P. 06/29/01
#if CONCENTRATOR()
            //      Purge(TRUE); // added on cond. P.P. 02/18/06  This "Clean" valve let sample into desorber rem P.P. 02/04/10
// 2 lines removed for Jack's variant, 1. P.P. 02/20/06
//      Set_valve(point_port_offset, injectV4_bit_number+8, TRUE);  // sample into desorber. P.P. 02/20/06  corrected 03/08/17  moved to start inject. P.P. 03/22/17
//      inj_V4_on = TRUE; // added. P.P. 03/18/17

//       Set_valve(point_port_offset, injectV5_bit_number, TRUE);  // rem - not needed. P.P. 03/04/06
      Equilibrate(FALSE); // added. P.P. 03/17/17
#endif // concentrator
#if !MANUAL_INJ() // cond added. P.P. 05/19/06
            if(!(pMethod->method_table.syringe_on_off)) {  // cond added. P.P. 04/28/06
                SamplePump(TRUE);
            }
#endif // !manual inj
        SetLamps(TRUE);

            AutoZero(TRUE);
#if !(WATER_PUMP()||TWO_COLUMNS())
            AutoZeroB(TRUE); // added. P.P. 06/21/17
#endif
#if WATER_PUMP()      // P.P. 01/29/17
            Equilibrate(TRUE); // P.P. 01/29/17, valve 4 ON on start sampling.
#if PMD_12LS()
      My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // valve 5 OFF. P.P. 01/29/17
#elif CBWUSB_IO()
      My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 0); // valve 5 OFF. P.P. 01/29/17
#endif
#elif TWO_COLUMNS() // P.P. 03/24/17
            //      Equilibrate(TRUE); // P.P. 01/29/17, valve 4 ON on start sampling.
#if PMD_12LS()
      My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // valve 5 OFF. P.P. 01/29/17
#elif CBWUSB_IO()
      My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 0); // valve 5 OFF. P.P. 01/29/17
#endif
#endif // water pump or 2 columns
// added to send info on cal or checkcal run. P.P. 07/25/04
#if ANALOG_MIX() // cond added. P.P. 08/17/04
            #if CBW()
#if (POINTS()<=4) // P.P. 07/25/04
#if !WATER_PUMP() // P.P. 01/29/17
      if((acquire_type==CAL)||(acquire_type==CHECKCAL)) {
	Diagnostics_Alarm(cal_info_bit, TRUE);
      }
      else {
	Diagnostics_Alarm(cal_info_bit, /*TRUE*/FALSE);  // corrected. P.P. 08/08/04
      }
#endif // !water_pump
#else // > 4 points. Added. P.P. 04/12/05
#endif // points<=4
#endif // cbw
#endif // analog_mix
// end of added code. P.P. 07/25/04

#endif // stand_alone. P.P. 08/23/02
        }
            break;
        case WAIT_SAMPLE_TIME:
#if !STAND_ALONE() // added. P.P. 07/15/03
            if(dwSampleTime<GetTickCount()) {
                MethodTable &method_table=pMethod->method_table;  // added. P.P. 04/27/04
                if(!safety_warning) pMainWindow->sl->Replace("");
                dwSampleTime = 0; // added. P.P. 04/27/06
//#if !GC321()  // no filter, cal, and sample valves in 321c_USB. P.P. 03/29/02 Cond rem 03/21/08
//        SamplePump(TRUE);
//#else
                // remove for the variant with flow into the desorber during it heats up. P.P. 02/20/06
#if CONCENTRATOR()
                dwMoistureOutTime = GetTickCount() + 120*1000UL; // P.P. 12/26/17 // ?? check - I made the dsb wait time for this! P.P. 08/31/18
//        Purge(FALSE); //  P.P. 02/18/06  But better make this later! 02/20/06  rem P.P. 02/04/10         rem P.P. 03/22/17
	Purge(TRUE); //  P.P. 12/26/17
//	Set_valve(point_port_offset, injectV4_bit_number+8, FALSE);  // back to inverted flow. P.P. 02/20/06 corrected 03/08/17
//        inj_V4_on = FALSE; // added. P.P. 03/18/17

//        Set_valve(point_port_offset, injectV5_bit_number, FALSE);  // but better keep for the time of desorbing. P.P. 02/20/06
// added to pre-heat desorber for 1 min. P.P. 02/20/10. Now up to 5 min, from dialog. P.P. 03/07/10
	Concentrator(TRUE); // enable desorber for pre-heating anyway. P.P. 03/07/10  // rem temp until pre-heat is added. P.P. 03/15/17 restored 03/16/17
/*  // moved down. P.P. 03/16/17
	if(pMethod->method_table.dsb_preheat_time) {  // new condition and time. P.P. 03/07/10
	  dwStartInjectTime = GetTickCount() + method_table.dsb_preheat_time*1000UL;
	  if(pMethod->method_table.inj_det_temp) {
//	    Concentrator(TRUE); // enable desorber for pre-heating. P.P. 02/20/10
	    SetDetTemperature(method_table.inj_det_temp);
	  }
	  else { // in case of desorb_temp set to 0 in Method.
//            Concentrator(FALSE); // for any case - turn it OFF. rem 03/07/10
	    SetDetTemperature(0); // added. P.P. 01/02/10
	  }
        }
	else {
	  dwStartInjectTime = GetTickCount();
        }
*/
// end. 02/20/10
#endif // concentrator
#if !WATER_PUMP() // P.P. 01/29/17 leave sample pump ON  for 2-col unit
#if !MANUAL_INJ() // cond added. P.P. 05/19/06
                if(!(pMethod->method_table.syringe_on_off)) {  // cond added. P.P. 04/28/06
                    SamplePump(FALSE);
                }
#endif // !manual inj
#endif // water_pump
                // To turn Filter valve OFF in case it is ON // added P.P. 01/29/02.
//        if((acq_runs==3)&&(acquire_type == CAL)) // P.P. Rem 2 lines. 05/02/04
//        Filter_valve(FALSE); //  P.P. 01/29/02.
                //  Cal Valve, if ON, should be ON only for the Sample time anyway.
//	 if((acquire_type != CAL)&&(acquire_type != CHECKCAL)) {   // cond added to leave cal gas ON. P.P. 03/23/05
#if !WATER_PUMP() // P.P. 01/29/17 leave cal valve as it was,  for 2-col unit
//#if !(GC312()||GC321()) // cond changed. P.P. 03/21/08, no cal valve in gc322
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
                if(!(pMethod->method_table.syringe_on_off)) {  // cond added. P.P. 04/28/06
                    SetCalValve(TRUE);
                }                                        // and removed. P.P. 09/02/05
#endif // !gc312
#endif // water_pump
//        }
#if !OLIN() // cond changed - not for any gc501. P.P. 10/23/04
#if !CONCENTRATOR()
#if !MANUAL_INJ() // cond added. P.P. 05/19/06
                if(!(pMethod->method_table.syringe_on_off)) {  // cond added. P.P. 04/28/06
#if !WATER_PUMP()  // P.P. 01/21/17
                    Equilibrate(TRUE);  // added. P.P. 10/30/02 rem temp. P.P. 07/29/04
#endif
                }
#endif // !manual inj
#else // with concentrator and equi valve case.P.P.03/07/17
                //	  Equilibrate(TRUE);  // added. P.P. 03/07/17   rem 05/09/17
#endif // !concentrator
#endif //  !olin

                AutoZero(FALSE);
                AutoZeroB(FALSE); // added. P.P. 06/21/17
// added to move measurement of uv_0[n] into WAIT_START_INJECT state. P.P. 10/05/03
                for(int n=0; n<NUMDETECTORS; n++) {
                    if(pMethod->det[n].uv_type) { // Far UV detector.
                        dwUV_0_Time=GetTickCount()+1*1000UL; // 1 sec restored. P.P. 10/05/03
                        if(n) LampB(FALSE); else Lamp(FALSE); // turn off lamp
                    }
                }
// end. P.P. 10/05/03
                pMainWindow->print("End waiting sample time\r\n");  // enabled P.P. 03/12/17

                comm_state=WAIT_START_INJECT;
#if CONCENTRATOR() // in GC301 or GC 501 - use equil time from table. added. P.P. 02/11/06
                // added code from above, modified. P.P. 03/16/17
	if(pMethod->method_table.dsb_preheat_time) {  // new condition and time. P.P. 03/07/10
//	  dwStartInjectTime = GetTickCount() + method_table.dsb_preheat_time*1000UL;
//	  dwStartInjectTime = GetTickCount() + (method_table.dsb_preheat_time + 120)*1000UL; // P.P. 12/26/17. everything shifted by 2 min for moisture removal.
	  dwStartInjectTime = GetTickCount() + (method_table.dsb_preheat_time + method_table.dsb_wait_time)*1000UL; // P.P. 03/27/18. everything shifted by wait time for moisture removal.
/*  // P.P. 2/26/17. Rem here and transfered to wait_inject_time state, after 2 min for moisture out.
	  if(pMethod->method_table.inj_det_temp) {
	    Concentrator(TRUE); // enable desorber for pre-heating. P.P. 02/20/10
	    SetDetTemperature(method_table.inj_det_temp);
	  }
	  else { // in case of desorb_temp set to 0 in Method.
//            Concentrator(FALSE); // for any case - turn it OFF. rem 03/07/10
	    SetDetTemperature(0); // added. P.P. 01/02/10
	  }
*/
        }
	else {
//	  dwStartInjectTime = GetTickCount();
//          dwStartInjectTime = GetTickCount() + method_table.equilibrate_time*1000UL; //P.P.  03/16/17
//	  dwStartInjectTime = GetTickCount(); //P.P.  03/16/17
//	  dwStartInjectTime = GetTickCount() + 120*1000UL; //P.P.  12/26/17  2 mi for moisture removal
	  dwStartInjectTime = GetTickCount() + method_table.dsb_wait_time*1000UL; //P.P.  03/27/18,  moisture removal
	}

//	dwDesorbTime = GetTickCount() + method_table.desorb_time*1000UL; // P.P. 02/11/06  rem 02/20/06   restored 03/12/17 rem 03/16/17
//        dwStartInjectTime = GetTickCount() + method_table.equilibrate_time*1000UL; //P.P. 02/11/06 restored 03/12/17
	dwStartInjectTime = dwStartInjectTime + method_table.equilibrate_time*1000UL; //P.P. 02/11/06 restored 03/12/17
//        dwStartInjectTime = GetTickCount(); // no need to equilibrate. P.P. 03/14/06
//	dwStartInjectTime = GetTickCount()+60*1000UL; // 1min to pre-heat desorber. P.P. 02/20/10
//	dwStartInjectTime = GetTickCount()+3*1000UL; // 3 s to equilibrate. P.P. 03/07/17
	dwEquilibrateTime = dwStartInjectTime; // P.P. 03/07/17
//	dwEquilibrateTime = 0; // P.P. 02/20/06
#elif GC501() // gc501 without concentrator
                dwStartInjectTime = GetTickCount() + method_table.equilibrate_time*1000UL; //P.P. 02/18/06
        dwEquilibrateTime = dwStartInjectTime + 1*1000UL; // changed. P.P. 02/30/02
//        dwStartInjectTime = GetTickCount() + (method_table.equilibrate_time - 1)*1000UL; //P.P. 12/30/02   moved up and changed. 02/18/06
#else //  in 301 or 312 without concentrator
#if !HV_501() // for Jack's GC only. P.P. 09/30/03
                dwStartInjectTime=GetTickCount()+6*1000UL; // 6 s for equil. P.P. 12/21/02
	dwEquilibrateTime = dwStartInjectTime + 1*1000UL; // P.P. 02/11/06
#else // regular case. P.P. 09/30/03
//        dwStartInjectTime=GetTickCount()+20*1000UL; // 20 s for equil. P.P. 03/14/03
//      dwStartInjectTime=GetTickCount()+5*1000UL; // 5 s for equil. P.P. 12/30/04
#if WATER_PUMP()  // P.P. 01/29/17
                dwStartInjectTime = GetTickCount() + 2*1000UL; // 2 s. P.P. 01/29/17
	dwEquilibrateTime = dwStartInjectTime + 1*1000UL; // P.P. 02/11/06
#else
                dwStartInjectTime = GetTickCount() + method_table.equilibrate_time*1000UL; //now in 301 as well. P.P. 09/25/07 11/06/07
                dwEquilibrateTime = dwStartInjectTime + 1*1000UL; // P.P. 02/11/06
#endif // !water_pump
#endif // !hv_501
//       dwStartInjectTime=GetTickCount()+1000UL; // 1 sec pause before injection
//       dwEquilibrateTime = GetTickCount() + 3*1000UL; // added. P.P. 10/30/02
//       dwEquilibrateTime = GetTickCount() + 6*1000UL; // changed. P.P. 12/06/02
// changed to turn Equil 1 s AFTER Inject start. P.P. 12/21/02
#endif // gc501, concentrator, 301  // P.P. 02/11/06
//#if GC321()
#if GC321()||GC326() //  P.P. 12/07/18
                if(/*!*/pMethod->method_table.syringe_on_off)  {         // added. P.P.
//          pMainWindow->sl->Replace("Syringe Injection Time");    // removed. P.P. 05/14/02
          TRACE("Syringe Injection Time");
          MessageBeep(MB_ICONASTERISK); //cue(); // cue sound - be ready in 3 sec.  P.P. 03/29/02
        }
        if(pMethod->method_table.syringe_on_off) {         // added. P.P. 04/29/02
	  if(!safety_warning) pMainWindow->sl->Replace("Ready?");
        }
        else {                                               // changed. P.P. 05/14/02
	  if(!safety_warning) pMainWindow->sl->Replace("Waiting inject start");
        }

                            My_cbDBitOut (4, FIRSTPORTA, autozeroB_bit_number, True);
                    My_cbDBitOut (CBWDEVICE_16FS, AUXPORT, 1, 0);
#elif GC312() // case added. P.P. 04/27/06
                #if MANUAL_INJ() // case added. P.P. 05/19/06
	if(!safety_warning) pMainWindow->sl->Replace("Be ready to inject");    // changed. P.P. 04/28/06
#else
        if(pMethod->method_table.syringe_on_off) {         // added. P.P. 04/29/02
	  if(!safety_warning) pMainWindow->sl->Replace("Be ready to inject");    // changed. P.P. 04/28/06
        }
        else {                                               // changed. P.P. 05/14/02
//        pMainWindow->sl->Replace("Waiting inject start");
	  if(!safety_warning) pMainWindow->sl->Replace("Equilibrate");
        }
#endif // manual inj
#else // 301 or 501
#if CONCENTRATOR()
                //        Purge(TRUE); // purge during equilibrate. P.P. 09/13/02  removed 02/11/06
//      pMainWindow->sl->Replace("Purge");   // replaced for line below. 02/11/06
	if(!safety_warning) pMainWindow->sl->Replace("Desorb");

#else
//	if(!safety_warning) pMainWindow->sl->Replace("Equilibrate");  // rem - messages are in common location. P.P. 07/31/10
#endif // concentrator
                pMainWindow->print("Waiting inject start...\r\n");   // enabled P.P. 03/12/17
#endif // GC321() or GC326()
            }
            else { // during sampling time // added P.P. 03/13/18
                if(dwSampleTime) {
                    sprintf(time_str, "Sampling: %d", (dwSampleTime - GetTickCount())/1000);
                    if(!safety_warning) pMainWindow->sl->Replace(time_str);
                }
            }
#else // stand_alone+gc321 combo added. P.P. 07/12/06
            //#if GC321()
#if GC321()||GC326() //  P.P. 12/07/18
      if(dwSampleTime&&(dwSampleTime<GetTickCount())) {
	comm_state=WAIT_START_INJECT; // added. P.P. 07/16/06
      	dwStartInjectTime=GetTickCount()+5*1000UL; // 5 s for equil. P.P. 07/16/06
	dwSampleTime = 0;
	MessageBeep(MB_ICONASTERISK); //cue(); // cue sound - be ready in 5 sec.
	if(/*!*/pMethod->method_table.syringe_on_off)  {         // added. P.P.
//              pMainWindow->sl->Replace("Syringe Injection Time");    // removed. P.P. 05/14/02
          TRACE("Syringe Injection Time");
	  if(!safety_warning) pMainWindow->sl->Replace("Be ready to inject");
	}
	else {
//	  if(!safety_warning) pMainWindow->sl->Replace(" "); // added. P.P. 07/16/06  // rem - messages are in common location. P.P. 07/31/10
	}
      }
#else  // regular stand-alone case added. P.P. 11/01/15
      if(dwSampleTime&&(dwSampleTime<GetTickCount())) {
	comm_state=RECEIVING_DATA; // added. P.P. 11/01/15
	dwSampleTime = 0;
        AutoZero(FALSE);
        AutoZeroB(FALSE); // added. P.P. 06/21/17
      }
#endif // gc321
#endif // !stand_alone. P.P. 07/15/03
// Diagnostics of carrier gas added to Wait SampleTime state. P.P. 08/10/04
#if DIAGNOSTICS()
            int Diagnostics_error_flag = Diagnostics(diagnostics_port);
      N2_error = ((Diagnostics_error_flag & BIT[N2_error_bit])!= 0);        // Changed. P.P. 02/21/03
#if TCD() // cond added. P.P. 08/15/04
      if(N2_error) {
        Acquire::TCDDetector(FALSE);    // Turn ON manually after this. P.P. 06/15/04
      }
#endif // tcd
//    Air_error = ((Diagnostics_error_flag & BIT[Air_error_bit])!=0); // removed. P.P. 06/15/04
      CalGas_error = ((Diagnostics_error_flag & BIT[CalGas_error_bit])!=0);
//#endif // diagnostics   moved down. P.P. 01/29/17
//    if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error) {  // oven excluded during temperature set.  P.P. 04/27/04
#if FID() // added. P.P. 08/07/05
      if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error||Oven_error||FID_error) {  // FID added during sample time.  P.P. 08/07/05
#else
      if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error||Oven_error) {  // oven added during sample time.  P.P. 12/04/04
#endif // fid
        Diagnostics_error = 1;                              //
      }
      else {
        Diagnostics_error = 0;
      }
//#if !(GC312()||GC321()) // cond changed. P.P. 03/21/08
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
      if(pMethod->method_table.alarms_on_off) {    // cond added. P.P. 12/04/04
        if(Diagnostics_error) {
          Diagnostics_Alarm(diagnostics_bit_number, TRUE); // added. P.P. 08/08/04
          Alarm(TRUE); // internal alarm added. P.P. 12/04/04   rem 01/05/09
        }
        else {
	  Diagnostics_Alarm(diagnostics_bit_number, FALSE); // added. P.P. 08/08/04
          Alarm(FALSE); // added. P.P. 12/04/04   // rem 01/05/09
	}
      }
      else { // case added. P.P. 12/04/04
        Diagnostics_Alarm(diagnostics_bit_number, FALSE);
//        Alarm(FALSE);  // rem 01/05/09
      }
#endif // ! gc312 or gc322 or gc326
//    Diagnostics_Alarm(diagnostics_bit_number, Diagn_error); // added. P.P. 08/08/04
#endif // diagnostics, moved here. P.P. 01/29/17
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
            if(N2_error || Air_error) {  // add later Oven and Oven_high errors. P.P. 02/23/03
        Fatal_error = 1;
// 	  Fatal_error_Str=N2_error? "N2_error": "Air_error";
	Fatal_error_Str=N2_error? "Carrier gas error": "Air_error"; // replacement. P.P. 08/22/04
      }
      else {
        Fatal_error = 0;
      }
      if(Fatal_error) {   // added to turn OFF the oven. P.P. 08/03/04
//	  comm_state = END_ACQUIRE; // 2 lines added to stop run. P.P. 08/08/04. Rem 08/10/04
      }
      else { // added to restore oven enable. 05/18/07
      }
#endif // diagnostics
// end of added diagnostics. P.P. 08/10/04
            data_A = Get_Data(0, cur_gain[0]);  // added. P.P. 08/02/10
            data_B = Get_Data(1, cur_gain[1]);  // added. P.P. 12/20/10
            break;

        case WAIT_START_INJECT:
        {
            MethodTable &method_table=pMethod->method_table; // moved here from beow. P.P. 02/11/06
            data_A = Get_Data(0, cur_gain[0]);  // added. P.P. 08/02/10
            data_B = Get_Data(1, cur_gain[1]);  // added. P.P. 12/20/10
#if !STAND_ALONE() // P.P. 07/15/03
            if(dwStartInjectTime<GetTickCount()) {
                pMainWindow->print("End waiting inject start\r\n"); // enabled P.P. 03/12/17
//        MethodTable &method_table=pMethod->method_table; // moved up. P.P. 02/11/06
                alarm_cal_off = 0;  // added. P.P. 10/12/08 12/12/08
                comm_state=RECEIVING_DATA;
                dwLastTime=0;
                dwStartInjectTime = 0; // added. P.P. 10/30/09
#if CONCENTRATOR()
                Equilibrate(FALSE);  // added. P.P. 03/07/17
#endif
#if QUICK_SCAN() // cond added. P.P. 01/17/11  and rem 01/29/17
                dwAnalysisTime = GetTickCount() + method_table.analysis_time*1000UL; // added. P,P, 01/15/11
#endif // quick scan
//#if GC321()  // cond removed. P.P. 09/11/15
//        dwInjectTime=GetTickCount()+3*1000UL;    // P.P. 06/13/02  replaced for code below. 03/21/08  P.P.
#if !TIME_MULT() // 03/16/08 made on cond
                dwInjectTime=GetTickCount()+method_table.inject_time*1000UL;
#if WATER_PUMP() // P.P. 01/29/17
                dwInjectTime2=GetTickCount()+45*1000UL;
	dwInjectTime2_off=dwInjectTime2+15*1000UL;
#endif
#else
                dwInjectTime=GetTickCount()+method_table.inject_time*10UL;  // testing times <1s. 03/11/08  P.P.
#endif // time_mult
#if !MANUAL_INJ() // cond added. P.P. 06/18/06
//	dwEquilibrateTime = GetTickCount() + method_table.equilibrate_time*1000UL; // added P.P. 05/25/04 and rem 04/01/08. P.P.
#if !CONCENTRATOR() // cond. added. P.P. 03/09/17
                dwEquilibrateTime = 0; // P.P. 06/18/06. Not needed when inject starts. added here 01/29/17
#endif
#else
                #if !CONCENTRATOR() // cond. added. P.P. 03/09/17
	dwEquilibrateTime = 0; // P.P. 06/18/06. Not needed when inject starts
#endif
#endif // !manual inj
                Equilibrate(FALSE); // added. P.P. 03/24/17
// x1-valve code added to GC321/322.    and GC312. 09/11/15
#if X1_VALVE() // cond replaced - in any GC. P.P. 07/10/05

#if QUICK_SCAN() // cond added. P,P, 01/17/11 here 06/17/11
                if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
                if(1) { // as it was
#endif // quick scan
//        if(!quick_scan_active) { // regular run. P.P. 12/20/10
                    if((method_table.x1_time)&&                                 // P.P. 06/18/06
                       ((method_table.x1_off_time - method_table.x1_time)>0)) { //
                        dwXvalveTime = GetTickCount()+ method_table.x1_time*1000UL; // changed. P.P. 07/30/04
                        dwXvalveOffTime = GetTickCount()+ method_table.x1_off_time*1000UL; // added. P.P. 06/18/06
                        dwAutoZero1 = dwXvalveTime + 3*1000UL;    // P.P. 08/06/04  // X1 time for switching range. P.P. 05/12/17
                        dwAutoZero1_off = dwAutoZero1 + 10*1000UL; // P.P. 08/06/04
                        dwRange_1 = dwXvalveTime; // change range for range_1. P.P. 05/12/17
                        dwRange_1_off = dwXvalveOffTime; // added to return range back. P.P. 05/12/17
                        dwAutoZero2 = dwXvalveOffTime + 3*1000UL;    //  Zeroing after switching range back. P.P. 05/12/17
                        dwAutoZero2_off = dwAutoZero2 + 3*1000UL; //

                    }
                    else {
                        dwXvalveTime = 0;
                        dwXvalveOffTime = 0; // added. P.P. 06/18/06
                        dwAutoZero1 = 0;
                        dwAutoZero1_off = 0;
                        dwRange_1 = 0;
                        dwRange_1_off = 0;
                        dwAutoZero2 = 0;    //
                        dwAutoZero2_off = 0; //
                    }
#if !WATER_PUMP() // only 1 x_valve in the variant with water pump. P.P. 01/07/17
// 3 more x_valves added. P.P. 06/20/10
///* // in GC322, x2 and x3 work now during sample time. P.P. 07/01/10. restore them for GC312 but not for GC322. P.P. 09/11/15
#if !GC321()  // P.P. 09/11/15
//#if !(GC321()||GC326()) //  P.P. 12/07/18 cond restore - in GC326, x2 and x3 times are as in regular GCs
                    // not finished for the 2nd switching range, do this later after tests. P.P. 05/12/17  Opened 06/20/17
                    if((method_table.x2_time)&&                                 // P.P. 06/20/10
                       ((method_table.x2_off_time - method_table.x2_time)>0)) { //
                        dwXvalve2Time = GetTickCount()+ method_table.x2_time*1000UL;
                        dwXvalve2OffTime = GetTickCount()+ method_table.x2_off_time*1000UL;
                        dwAutoZero3 = dwXvalve2Time + 3*1000UL;    //  now X2 valve for this as well as x5 and x3. P.P. 09/11/15
                        dwAutoZero3_off = dwAutoZero2 + 3*1000UL; //
                        dwRange_1_2 = dwXvalve2Time; // switch to new range. P.P. 05/12/17
                        dwRange_1_2_off = dwXvalve2OffTime; // added to return range back. P.P. 09/18/15
                        dwAutoZero4 = dwXvalve2OffTime + 3*1000UL;    //
                        dwAutoZero4_off = dwAutoZero3 + 3*1000UL; //
                    }
                    else {
                        dwXvalve2Time = 0;
                        dwXvalve2OffTime = 0;
                        dwAutoZero3 = 0;    //  now X2 valve for this as well as x1. P.P. 05/12/17
                        dwAutoZero3_off = 0; //
                        dwRange_1_2 = 0;
                        dwRange_1_2_off = 0;
                        dwAutoZero4 = 0;    //  now X2 valve for this as well as x1. P.P. 05/12/17
                        dwAutoZero4_off = 0; //
                    }

                    if((method_table.x3_time)&&
                       ((method_table.x3_off_time - method_table.x3_time)>0)) {
                        dwXvalve3Time = GetTickCount()+ method_table.x3_time*1000UL;
                        dwXvalve3OffTime = GetTickCount()+ method_table.x3_off_time*1000UL;
                    }
                    else {
                        dwXvalve3Time = 0;
                        dwXvalve3OffTime = 0;
                    }
#endif // not GC322. P.P. 09/11/15
//*/
                    if((method_table.x4_time)&&
                       ((method_table.x4_off_time - method_table.x4_time)>0)) { //
                        dwXvalve4Time = GetTickCount()+ method_table.x4_time*1000UL;
                        dwXvalve4OffTime = GetTickCount()+ method_table.x4_off_time*1000UL;
                    }
                    else {
                        dwXvalve4Time = 0;
                        dwXvalve4OffTime = 0;
                    }
                    if((method_table.x5_time)&&    // x5 added. P.P. 07/01/10  05/12/17: x5 is not used for changing range.
                       ((method_table.x5_off_time - method_table.x5_time)>0)) { //
                        dwXvalve5Time = GetTickCount()+ method_table.x5_time*1000UL;
                        dwXvalve5OffTime = GetTickCount()+ method_table.x5_off_time*1000UL;
#if FLOW_RATE()
                        X5_valve(FALSE); // P.P. 05/13/18 At start of inject, it stays OFF, to be default.
#endif // flow_rate                                                                         	  }
                    }
                    else {
                        dwXvalve5Time = 0;
                        dwXvalve5OffTime = 0;
                    }
#endif // !water_pump
                }  // !quick_scan_active. P.P. 12/20/10
// end of added code for 3 more valves. P.P. 06/20/10
#endif // x1-valve
// end of added code for x1-valve

#if QUICK_SCAN() // cond added. P,P, 01/17/11 here 06/17/11
                if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
                if(1) { // as it was
#endif // quick scan
//	if(!quick_scan_active) { // regular run. P.P. 12/20/10
                    if(/*!*/pMethod->method_table.syringe_on_off)  {         // added. P.P.
                        MessageBeep(MB_ICONHAND);   // inject now!.  P.P. 03/29/02
                    }
                    else {          // added - use valves in GC322. P.P. 04/01/08
                        Inject(TRUE); //
#if WATER_PUMP()  // P.P. 01/21/17
                        //	    Equilibrate(TRUE); // valve 4 ON (injection to 1st column, A) P.P. 01/21/17
	    Equilibrate(FALSE); // valve 4 OFF as Jack wants, incorrectly, P.P. 01/29/17
#endif
                    }
                } // !quick_scan_active. P.P. 12/20/10                 //
//#else // end of not GC321 condition moved up P.P. 09/11/15
#if QUICK_SCAN() // cond added. P,P, 01/17/11
                if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
                if(1) { // as it was
#endif // quick scan
#if CONCENTRATOR() // added. P.P. 02/05/06
                    // 3 lines uncommented for my variant with sampling direction not inverted when heating up. P.P. 02/20/06 commented 03/04/06    restored ON 03/22/17
	  Set_valve(point_port_offset, injectV4_bit_number+8, TRUE);  // added for Jack's variant 1. P.P. 02/20/06   added here as indicator. P.P. 03/22/17
	  inj_V4_on = TRUE; // added. P.P. 03/18/17
//	  Set_valve(point_port_offset, injectV5_bit_number+8, TRUE);  //
//	  Set_valve(point_port_offset, injectV4_bit_number+8, FALSE);  // added as a variant to invert flow. P.P. 02/20/06   03/16/17   rem 03/22/17
//          inj_V4_on = FALSE; // added. P.P. 03/18/17
//	  Set_valve(point_port_offset, injectV5_bit_number+8, FALSE);  //
//	  Purge(FALSE); // disabled for now, to enable later if needed. P.P. 02/20/06
#endif   // concentrator
#if MANUAL_INJ() // case added. P.P. 05/19/06
                    #if !CONCENTRATOR() // cond. added. P.P. 03/09/17
	  dwEquilibrateTime = 0; // not needed when inject starts
#endif
	  dwInjectTime=GetTickCount()+5*1000UL;
#else  // not manual inj
                    if(pMethod->method_table.syringe_on_off) {   // added. P.P. 04/28/06
#if !CONCENTRATOR() // cond. added. P.P. 03/09/17
                        dwEquilibrateTime = 0; // not needed when inject starts
#endif
                        MessageBeep(MB_ICONHAND);   // inject now!.  P.P. 04/28/06
                    }
#if !TIME_MULT() // 03/16/08 made on cond
                    dwInjectTime=GetTickCount()+method_table.inject_time*1000UL;
#if WATER_PUMP() // P.P. 01/29/17
                    dwInjectTime2=GetTickCount()+45*1000UL;
	  dwInjectTime2_off=dwInjectTime2+15*1000UL;
#endif
#else
                    dwInjectTime=GetTickCount()+method_table.inject_time*10UL;  // testing times <1s. 03/11/08
#endif // time_mult
//          dwShowMessageTime = dwInjectTime + 2*1000UL; // added. P.P. 11/01/09 moved to trans_meth
                    if(!(pMethod->method_table.syringe_on_off))  { // cond added. P.P. 04/28/06
                        Inject(TRUE);
                        if(!safety_warning) pMainWindow->sl->Replace("Inject"); // added here P.P. 03/15/18             // better this way
//	    Purge(TRUE);  // added here as Jack asked. P.P. 10/17/17  and rem 11/03/17
                    }
//#if GC501()&&SAMPLE_AHEAD()&&(POINTS()>1) // cond. added. P.P. 07/17/04
//#if SAMPLE_AHEAD()&&(POINTS()>1) // cond. changed. P.P. 08/22/04
#if SAMPLE_AHEAD() // even for 1-point case. P.P. 10/17/17
                    if(method_table.next_point_time) { //  P.P. 10/17/17
                        dwNextPointTime=GetTickCount()+method_table.next_point_time*1000UL; // P.P. 10/17/17
                    }
                    else {
                        dwNextPointTime = 0;
                    }
#else
                    dwNextPointTime = 0;
#endif
#if CONCENTRATOR() // code added. P.P. 02/11/06
                    if(method_table.purge_time) { // no sampling ahead! P.P. 10/17/17 sample ahead is separated from purge since now
	    dwPurgeTime=GetTickCount()+method_table.purge_time*1000UL; // added. P.P. 01/08/03; here 05/25/04
          }
	  else {
	    dwPurgeTime = 0;
          }
//	  if(method_table.equilibrate_time) { // equil as desorb time. case added. P.P. 03/14/06
//	    dwEquilibrateTime = GetTickCount()+method_table.equilibrate_time*1000UL;
	  if(method_table.desorb_time) { //desorb time is used. P.P. 03/15/17
	    dwDesorbTime = GetTickCount()+method_table.desorb_time*1000UL;
	    Concentrator(TRUE); // enable desorber
	  }
	  else {
//	    dwEquilibrateTime = 0;
	    dwDesorbTime = 0;   // P.P. 03/15/17
//	    Concentrator(FALSE); // added to disable desorber if its time set to 0. P.P. 01/02/10 rem to not disable setting T to 0 if needed. P.P. 03/07/18
	  }

#endif // concentrator
#endif // manual inj. P.P. 05/19/06
                } // !quick_scan_active P.P. 12/20/10
//#endif //GC321
#if QUICK_SCAN() // cond added. P.P. 01/17/11
                if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
                if(1) {
#endif // quick scan
                    if(pMethod->method_table.syringe_on_off)          // added. P.P. 04/29/02
                        if(!safety_warning) pMainWindow->sl->Replace("Inject");
                        else
//          pMainWindow->sl->Replace("Waiting inject time"); // P.P. 09/28/02,
                        if(!safety_warning) pMainWindow->sl->Replace("Inject");              // better this way
                    pMainWindow->print("Waiting inject time...\r\n");   // enabled P.P. 03/12/17
                    if(tempcontrol) tempcontrol->Start(); // start ramping if enabled
                    if(runMode.Acquire == 3) { // remote, added. P.P. 03/26/03
//            InitDAC();               // remove - not needed! P.P. 05/03/04 removed 04/28/06
                    }                          //
                } // !quick_scan_active, to make shorter statements
#if QUICK_SCAN() // cond added. P.P. 01/17/11
                if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
                if(1) {
#endif // quick scan
                    for(int n=0; n<NUMDETECTORS; n++) {  // moved here or no effect. P.P. 09/26/03
                        if(pMethod->det[n].uv_type) {              // Far UV detector.
//            dwUV_0_Time=GetTickCount()+2*1000UL;     // 2 sec. Moved to previous state. P.P. 10/05/03
                            dwUV_Max_Time = GetTickCount()+4*1000UL; // 4 sec // added. P.P. 09/26/03
                        }
                    }
                } // !quick_scan_active
#if RAMPING() // added for messages. P.P. 06/23/06
                uint rate1 = method_table.OvenRampR1;
	  uint rate2 = method_table.OvenRampR2;
	  ulong HoldTime1 = method_table.OvenHoldTime1;
	  ulong HoldTime2 = method_table.OvenHoldTime2;
	  ulong HoldTime3 = method_table.OvenHoldTime3;
#if QUICK_SCAN() // cond added. P,P, 01/17/11  here 06/17/11
        if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
        if(1) { // as it was
#endif // quick scan
	  uint HoldT1 = method_table.oven_temp;
	  uint HoldT2 = method_table.OvenHoldT2;
	  uint HoldT3 = method_table.OvenHoldT3;
	  number_of_beeps = 4; // added. P.P. 06/25/06
//	if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
	  if(HoldTime1) {
	    dwOvenHoldTime1 = GetTickCount() + method_table.OvenHoldTime1*1000UL;
	  }
	  if((rate1)&&(HoldT2>HoldT1)) {
	    dwOvenRamp1Time = dwOvenHoldTime1 + ((HoldT2-HoldT1)*60/rate1)*1000UL;
	  }
	  if(HoldTime2) {
	    dwOvenHoldTime2 = dwOvenRamp1Time + method_table.OvenHoldTime2*1000UL;
          }
	  if(rate2) {
	    if(HoldT3>HoldT2) {
	    dwOvenRamp2Time = dwOvenHoldTime2 + ((HoldT3-HoldT2)*60/rate2)*1000UL;
	    }
	    else if(HoldT3<HoldT2) {
	    dwOvenRamp2Time = dwOvenHoldTime2 + ((HoldT2-HoldT3)*60/rate2)*1000UL;
	    }
	  }
	  if(HoldTime3) {
	    dwOvenHoldTime3 = dwOvenRamp2Time + method_table.OvenHoldTime3*1000UL;
          }
        }  // !quick_scan_ative
#endif // ramping
            }
            else { // during wait inject time. Code added. P.P. 02/05/06
#if CONCENTRATOR()
                /*      // removed. P.P. 02/18/06
	Set_valve(point_port_offset, injectV4_bit_number+8, TRUE);  // added P.P. 08/27/02
	Set_valve(point_port_offset, injectV5_bit_number+8, TRUE);  //
*/
 //	temp_conc = Get_Temp_Concentrator();  // added and rem
/* //   removed - no wait inject after sample time. P.P. 03/14/06
	if(temp_conc >= method_table.inj_det_temp) {  // changed 02/20/06. P.P.
	  Concentrator(FALSE);
        }
	else if(temp_conc + 5.0 < method_table.inj_det_temp) {    // P.P. 02/20/06
	  Concentrator(TRUE);
	}
	else {
	} // nothing to do
*/
	if(dwMoistureOutTime < GetTickCount()) {  // P.P. 12/26/17  // restored. P.P.3/24/18
//	if(dwMoistureOutTime&&(dwMoistureOutTime < GetTickCount())) {  // P.P. 12/26/17
	  if(purge_on) {
	    Purge(FALSE); // 2 min has finished.
	    dwDesorberBatteryCheckTime = GetTickCount() + 20*1000.0; // 20 s to check battery, added 03/23/18
	  }
// added to heat up desorber. P.P. 02/20/10
/*
	  if(pMethod->method_table.inj_det_temp) {
//	     Concentrator(TRUE); // enable desorber for pre-heating. P.P. 02/20/10
	    SetDetTemperature(method_table.inj_det_temp);
*/
          void SetDsbTemperature(double t); // prototype. P.P. 07/30/18
	  if(pMethod->method_table.desorb_temp) { // its own temp set now. P.P. 07/27/18
	    SetDsbTemperature(method_table.desorb_temp);
// code added to check if dsb temp rises quick enough (dsb battery check). P.P. 03/23/18
	    if(dwDesorberBatteryCheckTime && (dwDesorberBatteryCheckTime < GetTickCount())) {
	      dwDesorberBatteryCheckTime = 0; // check only once. P.P. 03/23/18
	      if(temp_conc < 50.0) {
         	Str str="Desorber Battery Low";
                MessageBox(NULL,str, "Replenish Desorber Battery", MB_OK);  // run stops, messsage shown. P.P. 03/23/18
		return -1; // stop the run. P.P. 03/23/18
              }
            }
	  }
	  else { // in case of desorb_temp set to 0 in Method.
//          Concentrator(FALSE); // for any case - turn it OFF.
//	    SetDetTemperature(0); // added. P.P. 01/02/10
	    SetDsbTemperature(0); // P.P. 07/27/18
	  }
// end. 02/20/10

/*
 // removed. P.P. 02/20/06  restored 03/15/17 and rem - desorb time starts after inject starts. 03/16/17
	  if(dwDesorbTime && (dwDesorbTime < GetTickCount())) { // equilibrate now. P.P. 02/11/06
	    Concentrator(FALSE);   // 2 lines removed. P.P. 02/18/06  restored 03/15/17
//          Equilibrate(TRUE);
//          Purge(TRUE); // purge or not during equilibrate? P.P. 02/11/06
	    dwDesorbTime = 0;
	  }
*/
	  dwMoistureOutTime = 0;
	  sprintf(time_str, "Waiting start inject"); // P.P. 03/13/18
	  if(!safety_warning) pMainWindow->sl->Replace(time_str);
	}
	else {  // show message and remaining time during moisture removal time. P.P. 03/13/18
          if(dwMoistureOutTime) {
//	    sprintf(time_str, "Moisture removal: %d", (dwMoistureOutTime - GetTickCount())/1000);
	    sprintf(time_str, "Waiting ...");  // P.P. 03/16/18
	    if(!safety_warning) pMainWindow->sl->Replace(time_str);
          }
	}
#endif // concentrator
            }
// added code to measure uv_0[n]. P.P. 10/05/03
            for(int n=0; n<NUMDETECTORS; n++) {  // moved here or no effect. P.P. 09/26/03
                if((pMethod->det[n].detect_stat)&&(pMethod->det[n].uv_type)) {              // Far UV detector.
//        if(dwUV_0_Time&&(dwUV_0_Time<GetTickCount())) {
                    if(dwUV_0_Time) { // changed. P.P. 11/22/09
                        int32 data_in; // 16 bit - signed or unsigned
//	    gain[n] = pMethod->det[n].gain; // added to initialize. P.P. 10/05/03
                        gain[n] = 2; //  P.P. 11/22/09
                        data_in=Get_Data(n, gain[n]);
                        if(pMethod->det[n].invert) {    // from now, inversion is available. P.P. 11/06/09
                            data_in = -data_in;
                        }
                        int32 data=data_in;
//            data*=(uint32)pow10(gain[n]-pRun->runrawdata.gain[n]); // added to *100. P.P. 10/05/03
//            data*=(uint32)pow10(2-gain[n]); // added, to *100. P.P. 10/05/03
                        data*=100; // added, to *100. P.P. 11/22/09
                        uv_0[n] = data;                                        //
                    }
                }
            }
            if(dwUV_0_Time && (dwUV_0_Time<=GetTickCount())) {
                SetLamps(TRUE); //  turn on all needed lamps //
                dwUV_0_Time=0;
            }
// end of added code for uv_0[n]. P.P. 10/05/03

            // moved to turn Equil AFTER Inject starts. P.P. 12/21/02
// Diagnostics of carrier gas added to Wait SampleTime state. P.P. 08/10/04
#if QUICK_SCAN() // cond added. P.P. 01/17/11
            if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
            if(1) {
#endif
#if DIAGNOSTICS()
                int Diagnostics_error_flag = Diagnostics(diagnostics_port);
	  N2_error = ((Diagnostics_error_flag & BIT[N2_error_bit])!= 0);        // Changed. P.P. 02/21/03
#if TCD() // cond added. P.P. 08/15/04
	  if(N2_error) {
            Acquire::TCDDetector(FALSE);    // Turn ON manually after this. P.P. 06/15/04
          }
#endif // tcd
//	  Air_error = ((Diagnostics_error_flag & BIT[Air_error_bit])!=0); // removed. P.P. 06/15/04
	  CalGas_error = ((Diagnostics_error_flag & BIT[CalGas_error_bit])!=0);
//#endif // diagnostics   // mived down. P.P. 12/20/10
//	  if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error) {  // oven excluded during temperature set.  P.P. 04/27/04
#if FID() // added. P.P. 08/07/05
	  if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error||Oven_error||FID_error) {  // fid error added during equilibrate.  P.P. 08/07/05
#else
	  if(N2_error||Air_error||CalGas_error||Lamp_error||Board_error||Oven_error) {  // oven added during equilibrate.  P.P. 12/04/04
#endif // fid
	    Diagnostics_error = 1;                              //
          }
	  else {
	    Diagnostics_error = 0;
	  }
#endif // diagnostics
            } // !quick_scan_active. P.P. 12/20/10
//#if GC312() // cond added. P.P. 03/19/06
#if !GC312() // cond replaced. P.P. 04/08/06
#if QUICK_SCAN() // cond added. P.P. 01/17/11
            if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
            if(1) {
#endif // quick scan
                if(pMethod->method_table.alarms_on_off) {    // cond added. P.P. 12/04/04
                    if(Diagnostics_error) {
                        Diagnostics_Alarm(diagnostics_bit_number, TRUE); // added. P.P. 08/08/04
                        Alarm(TRUE); // internal alarm added. P.P. 12/04/04  // rem 01/05/09
                    }
                    else {
                        Diagnostics_Alarm(diagnostics_bit_number, FALSE); // added. P.P. 08/08/04
                        Alarm(FALSE); // added. P.P. 12/04/04  // rem 01/05/09
                    }
                }
                else { // case added. P.P. 12/04/04
                    Diagnostics_Alarm(diagnostics_bit_number, FALSE);
//          Alarm(FALSE);   // rem 01/05/09
                }
            } // !quick_scan_active
#endif // !GC312
//	  Diagnostics_Alarm(diagnostics_bit_number, Diagn_error); // added. P.P. 08/08/04
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
            #if QUICK_SCAN() // cond added. P.P. 01/17/11
	if(!quick_scan_active) { // condition added to skip all these steps if in quick scan run. P.P. 12/20/10
#else
	if(1) {
#endif // quick scan
	  if(N2_error || Air_error) {  // add later Oven and Oven_high errors. P.P. 02/23/03
	    Fatal_error = 1;
//	    Fatal_error_Str=N2_error? "N2_error": "Air_error";
	    Fatal_error_Str=N2_error? "Carrier gas error": "Air_error"; // replasement. P.P. 08/22/04
	  }
	  else {
	    Fatal_error = 0;
          }
	  if(Fatal_error) {   // added to turn OFF the oven. P.P. 08/03/04
//	    comm_state = END_ACQUIRE; // 2 lines added to stop run. P.P. 08/08/04. Rem 08/10/04
	  }
	  else { // case added to restore oven enable. 05/18/07
	  }
        } // !quick_scan_active. P.P. 12/20/10
#endif // diagnostics
// end of added diagnostics. P.P. 08/10/04
#else // stand alone. Case added for stand_alone+gc321 combo. P.P. 07/12/06 // quick_scan was not added to this case. P.P. 12/20/10
            //#if GC321()
#if GC321()||GC326() //  P.P. 12/07/18
	if(dwStartInjectTime&&(dwStartInjectTime<GetTickCount())) {
	  comm_state=RECEIVING_DATA;
          dwLastTime=0;
	  dwStartInjectTime = 0;
	  dwInjectTime=GetTickCount()+3*1000UL;    // P.P. 06/13/02
#if MANUAL_INJ() // cond added. P.P. 06/18/06
	  dwEquilibrateTime = 0; // P.P. 06/18/06. Not needed when inject starts
#endif // !manual inj
	  if(pMethod->method_table.syringe_on_off) {         // added. P.P. 04/29/02
	    if(!safety_warning) pMainWindow->sl->Replace("Inject");
	    MessageBeep(MB_ICONHAND);   // inject now!.  P.P. 037/16/06
	  }
#if RAMPING() // added for messages in stand-alone case. P.P. 07/16/06
	  uint rate1 = method_table.OvenRampR1;
	  uint rate2 = method_table.OvenRampR2;
	  ulong HoldTime1 = method_table.OvenHoldTime1;
	  ulong HoldTime2 = method_table.OvenHoldTime2;
	  ulong HoldTime3 = method_table.OvenHoldTime3;
	  uint HoldT1 = method_table.oven_temp;
	  uint HoldT2 = method_table.OvenHoldT2;
	  uint HoldT3 = method_table.OvenHoldT3;
	  number_of_beeps = 4; // added. P.P. 06/25/06
	  if(HoldTime1) {
	    dwOvenHoldTime1 = GetTickCount() + method_table.OvenHoldTime1*1000UL;
	  }
	  if((rate1)&&(HoldT2>HoldT1)) {
	    dwOvenRamp1Time = dwOvenHoldTime1 + ((HoldT2-HoldT1)*60/rate1)*1000UL;
	  }
	  if(HoldTime2) {
	    dwOvenHoldTime2 = dwOvenRamp1Time + method_table.OvenHoldTime2*1000UL;
          }
	  if(rate2) {
	    if(HoldT3>HoldT2) {
	      dwOvenRamp2Time = dwOvenHoldTime2 + ((HoldT3-HoldT2)*60/rate2)*1000UL;
	    }
	    else if(HoldT3<HoldT2) {
	    dwOvenRamp2Time = dwOvenHoldTime2 + ((HoldT2-HoldT3)*60/rate2)*1000UL;
	    }
	  }
	  if(HoldTime3) {
	    dwOvenHoldTime3 = dwOvenRamp2Time + method_table.OvenHoldTime3*1000UL;
          }
#endif // ramping
	}
#endif // gc321
#endif // !stand_alone. P.P. 07/15/03
        }
            break;

        case RECEIVING_DATA:
        {
            MethodTable &method_table=pMethod->method_table;  // restored. P.P. 11/18/02
            GraphWindow *pGraphWindow; // added P.P. 11/02/12
#if !STAND_ALONE() // P.P. 07/15/03
            // W.K. go to previous case: SamplePump((acquire_type != CAL) && method_table.cont_pump_on_off);
            // Equil OFF moved here from previous case. P.P. 12/21/02
#if !CONCENTRATOR() // cond. moved here. P.P. 03/14/06
            if(dwEquilibrateTime&&(dwEquilibrateTime < GetTickCount())) {
                dwEquilibrateTime = 0;   // moved here. 11/01/09
//	if(!safety_warning) pMainWindow->sl->Replace("Inject"); // added. P.P. 11/05/09  // rem - messages are in common location. P.P. 07/31/10
//#if !GC501() // cond added. P.P. 08/26/04
#if !OLIN() // cond changed - not for any 501. P.P. 10/23/04
//#if !CONCENTRATOR() // cond. added. P.P. 03/04/06
// #if !MANUAL_INJ() // cond added. P.P. 05/19/06    // rem P.P. 12/12/18
                if(!(pMethod->method_table.syringe_on_off)) { // cond added. P.P. 04/28/06
#if !WATER_PUMP()  // P.P. 01/21/17
                    Equilibrate(FALSE); // rem temp. P.P. 07/29/04. and restored 08/26/04
#endif
                }
//#endif // !manual inj   // rem P.P. 12/12/18
//#endif // !concentrator
#endif // !gc501 // !olin
//	dwEquilibrateTime = 0;  // moved up.
            }
#endif // !concentrator moved here. P.P. 03/14/06
//      temp_conc = Get_Temp_Concentrator();  // added and
            if(dwInjectTime && dwInjectTime<GetTickCount()) {
//	if(!safety_warning) pMainWindow->sl->Replace(""); // rem - messages all in one place now. P.P. 11/01/09
//#if !GC321() // cond rem P.P. 04/01/08
//#if !MANUAL_INJ() // cond added. P.P. 05/19/06   // rem P.P. 12/12/18
                if(!(pMethod->method_table.syringe_on_off)) { // cond added. P.P. 04/28/06
                    Inject(FALSE);
//          Purge(FALSE); // added. P.P. 10/17/17 and rem 11/03/17
#if WATER_PUMP() // P.P. 01/21/17
                    Equilibrate(FALSE); // valve 4 OFF (flush 1st column, A) P.P. 01/21/17
//	  dwInjectTime2=GetTickCount()+60*1000UL;               // P.P. 01/29/17 and  rem as Jack wants
//	  dwInjectTime2_off=GetTickCount()+90*1000UL;               // P.P. 01/29/17
#endif
                }
//	if(!safety_warning) pMainWindow->sl->Replace(""); // added. P.P. 11/05/09 // rem - messages are in common location. P.P. 07/31/10
//#endif // !manual inj  // rem P.P. 12/12/18
#if CONCENTRATOR() // added. P.P. 02/05/06
                // removed but might be needed! P.P. 02/20/06
	Set_valve(point_port_offset, injectV4_bit_number+8, FALSE);  // Jack's variant 1. P.P. 02/20/06 restored OFF 03/22/17
	inj_V4_on = FALSE; // added here. P.P. 03/22/17
	Concentrator(TRUE); // Enable setting temperature of desorber to purge after inject finished. P.P. 01/02/10
	if(dwPurgeTime) { // cond added. P.P. 03/14/10
	  Purge(TRUE); // added to start purging when inject finished. P.P. 02/04/10
	}
#endif // concentrator
#if !CONCENTRATOR() // cond. added. P.P. 02/18/06
#if !MANUAL_INJ() // cond added. P.P. 05/19/06
                if(!(pMethod->method_table.syringe_on_off)) { // cond added. P.P. 04/28/06
#if !WATER_PUMP() // P.P. 01/29/17 leave it on
                    SamplePump((acquire_type != CAL) && method_table.cont_pump_on_off); // moved from above to end Inject. P.P. 07/10/05
#endif
                }
#endif // !manual inj
#endif // !concentrator
                dwInjectTime=0;
                pMainWindow->sl->Replace(" ");  // added here P.P. 03/13/18

                pMainWindow->print("End waiting inject time\r\n");  // enabled P.P. 03/12/17
            }
#if WATER_PUMP() // added for 2nd injection. P.P. 01/29/17
            if(dwInjectTime2&&(dwInjectTime2<GetTickCount())) {
	Inject(TRUE);
	dwInjectTime2 = 0;
      }
      if(dwInjectTime2_off&&(dwInjectTime2_off<GetTickCount())) {
	Inject(FALSE);
	dwInjectTime2_off = 0;
      }
#endif
#if CONCENTRATOR()
            void SetDsbTemperature(double t); // P.P. 07/30/18
      if(dwInjectTime) {  // during injet time
//	if(dwEquilibrateTime&&(GetTickCount()<dwEquilibrateTime)) {
	if(dwDesorbTime&&(GetTickCount()<dwDesorbTime)) {      // maintain desorber temp during desorb time. P.P. 03/15/17
//	  if(pMethod->method_table.inj_det_temp) {
	  if(pMethod->method_table.desorb_temp) {   // P.P. 07/27/18
//	    SetDetTemperature(method_table.inj_det_temp); // P.P. 01/02/10
	    SetDsbTemperature(method_table.desorb_temp); // P.P. 07/27/18
//	    SetDetTemperature(method_table.inj_det_temp+25.0); // P.P. 11/01/17  25 degrees higher restored back, no higher temp. P.P. 11/16/17
	  }
	  else { // in case of desorb_temp set to 0 in Method. P.P. 09/13/02
//            Concentrator(FALSE); // for any case - turn it OFF.
//	    SetDetTemperature(0); // added. P.P. 01/02/10
	    SetDsbTemperature(0); // P.P. 07/27/18
	    Concentrator(FALSE); // for any case - turn it OFF. moved down P.P. 12/08/17
	  }
	}
	else {
//	  dwEquilibrateTime = 0;
	  dwDesorbTime = 0;          // desorb time is used, not equilibrate time P.P. 03/15/17
//	  Concentrator(FALSE);
//	  SetDetTemperature(0); // added. P.P. 01/02/10
	  SetDsbTemperature(0); // P.P. 07/27/18
	  Concentrator(FALSE); // for any case - turn it OFF. moved down P.P. 11/08/17
	  dwCleanPowerOffTime = 0;
	  dwCleanPowerOnTime = 0; // P.P. 10/26/17
	}
      }
      else {                // P.P. 09/18/02. after Inject time
	if(dwPurgeTime) {  // during Purge Time
	  if(dwPurgeTime < GetTickCount()) {  // changed. P.P. 02/20/06. Purge time finished, all off
//	    Write_to_DAC(1, 0);  // power off when purge starts. P.P. 10/31/17
//	    SetDetTemperature(0); // added. P.P. 01/02/10 replaced for direct OFF. P.P. 10//31/17 restored 11/01/17
	    SetDsbTemperature(0); // P.P. 07/27/18
	    Concentrator(FALSE);  // moved from above. P.P. 12/27/17
	    Fan(TRUE); // added to cool down the concentrator. P.P. 02/23/19
	    dwPurgeTime = 0;
	    Purge(FALSE); // P.P. 03/20/17
	  }
          else { // between inject finished and Purge time
	    if(pMethod->method_table.inj_det_temp) {
	      SetDsbTemperature(method_table.desorb_temp); // P.P. 07/27/18

	    }
	    else { // in case of desorb_temp set to 0 in Method. P.P. 09/13/02
//	      SetDetTemperature(0); // added. P.P. 01/02/10  restored 1/02/17
	      SetDsbTemperature(0); // P.P. 07/27/18
	      Concentrator(FALSE); // for any case - turn it OFF.
//	      Write_to_DAC(1, 0);  // power off in place of set to 0. P.P. 10/31/17  rem 11/01/17
	      Fan(TRUE); // added to cool down the concentrator. P.P. 02/23/19
	      dwPurgeTime = 0; // added. P.P. 02/20/06
	      dwCleanPowerOnTime = 0;
	      dwCleanPowerOffTime = 0; // P.P. 10/26/17
	    }
//	    sprintf(time_str, "Desorb: %d", (dwDesorbTime - GetTickCount())/1000); // added to show remaining time. P.P. 03/13/18
//	    if(!safety_warning) pMainWindow->sl->Replace(time_str);
          }
        }
      }
#endif // concentrator

// code added for messages during ramping. P.P. 06/24/06
#if RAMPING()
            //      safety_warning = 0; // added to test. P.P. 11/17/11
// replacement for the code /**/ below. P.P. 11/17/11
      if(!dwInjectTime) {  // order changed. P.P. 07/16/06
	if(pMethod->method_table.RampingEnabled) {
//	if(!dwInjectTime) { // moved up. P.P. 07/16/06
	  if(dwOvenHoldTime1&&(GetTickCount()<dwOvenHoldTime1)) {
	    pMainWindow->sl->Replace("Initial Hold");
	  }
	  else if((dwOvenHoldTime1)&&(GetTickCount()>=dwOvenHoldTime1)) {
	    dwOvenHoldTime1=0;
	    if(dwOvenRamp1Time) {   // cond added. P.P. 11/17/11
	      pMainWindow->sl->Replace("Ramp 1");
	      MessageBeep(MB_ICONHAND);   // start ramping.  P.P. 06/24/06
	    }
	    else {
	      pMainWindow->sl->Replace(" ");
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenRamp1Time&&(GetTickCount()<dwOvenRamp1Time)) {
	    pMainWindow->sl->Replace("Ramp 1");
	  }
// end
	  else if(dwOvenRamp1Time&&(GetTickCount()>=dwOvenRamp1Time)) {
	    dwOvenRamp1Time=0;
	    if(dwOvenHoldTime2) {  // cond added. P,P, 11/17/11
	      pMainWindow->sl->Replace("Hold Temp 2");
	      MessageBeep(MB_ICONHAND);   // end ramping.  P.P. 06/24/06
	    }
	    else {
	      pMainWindow->sl->Replace(" "); // no more ramping after that time
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenHoldTime2&&(GetTickCount()<dwOvenHoldTime2)) {
	    pMainWindow->sl->Replace("Hold Temp 2");
	  }
// end
	  else if(dwOvenHoldTime2&&(GetTickCount()>=dwOvenHoldTime2)) {
	    dwOvenHoldTime2=0;
	    if(dwOvenRamp2Time) {
	      pMainWindow->sl->Replace("Ramp 2");
	      MessageBeep(MB_ICONHAND);   // start ramping.  P.P. 06/24/06
            }
	    else {
	      pMainWindow->sl->Replace(" ");
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenRamp2Time&&(GetTickCount()<dwOvenRamp2Time)) {
	    pMainWindow->sl->Replace("Ramp 2");
	  }
// end
	  else if(dwOvenRamp2Time&&(GetTickCount()>dwOvenRamp2Time)) {
	    dwOvenRamp2Time=0;
	    if(dwOvenHoldTime3) {
	      pMainWindow->sl->Replace("Hold Temp 3");
	      MessageBeep(MB_ICONHAND);   // end ramping.  P.P. 06/24/06
            }
	    else {
	      pMainWindow->sl->Replace(" "); // no more ramping after that time
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenHoldTime3&&(GetTickCount()<dwOvenHoldTime3)){
	    pMainWindow->sl->Replace("Hold Temp 3");
	  }
// end
	  else if(dwOvenHoldTime3&&(GetTickCount()>=dwOvenHoldTime3)){
	    dwOvenHoldTime3 = 0;
	    pMainWindow->sl->Replace(" ");
	  }
	}
        else { // added. P.P. 06/25/06
	  if(!safety_warning) pMainWindow->sl->Replace("Isothermal");
        }
      }
#endif // ramping
// end of added code for messages during rumping. P.P. 06/24/06

#if WATER_PUMP() // code added for 2nd injection (into 2nd column). P.P. 01/07/17
            if(!dwInjectTime) {  // after 1st injection has finished. P.P. 07/16/06
	if((dwXvalveTime) && (dwXvalveTime<GetTickCount())) {                                // P.P. 06/18/06
	  Equilibrate(FALSE); // valve 4 is already OFF, just confirm (flush 1st column, A) P.P. 01/21/17
#if PMD_12LS()
	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 1); // bit 7, 2nd column ON. P.P. 01/07/17
//	  Inject(TRUE);    // rem - will be fired separately from x1_time. P.P. 01/29/17
#elif CBWUSB_IO()
	  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 1); // bit 7, 2nd column ON. P.P. 01/07/17
#endif
	  dwXvalveTime = 0;
	}
	if((dwXvalveOffTime) && (dwXvalveOffTime<GetTickCount())) {                                // P.P. 06/18/06
//          Inject(FALSE); // rem - will be off separately from x1_time. P.P. 01/29/17
//	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, 2nd column off. P.P. 01/07/17
#if PMD_12LS()
	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, 2nd column OFF. P.P. 01/07/17
//	  Inject(TRUE);    // rem - will be fired separately from x1_time. P.P. 01/29/17
#elif CBWUSB_IO()
	  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 0); // bit 7, 2nd column OFF. P.P. 03/11/18
#endif
	  dwXvalveOffTime = 0;
	  SamplePump(FALSE);           //
	  WaterPump(pointnumber, TRUE);  //
	  dwWaterPumpTime = GetTickCount() +10*1000UL; // 10 s for pumping out water. P.P. 01/09/17
        }
      }

#elif TWO_COLUMNS()
            if(!dwInjectTime) {  // after 1st injection has finished. P.P. 07/16/06
	if((dwXvalveTime) && (dwXvalveTime<GetTickCount())) {                                // P.P. 06/18/06
	  Equilibrate(FALSE); // valve 4 is already OFF, just confirm (flush 1st column, A) P.P. 01/21/17
//	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 1); // bit 7, 2nd column ON. P.P. 01/07/17
#if PMD_12LS()
	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 1); // bit 7, 2nd column ON. P.P. 01/07/17
#elif CBWUSB_IO()
	  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 1); // bit 7, 2nd column ON. P.P. 01/07/17
#endif
	  dwXvalveTime = 0;
	}
	if((dwXvalveOffTime) && (dwXvalveOffTime<GetTickCount())) {                                // P.P. 06/18/06
//          Inject(FALSE); // rem - will be off separately from x1_time. P.P. 01/29/17
//	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, 2nd column off. P.P. 01/07/17
#if PMD_12LS()
	  My_cbDBitOut(CBWDEVICE_12LS, FIRSTPORTA, 7, 0); // bit 7, 2nd column OFF. P.P. 01/07/17
//	  Inject(TRUE);    // rem - will be fired separately from x1_time. P.P. 01/29/17
#elif CBWUSB_IO()
	  My_cbDBitOut(USB_IO, FIRSTPORTA, 7, 1); // bit 7, 2nd column OFF. P.P. 01/07/17
#endif
	  dwXvalveOffTime = 0;
	  SamplePump(FALSE);           //
	  WaterPump(pointnumber, TRUE);  //
	  dwWaterPumpTime = GetTickCount() +10*1000UL; // 10 s for pumping out water. P.P. 01/09/17
        }
      }
#endif // water_pump or 2 columns

// added to sample ahead. P.P. 01/08/03, 04/25/04
//#if POINTS()>1 // cond. added. P.P. 07/05/04 and rem P.P. 11/07/17, sampling ahead with just one point is allowed.
#if SAMPLE_AHEAD() // cond above replaced for thi one. P.P. 11/07/17
#if !QUICK_SCAN() // cond added. P.P. 01/15/11
//      if(dwNextPointTime&&(dwNextPointTime < GetTickCount())) {
            if((dwNextPointTime>1UL)&&(dwNextPointTime < GetTickCount())) { // replacement. P.P. 11/14/04
//        if(acquire_type!=CAL) {
                if((acquire_type!=CAL)&&(acquire_type!=CHECKCAL)) { // changed. P.P. 07/25/04
                    ulong points=pMethod->method_table.points;
                    NextPointNumber = get_available_point(points, pointnumber+1);
//          pointnumber = NextPointNumber;  // should be changed - or it will be shown in window. P.P. 01/08/03
                    // leave pointnumber as current point for alarms, concen, TWA etc. P.P. 01/10/03
                    PointNumber(NextPointNumber);
                    //          pMainWindow->sl->Replace("Sampling Next Point"); // replaced. P.P. 04/05/03
//          char NextPoint[40]; // string to store messsage; P.P. 04/05/03
                    char NextPoint[40] = {0}; // string to store messsage; P.P. 04/25/03
                    sprintf(NextPoint, "Sampling Next Point # %d",NextPointNumber + 1);
                    if(!safety_warning) pMainWindow->sl->Replace(NextPoint);
                }
                SamplePump(TRUE);
//        dwNextPointTime = 0;
// code for quick scan. P.P. 01/15/11
// end. 01/15/11
                dwNextPointTime = 1UL; // to keep message in place. P.P. 11/14/04
            }
#if WATER_PUMP()  // P.P. 01/04/17
            if((dwWaterPumpTime != 0) && (dwWaterPumpTime < GetTickCount())) {
	WaterPump(pointnumber, FALSE);
	SamplePump(TRUE); // added. P.P. 01/29/17
	dwWaterPumpTime = 0;
      }
#endif
#else // quick scan variant
            if((dwNextPointTime>1UL)&&(dwNextPointTime < GetTickCount())) { // replacement. P.P. 11/14/04
	if((acquire_type!=CAL)&&(acquire_type!=CHECKCAL)) { // changed. P.P. 07/25/04
          if(pMethod->method_table.quick_scan_enabled) {
            if(!quick_scan_active) { // regular run
	      dwNextPointTime = GetTickCount() + (pMethod->method_table.quick_scan_time)*1000UL;
	      if(dwNextPointTime + 1000UL < dwAnalysisTime) { // enough time for quick scan. P.P. 01/15/11
		ulong points=pMethod->method_table.points;
//                NextPointNumber = get_available_point(points, pointnumber+1);
		NextPointNumber = get_available_point(points, NextPointNumber+1);  // P.P. 01/15/11
	  // leave pointnumber as current point for alarms, concen, TWA etc. P.P. 01/10/03
                PointNumber(NextPointNumber);
	        char NextPoint[40] = {0}; // string to store messsage; P.P. 04/25/03
                sprintf(NextPoint, "Sampling Next Point # %d",NextPointNumber + 1);
	        if(!safety_warning) pMainWindow->sl->Replace(NextPoint);
	      }
	      else {
		dwNextPointTime = 1UL; // keep message. P.P. 01/15/11
	      }
	    }
	    else {
            }
	  }
	  else { // regular variant, only one next point
	    ulong points=pMethod->method_table.points;
            NextPointNumber  = get_available_point(points, pointnumber+1);
	  // leave pointnumber as current point for alarms, concen, TWA etc. P.P. 01/10/03
            PointNumber(NextPointNumber);
	    char NextPoint[40] = {0}; // string to store messsage; P.P. 04/25/03
            sprintf(NextPoint, "Sampling Next Point # %d",NextPointNumber + 1);
	    if(!safety_warning) pMainWindow->sl->Replace(NextPoint);
	    dwNextPointTime = 1UL; // to keep message in place. P.P. 01/15/11
	  }
	}
	else {
	  dwNextPointTime = 0UL; // no message in cal. P.P. 11/14/04
	}
	SamplePump(TRUE);
//	dwNextPointTime = 1UL; // to keep message in place. P.P. 11/14/04  rem 01/15/11
      }
#endif //!quick_scan. P.P. 01/15/11
#endif // points>1. P.P. 07/05/04  now samplle_ahead condition. P.P. 11/07/17
// end of added for sampling ahead. P.P. 01/08/03

            if(dwUV_Max_Time && (dwUV_Max_Time<=GetTickCount())) { // added. P.P. 09/26/03
                dwUV_Max_Time=0;                                     //
            }                                                      //
// end of the added code.
#if X1_VALVE() // changed - standard case. P.P. 07/10/05
            if(dwXvalveTime&&(dwXvalveTime<GetTickCount())) {
#if OLIN() // cond added, specific for OLIN 501 only. P.P. 10/23/04
                Equilibrate(FALSE);
#endif // olin
        SetLamps(TRUE);

//#if GC321() // case added. P.P. 07/01/10  11/12/10
#if GC321()||GC326() //  P.P. 12/07/18
                X1_valve(TRUE);
#endif // gc321
                dwXvalveTime=0;
            }
            if(dwXvalveOffTime&&(dwXvalveOffTime<GetTickCount())) {
//#if GC321() // case added. P.P. 07/01/10 11/12/10
#if GC321()||GC326() //  P.P. 12/07/18
                X1_valve(FALSE);
#endif // gc321
                dwXvalveOffTime=0;
            }
// 3 more valves added. P.P. 06/20/10  here 11/12/10
#if !GC321() // x2 and x3 are in sampling time) P.P. 07/01/10 cond rem - in case time was set longer. 07/02/10
//#if !(GC321()||GC326()) //  P.P. 12/07/18  cond restored - in GC326, x2 and x3 ork same way as in regular GCs. P.P. 12/09/1
            if(dwXvalve2Time&&(dwXvalve2Time<GetTickCount())) {
                X2_valve(TRUE);
                dwXvalve2Time=0;
//        AutoZero(TRUE);  // added. In !GC322, x2 and x3 are used for aditional range switching rem 05/12/17
            }
            if(dwXvalve2OffTime&&(dwXvalve2OffTime<GetTickCount())) {
                X2_valve(FALSE);
                dwXvalve2OffTime=0;
            }
#if !FID()  // with FID in GC312, bit 4 (oven_door_bit) is used, so no X3 valve is available. P.P. 08/30/17
            if(dwXvalve3Time&&(dwXvalve3Time<GetTickCount())) {
//	X3_valve(TRUE);  // rem P.P. 10/16/17
                dwXvalve3Time=0;
//	AutoZero(TRUE);  // added. In !GC322, x2 and x3 are used for aditional range switching  rem 05/12/17 retored, additional autozeroing. P.P. 10/16/17  rem 10/17/17
            }
            if(dwXvalve3OffTime&&(dwXvalve3OffTime<GetTickCount())) {
//	X3_valve(FALSE);  /// rem. P.P. 10/16/17
                dwXvalve3OffTime=0;
//	AutoZero(FALSE);  // added.,additional autozeroing. P.P. 10/16/17 rem 10/17/17  rem 05/13/18

            }
#endif
#else
            if(dwXvalve2Time&&(dwXvalve2Time<GetTickCount())) {
	X2_valve(TRUE);
	dwXvalve2Time=0;
      }
      if(dwXvalve2OffTime&&(dwXvalve2OffTime<GetTickCount())) {
	X2_valve(FALSE);
	dwXvalve2OffTime=0;
      }
      if(dwXvalve3Time&&(dwXvalve3Time<GetTickCount())) {
	X3_valve(TRUE);
	dwXvalve3Time=0;
      }
      if(dwXvalve3OffTime&&(dwXvalve3OffTime<GetTickCount())) {
	X3_valve(FALSE);
	dwXvalve3OffTime=0;
      }
#endif // !gc321 //
            if(dwXvalve4Time&&(dwXvalve4Time<GetTickCount())) {
//        X4_valve(TRUE);  // rem P.P. 10/17/17 because this DO in port B is used for jnject_V4
                dwXvalve4Time=0;
            }
            if(dwXvalve4OffTime&&(dwXvalve4OffTime<GetTickCount())) {
//	X4_valve(FALSE);   // rem P.P. 10/17/17 because this DO in port B is used for jnject_V4
                dwXvalve4OffTime=0;
            }
            if(dwXvalve5Time&&(dwXvalve5Time<GetTickCount())) {
#if FLOW_RATE()
                X5_valve(TRUE);   // P.P. 05/13/18 turn ON the flow rate relay
#endif
//        X5_valve(TRUE);   // rem P.P. 10/17/17 because this DO in port B is used for desorber
//	AutoZero(TRUE);  // P.P. 11/08/12 AZ before increasing the gain  rem 05/12/17
                dwXvalve5Time=0;
            }
            if(dwXvalve5OffTime&&(dwXvalve5OffTime<GetTickCount())) {
#if FLOW_RATE()
                X5_valve(FALSE);  // P.P. 05/13/18 turn OFF the flow rate relay
#endif
//	X5_valve(FALSE);  // rem P.P. 10/17/17 because this DO in port B is used for desorber
                dwXvalve5OffTime=0;
            }
// end of code for 3 more valves. P.P. 06/20/10

            if(dwAutoZero1&&(dwAutoZero1<GetTickCount())){ // added. P.P. 05/12/17
                AutoZero(TRUE);
                dwAutoZero1 = 0;
            }
            if(dwAutoZero1_off&&(dwAutoZero1_off<GetTickCount())){ // added. P.P. 07/30/04
                AutoZero(FALSE);
                dwAutoZero1_off = 0;
            }
            if(dwAutoZero2&&(dwAutoZero2<GetTickCount())){ // added. P.P. 05/12/17
                AutoZero(TRUE);
                dwAutoZero2 = 0;
            }
            if(dwAutoZero2_off&&(dwAutoZero2_off<GetTickCount())){ // added. P.P. 07/30/04
                AutoZero(FALSE);
                dwAutoZero2_off = 0;
            }
            if(dwAutoZero3&&(dwAutoZero3<GetTickCount())){ // added. P.P. 06/20/17
                AutoZeroB(TRUE);
                dwAutoZero3 = 0;
            }

            if(dwAutoZero3_off&&(dwAutoZero3_off<GetTickCount())){ // added. P.P. 06/21/17
                AutoZeroB(FALSE);
                dwAutoZero3_off = 0;
            }
            if(dwAutoZero4&&(dwAutoZero4<GetTickCount())){ // added. P.P. 06/20/17
                AutoZeroB(TRUE);
                dwAutoZero4 = 0;
            }

            if(dwAutoZero4_off&&(dwAutoZero4_off<GetTickCount())){ // added. P.P. 06/20/17
                AutoZeroB(FALSE);
                dwAutoZero4_off = 0;
            }
// switching the range added. P.P. 05/12/17
            if(dwRange_1&&(dwRange_1<GetTickCount())){ // added. P.P. 05/12/17
                cur_gain[0] = gain_1[0];
//	cur_gain[1] = gain_1[1]; // rem to separate ranges switching in det A and det B. P.P. 06/20/17
                dwRange_1 = 0;
            }
            if(dwRange_1_off&&(dwRange_1_off<GetTickCount())){ // added. P.P. 05/12/17
                cur_gain[0] = gain[0];
//	cur_gain[1] = gain[1]; // rem P.P. 06/20/17
                dwRange_1_off = 0;
            }
            if(dwRange_1_2&&(dwRange_1_2<GetTickCount())){ // added. P.P. 05/12/17
//	cur_gain[0] = gain_1[0];
                cur_gain[1] = gain_1[1]; // rem to separate ranges switching in det A and det B. P.P. 06/20/17
                dwRange_1_2 = 0;
            }
            if(dwRange_1_2_off&&(dwRange_1_2_off<GetTickCount())){ // added. P.P. 05/12/17
//	cur_gain[0] = gain[0];
                cur_gain[1] = gain[1]; // rem P.P. 06/20/17
                dwRange_1_2_off = 0;
            }
// end of code for switching range. P.P. 05/12/17
#endif // x1-valve
// new code added. P.P. 07/14/06
//#elif STAND_ALONE()&&GC321() // case added for such combo, to show messages in stand_alone. P.P. 07/14/06
#elif STAND_ALONE()&&(GC321()||GC326()) //  P.P. 12/07/18
            if(dwInjectTime && dwInjectTime<GetTickCount()) {
	if(!safety_warning) pMainWindow->sl->Replace("");
	dwInjectTime = 0;
      }
#if RAMPING()
      if(!dwInjectTime) {  // order changed. P.P. 07/16/06
        if(pMethod->method_table.RampingEnabled) {
//	if(!dwInjectTime) { // moved up. P.P. 07/16/06
	  if(dwOvenHoldTime1&&(GetTickCount()<dwOvenHoldTime1)) {
	    pMainWindow->sl->Replace("Initial Hold");
	  }
	  else if((dwOvenHoldTime1)&&(GetTickCount()>=dwOvenHoldTime1)) {
	    dwOvenHoldTime1=0;
	    if(dwOvenRamp1Time) {   // cond added. P.P. 11/17/11
	      pMainWindow->sl->Replace("Ramp 1");
	      MessageBeep(MB_ICONHAND);   // start ramping.  P.P. 06/24/06
	    }
	    else {
	      pMainWindow->sl->Replace(" ");
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenRamp1Time&&(GetTickCount()<dwOvenRamp1Time)) {
	    pMainWindow->sl->Replace("Ramp 1");
	  }
// end
	  else if(dwOvenRamp1Time&&(GetTickCount()>=dwOvenRamp1Time)) {
	    dwOvenRamp1Time=0;
	    if(dwOvenHoldTime2) {  // cond added. P,P, 11/17/11
	      pMainWindow->sl->Replace("Hold Temp 2");
	      MessageBeep(MB_ICONHAND);   // end ramping.  P.P. 06/24/06
	    }
	    else {
	      pMainWindow->sl->Replace(" "); // no more ramping after that time
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenHoldTime2&&(GetTickCount()<dwOvenHoldTime2)) {
	    pMainWindow->sl->Replace("Hold Temp 2");
	  }
// end
	  else if(dwOvenHoldTime2&&(GetTickCount()>=dwOvenHoldTime2)) {
	    dwOvenHoldTime2=0;
	    if(dwOvenRamp2Time) {
	      pMainWindow->sl->Replace("Ramp 2");
	      MessageBeep(MB_ICONHAND);   // start ramping.  P.P. 06/24/06
            }
	    else {
	      pMainWindow->sl->Replace(" ");
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenRamp2Time&&(GetTickCount()<dwOvenRamp2Time)) {
	    pMainWindow->sl->Replace("Ramp 2");
	  }
// end
	  else if(dwOvenRamp2Time&&(GetTickCount()>dwOvenRamp2Time)) {
	    dwOvenRamp2Time=0;
	    if(dwOvenHoldTime3) {
	      pMainWindow->sl->Replace("Hold Temp 3");
	      MessageBeep(MB_ICONHAND);   // end ramping.  P.P. 06/24/06
            }
	    else {
	      pMainWindow->sl->Replace(" "); // no more ramping after that time
	    }
	  }
// added. P.P. 11/17/11
	  else if(dwOvenHoldTime3&&(GetTickCount()<dwOvenHoldTime3)){
	    pMainWindow->sl->Replace("Hold Temp 3");
	  }
// end
	  else if(dwOvenHoldTime3&&(GetTickCount()>=dwOvenHoldTime3)){
	    dwOvenHoldTime3 = 0;
	    pMainWindow->sl->Replace(" ");
	  }
	}
        else { // added. P.P. 06/25/06
	  if(!safety_warning) pMainWindow->sl->Replace("Isothermal");
        }
      }
#else // without ramping. P.P. 07/14/06
      if(!dwInjectTime) {
//	pMainWindow->sl->Replace("Isothermal");
//	if(!safety_warning) pMainWindow->sl->Replace(" "); // P.P. 07/22/06 removed, make this once as above already done. P.P. 10/18/09
      }
#endif // ramping
// end of new code for messaging in stand-alone+gc321/322. P.P. 07/14/06
#endif // !stand_alone. P.P. 07/15/03
            //pMainWindow->sl->SetStatus("Active");
            extern char data_rate;           // rate data received in pts/sec.
            DWORD dwCurTime=timeGetTime();//GetTickCount();
            if(dwCurTime>=dwLastTime+1000/data_rate) {
                if(!dwLastTime) dwLastTime=dwCurTime;
                else dwLastTime+=1000/data_rate;
                for(int n=0; n!=NUMDETECTORS; n++) {
                    if(pMethod->det[n].detect_stat) {
//            int32 data_in; // 32 bit - signed
                        int16 data_in; // 16 bit - signed. To transfer 16 bit number together with gain bits in a 32-bit number. P.P. 11/09/14
// returned back for 2 det case. P.P. 10/26/02
                        for(int gain_retry=0; gain_retry<3; gain_retry++) { // restrict max retry number
                            if(!(pMethod->det[n].auto_gain_on_off)) {   // added. P.P. 03/02/02
//              if(!(pMethod->det[n].auto_gain_on_off) || n) {   // and for det. B. P.P. 03/09/02.
// P.P. 05/06/02. To avoid switching gain between A and B, make gain B = gain A.
                                gain[n] = pMethod->det[n].gain;           // in case auto switched OFF during the run.
                                gain_1[n] = pMethod->det[n].gain_1;  // added as alternative. P.P. 11/09/12 restored. 01/04/13
#if HIGH_CONC_POINTS() // added. P.P. 03/30/11
                                if(acquire_type==JOB) { //cond added to not switch gain if in CAL mode. P.P. 06/24/11
		  if(pointnumber<4) { // first 4 points
		     if(gain[n]>=3) cur_gain[n] =3; //    // stay on Range 1000. P.P. 01/04/13
		     else if(gain[n]>=2) cur_gain[n] = gain[n]+1; // switch from 100 to 1000
		     else if(gain[n]>0) cur_gain[n] = gain[n]+2;  // switch from 1 to 100, from 10 to 1000.
		  }
		}
#endif
//                gain[n] = pMethod->det[0].gain;           // in case auto switched OFF during the run.
// added to change gain of DI700 during run. P.P. 10/12/03
// end of added code. P.P. 10/12/03
                            }
#if QUICK_SCAN() // cond added. P.P. 01/17/11. Change for using cur_gain, not done yet! P.P. 01/04/13
                            if(pMethod->method_table.quick_scan_enabled) {
                cur_gain[n] = gain[n];  // P.P. 01/04/13
		if(!quick_scan_active) {  // cond added. P.P. 01/01/11
		  data_in=Get_Data(n, cur_gain[n]);  // as it always was, det A or B
		}
	        else {  // in quick scan,
		  if(n) { // det B
#if GC312() // cond added. P.P. 01/19/11
		    data_in=Get_Data(n-1, cur_gain[n]);  // if det B active, Det A is for Quick scan
#else
		    data_in=Get_Data(n+2, cur_gain[n]);  // if det B active (ch 3), ch5 is for Quick scan  // P.P. 01/06/11 01/08/11
#endif
		  }
		  else { // det A
#if GC312()  // cond added. P.P. 01/19/11
		    data_in=Get_Data(n+1, cur_gain[n]);  // if det A active, Det B is for Quick scan
#else
		    data_in=Get_Data(n+2, cur_gain[n]);  // if det A active (ch 2), ch4 is for Quick scan  // P.P. 01/06/11
#endif
		  }
	        }
	        if(pMethod->det[n].invert) {    // from now, inversion is available. P.P. 11/06/09
		  data_in = -data_in;
		}
	      }
	      else {
                cur_gain[n] = gain[n]; // P.P. 01/04/13
		data_in=Get_Data(n, cur_gain[n]);  // as it always was, det A or B
		if(pMethod->det[n].invert) {    // from now, inversion is available. P.P. 11/06/09
		  data_in = -data_in;
		}
	      }
#else // without quick scan. Modified to use cur_gain. P.P. 01/04/13
#if X1_VALVE() // cond. added. P.P. 01/04/17
//#if !WATER_PUMP()  // P.P. 01/07/17
#if !(WATER_PUMP() || TWO_COLUMNS()) // P.P. 03/24/17
#endif // water_pump
#else
                            cur_gain[n] = gain[n];
#endif // x1 valve
                            data_in=Get_Data(n, cur_gain[n]);  // as it always was, det A or B

                            if(pMethod->det[n].invert) {    // from now, inversion is available. P.P. 11/06/09
                                data_in = -data_in;
                            }
#endif // quick scan
                            if(pMethod->det[n].auto_gain_on_off) {
//              if((pMethod->det[n].auto_gain_on_off) && (!n)) { // only for det A. P.P. 03/09/02
#if !Get_Data_Bipolar()
                                if(data_in < 4000U) { // low data // restored. P.P. 05/04/07
//		if(data_control_out < 4000U) { // low data // P.P. 04/06/07
#else
                                if(labs(data_in) < 2000U) { // low data  // P.P. 05/11/02. Max is 32767
//		if(labs(data_control_out) < 2000U) { // P.P. 04/06/07.
#endif
                                    if(gain[n] > 0) { // and gain less than maximum
                                        gain[n]--;  // increase the gain by 10
                                        cur_gain[n] = gain[n]; // added. P.P. 01/04/13
// added to work with auto_gain. P.P. 10/12/03
// end of added code. P.P. 10/12/03

                                        continue;   // measure again, with new gain
                                    }
                                }
#if !Get_Data_Bipolar()
                                    else if(data_in > 50000U) { // high data
//		else if(data_control_out > 50000U) { // high data
#else
                                else if(labs(data_in) > 25000U) { // high data   // P.P. 05/11/02. Max is 32767
//		else if(labs(data_control_out) > 25000U) { // P.P. 04/06/07.
#endif
                                    if(gain[n] < 2) { // and gain more than minimum  // resored, only 1, 10, 100 ranges. P.P. 05/12/17
//		  if(gain[n] < 3) { // and gain more than minimum. P.P. 12/11/09. new range 1000
//		  if(gain[n] < 4) { // and gain more than minimum. P.P. 02/15/11. range 10000 added. here 04/01/11 rem 11/09/12
                                        gain[n]++;  // decrease gain by 10
                                        cur_gain[n] = gain[n]; // added. P.P. 01/04/13
                                        continue; // measure again, with new gain
                                    }
                                }
                            }
//              gain[n] = gain[0]; // P.P. 05/06/02. Added to avoid gain switching in case of Det B active.
                            // removed for 2-det case. P.P. 10/26/02
                            break; // if data in range or gain can not be changed - take data
                        }

                        if(pMethod->det[n].uv_type) { // Far UV detector.
//              #if 0 // old method // replaced, needed for stand_alone. P.P. 10/07/03
#if STAND_ALONE() // no control of lamps. P.P. 10/07/03
                            //              data_in = - data_in; // Polarity reversed 11/06/09: removed, now via checkbox
#else // new method
//              data_in = Get_Data(n, gain[n]); // removed - we've got data few lines above. P.P. 09/09/03
                            if(dwUV_Max_Time) { // added to find max signal at lamp ON. P.P. 09/26/03
                                if(dwUV_0_Time) { // store uv_0 value;  // moved - hence FALSE here. P.P. 10/05/03
//                  uv_0[n]=data_in;    // replaced temp. 10/03/03,
//                  uv_0[n] = - 22282;  // corresponds to -6.8 V. Rem 10/05/03
//                  data_in=0;
                                }
                                else { // store max value. P.P. 09/26/03
//                  uv_Max[n]=data_in - uv_0[n]; // Max possible. P.P. 09/26/03
//                  uv_Max[n]=data_in*(int32)pow10(gain[n]-pRun->runrawdata.gain[n]) - uv_0[n]; // Max possible. P.P. 10/05/03
//                  uv_Max[n]=data_in*(int32)pow10(2-gain[n]) - uv_0[n]; // Max possible. P.P. 10/05/03
//		  uv_Max[n]=data_in*(int32)pow10(3-gain[n]) - uv_0[n]; // new range 1000. P.P. 12/11/09  restored. 06/20/11
                                    uv_Max[n]=data_in*(int32)pow10(3-cur_gain[n]) - uv_0[n]; // P.P. 01/04/13
//		  uv_Max[n]=data_in*(int32)pow10(4-gain[n]) - uv_0[n]; // new range 10000. P.P. 02/15/11. here 04/01/11
                                    data_in=0;
                                }
                            }

                            else {
//                data_in*=(uint32)pow10(gain[n]-pRun->runrawdata.gain[n]); // added. P.P. 10/05/03
//		data_in*=(uint32)pow10(2-gain[n]); // added. P.P. 10/05/03
                                data_in*=(uint32)pow10(3-gain[n]); // new range 1000. P.P. 12/11/09  06/20/11 // scaling left as it is even for ranges 1, 10, 100 only. P.P. 05/12/17
//		data_in*=(uint32)pow10(4-gain[n]); // new range 10000. P.P. 02/15/11. here 04/01/11
                                data_in=data_in-uv_0[n];
//              }  // moved down. P.P. 09/26/03
                                if(data_in<=0) data_in=0;//log(1); // to prevent log() error
//                else data_in = - 800000L*logl(data_in); // convert floating point to int with maximum digits
//                else data_in = - 7000L*logl(data_in); // decreased. P.P. 09/09/03
//                else data_in = - 23026L*logl(data_in); // changed. P.P. 09/28/03
                                else data_in = - 1000000L*logl(data_in); // changed. P.P. 11/22/09
//                if(uv_0[n] < 0) { // exclude log(neg. number). P.P. 09/02/03)
//                  data_in = data_in + 7000L*logl(-uv_0[n]); // decreased. P.P. 09/09/03
                                if(uv_Max[n] > 0) { // exclude log(neg. number). changed code. P.P. 09/26/03
//                  data_in = data_in + 7000L*logl(uv_Max[n]);    // P.P. 09/26/03
//                  data_in = data_in + 23026L*logl(uv_Max[n]);    // P.P. 09/28/03
                                    data_in = data_in + 1000000L*logl(uv_Max[n]);    // P.P. 11/22/09
                                }
                                else {
                                    data_in = 0;
                                }
                            } // moved from above. P.P. 09/26/03
#endif
                        }
                        {
//              cur_gain[n] = gain[n]; // will be used in Get_Temperature. P.P. 03/02/02  rem 01/04/13
//              uint32 data=data_in;
//              data*=(uint32)pow10(gain[n]-pRun->runrawdata.gain[n]);
//	      int32 data=data_in;
                            uint32 data=0;  // constructing data bits out of data_in and gain. P.P. 11/09/14
                            uint32 data_gain = 0;
#if !STAND_ALONE() // added. P.P. 10/07/03
                            if(!pMethod->det[n].uv_type) // Already done for FUV. cond added. P.P. 10/05/03
#endif // added. P.P. 10/07/03
//		if((GetTickCount()>dwAutoZero1)&&(GetTickCount()<dwXvalve5OffTime)) {   // P.P. 11/08/12
//		  data*=(uint32)pow10((gain[n]+1)-pRun->runrawdata.gain[n]);   // rem. P.P. 11/09/12
//		}
//                else {
//		  data*=(uint32)pow10(gain[n]-pRun->runrawdata.gain[n]); // rem. P.P. 12/26/12
//		  data=((uint32)data&0x8000FFFFUL); // otherwise, int flags is not recognized. P.P. 12/26/12
//		  data=((uint32)data&0x8000FFFFUL)+((uint32) gain[n]<<28); // otherwise, int flags is not recognized. P.P. 12/26/12
//    		  data=((uint32)data&0x8000FFFFUL)+((uint32) cur_gain[n]<<28); // write cur_gain. P.P. 01/04/13
//		  data=((uint32)data&0x80007FFFUL)+((uint32) cur_gain[n]<<28); // write cur_gain. P.P. 10/31/14
//		  data=((uint32)data&0x80007FFFUL)+((uint32) (cur_gain[n]&0x00000003UL)<<28); // write cur_gain. P.P. 11/06/14
//		  data=((data&0x80007FFFUL)|( ((int32) cur_gain[n])&0x00000003UL)<<28); // write cur_gain. P.P. 11/07/14
//	          data_gain =data|(0x0000FFFF&data_in);  // lower 16 bits contain data_in. P.P. 11/09/14
                                data_gain =data|(0x0000FFFF&((uint16) data_in));  // lower 16 bits are data_in with sign bit 15. P.P. 12/12/14
//		  data_gain =data_gain|((((uint32) cur_gain[n])&0x00000003UL));
                            uint32 gain_32 = ((uint32) cur_gain[n]<<28)&0x30000000UL; // bits28 and 29, the rest bits are 0. P.P. 11/13/14
                            data_gain =data_gain|gain_32;
                            data = data_gain; // uint32, 16 lowest bits =data_in as 16-bit unsigned; bits 28, 29 carry gain; all other = 0. P.P. 11/13/14
//	    }
                            int flags=0;
                            Integrator *p=pIntegrator[n];
                            if(p) {                                // detector may be off
#if !STAND_ALONE() // added. P.P. 07/15/03
#if !FID() // cond added. P.P. 10/02/05
                                if(!dwUV_0_Time) { // added. P.P. 09/26/03
#if !LAMP_DELAYED() // cond added. P.P. 10/07/09 added here 12/04/09
                                    CheckLamp(p->raw_data.time/data_rate); // turn on lamp if we are near a peak
#else // lamp delayed - check x1 on time. P.P. 10/07/09
                                    //		  if(dwXvalveTime&&(dwXvalveTime<GetTickCount())) {
//		    CheckLamp(p->raw_data.time/data_rate); // turn on lamp if we are near the 1st peak after x1time
//		  }
#endif // !lamp_delayed
                                }
#endif
#endif //!stand_alone
                                p->ProcessData(data);     // process new data normalized by gain. P.P. 02/12/02
                                flags=p->DetectPeaks();   // 2 lines rem to test. P.P. 12/26/12
                            }
#if !STAND_ALONE() // added. No alarms in stand_alone. P.P. 07/15/03
//#if !GC321()   // P.P. 03/29.02
#if !(GC321()||GC326()) //  P.P. 12/07/18
                            if((acquire_type==CAL) || !pMethod->method_table.alarms_on_off) {
                                if(!alarm_cal_off) {   // 10/12/08 12/12/08
                                    for(int n = 0; n<POINTS(); n++) { // Turn all alarms OFF. P.P. 10/13/01
#if !GC312() // cond added. P.P. 03/19/06
                                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,
#endif // !gc312
                                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,

#if COMMON_HIGH()  // added. P.P. 01/11/05
                                        current_state_alarm[n]=0; // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                                        High_Alarm(n, FALSE); // all alarms OFF
#endif // !gc312
#endif // common high
                                    }
#if COMMON_HIGH()  // added. P.P. 01/11/05
                                    #if !GC312() // cond added. P.P. 03/19/06
		  High_Alarm(common_high_bit, FALSE); // added. P.P. 01/11/05
#endif // !gc312
#endif // common high
                                    alarm_cal_off = 1; // 10/12/08  //12/12/08
                                } // 10/12/08                     //
                            }
                            else if(acquire_type==CHECKCAL) {  // case added. P.P. 07/25/04
                                // no action but a diagnostics alarm and/or forced calibrate might be added. P.P. 07/25/04
                            }
                            else { // if Alarm checked and not in calibration or check cal
                                flagsAlarm |= (flags&(2|4)); // High/Low alarm
                                HL_alarm = (flagsAlarm > 0);  // added. P.P. 11/18/02
                                const ulong &halarms=pMethod->method_table.halarms;
                                const ulong &lalarms=pMethod->method_table.lalarms;
//                if(!(halarms&pointnumber)) {        // W.K.: "this should be changed in all variants!" 11/27/02
                                if(!(halarms&(1L<<pointnumber))) {  //
#if COMMON_HIGH()  // added. P.P. 01/11/05
                                    current_state_alarm[pointnumber]=0; // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                                    if(high_alarm_state[pointnumber]) {   // 10/13/08
                                        High_Alarm(pointnumber, FALSE);  // temp rem P.P. 11/25/02
                                        high_alarm_state[pointnumber]=0;
                                    }
#endif // !gc312
#endif // common high
                                }
                                else {
                                    //                  Alarm(flagsAlarm? TRUE: FALSE); // moved to diagn alarms. P.P. 11/18/02
// P.P. 05/02/03. modified to hold alarm until it is changed: // added here P.P. 04/27/04
//                  High_Alarm(pointnumber,(flagsAlarm&2)? TRUE: FALSE); //High alarm go ON
                                    if(flagsAlarm&2) {
#if COMMON_HIGH()  // added. P.P. 01/11/05
                                        current_state_alarm[pointnumber]=1; // added. P.P. 01/11/05
#if !GC312() // cond added. P.P. 03/19/06
		    High_Alarm(common_high_bit, TRUE); // added. P.P. 01/14/05
#endif // !gc312
#else
#if !GC312() // cond added. P.P. 03/19/06
                                        if(!high_alarm_state[pointnumber]) {  // 10/13/08  12/12/08
                                            High_Alarm(pointnumber, TRUE);
                                            high_alarm_state[pointnumber]=1;
                                        }
#endif // !gc312
#endif // common high
//                    Low_Alarm(pointnumber, FALSE); // let it stay - may be another component. P.P.
                                    }
                                    else {
                                        if(high_alarm_state[pointnumber]) {  // 10/13/08
                                            High_Alarm(pointnumber, FALSE);
                                            high_alarm_state[pointnumber]=0;
                                        }
                                    }
                                }
#if !GC312() // cond added. P.P. 03/19/06
//                if(!(lalarms&pointnumber))          // W.K.: "this should be changed in all variants!" 11/27/02
                                if(!(lalarms&(1L<<pointnumber))) {  //
                                    if(low_alarm_state[pointnumber]) {   // 10/13/08
                                        Low_Alarm(pointnumber, FALSE);  // temp rem P.P. 11/25/02
                                        low_alarm_state[pointnumber]=0;
                                    }
                                }                                 // P.P. 11/27/02
                                else {
                                    //                  Alarm(flagsAlarm? TRUE: FALSE); // moved to diagn alarms. P.P. 11/18/02
// P.P. 05/02/03. modified to hold alarm until it is changed:
//                  Low_Alarm(pointnumber,(flagsAlarm&4)? TRUE: FALSE); //Low alarm go ON
                                    if(flagsAlarm&4) {
                                        if(!low_alarm_state[pointnumber]) {  // 10/13/08
                                            Low_Alarm(pointnumber, TRUE);
                                            low_alarm_state[pointnumber]=1;
                                        }
//                    High_Alarm(pointnumber, FALSE); // let it stay - may be another component. P.P.
                                    }
                                    else {
                                        if(low_alarm_state[pointnumber]) {  // 10/13/08
                                            Low_Alarm(pointnumber, FALSE);
                                            low_alarm_state[pointnumber]=0;
                                        }
                                    }
                                }
#endif // !gc312
                            }  // Alarms

#endif // !GC321() or 326
#endif // !stand_alone. P.P. 07/15/03
                            if(ndata==0)
                                rdata[0]='\0';

                            rdata[0]|=n?(0x80000000UL>>ndata):0;
                            ndata++;
                            rdata[ndata]=data;

                            if(ndata==32) {
                                WriteRawData(rdata, (ndata+1)*sizeof(rdata[0]));  // write raw data to the end of...
                                ndata=0;
                            }

//	      if(pCurRawDataLastPainted==0 || pCurRawData-pCurRawDataLastPainted>/*400*/333) {
                            if(pCurRawDataLastPainted==0 || pCurRawData-pCurRawDataLastPainted>/*400*/1) {
                                pMainWindow->Refresh(1);//??
                                pCurRawDataLastPainted=pCurRawData;
                            }
                        }
                    }
                }
            }
#if !STAND_ALONE() // added. P.P. 07/15/03
            const double oven_error_delta = 10; // added for diagnostics. P.P. 11/16/02
            const double oven_delta = 1; // same as on start. P.P. 11/25/02
// replacement from 101303: P.P. 04/30/04
            if(!pMethod->method_table.RampingEnabled) {    // without ramping. P.P. 02/23/03.
//	  if(meas_temperature < (method_table.oven_temp - oven_error_delta)){ // Diagnostics Oven low. P.P. 11/16/02
                if(temp_oven < (method_table.oven_temp - oven_error_delta)){ // Diagnostics Oven low. P.P. 07/03/05, 09/03/05
                    Oven_error = 1;   //
                    Oven_high_error = 0; // added. P.P. 02/23/03, 04/27/04
                }
//          else if(meas_temperature > (method_table.oven_temp - 2*oven_delta)){ // not low any more. P.P. 11/25/02
//          else if(meas_temperature > (method_table.oven_temp + oven_error_delta)){ // P.P. 02/23/03
                else if(temp_oven > (method_table.oven_temp + oven_error_delta)){ // P.P. 07/03/05, 09/03/05
                    Oven_error = 0;
                    Oven_high_error = 1; // added. P.P. 02/19/03
                }
//          else if(abs(meas_temperature - method_table.oven_temp) < 2*oven_delta){ // not low any more. P.P. 11/25/02
                else if(abs(temp_oven - method_table.oven_temp) < 2*oven_delta){ // not low any more. P.P. 07/03/05, 09/03/05
                    Oven_error = 0;
                    Oven_high_error = 0;
                }
                else {} // stays as it is. P.P.
            }
            else { // added, in case of ramping enabled. P.P. 02/23/03
                Oven_high_error = 0; // added - no nigh oven error if ramping. P.P. 11/19/05
//          if(meas_temperature < (method_table.oven_temp - oven_error_delta)){ // Diagnostics Oven low. P.P. 11/16/02
                if(temp_oven < (method_table.oven_temp - oven_error_delta)){ // Diagnostics Oven low. P.P. 07/03/05, 09/03/05
                    Oven_error = 1;   //
//            Oven_high_error = 0; // added. P.P. 02/23/03 Rem 11/19/05
                }
//          else if(meas_temperature > (method_table.oven_temp - 2*oven_delta)){ // not low any more. P.P. 11/25/02
//          else if(abs(meas_temperature - method_table.oven_temp) < 2*oven_delta){ // not low any more. P.P. 11/25/02
                else if(temp_oven > (method_table.oven_temp - 2*oven_delta)){ // P.P. 11/19/05
                    Oven_error = 0;   //
                }
                else {} // stays as it is. P.P.
            }
//#endif // !pmd_12. P.P. 11/28/04
// end of replacement. P.P. 04/30/04
//        int Diagnostics_Flow();   // P.P. 11/18/02  rem 04/30/04
/// code added from 501 of 13/10/03. P.P. 04/27/04
// new diagnostics messages added. P.P. 02/19/03
#if FID() // added. P.P. 08/07/05
            if(Oven_error||Oven_high_error||N2_error||Air_error||CalGas_error||Lamp_error||Board_error||FID_error) {  // FID added.  P.P. 08/07/05
#else
            if(Oven_error||Oven_high_error||N2_error||Air_error||CalGas_error||Lamp_error||Board_error) {  // changed.  P.P. 04/27/04
#endif // fid
                Diagnostics_error = 1;                              //
            }
            else {
                Diagnostics_error = 0;
            }
//	Diagnostics_Alarm(diagnostics_bit_number, Diagn_error); // added. P.P. 08/08/04
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
            if(N2_error || Air_error) {  // add later Oven and Oven_high errors. P.P. 02/23/03
	  Fatal_error = 1;  // any reaction to fatal errors?
//	  Fatal_error_Str=N2_error? "N2_error": "Air_error";
	  Fatal_error_Str=N2_error? "Carrier gas error": "Air_error"; // replacement. P.P. 08/22/04
	}
        else {
	  Fatal_error = 0;
        }
	if(Fatal_error) {   // P.P. 08/03/04, to turn the oven OFF
	}
	else { // added to restore oven enable. 05/18/07
	}
#endif // diagnostics
// added to check flow. P.P. 03/16/04
#if 0 /* FLOW()*/ // added to check flow. P.P. 03/16/04. Rem for current 501. P.P. 07/25/04
            double Get_Flow();        //
        const double flow_error_delta = 2.0;
        const double flow_delta = 1.0;
        double check_point = 0;
//        if(dwInjectTime) check_point = method_table.set_point_1; // during Inject
        if(dwInjectTime) check_point = pMethod->method_table.set_point_1; // during Inject. P.P. 07/05/04
//        else check_point = method_table.set_point;               // Inject = OFF
        else check_point = pMethod->method_table.set_point;  // Inject = OFF. P.P. 07/05/04
        double flow_data = Get_Flow();
//        if(flow_data<(check_point-flow_error_delta)) {
        if(abs(flow_data-check_point) > flow_error_delta) {  // variant with flow window
          Flow_error = 1;
        }
//        else if(flow_data>(check_point-flow_delta)) {
        else if(abs(flow_data-check_point) < flow_delta) {   // variant with flow window
          Flow_error = 0;
        }
        else {}; // stays as it is.
        if((dwInjectTime && ((dwInjectTime - 60*1000UL)<GetTickCount())) // 1 min before Jnject=OFF
               || (!dwInjectTime)) {  // 60 s for flow to set
//          Flow_error = (Flow_error || (Diagnostics_Flow()>0));
//          Flow_error = (Flow_error || N2_error); // P.P. 04/30/04
          Flow_error=0;  // P.P. 05/02/04
        }
#else
//        Flow_error = N2_error;    // P.P. 04/30/04
            Flow_error=0;    // replaced. P.P. 05/02/04
#endif // flow(). end of changes. P.P. 03/16/04
            Diagnostics_error = Diagnostics_error|| Flow_error; // moved from above. P.P. 05/02/04
#if PMD_12LS()&&PMD_16FS() // case added for 4-pnt GC301 with non-usb relays. P.P. 03/04/07
#if POINTS()<=4
            if(pMethod->method_table.alarms_on_off) {
	  Diagnostics_Alarm(diagnostics_bit_number, Diagnostics_error); // bit 5 in port B
	}
	else {
	  Diagnostics_Alarm(diagnostics_bit_number, FALSE); // bit 5 in port B
	}
#endif
#endif //  pmd_12
#if !GC312() // cond added. P.P. 03/19/06
            if(pMethod->method_table.alarms_on_off) {
//          if(acquire_type==CAL) {
                if((acquire_type==CAL)||(acquire_type==CHECKCAL)) { // changed. P.P. 07/25/04
//            Alarm(Oven_error || Flow_error || Lamp_error);
//            Alarm(Oven_error||Flow_error||Lamp_error||Board_error);
                    Alarm(Diagnostics_error);  // P.P. 04/30/04
                }
                else {
//            Alarm(Oven_error || Flow_error || Lamp_error || HL_alarm); // Local alarm. P.P. 11/18/02
//            Alarm(Oven_error||Flow_error||Lamp_error||HL_alarm||Board_error); // Board added to Local alarm. P.P. 06/20/03
                    Alarm(Diagnostics_error||HL_alarm);  // P.P. 04/30/04
                }
            }
            else {
                Alarm(FALSE);
            }
#endif // !gc312
            //// end of oven and flow diagnostics. P.P. 11/18/02
#else // in stand_alone case
#endif // !stand_alone. P.P. 07/15/03
        }
            break;
        case END_ACQUIRE:
            SendEndAcquire();
            //break;
        default:
            pMainWindow->sl->SetStatus("Inactive");
            break;
    }
    {

        switch(comm_state) {
            //case TRANS_METH:
//#if !STAND_ALONE() // added. P.P. 08/01/03
            case WAIT_TEMPERATURES:
                //case START_ACQUIRE:
//#if !STAND_ALONE() // moved. P.P. 08/15/03
            case WAIT_SAMPLE_TIME:   //added P.P. 10/26/02
#if !STAND_ALONE() // moved for stand alone with Toven conrtol. P.P. 01/06/16
            case WAIT_START_INJECT:  // added. P.P. 08/24/03
#endif
            case RECEIVING_DATA:
                //case END_ACQUIRE:
            {
                if(dwMotorOffTime && (dwMotorOffTime<GetTickCount())) {   // added to turn motor OFF in 3 s. P.P. 10/04/11
                    DoorMotor(FALSE);
                    dwMotorOffTime = 0;
                }
//	temp_oven=Get_Temperature(0); // restored here 04/17/09. Replaced for code below 10/30/09
                if(temp_oven_count>=temp_oven_count_max) { // read about 2 times per second. P.P. 10/30/09
                    temp_oven=Get_Temperature(0); // for Oven Temp. P.P. 07/03/05  restored 06/17/06  10/24/08
#if HEATED_LINE()
                    temp_line = Get_Temperature_Line();  // P.P. 03/29/17
#endif
#if !OVEN_EQ_INJ() // case added. P.P. 08/01/10
                    temp_det = Get_Temperature(1);
#endif
#if CONCENTRATOR()
                    //#elif CONCENTRATOR()
	  temp_conc = Get_Temp_Concentrator();  // added 01/02/10
#endif // concentrator
#if FID()
                    temp_fid = Get_Temperature_FID(fid_channel);  // added 07/27/18
#endif // fid
#if FLOW() // case added. P.P. 06/14/11
                    double Get_Flow_S(short channel); // prototype  // P.P. 06/14/11
	  flow_value = Get_Flow_S(sample_flow_chan); // measure once in 2 s, as oven temp.
	  if(flow_value<60) { // added. P.P. 06/20/11
	    Flow_error = 1;
	  }
	  else {
	    Flow_error = 0;
          }
#endif // flow
                    temp_oven_count = 0;
                }
                else {
                    temp_oven_count++;
                }
//        pressure_cylinder = Get_Pressure(pressure_channel); // added. P.P. 10/02/09 moved down. 10/04/09
//    #if !STAND_ALONE() // condition added. P.P. 08/17/02 And rem. P.P. 08/01/03
//#if !PMD_12LS() // cond added to save time. P.P. 11/28/04.
#if !(PMD_12LS()&&PMD_16FS()) //  P.P. 07/17/09.
                // add error messages here later. P.P. 10/22/09
	ShowTemperatures(pMainWindow);
#endif // !(PMD_12&&PMD_16together)
//    #endif

/// safety reading added. 09/07/07
//#if PMD_12LS()
#if PMD_12LS()&&PMD_16FS() // cond changed, 12LS is not the main A/D board. 09/18/07
                if(dwShowMessageTime < GetTickCount()) {  // replaced - now messages and measurements set by dwShowMessageTime. P.P. 11/01/09
                    dwShowMessageTime = GetTickCount() + 1*1000UL; // add 1 s
                    temp_oven_safety = Check_Temp_Safety(); // check not so often
#if GC312()
                    battery_voltage = GetBatteryVoltage(battery_channel); // P.P. 11/01/09
#endif // gc312
#if PRESSURE_SENSOR() // 10/23/09
                    pressure_cylinder = Get_Pressure(pressure_channel); // every 2 s. P.P. 10/04/09   1s
#endif // pressure sensor
#if HUMIDITY_SENSOR()
                    humidity_value = Get_Humidity(humidity_channel); // added. P.P. 06/26/17 here 07/27/17
#endif
#if H2_PRESSURE_SENSOR()
                    H2_pressure = Get_Pressure_H2(H2_channel);
#endif
#if CONCENTRATOR()&&FID()
                    temp_fid = Get_Temperature_FID(fid_channel); // measure and evaluate. P.P. 11/24/17
	  if(temp_fid>=t_fid) {
	    fid_on = 1;
	  }
	  else {
	    fid_on = 0;
          }
#endif
                    if(show_message) show_message = FALSE;
                    else show_message = TRUE;
                    if(temp_oven<0) {  // main reaction - the board is bad
                        Board_error = 1; // added. P.P. 10/22/09
                        EnableOven(FALSE); // turn oven OFF // added. P.P. 04/24/09
                        safety_warning = 1;
                    }
//	  else if(temp_oven_safety > (pMethod->method_table.oven_temp +30.0)) {
                    else if((!pMethod->method_table.RampingEnabled)&&(temp_oven_safety > (pMethod->method_table.oven_temp +30.0))) { // ramping added. P.P. 10/18/09
//	  else if((!pMethod->method_table.RampingEnabled)&&(temp_oven > (pMethod->method_table.oven_temp +30.0))) { // testing. P.P. 07/31/10
                        Board_error = 0; // added. P.P. 10/22/09
                        check_count=1; // replacement P.P. 04/19/09
                        safety_warning = 1; // rem temp. P.P. 11/17/11  and restored
                    }
//	  else if(temp_oven < (pMethod->method_table.oven_temp +10.0)) {   // testing. P.P. 07/31/10
                    else if((temp_oven < (pMethod->method_table.oven_temp +10.0))&&(temp_oven_safety < (pMethod->method_table.oven_temp +10.0))) {   // both checks. P.P. 08/01/10
                        Board_error = 0; // added. P.P. 10/22/09
                        check_count = 0;
                        if(temp_oven>=0) { // cond added. 09/18/07
                            Oven_high_error = 0; // 09/18/07
//	      Oven_error = 0; // 09/18/07 // rem. P.P. 08/01/10
                            EnableOven(TRUE); // turn oven ON, m.b. OK?
                            safety_warning = 0;
                        }
                    }
                    else {
                        Board_error = 0; // added. P.P. 10/22/09
                    }
//          if(check_count>2){ // if not a glitch
                    if(check_count){ // repl. P.P. 04/19/09
                        EnableOven(FALSE); // turn oven OFF - overheated!
                        Oven_high_error = 1; // added. 09/18/07
                        safety_warning = 1; // added. P.P. 04/17/09  // rem to test. P.P. 11/17/11 and restored
                    }
                    if(safety_warning) {
                        if(Board_error) {
                            if(show_message) { // case added. 09/18/07
                                pMainWindow->sl->Replace("A/D BOARD ERROR, OVEN DISABLED!");
                            }
                            else {
                                pMainWindow->sl->Replace("");
                            }
                        }
                        else if(check_count) {
                            if(show_message) {
                                pMainWindow->sl->Replace("OVEN OVERHEATED AND DISABLED!");  // blink each second
                            }
                            else {
                                pMainWindow->sl->Replace("");
                            }
                        }
                        else {
//          ShowTemperatures(pMainWindow);
                        }
                    }
// error messages inserted. P.P. 10/22/09
#if GC312() // added. P.P. 10/22/09
                        else if(battery_low) {
            if(show_message) {
	      pMainWindow->sl->Replace("Battery Low !!! Stop and Turn OFF GC");
	    }
	    else {
		pMainWindow->sl->Replace("");
	    }
	  }
	  else if(oven_high) {
	    if(show_message) {
	      pMainWindow->sl->Replace("Running on battery! Set oven to <= 100 C");
	    }
	    else {
		pMainWindow->sl->Replace("");
	    }
	  }
#endif // GC312
#if FLOW() // case added. P.P. 06/20/11
                        else if(Lamp_error||Oven_high_error||Oven_error||Flow_error) {  // replacement. P.P. 06/20/11
#else
                    else if(Lamp_error||Oven_high_error||Oven_error) {  // replacement. P.P. 07/31/10
#endif
                        char err_str[150] ={0};
                        char str_temp[20] = {0};
                        sprintf(err_str, "Errors: ");
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
                        if(Diagn_error) { // cond added. P.P. 07/31/10
              if(N2_error) {
//              sprintf(str_temp, "N2, ");
                sprintf(str_temp, "Carrier, "); // P.P. 07/23/04
                strcat(err_str, str_temp);
              }
              if(Air_error) {
                sprintf(str_temp, "Air, ");
                strcat(err_str, str_temp);
              }
              if(CalGas_error) {
                sprintf(str_temp, "Cal Gas, ");
                strcat(err_str, str_temp);
              }
            }
#endif // diagnostics
                        if(Lamp_error) {
                            sprintf(str_temp, "Lamp, ");
                            strcat(err_str, str_temp);
                        }
                        if(Oven_high_error) {
                            printf(str_temp, "Oven OVER, ");
                            strcat(err_str, str_temp);
                        }
                        else if(Oven_error) {
                            sprintf(str_temp, "Oven Low, ");
                            strcat(err_str, str_temp);
                        }
                        if(Flow_error) {
                            sprintf(str_temp, "Flow, ");
                            strcat(err_str, str_temp);
                        }
                        pMainWindow->sl->Replace(err_str);  // added. P.P. 08/01/10
                    }
                    else if(dwSampleTime) {  // hence, no warnings, no errors
                        pMainWindow->sl->Replace("Wait sample time");
                    }
#if !CONCENTRATOR() // cond added. P.P. 02/15/10
                    else if(dwEquilibrateTime) {
//	    pMainWindow->sl->Replace("Equilibrate");
                        if(pMethod->method_table.syringe_on_off) { // cond added. P.P. 04/28/06
                            pMainWindow->sl->Replace("Be ready to inject"); // added. P.P. 04/28/06
                        }
                        else {
                            pMainWindow->sl->Replace("Equilibrate");
                        }
                    }
#endif // !concentrator. P.P. 02/15/10
                    else if(dwInjectTime) {
                        pMainWindow->sl->Replace("Inject");
                    }
#if SAMPLE_AHEAD() // cond. added. P.P. 07/17/04
                    else if(dwNextPointTime&&(dwNextPointTime < GetTickCount())) { // restored. P.P. 11/14/04
                        pMainWindow->sl->Replace("Sampling next point");
                    }
#endif //sample_ahead
// end of error messages. P.P. 10/22/09
//	  else { // no safety warning and not in inject etc. - show message about pressure out of limits. P.P. 10/04/09
                        // removed - no gas messages during run. P.P. 10/30/09 and restored
#if PRESSURE_SENSOR() // new define. P.P. 10/22/09
#if WATER_PUMP()||TWO_COLUMNS() // P.P. 03/24/17
                        else if(pressure_cylinder<40) {
#else
#if H2_PRESSURE_SENSOR()
                        else if((pressure_cylinder<100)||(H2_pressure<100)) {   // P.P. 12/29/17
#else
                    else if(pressure_cylinder<100) {
#endif // H2_pressure
#endif // water_pump
                        if(show_message) {
                            pMainWindow->sl->Replace("PRESSURE LOW - ADD GAS!");  // blink each second
                        }
                        else {
                            pMainWindow->sl->Replace("");
                        }
                    }
#if WATER_PUMP()||TWO_COLUMNS() // P.P. 03/24/17
                        else if(pressure_cylinder>50) {
#else
#if H2_PRESSURE_SENSOR()
                        else if((pressure_cylinder>1600)||(H2_pressure>1600)) {  // P.P. 12/29/17
#else
                    else if(pressure_cylinder>1600) {
#endif
#endif
                        if(show_message) {
                            pMainWindow->sl->Replace("PRESSURE TOO HIGH!");  // blink each second
                        }
                        else {
                            pMainWindow->sl->Replace("");
                        }
                    }
#endif // pressure sensor

#if FID()  // case added to show message when fid is off (temp_fid < t_fid which is 60 C). P.P. 01/16/18
                        else if(!fid_on) {
	    if(show_message) {
	      pMainWindow->sl->Replace("FID is OFF!");  // blink each second
	    }
	    else {
	      pMainWindow->sl->Replace("");
	    }
	  }
#endif
                    else { // no messages P.P. 05/31/13
                        pMainWindow->sl->Replace("");
                    }

                } // brace added here - messages renew every 2 s now. P.P. 10/22/09
                ShowTemperatures(pMainWindow);  // added. 10/13/08 but only if safety restored. 12/12/08 restored 04/19/09
#endif // 12_ls&&16_fs
/// end of safety reading
            }
        }
// code added to control FID. P.P. 07/05/05
#if !GC312() // cond added, manual FID on/off in gc312. 02/13/08  This case left as it was for now. P.P. 10/22/09
#if FID() // case added. P.P. 07/03/05
        switch(comm_state) {
#if !STAND_ALONE()
      case WAIT_SAMPLE_TIME:
      case WAIT_START_INJECT:
#endif
      case RECEIVING_DATA:
      {
//	if(((pMethod->method_table.inj_det_temp) - 10.0)>Get_Temperature(1)) { // P.P. 07/03/05
	if(((pMethod->method_table.inj_det_temp) - 10.0)>temp_det) { // P.P. 09/30/05
	  if(!start_fid) {  // not started yet, or not needed already - check t-re.
	    FlameDetector(TRUE);  // ignite and wait 15 s
            start_fid = 1;
//	    dwFidDelayTime = GetTickCount()+15*1000UL; // 15 s delay
	    dwFidDelayTime = GetTickCount()+5*1000UL; // 3 s delay
	    dwFidTempTimeout = dwFidDelayTime + 20*1000UL; // 3+12 s waiting temperature. P.P. 07/24/05
	  }
	  else {
//	    if(GetTickCount() > dwFidDelayTime) {   // ignition sent but 15 s and no result
	    if(dwFidDelayTime && (GetTickCount() > dwFidDelayTime)) {   // ignition sent but 15 s and no result
	      dwFidDelayTime = 0;   // failure - turn OFF
	      FlameDetector(FALSE);
//              start_fid = 0;  // rem. P.P. 07/26/05
            }
	  }
	  if(dwFidTempTimeout && (dwFidTempTimeout<GetTickCount())) {
	    HydrogenValve(FALSE);
	    fid_on=0; // added. P.P. 08/05/05
	    FID_error=1;
	    dwFidTempTimeout = 0; // turn OFF only once. P.P. 07/24/05
	    Diagnostics_Alarm(diagnostics_bit_number, TRUE); // should be traced throughout first. P.P. 07/03/05.  enabled 08/05/05
	    if(comm_state==RECEIVING_DATA) { // cond added. P.P. 08/06/05
              if(FID_error) {  // added. P.P. 08/05/05
	        Fatal_error_Str="FID failure";
	      }
            }
	    if(!safety_warning) pMainWindow->sl->Replace("FID failure"); // but doesn't show up! P.P. 08/05/05
	    return -1; // stop. P.P. 07/03/05
	  }
        }
//	else if(((pMethod->method_table.inj_det_temp) - 2.0)<Get_Temperature(1)) { // temp OK - turn ignition OFF. P.P. 07/03/05
	else if(((pMethod->method_table.inj_det_temp) - 2.0)<temp_det) { // temp OK - turn ignition OFF. P.P. 07/03/05
	  if(start_fid) {
	    FlameDetector(FALSE);  // Temp OK - wait for next error. P.P. 07/24/05
	    start_fid = 0;
	  }
	  if(dwFidTempTimeout) {
	    dwFidTempTimeout = 0;  // Temp OK - wait for next error. P.P. 07/24/05
          }
	  fid_on=1;    // added. P.P. 08/05/05
          FID_error=0; //
	}
	else { // nothing to do - temperature OK
	  fid_on=1;    // added. P.P. 08/05/05
	  FID_error=0; //
	}
      }
    }
#endif // fid
#endif // !gc312
// end of added code to control FID
#if !STAND_ALONE() // cond moved here from below. P.P. 05/26/06
        switch(comm_state) {
            case WAIT_SAMPLE_TIME:
            case WAIT_START_INJECT:
            case RECEIVING_DATA:
            {
#if HEATED_LINE()
                void SetLineTemperature(double temperature);// prototype
	SetLineTemperature ((long) t_line_stored); // P.P. 03/31/17
#endif // heated line

/// new code to monitor battery and control temperature. P.P. 04/08/06
//    new code. P.P. 04/07/06
//	  double temp_del = 0;  // moved here. P.P. 04/15/06
                double temp_del = (pMethod->method_table.oven_temp-temp_oven); // moved here from below. P.P. 04/16/06
#if GC312() // added. P.P. 04/07/06
                if(battery_voltage>0.5) { // hence, on battery
            battery_run = 1; // added. P.P. 04/08/06
	    if(battery_voltage < 5.0) { // 12 V via 1/2 divider,  voltage < 10 V (low limit)
              battery_low = 1;
              if(oven_on) { // cond added. P.P. 10/30/09
	        SetOvenTemperature(0);
#if !OVEN_EQ_INJ() // cond added. P.P. 11/09/08  never happens in GC312
	        SetDetTemperature(0);
#endif
		oven_on = FALSE;
              }
//	      pMainWindow->sl->Replace("Battery Low !!! Stop and Turn OFF GC" ); // P.P. 04/07/06 Rem 04/08/06
	    }
//	    else { // battery OK - hold temperature <= 50 C. P.P. 04/07/06
	    else { // battery OK - hold temperature <= 60 C. P.P. 02/13/09
	      battery_low = 0; // added. P.P. 04/08/06
	      if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
//	        if(pMethod->method_table.oven_temp <= 50) { // regular work
		if(pMethod->method_table.oven_temp <= max_temp_bat) { // regular work P.P. 02/13/09
		  oven_high = 0; // added. P.P. 04/08/06
// code added. P.P. 06/16/06
		  if(!safety_warning) { // cond added. P.P. 04/24/09  restored 06/20/09
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
		    SetTemperatures(pMethod->method_table.oven_temp,
		      pMethod->method_table.inj_det_temp); // replacement. P.P. 06/16/06
//		  SetTemperatures(t_oven_stored, t_inj_stored); // replacement. P.P. 06/16/06
#else
		    SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06  restored 06/19/09
//                  }
#endif // !oven_eq_inj
		  } // moved here. P.P. 08/29/12

// end
//	        pMainWindow->sl->Replace("Waiting temperature" ); // P.P. 04/08/06 Rem 04/08/06
	        }
//	        else { // oven temp set > 50 C in Method - not allowed!
//		else { // oven temp set > 60 C in Method - not allowed!   P.P. 02/13/09
		else { // oven temp set > max oven temp when on battery, in Method - not allowed!   P.P. 10/15/09
		  oven_high = 1;
// code added. P.P. 06/16/06
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
		  SetTemperatures(0,0); // replacement. P.P. 06/16/06
#else
                  if(oven_on) {  // cond added. P.P. 10/30/09
		    SetOvenTemperature(0); // replacement. P.P. 06/16/06
		    oven_on = 0;
                  }
#endif // !oven_eq_inj
		}
              } // ramping enabled
            }   // battery OK
	  }
	  else { // battery not connected - work from external power, no restrictions. P.P. 04/07/06
            battery_run = 0; // All OK. Added. P.P. 04/08/06
	    battery_low = 0; //
            oven_high = 0;   //
//	    temp_del = (pMethod->method_table.oven_temp-temp_oven); // rem. P.P. 04/16/06
// code added. P.P. 06/16/06
	    if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
//	      oven_high = 0; // added. P.P. 04/08/06
	      if(!safety_warning) { // cond added. P.P. 04/24/09
#if !OVEN_EQ_INJ() // case added. P.P. 06/16/06
	        SetTemperatures(pMethod->method_table.oven_temp,
	          pMethod->method_table.inj_det_temp); // replacement. P.P. 06/16/06
//	      SetTemperatures(t_oven_stored, t_inj_stored); // replacement. P.P. 06/16/06
#else
	        SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06  restored 06/19/09
//              }
#endif // !oven_eq_inj
	      }  // moved here. P.P. 08/29/12
	    }
//	    pMainWindow->sl->Replace(" " ); // P.P. 04/07/06  Rem. 04/08/06
	  }
// end of added code to monitor battery and control temperature. P.P. 04/07/06

#else // regular GC
// end of new code. P.P. 04/07/06
// code replacement. P.P. 06/16/06
                if(!pMethod->method_table.RampingEnabled) {    // cond added. P.P. 06/16/06.
                    if(!safety_warning) { // cond added. P.P. 04/24/09
                        oven_high = 0; // added. P.P. 04/08/06
                        SetOvenTemperature(pMethod->method_table.oven_temp); // replacement. P.P. 06/16/06
//	    SetOvenTemperature(t_oven_stored); // replacement. P.P. 06/16/06
                    }
                }
// end of code replacement. P.P. 06/16/06

// new code. P.P. 10/06/11
                const double oven_door_delta = 1.0; // P.P. 10/26/08 10/31/08
//	if(!((pMethod->method_table.RampingEnabled)&&(comm_state == RECEIVING_DATA))) {    // cond changed. P.P. 11/10/11.
//	if(pMethod->method_table.RampingEnabled) {    // cond changed. P.P. 11/10/11.
//	if(1) { // now oven door will work in isothermal as well. P.P. 10/31/08 replaced for cond above, disable oven door when ramping. P.P. 11/10/11
                if(!pMethod->method_table.oven_door_manual) {  // added. P.P. 09/07/12
                    if((temp_oven - pMethod->method_table.oven_temp) < oven_door_delta){ // P.P. 07/03/05 // restored P.P. 10/19/08
//	  if(oven_door_on&&((temp_oven - method_table.oven_temp) < (-5.0))) { // P.P. 06/23/06
                        if(oven_door_on) { // P.P. 06/25/06  // cond restored. P.P. 10/24/08
                            OvenDoor(FALSE);   // moved here from Start(). P.P. 05/11/02
//	      dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
                            dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 10/04/11
                            DoorMotor(TRUE); //  P.P. 10/04/11
                        }
                    }
                    else if((temp_oven - pMethod->method_table.oven_temp) >( oven_door_delta + 2)) { // P.P. 10/26/08
                        if(!((pMethod->method_table.RampingEnabled)&&(comm_state == RECEIVING_DATA))) { // do not open if in ramping. P.P. 11/10/11.
                            if(!oven_door_on) {  // P.P. 10/26/08
                                OvenDoor(TRUE); //
//	      dwMotorOffTime = GetTickCount() + 3*1000UL; // to turn the motor 3 s later. P.P. 10/04/11
                                dwMotorOffTime = GetTickCount() + 2*1000UL; // to turn the motor 2 s later. P.P. 11/08/11
                                DoorMotor(TRUE); // P.P. 10/04/11
                            }
                        }
                    }
                    else {
                    } // nothing to do. P.P. 11/25/02
                } // ! oven door manual. P.P. 09/07/12
// end of new code. P.P. 10/06/11

#endif // GC312() P.P. 04/07/06
/// end of new code to monitor battery and control oven temperature. P.P. 04/08/06
#if !OVEN_EQ_INJ() // case added to control det temperature. P.P. 06/11/06
                const double det_delta=1;
	temp_det=Get_Temperature(1); // P.P. 10/24/08
	double temp_del_det = 0;  // added. P.P. 06/11/06
	temp_del_det = (pMethod->method_table.inj_det_temp-temp_det);
//// temp_del_det averaging added to decrease clicks due to noise. P.P. 06/11/06
//	SetDetTemperature(pMethod->method_table.inj_det_temp); // P.P. 06/16/06
	if(!safety_warning) { // cond added. P.P. 04/24/09
	  SetDetTemperature(t_inj_stored); // P.P. 06/16/06
	}
#endif
            }
        }
#else // stand-alone. Case added. P.P. 07/14/06
        //#if GC321() // stand_alone+gc321 combo - measure oven temperature. P.P. 07/14/06
#if GC321()||GC326() //  P.P. 12/07/18
    switch(comm_state) {
      case WAIT_SAMPLE_TIME:
      case WAIT_START_INJECT:
      case RECEIVING_DATA:
      {
	temp_oven = Get_Temperature(0); // replacement for averaging. P.P. 10/24/08
      }
    }
#elif MANUAL_INJ() // stand_alone with Toven control. P.P. 01/04/16
    switch(comm_state) {
      case WAIT_TEMPERATURES:
      case WAIT_SAMPLE_TIME:
      case WAIT_START_INJECT:
      case RECEIVING_DATA:
      {
	temp_oven = Get_Temperature(0); // replacement for averaging. P.P. 10/24/08
	SetOvenTemperature(pMethod->method_table.oven_temp); //  P.P. 01/05/16
/ // not needed here for stand_aplone; rem. 03/30/17
#if HEATED_LINE()
        temp_line = Get_Temperature_Line();  // P.P. 03/29/17
#endif
      }
    }
#endif // gc321
#endif  // !stand_alone, moved here. P.P. 05/26/06
    }
#else // in non-internal case
        // code remoed. P.P. 09/01/18
#endif // Internal
        if(retVal) {
            TRACE1("Acquire::Continue() return retVal=%d",retVal);
            return retVal;
        }

        MethodTable &method_table=pMethod->method_table;  // added. P.P. 04/30/04
        for(int n=0; n<NUMDETECTORS; n++) {
            Integrator *p=pIntegrator[n];
            if(p) {                                // detector may be off
//      if((acquire_type == CAL) && p->cal_passed) {  // added W.K. 11/23/02 to finish cal run if success. P.P.
                if((acquire_type == CAL)||(acquire_type == CHECKCAL)) {  // separated to finish at cal time
                    if(p->cal_passed) { // happens only in cal run
                        // calibration done!
#if INTERNAL()
                        Cal_error = FALSE; // added. P.P. 12/06/02
#endif //INTERNAL()
                        bForceCalibrateNextRun = FALSE; // added. P.P. 11/25/02
                        bForceCheckCalibrateNextRun = FALSE;

// added W.K. to wait 12 s before stop run on cal success. P.P. 12/12/02
//     	  extern char data_rate;    //rate data received in pts/sec.
//        uint analysis_time = p->raw_data.time/data_rate + 12; // sec
//        uint analysis_time = p->raw_data.time/data_rate + 30; // sec  // delay increased. P.P. 12/14/02
//        if(p->analysis_time > analysis_time)
//          p-> analysis_time = analysis_time;  // do not wait too long
                        //uint analysis_time = p->raw_data.time/data_rate; // sec
//in Integrator::Integrator()	  if(p->analysis_time > method_table.calibrate_time)  // P.P. 04/30/04
//	    p-> analysis_time = method_table.calibrate_time;  // stop at cal time. P.P. 12/30/02
                    } // cal passed
#if INTERNAL()
                    #if PMD_12LS()&&PMD_16FS() // case added for 4-pnt GC301 with non-usb relays. P.P. 03/03/07
#if POINTS()<=4
                #if DIAGNOSTICS() // cond. addded P.P. 09/01/18
	Diagnostics_Alarm(cal_error_bit_number, FALSE); //
#endif // diagnostics
#endif // points
#else // previous setting. P.P. 11/06/05
#endif // pmd // P.P. 11/06/05
#endif //INTERNAL()
//        continue;   // removed. 12/12/02
                }
                // end of added
// code added to stop at quick_scan_time. P.P. 12/20/10
#if QUICK_SCAN()
                //in Integrator::Integrator()
      if(method_table.quick_scan_enabled) {
        if(quick_scan_active) {     //  P.P. 12/24/10
	  if(p->analysis_time > method_table.quick_scan_time) { // P.P. 12/20/10
	     p-> analysis_time = method_table.quick_scan_time;  // stop at qs time. P.P. 12/20/10
	  }
	}
	else {
	     p-> analysis_time = method_table.analysis_time;  //
	}
      }
      else { // added. P.P. 01/17/11
	p-> analysis_time = method_table.analysis_time;  //
      }
#endif // quick scan
// end of added code. P.P. 12/20/10
                if(!p->acquire_finished) return 0;
                else TRACE("acquire finished"); // added. P.P. 10/02/05

                TRACE("1-Start NewDetectPeaks"); // added. P.P. 10/02/05
                void NewDetectPeaks(Job *pJob, Method *pMethod, Run *pRun, Integrator *pIntegrator[NUMDETECTORS], PeakList **pPeakList);
                TRACE("2-Start NewDetectPeaks"); // added. P.P. 10/02/05
                Job *pJob=(Job *)pMethod;
                TRACE("3-Start NewDetectPeaks"); // added. P.P. 10/02/05
                NewDetectPeaks(pJob, pMethod, pRun, pIntegrator, pPeakList);
                TRACE("End NewDetectPeaks"); // added. P.P. 10/02/05
            }
        }


#if INTERNAL()
        unsigned short InfoValue=0x00;  // line below replaced for this. P.P. 02/26/05
    if((pRun->type_of_run!=CAL)&&(pRun->type_of_run!=CHECKCAL)){  // changed. P.P. 07/25/04
        TWA_calc(pMethod, pRun, pPeakList, pRun->pointnumber);
#if CBW() // cond. added. P.P. 02/23/05
        InfoValue = BIT[pointnumber]; // changed to use NO contacts. P.P. 11/27/04
#endif
    }
#if ANALOG_MIX() // cond added - we might want info in such a case. P.P. 08/17/04
    #if (POINTS()<=4) // P.P. 07/25/04
  Diagnostics_Alarm(cal_info_bit,(pRun->type_of_run!=JOB)); // bit 4 in portC2.
#else // > 4 points - added. using digital code for points. P.P. 04/12/05
#if USB_3106()||USB_3103()||USB_3101() // case added. P.P. 05/20/07
  int UL_Stat=0;
  if((pRun->type_of_run!=CAL)&&(pRun->type_of_run!=CHECKCAL)){  //
#if 1 // for testing BCD format for info. P.P. 08/23/05
    InfoValue = ((pointnumber+1)/10)*6 + (pointnumber+1);  // BCD code. show points from 1.
#else  // binary code
    InfoValue = pointnumber;  // points start from 0. P.P. 04/19/05
#endif
  }
  else { // Cal or CheckCal
#if 1 // BCD format for info.
#if POINTS()<18 // P.P. 05/27/07
      InfoValue = 24; //
#else // points > 17 but should be <37. P.P. 05/27/07
      InfoValue = 56; //
#endif // points<18
    if(pRun->type_of_run==CHECKCAL) {
//      InfoValue = 56; //
    }
    else { // cal run
      InfoValue += 1; // 05/27/07
    }
#else
    InfoValue = pointnumber + 128;  // add cal bit (#7) = 1.
#endif
  }
  USHORT AuxPortValue = 0; // added. 07/29/07
#if USB_3106()
  UL_Stat = My_cbDOut(CBWDEVICE_3106, AUXPORT, InfoValue);
#elif USB_3103()
  UL_Stat = My_cbDIn(CBWDEVICE_3103, AUXPORT, &AuxPortValue); // changed to use same port for diagn alarms. P.P. 07/29/07
  AuxPortValue &= 0xC0;
  InfoValue |= AuxPortValue;
  UL_Stat = My_cbDOut(CBWDEVICE_3103, AUXPORT, InfoValue);
#elif USB_3101()
  UL_Stat = My_cbDOut(CBWDEVICE_3101, AUXPORT, InfoValue);
#endif // one of usb_31xx boards
  if(UL_Stat) { // DEBUG
    Str str="USB-31xx error";
//    MessageBox(NULL,str, "wPeak: Fix the error and continue", MB_OK); rem P.P. 11/25/06
  }
#elif PMD_12LS()||CBWUSB_IO() // as it was. 03/11/18
  if((pRun->type_of_run!=CAL)&&(pRun->type_of_run!=CHECKCAL)){  // changed. P.P. 07/25/04
#if 1 // for testing BCD format for info. P.P. 08/23/05
    InfoValue = ((pointnumber+1)/10)*6 + (pointnumber+1);  // BCD code. show points from 1. P.P. 08/25/05
#else
    InfoValue = pointnumber;  // points start from 0. P.P. 04/19/05
#endif
  }
  else { // Cal or CheckCal
#if 1 // for testing BCD format for info. change for 0 if binary. P.P. 08/25/05
    if(pRun->type_of_run==CHECKCAL) {
      InfoValue = 56; // P.P. 08/26/05
    }
    else {
      InfoValue = 57; // P.P. 08/23/05
    }
#else
    InfoValue = pointnumber + 128;  // add cal bit (#7) = 1. P.P. 04/19/05
#endif
  }
#if PMD_12LS()
  int err_12LS=My_cbDOut(CBWDEVICE_12LS, FIRSTPORTA, InfoValue); // reaction on error added. P.P. 09/02/05
#elif CBWUSB_IO()
  int err_12LS=My_cbDOut(USB_IO, FIRSTPORTA, InfoValue); // reaction on error added. P.P. 09/02/05
#endif
  if(err_12LS) { // DEBUG
    Str str="PMD-1208LS error";
//    MessageBox(NULL,str, "wPeak: Fix the error and continue", MB_OK); rem P.P. 11/25/06
  }
#endif // pmd_12ls or usb-io
#endif // points<=4
#endif // analog_mix. P.P. 08/17/04
#endif //INTERNAL()

        // Acquire/Calibration finished!
        // Now check if we need to start some another
        if(0!=CheckCalibration()) // calibration failed - Message Posted!
            return 1; // STOP

        // Check auto calibration
        DWORD dwTime=GetTime(); // What time is it now?
        extern DWORD cal_cycle_start_time;
        switch(runMode.Acquire) {
            case 1: // Continuous mode
            case 2: // Repeat mode
#if !STAND_ALONE() // added. P.P. 07/15/03
                if(bForceCheckCalibrateNextRun) {
                PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_CHECKCALIBRATE1, 0L);
                TRACE("PostMessage CM_CHECKCALIBRATE1");
                return 1;
            }
            else if(bForceCalibrateNextRun || pMethod->method_table.cal_cycle) { // W.K. added. P.P. 11/23/02
                //  if(pMethod->method_table.cal_cycle) {
//        if(cal_cycle_start_time+pMethod->method_table.cal_cycle*3600000L<=dwTime) {
                if(bForceCalibrateNextRun || cal_cycle_start_time+pMethod->method_table.cal_cycle*3600000L<=dwTime) { // W.K. added. P.P. 11/23/02
                    PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_CALIBRATE1, 0L);
                    TRACE("PostMessage CM_CALIBRATE1");
                    return 1;
                }
            }
            break;
#else       //  Remote stop in all cases.
            case 0: // added. P.P. 07/15/03
#endif // !stand_alone. P.P. 07/15/03.
            case 3: // remote Start/Stop
#if INTERNAL()
                if(CheckRemoteStop()) {
                TRACE("Remote Stop");
                return 1;
            }
#endif // internal
                break;
        }

        // check repeat/continuous
        TRACE("Check repeat/continuous");
        switch(runMode.Acquire) {
            case 0: // Manual mode - added to turn alarms OFF if not found. P.P. 08/17/04
// code added. P.P. 08/17/04
            {
#if INTERNAL() //??
                #if !NO_ALARMS()  // added for GC without alarms. P.P. 06/18/11
            BOOL bAlarm=FALSE;  // added. P.P. 08/17/04
            if((acquire_type!=CAL)&&(acquire_type!=CHECKCAL)) {
                if(pMethod->method_table.alarms_on_off) {
                    const ulong &halarms=pMethod->method_table.halarms;
                    const ulong &lalarms=pMethod->method_table.lalarms;
                    if(halarms&(1L<<pointnumber)) { // High alarm
                        if(flagsAlarm&2) bAlarm = TRUE;
                        else {
#if COMMON_HIGH() // added. P.P. 01/11/05
                            current_state_alarm[pointnumber]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                            High_Alarm(pointnumber, FALSE); // added to turn High Alarm off if not found. P.P. 05/02/03
#endif // !gc312
#endif // common high
                        }
                    }
                    if(lalarms&(1L<<pointnumber)) { // Low alarm
                        if(flagsAlarm&4) bAlarm = TRUE;
#if !GC312() // cond added. P.P. 03/19/06
                        else Low_Alarm(pointnumber, FALSE); // added to turn Low Alarm off if not found. P.P. 05/02/03
#endif // !gc312
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(bAlarm||Diagnostics_error||Cal_error); // int alarm added. P.P. 12/05/04
#endif // !gc312
// code added to turn HA OFF if no high alarms in all points. P.P. 01/11/05
#if COMMON_HIGH() // added. P.P. 01/11/05
                    BOOL common_HA = FALSE;
	  for(int n = 0; n<POINTS(); n++) {
	    if(current_state_alarm[n]>0) {
	      common_HA = TRUE;
            }
	  }
          if(!common_HA) {
#if !GC312() // cond added. P.P. 03/19/06
	    High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // !gc312
          }
#endif // common high
// end of added code. P.P. 01/11/05
                }
                else { // added: if alarm box not checked - turn alarms OFF. P.P. 05/02/03
// changed - alarms should be turned OFF in ALL points. P.P. 12/05/04
                    for(int n = 0; n<POINTS(); n++) { // Turn all alarms OFF. P.P. 10/13/01
#if !GC312() // cond added. P.P. 03/19/06
                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                        current_state_alarm[n]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                        High_Alarm(n, FALSE); // all alarms OFF
#endif // !gc312
#endif // common high
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(FALSE); // added. P.P. 12/05/04
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                    #if !GC312() // cond added. P.P. 03/19/06
	  High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // !gc312
#endif // common high
                }
                if(bAlarm) {
                    static char far str[]=", Alarm(s)";
                    //str[2]='A'+num;
                    pRun->comment+=str;
                }
            }
            else  { // case added: uncheck box and turn all alarms OFF in cal/checkCal run. P.P. 12/04/05
                if(!(pMethod->method_table.alarms_on_off)) {
                    for(int n = 0; n<POINTS(); n++) { // Turn all alarms OFF. P.P. 10/13/01
#if !GC312() // cond added. P.P. 03/19/06
                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                        current_state_alarm[n]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                        High_Alarm(n, TRUE); // all alarms OFF
#endif // !gc312
#endif // common high
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(FALSE); // added. P.P. 12/05/04
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                    #if !GC312() // cond added. P.P. 03/19/06
	  High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // !gc312
#endif // common high
                }
            }
            // end of aded code. P.P. 12/05/04
#endif // !NO_ALARMS. P.P. 06/18/11
#endif // INTERNAL()
                break;
            }
// end of added code to turn alarms ON/OFF in the end of run. P.P. 08/17/04
            case 1: // Continuous mode
            {
                if((acquire_type!=CAL)&&(acquire_type!=CHECKCAL)) { // changed. P.P. 07/25/04
/// code added from 101303. P.P. 04/30/04
                    BOOL bAlarm=FALSE; // moved up. P.P. 12/05/04 and restored.
#if INTERNAL()
                    #if !NO_ALARMS()  // added for GC without alarms. P.P. 06/18/11
                if(pMethod->method_table.alarms_on_off) {
                    const ulong &halarms=pMethod->method_table.halarms;
                    const ulong &lalarms=pMethod->method_table.lalarms;
                    if(halarms&(1L<<pointnumber)) { // High alarm
                        if(flagsAlarm&2) bAlarm=TRUE;
                        else {
#if COMMON_HIGH() // added. P.P. 01/11/05
                            current_state_alarm[pointnumber]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                            High_Alarm(pointnumber, FALSE); // added to turn High Alarm off if not found. P.P. 05/02/03
#endif // !gc312
#endif // common high
                        }
                    }
                    if(lalarms&(1L<<pointnumber)) { // Low alarm
                        if(flagsAlarm&4) bAlarm=TRUE;
#if !GC312() // cond added. P.P. 03/19/06
                        else Low_Alarm(pointnumber, FALSE); // added to turn Low Alarm off if not found. P.P. 05/02/03
#endif // !gc312
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(bAlarm||Diagnostics_error); // int alarm added. P.P. 12/05/04
#endif // !gc312
// code added to turn HA OFF if no high alarms in all points. P.P. 01/11/05
#if COMMON_HIGH() // added. P.P. 01/11/05
                    BOOL common_HA = FALSE;
	  for(int n = 0; n<POINTS(); n++) {
	    if(current_state_alarm[n]>0) {
	      common_HA = TRUE;
            }
	  }
          if(!common_HA) {
#if !GC312() // cond added. P.P. 03/19/06
	    High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // !gc312
          }
#endif
// end of added code. P.P. 01/11/05
                }
                else { // added: if alarm box not checked - turn alarms OFF. P.P. 05/02/03
// changed - alarms should be turned OFF in ALL points. P.P. 12/05/04
                    for(int n = 0; n<POINTS(); n++) { // Turn all alarms OFF. P.P. 10/13/01
#if !GC312() // cond added. P.P. 03/19/06
                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                        current_state_alarm[n]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                        High_Alarm(n, FALSE); // all alarms OFF
#endif // !gc312
#endif // common high
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(FALSE); // added. P.P. 12/05/04
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                    #if !GC312() // cond added. P.P. 03/19/06
	  High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // !gc312
#endif // common high
                }
#endif // !alarms   P.P. 06/18/11
#endif // internal
                    if(bAlarm) {
//	if(1) {      // temp, to test. P.P. 12/30/10
                        static char far str[]=", Alarm(s)";
                        //str[2]='A'+num;
                        pRun->comment+=str;
                        // Print report if needed. Moved here from below. 05/02/03
//2 lines rem. P.P.08/18/04
//	  void CheckShiftReportTime(MainWindow *pMainWindow, int nAlarm); // P.P. 11/14/03
// 	  CheckShiftReportTime(pMainWindow, (int) bAlarm);   // P.P. 11/14/03
//	}  rem P.P. 12/24/10
//	else { // added to print in the end of run if Immediate chosen. P.P. 10/13/03
#if GC501()||SHIFT_REPORT() // cond added. P.P. 08/18/04
                        void CheckShiftReportTime(MainWindow *pMainWindow, int nAlarm); // P.P. 11/14/03
	  CheckShiftReportTime(pMainWindow, (int) bAlarm); // P.P. 11/14/03
#endif // gc501 or shift_report
//	}  // rem. P.P. 08/18/04
// new variant. P.P. 12/24/10
                        if(acq_runs>1) {
                            TRACE("Repeat point due to alarm");
#if QUICK_SCAN() // cond added. P.P. 01/17/11
                            quick_scan_next_point = 0;
#endif
                        }
                        else {
                            acq_runs = 0;
//	    pMainWindow->cur_point=pointnumber+1; // go successfully to the next point
#if QUICK_SCAN() // cond added. P.P. 01/17/11 moved here 06/18/11
                            pMainWindow->cur_point=NextPointNumber+1; // quick scan next point. P.P. 01/15/11
	    quick_scan_next_point = 1;
#else
                            pMainWindow->cur_point=pointnumber+1; // go successfully to the next point
#endif
                        }
                    }
                    else { // no alarm - go to next point
                        acq_runs=0;
//          pMainWindow->cur_point=pointnumber+1; // go successfully to the next point
#if QUICK_SCAN() // cond added. P.P. 01/17/11
                        pMainWindow->cur_point=NextPointNumber+1; // quick scan next point  P.P. 01/15/11
	  quick_scan_next_point = 1;
#else
                        pMainWindow->cur_point=pointnumber+1; // go successfully to the next point
#endif
                    }
// end of new variant

                }
/// end of added code. P.P. 04/30/04
//        pMainWindow->cur_point=pointnumber+1;
// added to turn alarms OFF during cal/CheckCal run. P.P. 12/05/04
                else  { // case added: uncheck box and turn all alarms OFF in cal/checkCal run. P.P. 12/04/05
#if INTERNAL()
                    #if !NO_ALARMS()  // cond added. P.P. 06/18/11
                if(!(pMethod->method_table.alarms_on_off)) {
                    for(int n = 0; n<POINTS(); n++) { // Turn all alarms OFF. P.P. 10/13/01
#if !GC312() // cond added. P.P. 03/19/06
                        Low_Alarm(n, FALSE);  //P.P. 10/13/01,
#endif // !gc312
#if COMMON_HIGH() // added. P.P. 01/11/05
                        current_state_alarm[n]=0;  // added. P.P. 01/11/05
#else
#if !GC312() // cond added. P.P. 03/19/06
                        High_Alarm(n, FALSE); // all alarms OFF
#endif // !gc312
#endif // common high
                    }
#if !GC312() // cond added. P.P. 03/19/06
                    Alarm(FALSE); // added. P.P. 12/05/04
#if COMMON_HIGH() // added. P.P. 01/11/05
                    High_Alarm(common_high_bit, FALSE); // added - turn it OFF. P.P. 01/11/05
#endif // common high
#endif // !gc312
                }
#endif // !no_alarms P.P. 06/18/11
#endif // Internal
                }
// end of added code. P.P. 12/05/04
                PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_ACQUIRE1, 0L);
                TRACE("PostMessage CM_ACQUIRE1 Continious Mode");
            }
                break;
            case 2: // Repeat mode
            {
                if(dwRepeatTime) {
                    if(dwRepeatTime+pMethod->method_table.repeat_cycle*3600000L<=dwTime) {
                        pMainWindow->cur_point=0;//pointnumber+1;
                        PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_ACQUIRE1, 0L);
                        TRACE("PostMessage CM_ACQUIRE1 Repeat Mode");
                    }
                    else {
#if POINTS()>1
                        ulong points=pMethod->method_table.points;
                    if(get_available_point(points, pointnumber+1)>pointnumber) {
//                if(acquire_type!=CAL)
                        if(acquire_type==JOB) // changed. P.P. 07/25/04
                            pMainWindow->cur_point=pointnumber+1;
                        PostMessage(pMainWindow->HWindow,WM_COMMAND, CM_ACQUIRE1, 0L);
                        TRACE("PostMessage CM_ACQUIRE1 Repeat Mode next point");
                    }
#endif  // points>1
                        // continue waiting
                        return 0;
                    }
                }
                else {
//          comm_state = GET_STATUS;   // rem 08/26/10, here - 09/17/19
                    pMainWindow->Refresh();
                    dwRepeatTime=dwTime; // begin waiting
                    TRACE("Begin waiting Repeat Mode");
                    return 0;
                }
            }
                break;
        }
        TRACE("Acquire::Continue() end");
        return 1; // That's all, folks!
    }


//   Description:   Write raw data to the current job file.
    void Acquire::WriteRawData(uint32 *raw_data, int len)
    {
        if(pRun->MaxRawDataSize<pRun->runrawdata.RawDataSize+len/sizeof(uint32)) {
#ifdef __TURBOC__
            asm int 3; // DEBUG
#endif
            return;
        }
        hmemcpy(pCurRawData,raw_data,len);
        pCurRawData+=len/sizeof(uint32);
        pRun->runrawdata.RawDataSize+=len/sizeof(uint32);
    }


#if INTERNAL()
    static int timer_delay = 2; // to delay check remote start with DI700. P.P. 10/12/03 and OvenDoor. P.P. 10/04/11
void EvTimer(MainWindow *pMainWindow)
{
    static BOOL bAlreadyInTimer=FALSE;
//  if(bAlreadyInTimer) return;
//  if(1) return; // to exclude measurements on timer. P.P. 03/18/18
    if(time_delay>1) {    // code added to wait 5 s before staring access the usb boards. P.P. 03/18/18
        time_delay--;
//    bAlreadyInTimer=TRUE;
        return;
    }
    else if(time_delay>0) {
        time_delay--;
        int check1 = InitInternal();
        StartInternal(TRUE);
        return;
    }
    else {
        bAlreadyInTimer=TRUE;
    }
    // everything removed to check if any role of timer in crash. P.P. 08/04/03
#if !STAND_ALONE()   // the condition added. P.P. 08/13/02 and rem. 10/12/03
//  int32 Get_Data(int DetNum, int gain); // 4 lines added to update. P.P. 11/28/04
    int16 Get_Data(int DetNum, int gain); // P.P. 11/09/14
    double Get_Temperature(BOOL InjDet);
    double GetBatteryVoltage(int channel); // added. P.P. 04/07/06
    double Get_Pressure(int channel); // P.P. 10/02/09
#if HEATED_LINE()
    void SetLineTemperature(double temperature);// prototype
#endif // heated line
//  void Fan(BOOL on); // added. P.P. 02/23/10
    temp_oven = Get_Temperature(0);       // and oven only.
#if HEATED_LINE()
    temp_line = Get_Temperature_Line();  // P.P. 03/29/17
	SetLineTemperature ((long) t_line_stored); // P.P. 03/31/17
#endif
    data_A = Get_Data(0, cur_gain[0]); //det A, Range as set during run. P.P. 06/04/06 restored 10/28/08
    data_B = Get_Data(1, cur_gain[1]); // added for det B, Range as set during run. P.P. 06/04/06
#if GC312() // moved here. P.P. 10/23/09
    battery_voltage = GetBatteryVoltage(battery_channel); // P.P. 04/07/06
  if(battery_voltage>0.5) { // working from battery. P.P. 10/23/09
    battery_run = 1;
    if(battery_voltage < 5) { // hence, the battery is < 10 V
      battery_low = 1;
    }
    else {
      battery_low = 0;
    }
  }
  else { // working from power supply
    battery_run = 0;
    battery_low = 0; // ?? in fact, we do not know
  }
#endif // gc312
#if PRESSURE_SENSOR() // P.P. 10/23/09
    pressure_cylinder = Get_Pressure(7); // P.P. 10/02/09
#endif // pressure sensor
#if HUMIDITY_SENSOR() // P.P. 06/26/17  here 07/27/17
    humidity_value = Get_Humidity(6);
#endif // humidity
#if H2_PRESSURE_SENSOR()
    H2_pressure = Get_Pressure_H2(H2_channel); // P.P. 12/29/17
#endif
#if !OVEN_EQ_INJ()                     //
    temp_det = Get_Temperature(1);       // added. P.P. 07/03/05
#else // added. P.P. 02/20/06
#if CONCENTRATOR()
    temp_conc = Get_Temp_Concentrator();  // added and rem
//  if(!fan_on) {  /// added to cool down the desorber. P.P. 02/23/10
//    Fan(TRUE);
//  }
#endif // concentrator
#endif // oven=inj                                //
/// safety reading added. 09/07/07
    int oven_over = 0;
    static int count_step1 = 0; // moved here from below, to work without 1208 board. P.P. 03/13/18
    if(count_step1) {
        count_step1 = 0;
    }
    else {
        count_step1 = 1;
    }
//#if PMD_12LS()
#if PMD_12LS()&&PMD_16FS() // cond changed. 09/18/07
/**/ // rem temp. 10/13/08  and restored. P.P. 04/17/09
//  Acquire::Check_Temp_Safety(); // 04/20/09
//  static int count_step1 = 0;
    temp_oven_safety = Check_Temp_Safety();
// added to take action if temp <0 (board is bad) 09/18/07
    if(temp_oven<0){
        Oven_error = 1;
        safety_warning =1; // P.P. 04/17/09
        if(count_step1) {
            Set_valve(fluidics_port_offset, temperature_bit_number, FALSE); // P.P. 09/07/07
            oven_enabled = FALSE;
//      pMainWindow->sl->SetStatus("A/D BOARD ERROR, OVEN DISABLED!");
            pMainWindow->sl->Replace("A/D BOARD ERROR!");
        }
        else {
//      ShowTemperatures(pMainWindow);
            pMainWindow->sl->Replace("OVEN DISABLED!");
        }
    }
// end
//  else if(!first_start) {  // P.P. 04/20/09
//    if(temp_oven_safety > t_oven_stored + 30.0) { //
    else if(temp_oven_safety > t_oven_stored + 30.0) { // disabled, to work without checking T with 1208. P.P. 02/22/18  restored 02/25/18
//  else if(0) {
        if(!first_start) {  // P.P. 04/20/09 moved here. P.P. 10/04/09
//        EnableOven(FALSE); // turn oven OFF - overheated!
            safety_warning =1; // P.P. 04/19/09
            oven_over = 1;
            Set_valve(fluidics_port_offset, temperature_bit_number, FALSE); // P.P. 09/07/07
            oven_enabled = FALSE;
//    pMainWindow->sl->SetStatus("OVEN DISABLED DUE TO OVERHEAT!");
            if(count_step1) {
                pMainWindow->sl->Replace("OVEN OVERHEAT!");
            }
            else {
                pMainWindow->sl->Replace("OVEN DISABLED!");
            }
        }
    }
    else if(temp_oven_safety < t_oven_stored + 10.0) { //
        oven_over = 0;
        safety_warning=0; // 04/19/09
        if(!oven_enabled) { // cond added. P.P. 02/18/10
//      Set_valve(fluidics_port_offset, temperature_bit_number, TRUE); // added P.P. 02/15/10  // rem to allow manual disable oven. P.P. 07/31/10
//      oven_enabled = TRUE;                                         //
        }
    }
    else {
    }
#endif // moved here, 1608 &&1208, for temp safety reading. P.P. 03/13/18
// messages on pressure added. P.P. 10/04/09
#if GC312() // messages on battery moved here. P.P. 10/23/09
    if(!safety_warning) {
    if(battery_run) {
      if(battery_low) {
        if(count_step1) {
	  pMainWindow->sl->Replace("Battery Low !!! Stop and Turn OFF GC" );
        }
        else {
	  pMainWindow->sl->Replace("");
	}
      }
      else if(t_oven_stored > max_temp_bat) { // not allowed.  P.P. 10/23/09
	if(count_step1) {
	  pMainWindow->sl->Replace("Running on battery, set oven to <= 100 C" ); // P.P. 10/23/09
	}
	else {
	  pMainWindow->sl->Replace("");
	}
      }
//    }
#if PRESSURE_SENSOR() // P.P. 10/23/09. We use it only in GC312
#if H2_PRESSURE_SENSOR()
      else if((pressure_cylinder<100)||(H2_pressure<100)) {   // P.P. 12/29/17
#else
      else if(pressure_cylinder<100) {
#endif
	if(count_step1) {
	  pMainWindow->sl->Replace("PRESSURE LOW - ADD GAS!");  // blink each second
        }
        else {
	  pMainWindow->sl->Replace("");
        }
      }
#if H2_PRESSURE_SENSOR()
      else if((pressure_cylinder>1600)||(H2_pressure>1600)) {   // P.P. 12/29/17
#else
      else if(pressure_cylinder>1600) {
#endif
	if(count_step1) {
	  pMainWindow->sl->Replace("PRESSURE TOO HIGH!");  // blink each second
        }
        else {
	  pMainWindow->sl->Replace("");
        }
      }
#endif // pressure sensor. Moved here from below. P.P. 01/16/18
#if FID()  // case added to show message when fid is off (temp_fid < t_fid which is 60 C). P.P. 01/16/18
      else if(!fid_on) {
	if(count_step1) {
	  pMainWindow->sl->Replace("FID is OFF!");  // blink each second
	}
	else {
	  pMainWindow->sl->Replace("");
	}
      }
#endif

      else { // no messages, P within limits. P.P. 10/04/09
      }
//#endif // pressure sensor
    }
    else { // not on battery - check pressure only
#if PRESSURE_SENSOR() // P.P. 10/23/09. We use it only in GC312
#if H2_PRESSURE_SENSOR()
      if((pressure_cylinder<100)||(H2_pressure<100)) {   // P.P. 12/29/17
#else
      if(pressure_cylinder<100) {
#endif
	if(count_step1) {
	  pMainWindow->sl->Replace("PRESSURE LOW - ADD GAS!");  // blink each second
        }
        else {
	  pMainWindow->sl->Replace("");
        }
      }
#if H2_PRESSURE_SENSOR()
      else if((pressure_cylinder>1600)||(H2_pressure>1600)) {   // P.P. 12/29/17
#else
      else if(pressure_cylinder>1600) {
#endif
	if(count_step1) {
	  pMainWindow->sl->Replace("PRESSURE TOO HIGH!");  // blink each second
        }
        else {
	  pMainWindow->sl->Replace("");
        }
      }
#endif // pressure sensor, moved here from below. P.P. 01/16/18
#if FID()  // case added to show message when fid is off (temp_fid < t_fid which is 60 C). P.P. 01/16/18
      else if(!fid_on) {
	if(count_step1) {
	  pMainWindow->sl->Replace("FID is OFF!");  // blink each second
	}
	else {
	  pMainWindow->sl->Replace("");
	}
      }
#endif
      else { // no messages, P within limits. P.P. 10/04/09
      }
//#endif // pressure sensor
    }
  }
#else // regular GC, not 312, but with pressure sensor. P.P. 01/30/17
    if(!safety_warning) {
#if PRESSURE_SENSOR() // P.P. 10/23/09. We use it only in GC312
#if WATER_PUMP()||TWO_COLUMNS() // cond added, for 301 with 2 columns. P.P. 12/30/17
        if(pressure_cylinder<40) {
#else
#if H2_PRESSURE_SENSOR()
        if((pressure_cylinder<100)||(H2_pressure<100)) {   // P.P. 12/29/17
#else
        if(pressure_cylinder<100) {
#endif  // h2 pressure
#endif // (water or 2 columns
            if(count_step1) {
                pMainWindow->sl->Replace("PRESSURE LOW - ADD GAS!");  // blink each second
            }
            else {
                pMainWindow->sl->Replace("");
            }
        }
#if WATER_PUMP()||TWO_COLUMNS() // cond added, for 301 with 2 columns. P.P. 12/30/17
            else if(pressure_cylinder>50) {
#else
#if H2_PRESSURE_SENSOR()
            else if((pressure_cylinder>1600)||(H2_pressure>1600)) {   // P.P. 12/29/17
#else
        else if(pressure_cylinder>1600) {
#endif
#endif // water pump or 2 columns
            if(count_step1) {
                pMainWindow->sl->Replace("PRESSURE TOO HIGH!");  // blink each second
            }
            else {
                pMainWindow->sl->Replace("");
            }
        }
//#endif // pressure sensor. Moved here from below. P.P. 01/16/18
#else // without presure sensor. added here P.P. 07/14/18
        #if FID()  // case added to show message when fid is off (temp_fid < t_fid which is 60 C). P.P. 01/16/18
//      else if(!fid_on) {
    if(!fid_on) {
      if(count_step1) {
	pMainWindow->sl->Replace("FID is OFF!");  // blink each second
      }
      else {
	pMainWindow->sl->Replace("");
      }
    }
#endif
#endif // pressure sensor. Moved here from above. P.P. 07/14/18
    }   // added P.P. 07/14/18
    else { // no messages, P within limits. P.P. 10/04/09
    }
//#endif // pressure sensor // moved from below. P.P. 03/23/17  moved up 01/16/18
//  } // rem P.P. 07/14/18
//#endif // pressure sensor
#endif // gc312 or egular GC
#if FLOW()  // P.P. 06/20/11
    double Get_Flow_S(short channel); // prototype
  flow_value = Get_Flow_S(sample_flow_chan);  // replaced. P.P. 06/18/11
  if(flow_value<60) {  // added. P.P. 06/20/11
    Flow_error = 1;
  }
  else {
    Flow_error = 0;
  }
#endif // flow
    ShowTemperatures(pMainWindow); // condition restored. 07/15/03

// added for flow -> an out. P.P. 05/07/04
#if FLOW()&&OLIN() // cond changed - flow analog output only in that case. P.P. 06/21/05
    void Send_Flow_to_DAC(double FlowValue); // prototype
//  Send_Flow_to_DAC(flow_value); // added. P.P. 05/07/04
  Send_Flow_to_DAC(flow_value/10.0); // changed. P.P. 08/10/04
#endif // flow and olin
// added from 05/10/04
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
    #if !DEMO() // cond added. P.P. 08/12/05
  static BOOL N2_error_event = 0;
  N2_error = ((Diagnostics(diagnostics_port)&BIT[N2_error_bit])!=0);   // added. P.P. 05/10/04
//#if GC501() // removed. P.P. 08/15/04
  if(N2_error) {
#if TCD() // cond added. P.P. 08/15/04
    Acquire::TCDDetector(FALSE);    // Turn ON manually after this. P.P. 05/10/04
#endif // tcd
  }
  else { // added to restore temperature bit. P.P. 08/03/04
  }
#endif // !demo. P.P. 08/12/05
//#endif // diagnostics  // moved down. P.P. 10/14/09
#if TCD() // cond added. P.P. 08/15/04
  // added to show TCD state in the status line. P.P. 05/30/04
//  BOOL TCD_on = ((Diagnostics(control_port_offset)&BIT[TCD_bit_number])!=0);
  BOOL TCD_on = ((~Diagnostics(control_port_offset)&BIT[TCD_bit_number])!=0);  // Low is ON. P.P. 02/20/09
  if(TCD_on) {
    if(!Cal_error) { 	   // added to restore cal failed message. P.P. 07/26/04
      if(!N2_error) {
	if(!safety_warning) pMainWindow->sl->Replace("TCD is ON");
      }
      else {
	if(!safety_warning) pMainWindow->sl->Replace("Carrier low: Oven OFF");
      }
    }
    else {
      if(!N2_error) {
	if(!safety_warning) pMainWindow->sl->Replace("Calibration failed. TCD is ON");
      }
      else {
	if(!safety_warning) pMainWindow->sl->Replace("Cal.failed. Carrier low: Oven OFF");
      }
    }
  }
  else {
    if(!Cal_error) {       // added. P.P. 07/26/04
      if(!N2_error) {
	if(!safety_warning) pMainWindow->sl->Replace("TCD is OFF");
      }
      else {
	if(!safety_warning) pMainWindow->sl->Replace("Carrier low: Oven, TCD OFF");
      }
    }
    else {
      if(!N2_error) {
	if(!safety_warning) pMainWindow->sl->Replace("Cal. failed. TCD is OFF");
      }
      else {
	if(!safety_warning) pMainWindow->sl->Replace("Cal. failed. Carrier low: Oven, TCD OFF");
      }
    }
  }
// end of added code.
#endif // tcd, replacement. P.P. 08/15/04
#endif // diagnostics // moved here from above. P.P. 10/14/09
// end of added code for flow. P.P. 05/07/04
//#elif GC321() // case added for stand_alone+GC321. P.P. 07/14/06
#elif GC321()||GC326() //  P.P. 12/07/18
    double Get_Temperature(BOOL InjDet);
  int16 Get_Data(int DetNum, int gain); // added to update. P.P. 05/28/05
  temp_oven = Get_Temperature(0);       // update oven only.
  data_A = Get_Data(0, cur_gain[0]); //det A, Range as set during run. P.P. 06/04/06
  data_B = Get_Data(1, cur_gain[1]); //det B, Range as set during run. P.P. 06/04/06
  ShowTemperatures(pMainWindow);
#elif MANUAL_INJ() // case added for stand_alone with Toven control. P.P. 01/04/16
  double Get_Temperature(BOOL InjDet);
  int16 Get_Data(int DetNum, int gain); // added to update. P.P. 05/28/05
  temp_oven = Get_Temperature(0);       // update oven only.
  data_A = Get_Data(0, cur_gain[0]); //det A, Range as set during run. P.P. 06/04/06
  data_B = Get_Data(1, cur_gain[1]); //det B, Range as set during run. P.P. 06/04/06
  ShowTemperatures(pMainWindow);
  SetOvenTemperature(t_oven_stored);  // added temporarily. May not work with pulsing. P.P. 01/06/16
#else // stand_alone - show only det readings. Case added. P.P. 05/28/06
//  int32 Get_Data(int DetNum, int gain); // added to update. P.P. 05/28/05
  int16 Get_Data(int DetNum, int gain); // changed. P.P. 10/27/15
//  data_A = Get_Data(0, 2); // only det A, Range 100 at this time. Corrected. P.P. 04/22/06
  data_A = Get_Data(0, cur_gain[0]); //det A, Range as set during run. P.P. 06/04/06
  ShowTemperatures(pMainWindow);
#endif // !stand_alone or DI. P.P. 05/28/06
    if(runMode.Acquire==3) {
//  ShowTemperatures(pMainWindow); // added temporarily. P.P. 08/27/02
//#if !(GC312()||STAND_ALONE()||GC321()) // cond changed. P.P. 04/13/08
#if !(GC312()||GC321()||GC326()||STAND_ALONE()) //  P.P. 12/07/18
#if !(POINTS()>=20) // // no input for dig board. P.P. 06/20/11
        if(CheckRemoteStart()) {
            TRACE("Remote Start");
            PostMessage(pMainWindow->HWindow, WM_COMMAND, CM_ACQUIRE, 0L);
            if(!safety_warning) pMainWindow->sl->Replace("" ); //??  // line added. P.P. 08/24/02
        }
#if REMOTE_CAL()  // added. P.P. 11/27/02
            else if(CheckRemoteCalibration()) {
      TRACE("Remote Calibrate");
      PostMessage(pMainWindow->HWindow, WM_COMMAND, CM_CALIBRATE1, 0L);  // changed from acquire to calibrate1. P.P. 11/29/02
      if(!safety_warning) pMainWindow->sl->Replace("");
    }
#if GC501() // cond added. P.P. 11/21/04
    else if(CheckRemoteCheckCalibration()) {
      TRACE("Remote Check Calibrate");
      PostMessage(pMainWindow->HWindow, WM_COMMAND, CM_CHECKCALIBRATE1, 0L);
      if(!safety_warning) pMainWindow->sl->Replace("");
    }
#endif // gc501
#endif // remote cal
        else {
            if(!safety_warning) pMainWindow->sl->Replace("Waiting remote start command..." ); //??
        }
#endif // !points>=20
#endif // !gc312
    }
//#if REMOTE_CAL() // added. P.P. 11/27/02 to enable work without remote cal.
    else {
        // Continuous mode
        // Repeat mode
//#if !(GC312()||GC321()) // cond added. 04/13/08
#if !(GC312()||GC321()||GC326()) //  P.P. 12/07/18
#if REMOTE_CAL() // moved from above. P.P. 08/04/03
        if(CheckRemoteCalibration()) {
      if(pMainWindow->pIntegration) { // already running
	TRACE("Remote Set next run to Calibration");
        bForceCalibrateNextRun = TRUE;
        bForceCheckCalibrateNextRun = FALSE;
      }
      else {  // W.K.'s changed code to remotely start calibration. P.P. 11/23/02
        PostMessage(pMainWindow->HWindow, WM_COMMAND, CM_CALIBRATE1, 0L);
        bForceCalibrateNextRun = FALSE; // added. P.P. 11/25/02
        bForceCheckCalibrateNextRun = FALSE;
      }
    }
#if GC501() // cond added - check cal only for 501. P.P. 11/21/04
    else { // added to prevent competition. P.P. 05/21/04
      if(CheckRemoteCheckCalibration()) {
        if(pMainWindow->pIntegration) { // already running
          TRACE("Remote Set next run to Check Calibration");
          bForceCheckCalibrateNextRun = TRUE;
          bForceCalibrateNextRun = FALSE;
        }
        else {
          PostMessage(pMainWindow->HWindow, WM_COMMAND, CM_CHECKCALIBRATE1, 0L);
	  bForceCheckCalibrateNextRun = FALSE;
          bForceCalibrateNextRun = FALSE;
        }
      }
    } // added. P.P. 05/21/04
#endif // gc501
//    pMainWindow->sl->Replace("" ); //?? // removed, to show TCD. P.P. 05/30/04
#endif // remote cal. moved here 08/04/03
#endif // !(GC312 or GC322). 04/13/08
    }

// added to turn flame ignition OFF in several s if started. P.P. 06/28/05
#if FID()
    #if !DEMO() // cond. added. P.P. 08/12/05
#if !GC312() // cond added, manual control in gc312. 02/13/08
   static int seconds_temp = 0;     // changed and
  static int seconds_ignite = 0;   // added. P.P. 07/26/05
  static double current_det_temp = 0; // added. P.P. 09/30/05
  if(start_fid) {
//    if(seconds_ignite<3) {  // P.P. 07/26/05
    if(seconds_ignite<5) {  // P.P. 09/09/05
      seconds_ignite++;
      current_det_temp = temp_det; // P.P. 09/30/05
      if(!safety_warning) pMainWindow->sl->Replace("Igniting flame" ); // P.P. 08/20/05
    }
    else { // 3 // 5 s passed
      Acquire::FlameDetector(FALSE);
      seconds_ignite=0;
//      seconds_temp=3;
      seconds_temp=5;  // P.P. 09/09/05
      start_fid=0;
      current_det_temp = temp_det; // P.P. 09/30/05
    }
  }
  if(seconds_temp) {
//    if(seconds_temp<15) {
//    if(seconds_temp<25) {  // P.P. 09/09/05
//    if(seconds_temp<25) {  // P.P. 09/09/05
    if(seconds_temp<25) {  // P.P. 09/09/05
      if(1) {  // for future changes if other fluidics boards. P.P. 11/11/05
	if(temp_det > (current_det_temp + 1)) { // positive slope. P.P. 09/30/05
	}
        else {
	  seconds_temp++;
        }
        current_det_temp=temp_det;
	if((t_fid>0)&&(temp_det<(t_fid-10.0))) { // t_fid stored after 1st run (?) P.P. 08/28/05
	  if(!safety_warning) pMainWindow->sl->Replace("Waiting FID temperature" ); // P.P. 08/20/05
        }
	else if((t_fid>0)&&(temp_det>t_fid)) { // added P.P. 09/24/05
	  if(!safety_warning) pMainWindow->sl->Replace("FID is ON" ); // P.P. 08/20/05
	}
	else {
        }
      }
      else { // H2 valve shut off - nothing to wait. P.P. 09/11/05
        seconds_temp=0; // hydrogen is off already
	if(!safety_warning) pMainWindow->sl->Replace("FID is OFF" ); // P.P. 08/20/05
      }
    }
    else { // >= 25 s and failed to ignite - Hydrogen off and alarm. P.P. 07/26/05
//      if(temp_det<50.0) { // later - from inj/det in method
//      if((t_fid>0)&&(temp_det<(t_fid-10.0))) { // t_fid stored after 1st run (?) P.P. 08/28/05
      if(temp_det<(t_fid-10.0)) { // be sure to turn H2 OFF. P.P. 09/24/05
//      if(temp_det<pMethod->method_table.inj_det_temp) { // later - from inj/det in method
	Acquire::HydrogenValve(FALSE); // close valve if failed to ignite
        Acquire::Diagnostics_Alarm(diagnostics_bit_number, TRUE); // alarm if failed ignite. ?: may be Local Alarm? P.P. 09/24/05
	fid_on = 0; // added. P.P. 08/05/05
	if(!safety_warning) pMainWindow->sl->Replace("FID turned OFF" ); // P.P. 08/20/05
      }
      else {        // case added. P.P. 08/05/05 Temperature OK - see if H2 is ON
	if(1) { // for future changes with other boards
	  fid_on = 1;
	  if(!safety_warning) pMainWindow->sl->Replace("FID is ON" ); // P.P. 08/20/05
	}
	else { // H2 valve closed - T-re doesn't matter
          fid_on = 0;
	  if(!safety_warning) pMainWindow->sl->Replace("FID is OFF" ); // P.P. 08/20/05
        }
      }
      seconds_temp=0; // to turn hydrogen off once
    }
  }
  if(fid_on) {  // hence, flame should be ON. case added. P.P. 08/05/05
//    if(temp_det<50.0) {  // check each second
//    if((t_fid>0)&&(temp_det<(t_fid-10.0))) { // t_fid stored after 1st run (?) P.P. 08/28/05
    if(temp_det<(t_fid-10.0)) { // be sure to turn it OFF P.P. 09/24/05
//    if(temp_det<pMethod->method_table.inj_det_temp) { // later - from inj/det in method
      Acquire::HydrogenValve(FALSE); // close valve if flame OFF
      Acquire::Diagnostics_Alarm(diagnostics_bit_number, TRUE); // alarm if failed ignite. ?: m.b. Local Alarm? P.P. 09/24/05
      fid_on = 0; // no more checks or actions
      if(!safety_warning) pMainWindow->sl->Replace("FID turned OFF" ); // added. P.P. 08/20/05
    }
    else {
      if(1) { // for future changes with other boards
	if(!safety_warning) pMainWindow->sl->Replace("FID is ON" ); // P.P. 08/20/05
      }
      else {
	if(!safety_warning) pMainWindow->sl->Replace("FID is OFF" ); // P.P. 09/11/05
      }
    }
  }
#endif // !gc312
#endif // !demo. P.P. 08/12/05
#endif // fid
// end of ignition control.
// temp control added. P.P. 01/06/06
//#if !STAND_ALONE() // cond added. P.P. 01/16/06
//#if (!STAND_ALONE())||GC321() // cond changed. P.P. 07/12/06
#if (!STAND_ALONE())||GC321()||GC326() //  P.P. 12/07/18
//#if GC312() // case added to check if operating on battery - restrict t_oven <=50 C. P.P. 04/07/06
#if GC312() // case added to check if operating on battery - restrict t_oven <=60 C. P.P. 02/13/09
    // added to check lamp state in GC312. P.P. 11/25/06. Moved here 12/19/06

// added to see if the lamp should be on, or it is disabled. P.P. 10/14/09
#if PMD_16FS() // cases separated. P.P. 11/12/10
  USHORT BitValue;
  My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
  if(BitValue) lamp_on = 0;  // only one lamp
  else lamp_on = 1;
#endif // 1608FS
// end of added code. 11/25/06
  double temp_del = 0;  // moved up. P.P. 04/07/06.  not needed, can be removed. 12/19/06
  if(battery_voltage>0.5) { // hence, on battery
    if(battery_voltage < 5.0) { // 12 V via 1/2 divider,  voltage < 10 V (low limit)
      SetOvenTemperature(0);
#if !OVEN_EQ_INJ()
      SetDetTemperature(0); // added. P.P. 06/16/06
#endif
      oven_on = FALSE;
    }
    else { // battery OK - hold temperature <= 60 C. P.P. 02/13/09
//      if(t_oven_stored <= 50) { // regular work
      if(t_oven_stored <= max_temp_bat) { // regular work  P.P. 02/13/09
	SetOvenTemperature(t_oven_stored); // added. P.P. 06/16/06
#if !OVEN_EQ_INJ()
        SetDetTemperature(t_inj_stored); // added. P.P. 06/16/06
#endif
//	      pMainWindow->sl->Replace(" " ); // P.P. 04/07/06. Replaced for code below 12/19/06
// code added to display messages in GC312. P.P. 12/19/06
	if(start_fid) {  // added to leave message on ignition. 02/16/08
	}
// messges restored for GC312 without fid. P.P. 10/14/09
        if((!lamp_on)&&(!oven_enabled)) {
	  if(!safety_warning) {
	    if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	      pMainWindow->sl->Replace("Lamp is OFF, Oven disabled" );
	    }
          }
        }
        else if(!oven_enabled) {
	  if(!safety_warning) {
	    if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	      pMainWindow->sl->Replace("Oven disabled" );
	    }
          }
        }
        else if(!lamp_on) {
	  if(!safety_warning) {
	    if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	      pMainWindow->sl->Replace("Lamp is OFF" );
	    }
          }
        }
        else {
	  if(!safety_warning) {
	    if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	      pMainWindow->sl->Replace("Ready for acquisition" );
              if(number_of_beeps>0) {
                MessageBeep(MB_ICONHAND);
		number_of_beeps--;
	      }
            }
          }
        }
// end of code for messages. P.P. 12/19/06

      }
//      else { // oven temp set > 50 C in Method - not allowed!
      else { // oven temp set > 60 C in Method - not allowed!  P.P. 02/13/09 >100 now 10/14/09
	SetOvenTemperature(0);
        oven_on = FALSE;
#if !OVEN_EQ_INJ()
        SetDetTemperature(0); // added. P.P. 06/16/06
        det_on = FALSE;
#endif
//        pMainWindow->sl->Replace("Running on battery, set oven to <= 50 C" ); // P.P. 04/07/06
//	if(!safety_warning) pMainWindow->sl->Replace("Running on battery, set oven to <= 60 C" ); // P.P. 02/13/09
//	if(!safety_warning) pMainWindow->sl->Replace("Running on battery, set oven to <= 100 C" ); // moved up. P.P. 10/23/09
      }
    }
  }
  else { // battery not connected - work from external power, no restrictions. P.P. 04/07/06
    SetOvenTemperature(t_oven_stored); // added. P.P. 06/16/06
#if !OVEN_EQ_INJ()
    SetDetTemperature(t_inj_stored); // added. P.P. 06/16/06
#endif
//	  pMainWindow->sl->Replace(" " ); // P.P. 04/07/06  // replaced for code below. P.P. 12/19/06
// code added to display messages in GC312. P.P. 12/19/06
    if(start_fid) {  // added to leave message on ignition. 02/16/08
    }
// messages restored for GC312 without FID. P.P. 10/14/09
    if((!lamp_on)&&(!oven_enabled)) {
      if(!safety_warning) {
	if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	  pMainWindow->sl->Replace("Lamp is OFF, Oven disabled" );
	}
      }
    }
    else if(!oven_enabled) {
      if(!safety_warning) {
	if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
          pMainWindow->sl->Replace("Oven disabled" );
	}
      }
    }
    else if(!lamp_on) {
      if(!safety_warning) {
	if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
          pMainWindow->sl->Replace("Lamp is OFF" );
	}
      }
    }
    else {
      if(!safety_warning) {
	if(!count_step1) {  // in case there is a low battery message - switch between them. P.P. 10/23/09
	  pMainWindow->sl->Replace("Ready for acquisition" );
	}
      }
      if(number_of_beeps>0) {
        MessageBeep(MB_ICONHAND);
        number_of_beeps--;
      }
    }
// end of code for messages. P.P. 12/19/06
  }
#else  // regular case
// added to pulse power to oven. P.P. 03/31/07
    int count_delay = 4;
    static int count_s = count_delay;
//  if(oven_door_on) {
    if(0) {   // disabled - we use oven door for 2nd pump. P.P. 06/20/11
//	  SetOvenTemperature(t_oven_stored - 5.0); // changed. P.P. 06/23/06
        SetOvenTemperature(t_oven_stored - 3.0); // changed. P.P. 06/30/06
    }
    else {
//    SetOvenTemperature(t_oven_stored); //  P.P. 06/23/06 // replaced for code below. P.P. 03/30/07
// changed to avoid delays in case of pulsing. P.P. 03/30/07
        double temp_del = (t_oven_stored-temp_oven);
//    if((temp_del<=0)&&oven_on) {
        if(temp_del<=0) {  // P.P. 03/31/07
            SetOvenTemperature(0);
            oven_on = FALSE;
            count_s = count_delay;
        }
        else if(temp_del>5.0) {
            SetOvenTemperature(200);
            oven_on = TRUE; //
            count_s = count_delay;
        }
        else if(temp_del>3.0) {
            if(count_s>0) {
                count_s=count_s-2;
                SetOvenTemperature(0);
                oven_on = FALSE;
            }
            else {
                SetOvenTemperature(200);
                oven_on = TRUE; //
                count_s = count_delay;
            }
        }
        else if(temp_del>1.0) {
            if(count_s>0) {
                count_s--;
                SetOvenTemperature(0);
                oven_on = FALSE;
            }
            else {
                SetOvenTemperature(200);
                oven_on = TRUE; //
                count_s=count_delay;
            }
        }
        else { // temp_delay <=1.0 but >0
            if(oven_on) {
                SetOvenTemperature(0);
                oven_on = FALSE;
            }
        }

// end of changed code. P.P. 03/30/07
    }
#if !OVEN_EQ_INJ()
    SetDetTemperature(t_inj_stored); // added. P.P. 06/16/06
#endif
//	if((temp_oven - t_oven_stored) < 3.0){ // P.P. 06/18/06
//	if(oven_door_on&&((temp_oven - t_oven_stored) < -5.0)){ // P.P. 06/23/06
//  if(oven_door_on&&((temp_oven - t_oven_stored) < -3.0)){ // P.P. 06/30/06
//  if(oven_door_on&&((temp_oven - t_oven_stored) < 0.5)){ // P.P. 10/24/08
#if RAMPING() // cond added. P.P. 10/31/08
    if (!door_manual) { // added. P.P. 09/07/12
    if(oven_door_on&&((temp_oven - t_oven_stored) < 1.0)){ // P.P. 10/31/08
      Acquire::OvenDoor(FALSE);   //
      timer_delay = 2; // to turn motor off 2-3 second later. P.P. 10/04/11
      if(!motor_on) {
        Acquire::DoorMotor(TRUE); // P.P. 10/04/11
      }
    }
 // rem 06/23/06
    else if((!oven_door_on)&&((temp_oven - t_oven_stored) >3.0)) { // added. P.P. 10/31/08  Open the door. restored and modified. P.P. 10/06/11
      Acquire::OvenDoor(TRUE);
//      timer_delay = 2;
      timer_delay = 1; // P.P. 11/08/11
      if(!motor_on) {
        Acquire::DoorMotor(TRUE); // P.P. 10/06/11
      }
    }
    else {
    }
    if(motor_on>0) {
      if(timer_delay<=0) {   // added to turn motor OFF in 3 s. P.P. 10/04/11
        Acquire::DoorMotor(FALSE);
        timer_delay = 0;
      }
      else {
        timer_delay--;
      }
    }
  } // ! door_manual
#endif // ramping
// added to check lamp state. P.P. 11/22/06 11/25/06
//#if PMD_16FS()&&GC321() //
//#if PMD_16FS()&&(GC321()&&(!PMD_12LS())) // 03/21/08
#if PMD_16FS()&&(GC321()||GC326())&&(!PMD_12LS()) //  P.P. 12/07/18
    USHORT d_lamp = 0;
//  My_cbAIn(CBWDEVICE_16FS, 4, BIP10VOLTS, (USHORT *)&d_lamp);
  My_cbAIn(CBWDEVICE_16FS, 5, BIP10VOLTS, (USHORT *)&d_lamp);   // corrected. bit 5 is lamp input. P.P. 07/17/10
  if(d_lamp>40960) lamp_on=0;
  else if(d_lamp<35388) lamp_on = 1;
  else {
  }
#elif PMD_16FS() // added 03/21/08, some other DIO board is used for fluidics
    USHORT BitValue;
    My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
    if(BitValue) lamp_on = 0;  // only one lamp
    else lamp_on = 1;
#endif
// end of added code. 11/22/06
//#if GC321()
#if GC321()||GC326() //  P.P. 12/07/18
    if((!oven_enabled)&&oven_door_on&&(!lamp_on)) {  // detailed messages added. P.P. 07/02/06
    if(!safety_warning) pMainWindow->sl->Replace("Oven disabled, door is open, Lamp is OFF" ); // P.P. 07/23/06
  }
  else if((!oven_enabled)&&oven_door_on) {  // detailed messages added. P.P. 07/02/06
    if(!safety_warning) pMainWindow->sl->Replace("Oven disabled, door is open" ); // P.P. 06/23/06
  }
#else // not 321
#if FLOW()
    if((!oven_enabled)&&(!lamp_on)&&Flow_error) {  // detailed messages added. P.P. 07/02/06
    if(!safety_warning) pMainWindow->sl->Replace("Oven disabled, Lamp is OFF, Flow is low" ); // P.P. 06/20/11
  }
#else
#if !TCD() // added cond. P.P. 01/30/17
    if((!oven_enabled)&&(!lamp_on)) {  // detailed messages added. P.P. 07/02/06
//    if(!safety_warning) pMainWindow->sl->Replace("Oven disabled, door is open, Lamp is OFF" ); // P.P. 07/23/06
        if(!safety_warning) pMainWindow->sl->Replace("Oven disabled, Lamp is OFF" ); // P.P. 04/26/13 no door if !321
    }
#endif // tcd
#endif // flow
#endif // gc321
#if !TCD() // added cond. P.P. 01/30/17
    else if((!lamp_on)&&(!oven_enabled)) {
        if(!safety_warning) pMainWindow->sl->Replace("Lamp is OFF, Oven disabled" ); // P.P. 07/22/06
    }
#endif
//#if GC321() // cond added. P.P. 06/20/11
#if GC321()||GC326() //  P.P. 12/07/18
        else if(oven_door_on&&(!lamp_on)) {  // detailed messages added. P.P. 07/02/06
    if(!safety_warning) pMainWindow->sl->Replace("Lamp is OFF, door is open"); // P.P. 07/23/06
  }
#endif
#if !TCD() // added cond. P.P. 01/30/17
    else if(!oven_enabled) {
        if(!safety_warning) pMainWindow->sl->Replace("Oven disabled" ); // P.P. 06/23/06
    }
#endif
#if !FLOW()
#if !TCD() // added cond. P.P. 01/30/17
    else if(!lamp_on) {
        if(!safety_warning) pMainWindow->sl->Replace("Lamp is OFF" ); // P.P. 07/02/06
    }
#endif
#endif // flow
//#if GC321() // cond added. P.P. 06/20/11
#if GC321()||GC326() //  P.P. 12/07/18
        else if(oven_door_on) {
    if(!safety_warning) pMainWindow->sl->Replace("cooling oven" ); // P.P. 06/23/06
  }
#endif
#if FLOW()
        else if((!lamp_on)&&Flow_error) {
    if(!safety_warning) pMainWindow->sl->Replace("Lamp is OFF, Flow is low" ); // P.P. 07/02/06
  }
  else if(!lamp_on) {
    if(!safety_warning) pMainWindow->sl->Replace("Lamp is OFF" ); // P.P. 07/02/06
  }
  else if(Flow_error) {
    if(!safety_warning) pMainWindow->sl->Replace("Flow is low" ); // P.P. 06/20/11
  }
#endif
#if !TCD() // added cond. P.P. 01/30/17
    else {
#endif
        if(!safety_warning) pMainWindow->sl->Replace("Ready for acquisition" ); // P.P. 06/23/06
        if(number_of_beeps>0) {
            MessageBeep(MB_ICONHAND);   // added.  P.P. 06/23/06
            number_of_beeps--;
        }
#if !TCD() // added cond. P.P. 01/30/17
    }
#endif
#endif // gc312
#elif STAND_ALONE()&&MANUAL_INJ()
    // add code here to control Toven while in standby. P.P. 01/05/16
#endif // !stand_alone. 01/16/06
// end of added temp control. P.P. 01/06/06
    bAlreadyInTimer=FALSE;
}

//void InitInternal()  // modified 11/11/02. P.P. If DDA_2 digital needed - should be addded later.
int InitInternal()  // modified 01/01/07. P.P. to exit if board number is incorrect.
{
int check = 0; // added. P.P. 01/01/07
//  #if MYDEBUG()
#if MYDEBUG() || DEMO() // P.P. 08/24/04
//  return;
return 0;  // 0 is OK. P.P. 01/01/07
#endif
#if PMD_12LS()||CBWUSB_IO() //  P.P. 03/13/18 cond removed. P.P. 08/24/18
#if PMD_12LS()
My_cbDConfigPort(CBWDEVICE_12LS, FIRSTPORTA, DIGITALOUT);
#elif CBWUSB_IO() // P.P. 03/13/18
My_cbDConfigPort(USB_IO, FIRSTPORTA, DIGITALOUT);
My_cbDConfigPort(USB_IO, FIRSTPORTB, DIGITALOUT); // added for new variant.
My_cbDConfigPort(USB_IO, FIRSTPORTCL, DIGITALOUT); // P.P. 03/13/18
My_cbDConfigPort(USB_IO, FIRSTPORTCH, DIGITALOUT); //
#endif
#if PMD_16FS() // case added, 2 boards together. P.P. 11/25/06
#if PMD_12LS()
My_cbDConfigPort(CBWDEVICE_12LS, FIRSTPORTB, DIGITALOUT); // only outputs now for 1208. P.P. 08/24/18
#endif // 12LS or usb_io
// board's # checking removed - not finished yet. P.P. 02/19/07
#if SERIAL_NUMBER()
// checking the board # for security was added. P.P. 01/01/07
DWORD mfg_sernum = 0;
Str Myascii; // to interpret as hex. P.P. 03/17/17
int My_cbGetConfig (int InfoType, int BoardNum, int DevNum, int ConfigItem, DWORD *ConfigVal); // prototype P.P. 03/16/17
int err_message;
int stringLen = 9;
err_message = My_cbGetConfigString (BOARDINFO, 1, 0, BIMFGSERIALNUM, serNum, &stringLen);
if(ser_num ==48) {  // my particular board. P.P. 03/12/17
check= 1; // correct board #,
}
else {
check = 0; //incorrect board #, results in exit from wpeak. P.P. 01/15/17
}
// end of checking. P.P. 01/01/07
#endif // serial number
int j = 0;
/*
 #if MANUAL_INJ() // case added, no valves. P.P. 05/13/06. 12/13/18 -  rem, bits 4 and 5 must be inputs
  for(j=0; j<8; j++) {
    My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
  }
#else // bit 5 - for lamp A input and bit 4 for lamp B input. P.P. 08/24/18
*/
/*                                                // replaced - bits 6 and 7 were for input!
  for(int j=0; (j<8)&&(j!=5); j++) {
    My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
  }
  My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, 5, DIGITALIN);
*/
for(j=0; j<8; j++) {  // changed. P.P. 03/31/09
#if PMD_12LS()
if(j!=5) {           //  bit 5 still for lamp A. P.P. 08/24/18
#elif CBWUSB_IO()
if((j<4)||(j>5)) {    // now bit 4 is also input, for lamp B P.P. 12/18/18,
#else
if(1) {
#endif
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
}
else {
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALIN);
}
}
// #endif // manual_inj rem P.P. 12/13/18
#endif // pmd_16fs


#elif PMD_12FS()
My_cbDConfigPort(CBWDEVICE_12FS, FIRSTPORTA, DIGITALOUT);
//  My_cbDConfigPort(CBWDEVICE_12FS, FIRSTPORTB, DIGITALIN);
My_cbDConfigPort(CBWDEVICE_12FS, FIRSTPORTB, DIGITALOUT); // 4-pnt gc301. P.P. 03/03/07
#elif PMD_16FS() // case added  no other dio, use this board. P.P. 03/24/06
int j = 0;
#if STAND_ALONE()
for(j=0; j<8; j++) {
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
}
#else
#if MANUAL_INJ() // case added, no valves. P.P. 05/13/06
//#if MANUAL_INJ()||STAND_ALONE() // P.P. 11/01/15
for(j=0; j<8; j++) {
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
}
#else // bit 5 - for lamp A input
/*
  for(int j=0; (j<8)&&(j!=5); j++) {
    My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
  }
  My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, 5, DIGITALIN);
*/
for(j=0; j<8; j++) {  // changed. P.P. 03/31/09, here 07/17/10
#if PMD_12LS()  // P.P.12/18/18
if(j!=5) {
#elif CBWUSB_IO()
if((j<4)||(J>5)) {          // lamp B and lamp A inputs. P.P. 08/24/18
#else
if(1) {
#endif
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALOUT);
}
else {
My_cbDConfigBit(CBWDEVICE_16FS, AUXPORT, j, DIGITALIN);
}
}
#endif // manual_inj
#endif // stand-alone
#endif // pmd_12LS or 12FS. P.P. 06/17/05
#if USB_3112() // case added, 8 dig. I/O to be used for diagnostics. P.P. 04/15/06
My_cbDConfigPort(CBWDEVICE_3112, AUXPORT, DIGITALIN);
#elif USB_3106() // added. P.P. 05/20/07
My_cbDConfigPort(CBWDEVICE_3106, AUXPORT, DIGITALOUT);  // for point info output in BCD or binary code. P.P. 05/20/07
#elif USB_3103() // added. P.P. 05/20/07
//  My_cbDConfigPort(CBWDEVICE_3103, AUXPORT, DIGITALOUT);  // for point info output in BCD or binary code. P.P. 05/20/07
for(j=0; j<8; j++) {
My_cbDConfigBit(CBWDEVICE_3103, AUXPORT, j, DIGITALOUT); // bits configured individually. P.P. 05/25/07
}
#elif USB_3101() // added. P.P. 05/20/07
My_cbDConfigPort(CBWDEVICE_3101, AUXPORT, DIGITALOUT);  // for point info output in BCD or binary code. P.P. 05/20/07
#endif // usb_3112
// code added to configure USB-ERB24 relay board. P.P. 05/06/06
#if USB_1024LS()||USB_DIO24() // added as a replacement for the 2nd 12LS board. P.P. 07/13/07
My_cbDConfigPort(CBWDEVICE_1024, FIRSTPORTA, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024, FIRSTPORTB, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024, FIRSTPORTCL, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024, FIRSTPORTCH, DIGITALOUT);
#if USB_1024LS_2()||USB_DIO24_2() // added for alarms in 12-point gc301c. P.P. 11/06/07
My_cbDConfigPort(CBWDEVICE_1024_2, FIRSTPORTA, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024_2, FIRSTPORTB, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024_2, FIRSTPORTCL, DIGITALOUT);
My_cbDConfigPort(CBWDEVICE_1024_2, FIRSTPORTCH, DIGITALOUT);
#endif //1024LS_2 or dio24_2
#endif // usb_1024 or usb_dio24

// end of added code to configure USB-ERB24 board. P.P. 05/06/06
t_oven_stored = -1000.0;  //;  // replaced. P.P. 07/07/06
return check; // added. P.P. 01/01/07
}
// New function to show voltage together with temperature. P.P. 10/18/02
double ShowVolts(int DetNum)
{
#if MYDEBUG() || DEMO()  // added. P.P. 11/06/02
    return 3.450;
#endif
    int gain = cur_gain[DetNum];
#if (PMD_12LS()||PMD_16FS()||PMD_12FS()) // one of PMD - read saved data. P.P. 07/27/08
    int16 data = 0;
    if(DetNum) {
        data = data_B;
    }
    else {
        data = data_A;
    }
#endif // pmd
#if Get_Data_Bipolar()   //added. P.P. 06/29/02
    const double vmin=-10.0; // gain=1
    const double vmax=+10.0;
//   double voltage = data * ((vmax-vmin)/65536.0) +vmin; // P.P. 07/03/02. 0 in the middle.
//   double voltage = data * ((vmax-vmin)/65536.0); // P.P. 10/18/02.
    double voltage = (data *(vmax-vmin))/65536.0; // P.P. 07/05/05.
#else // unipolar
    double voltage = (10.0*data_in)/65536.0;   // P.P 05/12/01
#endif // bipolar
    return voltage;
}

//// New diagnostics functions. P.P. 11/15/02
int Diagnostics_Lamp()  // lamps A and/or B failure ?
{
#if DEMO()||TCD()||FID()||WATER_PUMP() // P.P. 01/30/17. changed - no lamp present if tcd + fid. P.P. 02/18/11 here 03/02/12
    return 0;  //
#else        //
    int Lamp_error_flag = 0;
    return Lamp_error_flag;
#endif // demo, ... Added. P.P. 08/22/04
}
/// replaced for all diagn at once: P.P. 04/27/04
static int Diagnostics(int diagn_port)   // digital input from diagnostics port. P.P. 02/19/03
{                                 // changed to allow reading from different ports. P.P. 05/30/04
int Diagnostics_error_flag = 0;
#if !MYDEBUG()
#if !(DEMO()||STAND_ALONE()) // P.P. 07/15/08
#if PMD_12LS()||CBWUSB_IO() // P.P. 03/13/18
#if CBW()
// added to use 16FS board. P.P. 02/18/07
#if PMD_16FS() // case added. P.P. 03/19/06  no other boards - only lamp (bit 5) is checked. 04/15/06
USHORT BitValue;
My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
Diagnostics_error_flag = (int)(BitValue);
#endif //pmd_16fs. P.P. 02/18/07
#endif // cbw
#elif USB_3112() // case added to use dig port of this board for diagn. P.P. 04/15/06
USHORT AuxPortValue;
My_cbDIn (CBWDEVICE_3112, AUXPORT, &AuxPortValue);
Diagnostics_error_flag = (int)(AuxPortValue);
#elif PMD_16FS() // case added. P.P. 03/19/06  no other boards - only lamp (bit 5) is checked. 04/15/06
USHORT BitValue;
My_cbDBitIn (CBWDEVICE_16FS, AUXPORT, 5, &BitValue);
Diagnostics_error_flag = (int)(BitValue);
#endif // pmd_12ls or pmd_16fs
#endif // !demo or  stand_alone. P.P. 08/24/04
#endif // MYDEBUG()
return Diagnostics_error_flag;
}
//// end of diagnostics functions. P.P.
/// end of replacement

/// N2 pressure diagnostics:
int Diagnostics_Flow()   // digital input from carrier pressure switch. P.P. 11/18/02
{
int Flow_error_flag = 0;
#if !MYDEBUG()
#if !DEMO() // P.P. 08/24/04
#if DIAGNOSTICS() // cond added. P.P. 08/15/04
#endif // diagnostics. P.P. 08/15/04
#endif // !demo. P.P. 08/24/04
#endif // !MYDEBUG()
return Flow_error_flag; // restored. P.P. 04/30/04
}
//// end of diagnostics functions. P.P.

// new function added to measure flow. P.P. 11/17/02
#if FLOW()
double Get_Flow()
{
  return 1.0;
}

// new Get_Flow_S(int) function, for a sample flow. P.P. 05/23/04
double Get_Flow_S(short channel)
{
double flow_value = 0; // added. P.P. 08/11/03
//  #if MYDEBUG() || DEMO()  // added. P.P. 11/06/02
#if (MYDEBUG()&&(!DEMO()))  // changed. P.P. 08/24/04
 return 10.0;
#endif
  uint16 data_in;        // moved here. P.P. 06/13/11
  uint32 sum_data_in=0;  //

#if PMD_16FS() // case added. P.P. 06/13/11
  int max_meas_number = 1; // no averaging. P.P. 06/13/11
  int Gain = BIP10VOLTS;
  for(int i=0; i<max_meas_number; i++) { //
    My_cbAIn(CBWDEVICE_16FS, 7, Gain, (USHORT*)&data_in);
    sum_data_in += data_in;  //
    data_in = (uint16) sum_data_in;
  }
#endif // pmd
#if Get_Data_Bipolar()   //added. P.P. 06/29/02
  const double vmin=-10.0; // gain=1
  const double vmax=+10.0;
  double voltage = data_in*((vmax-vmin)/65536.0) +vmin; // P.P. 07/03/02. 0 in the middle.
#else
  double voltage = (10.0*data_in)/65536.0;   // P.P 05/12/01
#endif
  double A0;                           // added. P.P. 08/24/04
  double exp_mul;                      //
  extern double A0_value_S;    // added instead. P.P. 08/27/04
  extern double exp_value_S;
  if(A0_value_S!=0) {
    A0=A0_value_S;
    exp_mul=exp_value_S;
  }
  else {                               //
    A0 = 4.048;       // new calibration. P.P. 06/18/11
    exp_mul = 0.9151; //
  }                                    //
  if(voltage>100.0) {   // changed for 12 V supply. P.P. 08/22/04
    return 9999;
  }
  else if(voltage>1.0) {                 // flow=0 at 1 V; Vmax=5V. P.P. 06/18/11
    return (voltage*27.098-30.95)*0.4037+1.4658;  // P.P. 08/24/04
  }
  else {
    return 0;
  }
}
// end of Get_Flow_S function. P.P. 05/23/04

// end of flow measurement function.

// new function added to write flow to an out. P.P. 05/07/04
static void Send_Flow_to_DAC(double FlowValue)
{
//#if DEMO()||MYDEBUG()||GC312() // P.P. 04/02/06
#if DEMO()||MYDEBUG()||GC312()||1 // P.P. 12/08/18  disabled.
   return;
#endif
 int flow_DAC_chan = 5;
 const unsigned int Max_DAC_Value = 4095;  //for 12-bit DAC, unsigned. P.P.
// double Max_flow = pDetector->AnalogRange;    // rem. P.P. 05/08/04
//  if(Max_flow<=0) Max_flow = 1.0; // avoid /0 //
  const double Max_flow = 100.0;  // temp to test. Left this way. P.P. 05/08/04
  if(FlowValue > Max_flow) // the same as for concen. P.P. 05/07/04
    FlowValue = Max_flow;
  else if(FlowValue < 0) FlowValue = 0; // P.P. 05/08/04
  double DAC_Value = ((1000.00*FlowValue)/Max_flow)*Max_DAC_Value;
  int data_value = (int) (DAC_Value/1000.00);
  if(data_value > Max_DAC_Value) data_value = Max_DAC_Value; // added P.P. 12/12/02
// add the board and write to it, if needed. P.P. 09/01/18
}
#endif // flow, moved from above. P.P. 08/15/04
#endif /// whhere it started? find out! P.P. 09/01/18


#if TOTAL_SUMMARY()
#include "tot_prn.h"
#else
#include "summ_prn.h"
#endif
#include "peak_app.h"
    long CreateFile(Job *pJob, int num, BOOL bAppend, UINT cf, const char *file);

    void SaveShiftReport(Job *pJob, int num)
    {
        TRACE("SaveShiftReport");
        static char file[80];
        pJob->MakeName(file, ".shf");

        long l=CreateFile(pJob, num, TRUE, CF_TEXT, file);
        if(l<=0) CreateFile(pJob, num, FALSE, CF_TEXT, file); // if cannot append - create file
        TRACE("SaveShiftReport end");
    }
    void PrintShiftReport(MainWindow *pMainWindow, Job *pJob, int num)
    {
        TRACE1("PrintShiftReport %d", num);
        PeakApp *pApp=((PeakApp *)pMainWindow->GetApplication());
        Printer *pPrinter=pApp;
        if( pPrinter&&pJob ) {
            TRACE("PrintShiftReport new SummaryPrintout");
#if TOTAL_SUMMARY() //??
            TotalPrintout *pPrintout = new TotalPrintout(pApp->Name, pJob, num);
#else
            SummaryPrintout *pPrintout = new SummaryPrintout(pApp->Name, pJob, num);
#endif
            if( pPrintout ) {
                TRACE("PrintShiftReport before Print");
                pPrinter->Print(pMainWindow, pPrintout, FALSE, FALSE);
                TRACE("PrintShiftReport after Print");
                delete pPrintout;
            }
        }
        TRACE("PrintShiftReport end");
    }

// Modified function CheckShiftReportTime. P.P. 11/14/03
    void CheckShiftReportTime(MainWindow *pMainWindow, int nAlarm)
    {
        TRACE1("CheckShiftReportTime nAlarm=%d", nAlarm);
        static BOOL bInUse=0;
        if(bInUse) return; // In Print Dialog? or some another error?
        bInUse=TRUE; //

//  static DWORD dwLastShift=0;
        Job *pJob=(Job *)SendMessage(pMainWindow->HWindow,WM_GET_JOB,0,0L);
        Method *pMethod=(Method *)pJob;

        for(int n=0; n<NUMDETECTORS; n++) {
            Detector *pDetector=&pMethod->det[n];
            if(pMethod->det[n].detect_stat == 0) continue; // active detector??

            int num=n;

            switch(shiftMode.SaveReport) {
                case 0: break;
                case 1:
                    if(nAlarm==2) SaveShiftReport(pJob, num);  // end of shift only. P.P. 11/14/03
                    break;
                case 2:
                    if(nAlarm) SaveShiftReport(pJob, num);     // end of shift or alarm. P.P. 11/14/03
                    break;
            }

// added to print every time the run finishes. P.P. 10/13/03
            if(shiftMode.bPrintAlarm) {
                PrintShiftReport(pMainWindow, pJob, num);
            }
            else {
// end of added. P.P. 10/13/03
                switch(shiftMode.PrintReport) {
                    case 0: break;
                    case 1:
                        if(nAlarm==2) PrintShiftReport(pMainWindow, pJob, num); // end of shift only. P.P. 11/14/03
                        break;
                    case 2:
                        if(nAlarm) PrintShiftReport(pMainWindow, pJob, num); // shift end or alarm. P.P. 11/14/03
                        break;
                }
            } // added. P.P. 10/13/03
        }

        bInUse=FALSE;
        TRACE("CheckShiftReportTime end");
    }

// added for exit from the Wpeak. P.P. 05/30/04
    void StartInternal(BOOL on)
    {
//  #if MYDEBUG()
#if MYDEBUG()||DEMO()  // changed. P.P. 08/24/04
        return;
#endif
        int port_value;
        if(on) {
            port_value = 0xFF;
        }
        else {
            port_value = 0x00;
        }
#if PMD_12LS()||PMD_12FS()||CBWUSB_IO() // initialize portA to LOW. P.P. 03/13/18
        int ret = 0;
//  My_cbDOut(CBWDEVICE_12LS, FIRSTPORTA, 0x00);
#if PMD_16FS() // added, 2 boards together, no relay boards. P.P. 11/25/06
#if PMD_12LS() // cases separated. 07/27/08
    My_cbDOut(CBWDEVICE_12LS, FIRSTPORTA, 0x00); // all relays off. P.P. 11/25/06
    My_cbDOut(CBWDEVICE_12LS, FIRSTPORTB, 0x00); // all relays off. P.P. 12/30/09
    ret = My_cbAOut(CBWDEVICE_12LS, 0, UNI5VOLTS, (USHORT) 0x0000); // for 10-bit 12LS, 0-5 V range. P.P. 06/17/05
    ret = My_cbAOut(CBWDEVICE_12LS, 1, UNI5VOLTS, (USHORT) 0x0000); // for 10-bit 12LS. P.P. 06/30/05
#elif CBWUSB_IO()  // case added P.P. 03/13/18
    My_cbDOut(USB_IO, FIRSTPORTA, 0x00); // all relays off. P.P. 11/25/06
  My_cbDOut(USB_IO, FIRSTPORTB, 0x00); // all relays off. P.P. 06/18/11
  My_cbDOut(USB_IO, FIRSTPORTCL, 0x00); // all relays off. P.P. 03/13/18
  My_cbDOut(USB_IO, FIRSTPORTCH, 0x0C); // bits 2 and 3in port CH hight to get HV A and B OFF ar start. P.P. o8/24/18
#elif PMD_12FS()
  My_cbDOut(CBWDEVICE_12FS, FIRSTPORTA, 0x00); // all relays off. P.P. 11/25/06
//#if CONCENTRATOR() // then PORTB is output. P.P. 12/30/09      // only outputs, configured for all the cases. P.P. 09/01/18
  My_cbDOut(CBWDEVICE_12FS, FIRSTPORTB, 0x00); // all relays off. P.P. 12/30/09
//#endif // concentrator
  ret = My_cbAOut(CBWDEVICE_12FS, 0, UNI4VOLTS, (USHORT) 0x0000); // for 10-bit 12FS, 0-4 V range. P.P. 06/17/05
  ret = My_cbAOut(CBWDEVICE_12FS, 1, UNI4VOLTS, (USHORT) 0x0000); // for 10-bit 12FS. P.P. 06/17/05
#endif // 12LS or 12FS
//  My_cbDOut(CBWDEVICE_12LS, FIRSTPORTB, 0x00); // added for 301-4pnt, for points. P.P. 02/19/07 rem 05/25/07
    int u = 0;
    for(u=0; u<8; u++) {
#if PMD_12LS()         //  P.P. 12/18/18
        if((u!=5)) {  // this is input
#elif CBWUSB_IO()
            if((u<4)||(u>5)) {  // this are inputs
#else
    if(1) {
#endif
            My_cbDBitOut(CBWDEVICE_16FS, AUXPORT, u, 0); // all outputs = 0 on start P.P. 08/24/18
        }
    }
#else // without 16_fs board                 // not be ever but left. P.P. 09/01/18
    #if PMD_12LS() // cases separated. 07/27/08
  My_cbDOut(CBWDEVICE_12LS, FIRSTPORTA, 0x12); // changed - HV=high for lampA and lampB off. P.P. 07/07/06
  ret = My_cbAOut(CBWDEVICE_12LS, 0, UNI5VOLTS, (USHORT) 0x0000); // for 10-bit 12LS, 0-5 V range. P.P. 06/17/05
  ret = My_cbAOut(CBWDEVICE_12LS, 1, UNI5VOLTS, (USHORT) 0x0000); // for 10-bit 12LS. P.P. 06/30/05
#elif PMD_12FS() // added. 07/27/08
  My_cbDOut(CBWDEVICE_12FS, FIRSTPORTA, 0x12); // changed - HV=high for lampA and lampB off. P.P. 07/07/06
  ret = My_cbAOut(CBWDEVICE_12FS, 0, UNI4VOLTS, (USHORT) 0x0000); // for 10-bit 12FS, 0-4 V range. P.P. 06/17/05
  ret = My_cbAOut(CBWDEVICE_12FS, 1, UNI4VOLTS, (USHORT) 0x0000); // for 10-bit 12FS. P.P. 06/17/05
#elif CBWUSB_IO() // case added. P.P. 03/13/18
  My_cbDOut(USB_IO, FIRSTPORTA, 0x00);
  My_cbDOut(USB_IO, FIRSTPORTB, 0x00);
  My_cbDOut(USB_IO, FIRSTPORTCL, 0x00);
  My_cbDOut(USB_IO, FIRSTPORTCH, 0x0C); // changed - HV=high for lampA and lampB off. P.P. 08/24/18
#endif
#endif // PMD_16FS. 07/27/08
#endif // pmd_12, LS or FS, and usb-io. P.P. 06/17/05
#if USB_1024LS()||USB_DIO24() // added as an alternative to 12LS_2 board. 07/13/07
        My_cbDOut(CBWDEVICE_1024, FIRSTPORTA, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024, FIRSTPORTB, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024, FIRSTPORTCL, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024, FIRSTPORTCH, 0x00); // added to initialize to OFF
#if USB_1024LS_2()||USB_DIO24_2() // added for alarms in 12-pnt gc301c. 11/06/07
  My_cbDOut(CBWDEVICE_1024_2, FIRSTPORTA, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024_2, FIRSTPORTB, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024_2, FIRSTPORTCL, 0x00); // added to initialize to OFF
  My_cbDOut(CBWDEVICE_1024_2, FIRSTPORTCH, 0x00); // added to initialize to OFF
#endif // 1024_2 or dio24_2
#endif // 1024 r dio24

// initialization of PCI24 and PCI48 boards added. P.P. 06/17/05
// end of PCI 24/48 initialization. P.P. 06/17/05
#if TCD() // cond added. P.P. 08/15/04
        #if !GC312()  // cond added. TCD in gc312 is as lamp B. P.P. 02/20/09
  Acquire::TCDDetector(on);           // TCD is ON on start of Wpeak, off on exit.
#endif // gc312
#endif // tcd
#if INTERNAL()
        if(!on) {
//    Acquire::SetLamps(FALSE); // added. P.P. 06/21/06
    }
#endif // internal
#if MANUAL_INJ()// added. P.P. 06/27/06
        #if PMD_16FS()
  My_cbDOut(CBWDEVICE_16FS, AUXPORT, 0x00);  // all do off. P.P. 06/27/06
#endif // pmd-16fs
#endif // manual_inj
#if USB_3101()||USB_3103()||USB_3106()||USB_3112()
        int UL_Stat=0;
#if USB_3101()
  My_cbDOut(CBWDEVICE_3101, AUXPORT, 0x00);  // all off.
  for(int i=0; i<4; i++) {
    UL_Stat=My_cbAOut(CBWDEVICE_3101, i, UNI10VOLTS, 0x0000); // for 16-bit 1range. P.P. 05/20/07
  }
#elif USB_3103()
  My_cbDOut(CBWDEVICE_3103, AUXPORT, 0x00);  // all off.
  for(int i=0; i<8; i++) {
    UL_Stat=My_cbAOut(CBWDEVICE_3103, i, UNI10VOLTS, 0x0000); // for 16-bit 1range. P.P. 05/20/07
  }
#elif USB_3106()
  My_cbDOut(CBWDEVICE_3106, AUXPORT, 0x00);  // all off.
  for(int i=0; i<16; i++) {
    UL_Stat=My_cbAOut(CBWDEVICE_3106, i, UNI10VOLTS, 0x0000); // for 16-bit 1range. P.P. 05/20/07
  }
#elif USB_3112() // dig bits were used for input in one of old variants. P.P. 05/20/07
  for(int i=0; i<8; i++) {
    UL_Stat=My_cbAOut(CBWDEVICE_3112, i, UNI10VOLTS, 0x0000); // for 16-bit 1range. P.P. 05/20/07
  }
#endif // one of usb AO boards
#endif // usb analog output boards
    }

// end of acquire.cpp














