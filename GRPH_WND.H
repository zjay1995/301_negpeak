#ifndef  _GRPH_WND_H
#define  _GRPH_WND_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

PTWindowsObject newGraphWindow(PTWindowsObject AParent, int num, LPSTR ATitle, PTModule AModule = NULL);
PTWindowsObject newRunGraphWindow(PTWindowsObject AParent, LPSTR ATitle, PTModule AModule = NULL);

#endif

