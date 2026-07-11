// modbus64.h -- Modbus TCP slave for WPEAK64 remote monitoring.
//
// Ports the legacy serial Modbus ASCII slave (MODBUS.CPP's ModBusServer,
// function codes 0x03/0x04 only, driven by GetModBusRegister()) onto
// standard Modbus TCP -- the framing SCADA/PLC integrators actually expect
// today. Registers are still packed the legacy way: each concentration is
// a 32-bit float across two consecutive 16-bit registers, high word first.
//
// Cross-platform (Winsock on Windows, BSD sockets on Linux); works with any
// standard Modbus TCP master (e.g. a PLC, or `mbpoll -t 4:float -r1 ...`).

#ifndef MODBUS64_H
#define MODBUS64_H

#include "method64.h"
#include <cstdint>
#include <string>
#include <vector>

namespace wpeak64 {

// Minimal Modbus TCP slave answering function codes 3 (Read Holding
// Registers) and 4 (Read Input Registers) identically -- the legacy
// GetModBusRegister() didn't distinguish them either. One request is
// serviced per Poll() call (accept, read, respond, close); call Poll()
// periodically from the acquisition loop.
class ModbusServer {
public:
    ModbusServer();
    ~ModbusServer();

    bool Start(int port, std::string &err);
    void Stop();

    // Replace the register table served to clients (call after every run
    // so a poll always sees the latest results).
    void SetRegisters(const std::vector<uint16_t> &regs);

    // Service one pending connection if any (non-blocking). Returns true
    // if a request was answered.
    bool Poll(std::string &err);

private:
    int listen_fd_ = -1;
    std::vector<uint16_t> regs_;
};

// Pack a run's identified concentrations into the legacy register layout:
// two registers per method component, in method order (big-endian 32-bit
// float, high word first); unmatched/negative/uncalibrated components read
// 0.0. Detector A's components are packed first, then detector B's (when
// present) immediately after, so a single register map covers both.
std::vector<uint16_t> BuildModbusRegisters(const Method &m,
                                           const std::vector<ReportRow> &rows,
                                           const std::vector<ReportRow> *rows_b = nullptr);

} // namespace wpeak64

#endif // MODBUS64_H
