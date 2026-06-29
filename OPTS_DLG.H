#ifndef  _OPTS_DLG_H
#define  _OPTS_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

struct OptionsSaveStruct {
  int bSaveEnvironment;
  int bSaveDesktop;
  int bSaveWindow;
};

PTWindowsObject newOptionsSaveDialog(PTWindowsObject AParent, OptionsSaveStruct *data, PTModule AModule = NULL);

#endif
