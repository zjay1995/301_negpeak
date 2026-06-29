#ifndef  _MAIN_WND_H
#define  _MAIN_WND_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);
class Preferences;
PTWindowsObject newMainWindow(char far *ATitle, int MenuId, LPSTR HelpFile, Preferences &Pref, PTModule AModule = NULL);

#endif

