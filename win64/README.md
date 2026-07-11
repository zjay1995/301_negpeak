# WPEAK64 — 64-bit Windows port of the GC301c peak-detection core

A modern C++17 port of the analytical core of WPEAK 2.4.43 (the GC301c
Process Gas Chromatograph software), buildable for **64-bit Windows** with
MinGW-w64 and natively on Linux for testing.

## What is ported

| Legacy source | Ported to | Contents |
|---------------|-----------|----------|
| `PEAK.H` | `peak64.h` | `Peak` record |
| `RAW_DATA.CPP` | `integrator64.cpp` | `DataQueue` segment averaging, sample scaling |
| `CALC.CPP` / `CALC.H` | `peak64.h` / `integrator64.cpp` | Noise & baseline estimation and the full positive-peak recognition state machine (`BETWEEN_PEAKS` → … → `END_OF_PEAK`), tangent skimming (`peak_alg==1`), MinArea/MinHeight gating, **and the negative-peak detector added to the main algorithm** |
| `METHOD.H` / `CMPONENT.H` / `CALC.CPP` | `method64.h` / `method64.cpp` | Method (detector settings + component table) as an INI file, `CheckRT` retention-time matching, `PeakMatchup` first-match/`known_peaks` semantics, concentrations from response factors (height or area per `detect_meth`), CSV report export |

Negative peaks are detected and displayed but excluded from quantitation
(negative `Height`, no peak number) — same policy as the GC301c build.

## Features

- **Chromatogram import**: CSV, one sample per line (`value` or `time,value`;
  header/comment lines tolerated). Timing comes from the method's
  `data_rate`, as in the original raw-data files.
- **Method files** (`*.ini`): detector settings (`segment_width`,
  `nandb_time/len`, `min_height`, `min_area`, `peak_alg`, `noise_reduct`,
  `detect_meth`, `known_peaks`, `data_rate`, `analysis_time`) plus a
  `[component]` table (`name`, `rt`, `window`, `response`, `active`).
  See `sample_method.ini`.
- **Component identification**: legacy `CheckRT` rule — a peak matches a
  component when |RT − component RT| ≤ window; first active match wins;
  `known_peaks=1` drops unidentified peaks from the report.
- **Concentrations**: `response × height` or `response × area` per
  `detect_meth`; peaks without a response factor report `n/cal`.
- **Report export**: CSV via `-o` (CLI) or File → Export Report (GUI).
- `sample_run.csv` + `sample_method.ini` form a working example
  (3 identified positive peaks + 1 negative peak).

## Data acquisition & instrument control (`wpeak64_acq`)

The legacy Measurement Computing DAQ layer is replaced with commodity
hardware, abstracted in `hw64.h`:

- **ADS1115** 16-bit I2C ADC (`ads1115.cpp`) for the detector signal and
  temperature sensors — Linux `i2c-dev`, register-level single-shot reads,
  configurable PGA full-scale range and data rate, with **dynamic gain
  ranging** per channel (`gainrange.h`): a saturated reading re-reads at a
  coarser range before it's ever returned, a weak one steps to a finer
  range for the next sample, and every count is normalized back to the
  method's configured reference range — the ADS1115 equivalent of the
  legacy `pow10_array`/`cur_autoscale` autorange normalization, so a
  mid-run range change is invisible to the integrator. `wpeak64_acq
  monitor` shows the active range (`[4096mV]` etc.) alongside each reading.
- **GPIO lines** (`gpio64.cpp`, Linux gpiochip character device) for the
  relays/valves/heaters/lamp/pump the legacy `Set_valve()` bits drove.
- **Simulation backend** (`backend=sim`, works on Windows and Linux):
  modeled detector signal (incl. a negative peak) and first-order thermal
  zones, so methods and calibrations can be exercised without hardware.
  Real hardware (`backend=ads1115`) requires a Linux controller (e.g. a
  Raspberry Pi wired to the GC).

`acquire64.cpp` runs the legacy sequence: **EQUILIBRATE** (heaters under
bang-bang hysteresis control, wait for all zones in band) → **SAMPLE**
(pump + sample/cal valve) → **INJECT** (injection valve + lamp) →
**ANALYZE** (ADC sampled at `data_rate`, each point fed live to the
integrator) → **PURGE**. Hardware pins, temperature zones and phase times
come from the method file (`[hardware]`, `[tempzone]`, `[timing]`).

