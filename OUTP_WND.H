#ifndef  _OUTP_WND_H
#define  _OUTP_WND_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

PTWindowsObject newOutputWindow(PTWindowsObject AParent, PTModule AModule=NULL);

#endif
