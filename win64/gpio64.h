// gpio64.h -- relay/valve/heater outputs via the Linux GPIO character device
// (/dev/gpiochipN, GPIO uAPI v1 handle requests). Replaces the legacy
// Measurement Computing digital-output bits (Set_valve etc. in ACQUIRE.CPP).

#ifndef GPIO64_H
#define GPIO64_H

#include "hw64.h"

#ifdef __linux__

namespace wpeak64 {

class GpioOut : public DigitalOut {
public:
    // lines: the set of GPIO line offsets that will be driven (each valid
    // offset from HardwareConfig). Returns nullptr and fills err on failure.
    static GpioOut *Open(const std::string &chip_path,
                         const std::vector<int> &lines, std::string &err);
    ~GpioOut() override;

    void Set(int line, bool on) override;
    bool Get(int line) const override;

private:
    GpioOut() {}
    struct LineState { int line; int fd; bool value; };
    std::vector<LineState> states_;
};

} // namespace wpeak64

#endif // __linux__
#endif // GPIO64_H
