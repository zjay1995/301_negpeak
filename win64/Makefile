# WPEAK64 -- 64-bit port of the GC301c WPEAK peak-detection core.
#
#   make test    - build the core natively (Linux) and run the self-test
#   make win64   - cross-compile the 64-bit Windows executables (MinGW-w64)
#   make all     - both
#
# Windows targets are statically linked so the .exe files run on a stock
# 64-bit Windows machine with no extra runtime DLLs.

CXX      ?= g++
WINCXX   ?= x86_64-w64-mingw32-g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra

CORE = integrator64.cpp method64.cpp
ACQ  = $(CORE) hw64.cpp acquire64.cpp ads1115.cpp gpio64.cpp runstore64.cpp
HDRS = peak64.h synth64.h method64.h hw64.h ads1115.h gpio64.h acquire64.h runstore64.h

all: test win64

test: build/wpeak64_cli build/wpeak64_acq
	./build/wpeak64_cli

build/wpeak64_cli: wpeak64_cli.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) wpeak64_cli.cpp $(CORE) -o $@

# acquisition/control tool: ADS1115 + GPIO on Linux, simulation everywhere
build/wpeak64_acq: wpeak64_acq.cpp $(ACQ) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) wpeak64_acq.cpp $(ACQ) -o $@ -lpthread

win64: build/wpeak64_cli.exe build/wpeak64.exe build/wpeak64_acq.exe

build/wpeak64_cli.exe: wpeak64_cli.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static wpeak64_cli.cpp $(CORE) -o $@

build/wpeak64_acq.exe: wpeak64_acq.cpp $(ACQ) $(HDRS)
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static wpeak64_acq.cpp $(ACQ) -o $@

build/wpeak64.exe: wpeak64_gui.cpp $(ACQ) $(HDRS)
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static -mwindows wpeak64_gui.cpp $(ACQ) -o $@ -lgdi32 -luser32 -lcomdlg32

clean:
	rm -rf build

.PHONY: all test win64 clean
