#ifndef  _RUNL_DLG_H
#define  _RUNL_DLG_H

#ifndef ___DEFS_H
#include <_defs.h>		// _CLASSDEF
#endif
_CLASSDEF(TWindowsObject);
_CLASSDEF(TModule);

class Job;

#ifndef  _IND_H
#include "ind.h"
#endif

struct RunListDialogInfo {
  int bMul;
  Job *pJob;
  Indexes ind;
};

PTWindowsObject newRunListDialog(PTWindowsObject AParent, int ResourceId, RunListDialogInfo *pInfo, PTModule AModule = NULL);


PTWindowsObject newStandardListDialog(PTWindowsObject AParent, int ResourceId, int *nStandard, PTModule AModule=NULL);

#endif
