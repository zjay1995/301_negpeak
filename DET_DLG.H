#ifndef  _DET_DLG_H
#define  _DET_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);


class Detector;

PTWindowsObject newDetectorDlg(PTWindowsObject AParent, int ResourceId, char *Name, Detector *pDetector, int num, PTModule AModule = NULL);

#endif
