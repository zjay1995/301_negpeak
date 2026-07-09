// ads1115.h -- ADS1115 16-bit I2C ADC driver for WPEAK64 (Linux i2c-dev).
// Single-shot, single-ended reads on AIN0..AIN3 with configurable PGA
// full-scale range and data rate. Register map per the TI ADS1115 datasheet:
//   0x00 conversion register, 0x01 config register.

#ifndef ADS1115_H
#define ADS1115_H

#include "hw64.h"

#ifdef __linux__

namespace wpeak64 {

class Ads1115 : public Adc {
public:
    // Opens the i2c-dev device and claims the slave address.
    // Returns nullptr and fills err on failure.
    static Ads1115 *Open(const HardwareConfig &hw, std::string &err);
    ~Ads1115() override;

    long   ReadCounts(int channel) override;   // blocking single-shot conversion
    double CountsToVolts(long counts) const override;

private:
    Ads1115(int fd, int pga_bits, double fsr_v, int dr_bits);
    int    fd_;
    int    pga_bits_;   // config register PGA field
    double fsr_v_;      // full-scale range, volts
    int    dr_bits_;    // config register DR field
};

} // namespace wpeak64

#endif // __linux__
#endif // ADS1115_H
