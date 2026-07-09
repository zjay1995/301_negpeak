// gpio64.cpp -- Linux GPIO character-device outputs. See gpio64.h.

#ifdef __linux__

#include "gpio64.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

namespace wpeak64 {

GpioOut *GpioOut::Open(const std::string &chip_path,
                       const std::vector<int> &lines, std::string &err)
{
    int chip = open(chip_path.c_str(), O_RDWR);
    if(chip < 0) { err = "gpio: cannot open " + chip_path; return nullptr; }

    GpioOut *g = new GpioOut();
    for(int line : lines) {
        if(line < 0) continue;
        struct gpiohandle_request req;
        std::memset(&req, 0, sizeof req);
        req.lineoffsets[0] = (uint32_t)line;
        req.lines = 1;
        req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        req.default_values[0] = 0;
        std::strncpy(req.consumer_label, "wpeak64", sizeof req.consumer_label - 1);
        if(ioctl(chip, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
            err = "gpio: cannot request line " + std::to_string(line) +
                  " on " + chip_path;
            close(chip);
            delete g;
            return nullptr;
        }
        g->states_.push_back({ line, req.fd, false });
    }
    close(chip);   // line handles stay valid after the chip fd closes
    return g;
}

GpioOut::~GpioOut()
{
    for(auto &s : states_) {
        struct gpiohandle_data d; d.values[0] = 0;   // de-energize on exit
        ioctl(s.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d);
        close(s.fd);
    }
}

void GpioOut::Set(int line, bool on)
{
    for(auto &s : states_) {
        if(s.line == line) {
            struct gpiohandle_data d; d.values[0] = on ? 1 : 0;
            ioctl(s.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d);
            s.value = on;
            return;
        }
    }
}

bool GpioOut::Get(int line) const
{
    for(const auto &s : states_)
        if(s.line == line) return s.value;
    return false;
}

} // namespace wpeak64

#endif // __linux__
