#ifndef  _DETECTOR_H
#define  _DETECTOR_H

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
class WCPPLIB_CLASS iwstream;
class WCPPLIB_CLASS owstream;

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#ifndef  _CONSTS_H
#include "consts.h"
#endif

#ifndef  _CMPONENT_H
#include "cmponent.h"
#endif
class Peak;
class Method;
class Run;

class Detector {
 public:
  Str Label;

  ulong NandBtime;	// Noise And Baseline
  ulong NandBlen;
  int peak_alg;	// 0 - baseline projection, 1 - Tangent Skimming, 2 - Optimization
  int detect_meth;	// 0 - height ; 1 - area
  int gain;		// initial gain for GC 311
  int gain_1;           // gain for specified time interval. P.P. 11/09/12
  int auto_gain_on_off; // auto change gain
  int plot_atten;	// plot attenuation for GC 311
  int detect_stat;	// 1 - on ; 0 - off
  ulong MinArea;	// Min. Area
  ulong MinHeight;	// Min. Height
//  ulong AnalogRange;    // 4-20 mA max concentration, at 20 mA. P.P. 03/19/02
  ulong AnalogRange[COMPONENT_ROWS];    // 4-20 mA max concentration, at 20 mA. P.P. 04/22/07
  uint  segment_width;  // For scaled_sample ( average )
  Str units;	// PPM/PPB
  int lamp_save; // lamp save mode
  int uv_type;          // P.P. 10/11/02
  int known_peaks; // P.P. 09/17/05
  int ext_det;     // P.P. 11/14/08
  int invert;      // P.P. 03/25/09 here 11/06/09
  int data_to_excel; // P.P. 02/24/12
  Component components[COMPONENT_ROWS];  // components form & calibration standards
  int FindRefCompIndex();
  Component *FindRefComp();

  Detector();
  ~Detector();

  void *read(iwstream &is, unsigned long version);
  void write(owstream &os);

  int Concentration(Method *pMethod, Run *pRun, Peak *pPeak, int nDetector, double *pConcentration);
};

#endif