```sh
wpeak64_acq -m method.ini monitor -t 30          # watch ADC + temperatures
wpeak64_acq -m method.ini cal -s 1 -C cal.ini    # run calibration standard 1
wpeak64_acq -m method.ini run -p 2 -C cal.ini -j jobdir   # one run, point 2
wpeak64_acq -m method.ini continuous -C cal.ini -j jobdir # scheduled operation
wpeak64_acq history -j jobdir                    # print the run history
wpeak64_acq twa -j jobdir                        # TWA / STEL exposure report
# --sim / --sim-scale X : force the simulated instrument
```

An optional detector **autozero** (legacy `Acquire::AutoZero`) runs between
equilibration and sampling: set `autozero=<gpio>` in `[hardware]` and
`autozero_time=<s>` in `[timing]`.

## Detector B (dual detector)

Legacy `NUMDETECTORS=2`: a second, fully independent detector — its own
segment width/noise window/MinHeight/MinArea/peak algorithm, `detect_meth`,
component table and ADC channel — sampled in the same ANALYZE loop as
detector A (`analysis_time`/`data_rate` are shared; everything else is
separate). Enable it with `[detector_b]` + `enabled=1` + `adc_channel=`,
and a `[component_b]` table (same keys as `[component]`). Every path that
handles detector A's results also handles detector B's: run output prints
a second "Detector B" peak table, `history` shows a `DetB` summary column,
`SaveRun` writes `trace_b.csv`/`report_b.csv` alongside the normal per-run
files, and calibration runs update `[component_b]` standards the same way
`-s N` updates `[component]`. The GUI Settings dialog has an "Enable
Detector B" checkbox with its ADC channel/min height/detect method, and
the status bar shows its live peak count/alarm state.

## Analog concentration outputs (DAC)

Legacy `Write_conc_to_DAC` / USB-3106-class analog output boards, ported
onto commodity MCP4725 12-bit I2C DACs (`mcp4725.cpp`, one chip per output
channel — Linux `i2c-dev`, same pattern as the ADS1115 driver). Configure
`dac_i2c_addrs=<addr,addr,...>` and `dac_vref=<V>` in `[hardware]`, then
give a component `dac_channel=<index>` (into that list) and `dac_range=
<conc>` (the concentration that maps to `dac_vref` at the DAC). After every
run, each identified/calibrated component with a DAC channel gets written
0 V at zero concentration up to `dac_vref` at `dac_range` (clamped), for
both detector A and B. `wpeak64_acq monitor` shows the last-written volts
per channel; the GUI Element Table editor has DAC Channel/DAC Range
columns next to the alarm limits.

## Remote monitoring (Modbus TCP)

The legacy `MODBUS.CPP`/`GetModBusRegister` was a serial Modbus ASCII slave
(function codes 0x03/0x04 only) packing each component's concentration as a
32-bit float across two registers. This port serves the same register
layout over standard **Modbus TCP** (`modbus64.cpp`) instead, since that's
what SCADA/PLC integrators expect today: `--modbus-port N` on `run`, `cal`,
`continuous` or `monitor` starts a listener on port `N` and updates the
register table after every completed run (function codes 3 "Read Holding
Registers" and 4 "Read Input Registers" answer identically, as the legacy
slave did). Registers are packed two-per-component, high word first, in
method order — detector A's components first, then detector B's when
`det_b_enabled` — so `register[2*i]:register[2*i+1]` is component `i`'s
current concentration (0.0 for unmatched/negative/uncalibrated peaks):

```sh
wpeak64_acq -m method.ini continuous -j jobdir --modbus-port 502
# from a Modbus TCP master, e.g.:
mbpoll -m tcp -a 1 -t 4:float -r 1 -c 2 127.0.0.1 -p 502
```

## TWA / STEL

`wpeak64_acq twa -j jobdir` computes the exposure report over the stored
run history (legacy `TWA_results` / `STEL_results`): per sample point and
component the time-weighted average (mean over runs), min/max, and STEL —
the highest mean over any window of up to 15 consecutive runs (legacy
`STEL_RUNS`). Calibration runs are excluded.

## Run persistence & history

