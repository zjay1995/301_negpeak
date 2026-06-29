#ifndef  _PREF_DLG_H
#define  _PREF_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);


class Preferences;

PTWindowsObject newPreferencesDialog(PTWindowsObject AParent, Preferences *pPreferences, PTModule AModule = NULL);

#endif
