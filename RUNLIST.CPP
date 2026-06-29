#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _RUN_H
#include "run.h"
#endif

#ifndef  _RUNLIST_H
#include "runlist.h"
#endif

#include "debug.h"

RunList::RunList()
{
  //TRACE1("RunList=%p",this);
}

RunList::~RunList()
{
  //TRACE1("~RunList=%p",this);
}

void *RunList::read(iwstream &is, unsigned long version)
{
  return this;
}

void RunList::write(owstream &os)
{
}
