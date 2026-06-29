#include "includes.h"
#pragma hdrstop

#ifndef  _REPORT_H
#include "report.h"
#endif

Report::Report()
// : bMethod(1), bDetectors(0), bComponents(0), bStandards(0),
 : bMethod(1), bDetectors(1), bComponents(0), bStandards(0),    // detector info added. P.P. 08/18/17
 bRunList(0), bGraph(1), bTable(1),
 bSTL(0), bTWA(0), num(0)
{
}