With `-j jobdir` every acquisition is stored (replacing the legacy binary
.job file): `jobdir/run_NNNNNN/` holds `meta.ini` (type, point, standard,
noise/baseline, timestamp, alarm state), `trace.csv` (reloadable with
`wpeak64_cli -d` or the GUI) and `report.csv`; `jobdir/runlist.csv` is the
append-only run history shown by the `history` mode.

## Alarms & relay outputs

Per-component H/L concentration limits (`alarm_high` / `alarm_low` in
`[component]`, legacy GetAlarmFlags semantics). Alarm state is shown in
reports, the GUI (red rows) and the run history, and drives the common
`alarm_high_line` / `alarm_low_line` relays; alarms hold between runs and
clear when a run returns in-limits. Negative and uncalibrated peaks never
alarm.

## Scheduling (continuous mode) & multipoint

`continuous` cycles every sample point of the manifold, pauses
`repeat_interval` seconds between cycles, and automatically inserts a
calibration run (standard `auto_cal_standard`) every `auto_cal_every`
runs — the legacy Continuous/Repeat modes with scheduled auto-cal.
The manifold is configured with `point_valves=<gpio,gpio,...>` in
`[hardware]` (one point-select valve per sample point, used in place of
`sample_valve` during SAMPLE); reports and history are tagged with the
point number. Ctrl+C stops cleanly between phases.

## Calibration tables

Multi-standard calibration with the legacy semantics (`CalcConcVars` /
`Detector::Concentration`): each component holds up to 8 standards; the
concentration-vs-response curve is piecewise linear through the sorted
standards, with the segment below the first standard anchored at the
origin and responses above the last standard extrapolated on the line
through the origin and that standard. Standard concentrations live in the
method (`std1=`…`std8=` per `[component]`); measured responses live in a
calibration file updated by `wpeak64_acq cal -s N` and used by every
report (`-C cal.ini`, CLI and acquisition alike). Components without a
calibration table fall back to the fixed `response` factor. Negative
peaks never calibrate or quantify.

## What is NOT ported

The Borland OWL GUI (≈40 windows/dialogs, replaced here with a single
native Win32/GDI window) and the legacy binary job/method file formats
(replaced with the INI/CSV formats documented above) are not byte-for-byte
ported — those layers are tied to Win16/32-era Borland C++, and a faithful
reproduction would be a separate project. Functionally-equivalent modern
replacements exist for everything else described in this document
(Modbus TCP instead of the legacy serial ASCII slave, MCP4725 I2C DACs
instead of the legacy USB analog output boards, etc.).

## Programs

- **`wpeak64.exe`** — native Win32/GDI (64-bit) GUI with three windows:
  the chromatogram viewer (trace + baseline, numbered peaks with component
  names, `NEG` markers, peak/concentration/alarm table), a live
  **acquisition window** (View > Acquisition: current phase, zone
  temperatures, growing trace while a run executes), and the **element
  table** (View > Element Table: components with RT windows, response
  factors, alarm limits, calibration points, latest concentrations and
  alarm states). The Run menu starts a run or a calibration on a
  background thread (simulation backend unless the method configures real
  hardware). Command line: `wpeak64.exe [run.csv] [method.ini] [/autorun]`.
- **`wpeak64_cli.exe`** — console version:
  `wpeak64_cli [-d run.csv] [-m method.ini] [-o report.csv]`. With no
  arguments it runs the synthetic demo and its exit code doubles as a
  self-test (expects 3 positive + 1 negative peak).

Both are statically linked — no runtime DLLs needed on a stock 64-bit
Windows 10/11 machine.

## Building

```sh
make test    # native Linux build + algorithm self-test
make win64   # cross-compile the two Windows executables (needs g++-mingw-w64-x86-64)
```

On Windows itself, any x86_64 MinGW-w64 distribution (e.g. MSYS2:
`pacman -S mingw-w64-x86_64-gcc make`) builds the same targets.

## Verified

- Native self-test: baseline 999/1000 recovered, peaks at 40/60/105 s found
  with correct heights, negative peak at 85 s detected as `Height=-298`,
  not quantified.
- `wpeak64_cli.exe` (PE32+ x86-64) produces identical output under Wine.
- `wpeak64.exe` GUI renders the chromatogram with peak markers (verified
  under Wine + Xvfb screenshot).
