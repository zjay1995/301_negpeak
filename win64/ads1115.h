// ads1115.h -- ADS1115 16-bit I2C ADC driver for WPEAK64 (Linux i2c-dev).
// Single-shot, single-ended reads on AIN0..AIN3 with dynamic PGA gain
// ranging per channel (see gainrange.h) and configurable data rate.
// Register map per the TI ADS1115 datasheet: 0x00 conversion register,
// 0x01 config register.

#ifndef ADS1115_H
#define ADS1115_H

#include "hw64.h"

#ifdef __linux__

namespace wpeak64 {

class Ads1115 : public Adc {
public:
    // Opens the i2c-dev device and claims the slave address. hw.pga_mv is
    // the reference range: all returned counts are normalized to it, even
    // when auto-ranging measures a sample at a different physical range.
    // Returns nullptr and fills err on failure.
    static Ads1115 *Open(const HardwareConfig &hw, std::string &err);
    ~Ads1115() override;

    long   ReadCounts(int channel) override;   // blocking single-shot conversion, auto-ranged
    double CountsToVolts(long counts) const override;
    int    CurrentRangeMv(int channel) const override;   // active PGA range, for diagnostics

private:
    Ads1115(int fd, int ref_pga_bits, double ref_fsr_v, int dr_bits);
    long ReadRaw(int channel, int pga_bits);    // one physical conversion at a given range

    int    fd_;
    int    ref_pga_bits_;   // method-configured reference range (config register PGA field)
    double ref_fsr_v_;      // reference full-scale range, volts
    int    dr_bits_;        // config register DR field

    struct ChanState { int bits = 0; bool inited = false; };
    ChanState chan_[4];      // current auto-range state, one per AIN0..AIN3
};

} // namespace wpeak64

#endif // __linux__
#endif // ADS1115_H
