#ifndef  _STND_DLG_H
#define  _STND_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

class Method;
class PeakList;
PTWindowsObject newStandardsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PeakList **pPeakList, PTModule AModule = NULL);

#endif
