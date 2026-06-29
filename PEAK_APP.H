#ifndef  _PEAK_APP_H
#define  _PEAK_APP_H


#ifndef __OWL_H
#include <owl.h>
#endif

#ifndef  _WINUTIL_WPRINT_H
#include "winutil\wprint.h"
#endif

#ifndef  _PREF_H
#include "pref.h"
#endif

class PeakApp : public TApplication, public Printer {
  typedef TApplication Inherited;
 public:
  PeakApp(LPSTR AName, HINSTANCE hInstance,
		  HINSTANCE hPrevInstance, LPSTR lpCmd,
		  int nCmdShow);
  ~PeakApp();
  virtual void InitMainWindow();
  virtual void IdleAction();
  virtual void Error(int ErrorCode);

  virtual BOOL ExecPrintDialog(PTWindowsObject parent);

  Preferences Pref;

  BOOL Ctl3dAvailable() const;
  BOOL Ctl3dEnabled() const { return Ctl3dOn; }
  void EnableCtl3d(BOOL enable);
  void EnableCtl3dAutosubclass(BOOL enable);

  operator HINSTANCE(void) const { return hInstance; }
 protected:
  HMODULE Ctl3dModule;
  BOOL Ctl3dOn;
};


#endif
