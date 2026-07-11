// mcp4725.h -- MCP4725 12-bit I2C DAC driver for WPEAK64 (Linux i2c-dev).
// One chip per analog output channel (the chip has a single output); writes
// use fast-mode command (no EEPROM wear). Ports the legacy Write_conc_to_DAC
// analog-output-board path onto commodity I2C hardware.

#ifndef MCP4725_H
#define MCP4725_H

#include "hw64.h"
#include <vector>

#ifdef __linux__

namespace wpeak64 {

class Mcp4725AnalogOut : public AnalogOut {
public:
    // i2c_dev: e.g. "/dev/i2c-1". addrs: one 7-bit I2C address per DAC
    // channel (index = channel number passed to Write). vref: DAC full-scale
    // output voltage, used only to clamp/report -- the MCP4725 itself has no
    // reference pin (its VDD is the implicit full scale).
    static Mcp4725AnalogOut *Open(const std::string &i2c_dev,
                                  const std::vector<int> &addrs, double vref,
                                  std::string &err);
    ~Mcp4725AnalogOut() override;

    void   Write(int channel, double volts) override;
    double LastVolts(int channel) const override;

private:
    Mcp4725AnalogOut(std::vector<int> fds, double vref);
    std::vector<int>    fds_;
    std::vector<double> last_;
    double vref_;
};

} // namespace wpeak64

#endif // __linux__
#endif // MCP4725_H
