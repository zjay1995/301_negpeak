// hw64.h -- hardware abstraction for WPEAK64 data acquisition.
//
// The legacy GC301c drove Measurement Computing USB DAQ boards (cbw.h) for
// the detector signal and relay/valve/heater bits. This port replaces them
// with:
//   - ADS1115 16-bit I2C ADC for analog inputs (detector + temperature
//     sensors), Linux i2c-dev backend (ads1115.cpp)
//   - GPIO lines for relays/valves/heaters/lamp/pump, Linux gpiochip
//     character-device backend (gpio64.cpp)
//   - a simulation backend (SimHardware) available on every OS, which
//     models the detector signal and first-order thermal zones, so methods
//     and calibrations can be exercised without the instrument.
//
// Real-hardware backends compile on Linux only (the ADS1115 is an I2C
// device; the controller is expected to be a Linux SBC such as a Raspberry
// Pi wired to the GC). On Windows the acquisition tool runs in simulation
// mode; analysis of acquired data files works everywhere.

#ifndef HW64_H
#define HW64_H

#include <string>
#include <vector>
#include <cstdint>

namespace wpeak64 {

// ---- interfaces -------------------------------------------------------------
class Adc {
public:
    virtual ~Adc() {}
    // Single-shot read of one single-ended channel (0..3), in raw ADC counts
    // (ADS1115: -32768..32767 at the configured PGA full-scale range).
    virtual long ReadCounts(int channel) = 0;
    // Volts for the same reading, using the configured full-scale range.
    virtual double CountsToVolts(long counts) const = 0;
    // Active PGA full-scale range in mV for this channel (post auto-ranging,
    // see gainrange.h), for diagnostics/monitor display. 0 = not applicable
    // (e.g. the simulation backend, which doesn't need gain ranging).
    virtual int CurrentRangeMv(int) const { return 0; }
};

class DigitalOut {
public:
    virtual ~DigitalOut() {}
    virtual void Set(int line, bool on) = 0;
    virtual bool Get(int line) const = 0;
};

// ---- configuration (parsed from the method file [hardware] section) ---------
struct HardwareConfig {
    std::string backend   = "sim";          // "sim" or "ads1115"
    // ADS1115
    std::string i2c_dev   = "/dev/i2c-1";
    int         i2c_addr  = 0x48;           // ADDR pin to GND
    int         adc_channel = 0;            // detector signal input AIN0..3
    int         pga_mv    = 2048;           // full-scale range: 256|512|1024|2048|4096|6144 mV
    int         sps       = 128;            // 8|16|32|64|128|250|475|860 samples/s
    // GPIO
    std::string gpio_chip = "/dev/gpiochip0";
    // control line assignments (GPIO line offsets; -1 = not present)
    int sample_valve = -1;
    int inject_valve = -1;
    int cal_valve    = -1;
    int purge_valve  = -1;
    int pump         = -1;
    int lamp         = -1;
    int fan          = -1;
    int autozero     = -1;   // detector autozero command (legacy AutoZero bit)
    // multipoint manifold: one point-select valve per sample point
    // (legacy POINTS(); empty = single point on sample_valve)
    std::vector<int> point_valves;
    // concentration alarm relays (common across points, as legacy COMMON_HIGH)
    int alarm_high_line = -1;
    int alarm_low_line  = -1;
};

// One controlled temperature zone (oven, injector, detector...), read via an
// ADS1115 channel and driven by a heater relay with hysteresis -- the same
// on/off control the legacy oven used.
struct TempZone {
    std::string name = "oven";
    int    adc_channel = 1;      // ADS1115 input carrying the sensor signal
    int    heater_line = -1;     // GPIO line of the heater relay
    double setpoint_c  = 0;      // deg C; 0 = zone disabled
    double hysteresis_c = 2;     // +/- band
    // linear sensor conversion: temperature = scale * volts + offset
    // (e.g. LM35: scale=100, offset=0; TMP36: scale=100, offset=-50)
    double scale  = 100;
    double offset = 0;
};

// Run phase timing (legacy method_table times), seconds.
struct TimingConfig {
    long equil_time  = 5;    // wait for temperature zones in band
    long autozero_time = 0;  // s: energize the autozero line after equil (0 = skip)
    long sample_time = 10;   // sample pump/valve on
    long inject_time = 5;    // injection valve energized
    long purge_time  = 10;   // post-run purge
    // scheduling (continuous/repeat modes, legacy RunMode + repeat_cycle)
    long repeat_interval = 0;   // s between run starts (0 = back-to-back)
    int  auto_cal_every  = 0;   // insert a cal run every N runs (0 = never)
    int  auto_cal_standard = 1; // which standard the auto-cal measures
};

// ---- simulation backend ------------------------------------------------------
// Detector channel: baseline + gaussian peaks scaled by sim_scale (and only
// present after an injection). Temperature channels: first-order thermal
// model responding to the heater lines. Deterministic noise.
class SimHardware : public Adc, public DigitalOut {
public:
    SimHardware(const HardwareConfig &hw, const std::vector<TempZone> &zones,
                double sim_scale);

    // Adc
    long   ReadCounts(int channel) override;
    double CountsToVolts(long counts) const override;
    // DigitalOut
    void Set(int line, bool on) override;
    bool Get(int line) const override;

    // advance simulated time; the acquisition loop calls this instead of
    // sleeping, so simulated runs execute instantly.
    void AdvanceSeconds(double dt);
    double NowSeconds() const { return t_; }

    // peaks are timed from injection; point_factor scales the peak pattern so
    // different sample points show different concentrations in simulation
    void MarkInjection(double point_factor = 1.0)
    { inject_t_ = t_; point_factor_ = point_factor; }

private:
    HardwareConfig hw_;
    std::vector<TempZone> zones_;
    std::vector<double> zone_temp_c_;
    double sim_scale_;
    double point_factor_ = 1.0;
    double t_ = 0;                 // simulated seconds
    double inject_t_ = -1;         // time of injection, -1 = none yet
    bool   lines_[64] = {};
    unsigned rng_ = 20260709;

    double DetectorVolts();
    long   VoltsToCounts(double v) const;
};

// Factory: create the configured backend. Returns nullptr and fills err if
// the backend is unavailable (e.g. "ads1115" on Windows).
struct Hardware {
    Adc        *adc = nullptr;
    DigitalOut *out = nullptr;
    SimHardware *sim = nullptr;    // non-null when backend=="sim"
};
bool OpenHardware(const HardwareConfig &hw, const std::vector<TempZone> &zones,
                  double sim_scale, Hardware &h, std::string &err);
void CloseHardware(Hardware &h);

} // namespace wpeak64

#endif // HW64_H
