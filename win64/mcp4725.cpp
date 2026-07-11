// mcp4725.cpp -- MCP4725 driver over Linux i2c-dev. See mcp4725.h.

#ifdef __linux__

#include "mcp4725.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cmath>

namespace wpeak64 {

Mcp4725AnalogOut *Mcp4725AnalogOut::Open(const std::string &i2c_dev,
                                         const std::vector<int> &addrs, double vref,
                                         std::string &err)
{
    std::vector<int> fds;
    for(int addr : addrs) {
        int fd = open(i2c_dev.c_str(), O_RDWR);
        if(fd < 0) {
            err = "mcp4725: cannot open " + i2c_dev;
            for(int f : fds) close(f);
            return nullptr;
        }
        if(ioctl(fd, I2C_SLAVE, addr) < 0) {
            close(fd);
            char buf[64]; std::snprintf(buf, sizeof buf, "0x%02X", addr);
            err = "mcp4725: cannot claim I2C address " + std::string(buf);
            for(int f : fds) close(f);
            return nullptr;
        }
        fds.push_back(fd);
    }
    return new Mcp4725AnalogOut(fds, vref);
}

Mcp4725AnalogOut::Mcp4725AnalogOut(std::vector<int> fds, double vref)
    : fds_(std::move(fds)), last_(fds_.size(), 0), vref_(vref)
{
}

Mcp4725AnalogOut::~Mcp4725AnalogOut()
{
    for(int fd : fds_) close(fd);
}

void Mcp4725AnalogOut::Write(int channel, double volts)
{
    if(channel < 0 || (size_t)channel >= fds_.size()) return;
    if(volts < 0) volts = 0;
    if(volts > vref_) volts = vref_;
    last_[(size_t)channel] = volts;

    // fast-mode write: 2 bytes, C2C1=00 (write DAC register, no EEPROM),
    // PD1PD0=00 (normal power mode), 12-bit code in the low 12 bits.
    unsigned code = (unsigned)std::lround(volts / vref_ * 4095.0);
    if(code > 4095) code = 4095;
    uint8_t buf[2] = { (uint8_t)((code >> 8) & 0x0F), (uint8_t)(code & 0xFF) };
    ssize_t n = write(fds_[(size_t)channel], buf, 2);
    (void)n;   // best-effort; ReadCounts-side callers already tolerate a dropped I2C write
}

double Mcp4725AnalogOut::LastVolts(int channel) const
{
    return (channel >= 0 && (size_t)channel < last_.size()) ? last_[(size_t)channel] : 0;
}

} // namespace wpeak64

#endif // __linux__
