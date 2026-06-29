#ifndef  _ABOUT_H
#define  _ABOUT_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);


TWindowsObject *newAboutDialog(TWindowsObject *AParent, int ResourceId, TModule *AModule=NULL);


#endif
