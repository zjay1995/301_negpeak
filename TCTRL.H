#ifndef  _TCTRL_H
#define  _TCTRL_H

class Method;
class TempControl
{
 public:
  TempControl(Method *_pMethod);
  ~TempControl();
  enum {
    T_INIT  = 0,
    T_HOLD1 = 1,
    T_RAMP1 = 2,
    T_HOLD2 = 3,
    T_RAMP2 = 4,
    T_HOLD3 = 5
  } mode;
  Method *pMethod;

  void Start();
  void ControlTemperature(); //

 protected:
  void OutputTemperatures(void); // output temp to DAC board

  void Begin(); // ctor
  void End(); // dtor
  void SetupNextMode(void);
  void ReadTemperatures(void);

  double  t_inj;            /* Inj temperature to be sent to DAC. */
  double  t_oven;           /* Oven temperature to be sent to DAC. */

  LONG    switch_time;      /* Total time (in ticks) for a given mode. */
  double  t_start;          /* Temperature at the start of a mode. */
  double  rate;             /* Temperature ramping rate. */
  int   half_period_term; //??  /* To switch temp on the 1/2 period(3/2,5/2)*/
  DWORD mode_start_time;  // Time at start of mode in counts
  DWORD last_time;
};

#endif