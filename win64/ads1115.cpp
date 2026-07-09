// ads1115.cpp -- ADS1115 driver over Linux i2c-dev. See ads1115.h.

#ifdef __linux__

#include "ads1115.h"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>

namespace wpeak64 {

// config register fields (datasheet section 9.6.3)
static const int REG_CONVERSION = 0x00;
static const int REG_CONFIG     = 0x01;
static const uint16_t OS_SINGLE  = 0x8000;  // start single conversion / idle
static const uint16_t MODE_SINGLE = 0x0100;
static const uint16_t COMP_DISABLE = 0x0003;

struct PgaChoice { int mv; int bits; double fsr_v; };
static const PgaChoice kPga[] = {
    { 6144, 0, 6.144 }, { 4096, 1, 4.096 }, { 2048, 2, 2.048 },
    { 1024, 3, 1.024 }, {  512, 4, 0.512 }, {  256, 5, 0.256 },
};
struct DrChoice { int sps; int bits; };
static const DrChoice kDr[] = {
    { 8, 0 }, { 16, 1 }, { 32, 2 }, { 64, 3 },
    { 128, 4 }, { 250, 5 }, { 475, 6 }, { 860, 7 },
};

static bool WriteReg(int fd, int reg, uint16_t val)
{
    uint8_t buf[3] = { (uint8_t)reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
    return write(fd, buf, 3) == 3;
}

static bool ReadReg(int fd, int reg, uint16_t &val)
{
    uint8_t r = (uint8_t)reg;
    if(write(fd, &r, 1) != 1) return false;
    uint8_t buf[2];
    if(read(fd, buf, 2) != 2) return false;
    val = (uint16_t)((buf[0] << 8) | buf[1]);
    return true;
}

Ads1115 *Ads1115::Open(const HardwareConfig &hw, std::string &err)
{
    int pga_bits = -1; double fsr_v = 0;
    for(const auto &p : kPga)
        if(p.mv == hw.pga_mv) { pga_bits = p.bits; fsr_v = p.fsr_v; }
    if(pga_bits < 0) { err = "ads1115: invalid pga_mv (256|512|1024|2048|4096|6144)"; return nullptr; }

    int dr_bits = -1;
    for(const auto &d : kDr)
        if(d.sps == hw.sps) dr_bits = d.bits;
    if(dr_bits < 0) { err = "ads1115: invalid sps (8..860)"; return nullptr; }

    int fd = open(hw.i2c_dev.c_str(), O_RDWR);
    if(fd < 0) { err = "ads1115: cannot open " + hw.i2c_dev; return nullptr; }
    if(ioctl(fd, I2C_SLAVE, hw.i2c_addr) < 0) {
        close(fd);
        char buf[64]; std::snprintf(buf, sizeof buf, "0x%02X", hw.i2c_addr);
        err = "ads1115: cannot claim I2C address " + std::string(buf);
        return nullptr;
    }
    // probe: read config register
    uint16_t cfg;
    if(!ReadReg(fd, REG_CONFIG, cfg)) {
        close(fd);
        err = "ads1115: no response on " + hw.i2c_dev;
        return nullptr;
    }
    return new Ads1115(fd, pga_bits, fsr_v, dr_bits);
}

Ads1115::Ads1115(int fd, int pga_bits, double fsr_v, int dr_bits)
    : fd_(fd), pga_bits_(pga_bits), fsr_v_(fsr_v), dr_bits_(dr_bits)
{
}

Ads1115::~Ads1115()
{
    if(fd_ >= 0) close(fd_);
}

long Ads1115::ReadCounts(int channel)
{
    if(channel < 0 || channel > 3) return 0;
    // MUX 100..111 = single-ended AIN0..AIN3 vs GND
    uint16_t mux = (uint16_t)(0x4 + channel) << 12;
    uint16_t cfg = OS_SINGLE | mux |
                   (uint16_t)(pga_bits_ << 9) | MODE_SINGLE |
                   (uint16_t)(dr_bits_ << 5) | COMP_DISABLE;
    if(!WriteReg(fd_, REG_CONFIG, cfg)) return 0;

    // wait for conversion complete (OS bit reads 1 when idle)
    for(int tries = 0; tries < 1000; tries++) {
        uint16_t c;
        if(!ReadReg(fd_, REG_CONFIG, c)) return 0;
        if(c & OS_SINGLE) break;
        struct timespec ts = { 0, 200000 };   // 0.2 ms
        nanosleep(&ts, nullptr);
    }
    uint16_t raw;
    if(!ReadReg(fd_, REG_CONVERSION, raw)) return 0;
    return (long)(int16_t)raw;
}

double Ads1115::CountsToVolts(long counts) const
{
    return (double)counts * fsr_v_ / 32768.0;
}

} // namespace wpeak64

#endif // __linux__
