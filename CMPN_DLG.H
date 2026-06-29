#ifndef  _CMPN_DLG_H
#define  _CMPN_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

class Method;

PTWindowsObject newComponentsDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, PTModule AModule = NULL);

#endif
