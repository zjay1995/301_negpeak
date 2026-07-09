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

Negative peaks are detected and displayed but excluded from quantitation
(negative `Height`, no peak number) — same policy as the GC301c build.

## What is NOT ported

The Borland OWL GUI (≈40 windows/dialogs), hardware acquisition
(Measurement Computing USB DAQ, relays, valves, temperature control),
calibration/component tables, Modbus, TWA/STEL reporting, and file I/O.
Those layers are tied to Win16/32-era Borland C++ and physical instrument
hardware; a full port would be a separate project.

## Programs

- **`wpeak64.exe`** — native Win32/GDI (64-bit) chromatogram viewer: runs the
  integrator on a built-in synthetic run and draws the trace, baseline,
  numbered positive peaks and `NEG`-marked negative peaks with a peak table.
- **`wpeak64_cli.exe`** — console version printing the peak table; its exit
  code doubles as a self-test (expects 3 positive + 1 negative peak).

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
