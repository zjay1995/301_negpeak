#ifndef  _RUNL_WND_H
#define  _RUNL_WND_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

PTWindowsObject newRunListWindow(PTWindowsObject AParent, PTModule AModule=NULL);

#endif
