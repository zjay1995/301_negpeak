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
WINDRES  ?= x86_64-w64-mingw32-windres
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra

CORE = integrator64.cpp method64.cpp
ACQ  = $(CORE) hw64.cpp acquire64.cpp ads1115.cpp mcp4725.cpp gpio64.cpp runstore64.cpp
# Modbus TCP is only linked into wpeak64_acq (not the GUI, which has no need
# for it yet) since it needs -lws2_32 on the Windows build.
MODBUS = modbus64.cpp
HDRS = peak64.h synth64.h method64.h hw64.h ads1115.h mcp4725.h gpio64.h acquire64.h runstore64.h gainrange.h modbus64.h

all: test win64

test: build/wpeak64_cli build/wpeak64_acq build/gainrange_test build/modbus64_test
	./build/wpeak64_cli
	./build/gainrange_test
	./build/modbus64_test

# pure ADS1115 auto-ranging logic, no I2C/hardware needed (see gainrange.h)
build/gainrange_test: gainrange_test.cpp gainrange.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) gainrange_test.cpp -o $@

# Modbus TCP wire-protocol test: a client thread and Poll() in the same
# process, no wpeak64_acq/hardware timing involved (see modbus64_test.cpp)
build/modbus64_test: modbus64_test.cpp modbus64.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) modbus64_test.cpp modbus64.cpp $(CORE) -o $@ -lpthread

build/wpeak64_cli: wpeak64_cli.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) wpeak64_cli.cpp $(CORE) -o $@

# acquisition/control tool: ADS1115 + GPIO on Linux, simulation everywhere
build/wpeak64_acq: wpeak64_acq.cpp $(ACQ) $(MODBUS) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) wpeak64_acq.cpp $(ACQ) $(MODBUS) -o $@ -lpthread

win64: build/wpeak64_cli.exe build/wpeak64.exe build/wpeak64_acq.exe

build/wpeak64_cli.exe: wpeak64_cli.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static wpeak64_cli.cpp $(CORE) -o $@

build/wpeak64_acq.exe: wpeak64_acq.cpp $(ACQ) $(MODBUS) $(HDRS)
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static wpeak64_acq.cpp $(ACQ) $(MODBUS) -o $@ -lws2_32

# toolbar icons reused from the original Peak Works software (icons/*.bmp),
# embedded as Windows resources -- see wpeak64.rc
build/wpeak64_res.o: wpeak64.rc icons/open_data.bmp icons/save.bmp icons/help.bmp
	@mkdir -p build
	$(WINDRES) wpeak64.rc -O coff -o $@

build/wpeak64.exe: wpeak64_gui.cpp $(ACQ) $(HDRS) build/wpeak64_res.o
	@mkdir -p build
	$(WINCXX) $(CXXFLAGS) -static -mwindows wpeak64_gui.cpp $(ACQ) build/wpeak64_res.o \
		-o $@ -lgdi32 -luser32 -lcomdlg32 -lcomctl32

clean:
	rm -rf build

.PHONY: all test win64 clean
