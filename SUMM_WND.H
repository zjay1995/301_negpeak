#ifndef  _SUMM_WND_H
#define  _SUMM_WND_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

PTWindowsObject newSummaryWindow(PTWindowsObject AParent, int num, char far *ATitle, PTModule AModule = NULL);

#endif
