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
  configurable PGA full-scale range and data rate.
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
wpeak64_acq -m method.ini run -C cal.ini -o report.csv -D trace.csv
# --sim / --sim-scale X : force the simulated instrument
```

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

The Borland OWL GUI (≈40 windows/dialogs), Modbus, TWA/STEL reporting,
and the legacy binary job/method file formats. Those layers are tied to
Win16/32-era Borland C++; a full port would be a separate project.

## Programs

- **`wpeak64.exe`** — native Win32/GDI (64-bit) chromatogram viewer: File menu
  (Open Data CSV, Open Method INI, Export Report CSV), trace + baseline,
  numbered positive peaks labelled with their matched component names,
  `NEG`-marked negative peaks, and a peak/concentration table. Optional
  command line: `wpeak64.exe [run.csv] [method.ini]`. Starts with a built-in
  synthetic demo when no files are given.
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
