#ifndef  _METH_DLG_H
#define  _METH_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

class Method;
class Preferences;
PTWindowsObject newMethodDlg(PTWindowsObject AParent, int ResourceId, Method *pMethod, Preferences &Pref, PTModule AModule = NULL);

#endif
