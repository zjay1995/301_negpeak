#include "includes.h"
#pragma hdrstop

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmd, int nCmdShow)
{
  PeakApp app("wPeak", hInstance, hPrevInstance,
		lpCmd, nCmdShow);
  app.Pref.Read();
  if(app.Ctl3dAvailable())
    app.EnableCtl3d(app.Pref.bUse3dControls);
  app.EnableCtl3dAutosubclass(TRUE);
  app.Run();
  app.EnableCtl3dAutosubclass(FALSE);
  return app.Status;
}


