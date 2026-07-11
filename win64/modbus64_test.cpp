// modbus64_test.cpp -- standalone test of the Modbus TCP slave (modbus64.h),
// independent of wpeak64_acq's timing. A client thread sends a real MBAP
// request over a loopback TCP socket while the main thread services it with
// Poll(), so the wire protocol (framing, register packing, error codes) is
// exercised end to end.

#include "modbus64.h"
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

using namespace wpeak64;

static int g_fail = 0;

static void Check(bool cond, const char *what)
{
    std::printf("%s: %s\n", cond ? "ok" : "FAIL", what);
    if(!cond) g_fail++;
}

// Connects to 127.0.0.1:port, sends a Read Holding Registers request for
// [start, start+qty), and returns the raw response bytes (empty on failure).
static std::vector<uint8_t> QueryRegisters(int port, uint16_t start, uint16_t qty, uint8_t func = 0x03)
{
#ifdef _WIN32
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // brief retry: the server thread may not have called Start() yet
    for(int tries = 0; tries < 200; tries++) {
        if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    uint8_t req[12] = {
        0, 1,                                  // transaction id
        0, 0,                                  // protocol id
        0, 6,                                  // length
        1,                                     // unit id
        func,
        (uint8_t)(start >> 8), (uint8_t)(start & 0xFF),
        (uint8_t)(qty >> 8),   (uint8_t)(qty & 0xFF),
    };
    send(fd, (const char *)req, sizeof req, 0);

    uint8_t buf[300];
    int n = recv(fd, (char *)buf, sizeof buf, 0);
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
    if(n <= 0) return {};
    return std::vector<uint8_t>(buf, buf + n);
}

int main()
{
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    const int kPort = 15502;

    ModbusServer srv;
    std::string err;
    Check(srv.Start(kPort, err), "server starts listening");

    // Two 32-bit floats: 12.5 and -3.25, high word first (legacy packing).
    std::vector<uint16_t> regs = {
        0x4148, 0x0000,   // 12.5f
        0xC050, 0x0000,   // -3.25f
    };
    srv.SetRegisters(regs);

    std::vector<uint8_t> resp;
    std::atomic<bool> client_done{false};
    std::thread client([&]() {
        resp = QueryRegisters(kPort, 0, 4);
        client_done = true;
    });

    // service requests for up to ~1s
    for(int i = 0; i < 200 && !client_done; i++) {
        srv.Poll(err);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    client.join();

    Check(!resp.empty(), "client received a response");
    if(!resp.empty()) {
        Check(resp.size() == 9 + 8, "response length matches 4 registers");
        Check(resp[7] == 0x03, "function code echoed back");
        Check(resp[8] == 8, "byte count is 8 (4 registers)");
        float a, b;
        uint32_t bitsA = ((uint32_t)resp[9] << 24) | ((uint32_t)resp[10] << 16) |
                         ((uint32_t)resp[11] << 8)  |  (uint32_t)resp[12];
        uint32_t bitsB = ((uint32_t)resp[13] << 24) | ((uint32_t)resp[14] << 16) |
                         ((uint32_t)resp[15] << 8)  |  (uint32_t)resp[16];
        std::memcpy(&a, &bitsA, 4);
        std::memcpy(&b, &bitsB, 4);
        Check(a == 12.5f, "first register pair decodes to 12.5");
        Check(b == -3.25f, "second register pair decodes to -3.25");
    }

    // out-of-range request -> exception response (illegal data address)
    std::vector<uint8_t> resp2;
    std::atomic<bool> client2_done{false};
    std::thread client2([&]() {
        resp2 = QueryRegisters(kPort, 0, 100);   // only 4 registers exist
        client2_done = true;
    });
    for(int i = 0; i < 200 && !client2_done; i++) {
        srv.Poll(err);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    client2.join();
    Check(resp2.size() == 9, "exception response is 2-byte PDU");
    if(resp2.size() == 9) {
        Check(resp2[7] == (0x03 | 0x80), "exception function code has the error bit set");
        Check(resp2[8] == 0x02, "exception code is illegal data address");
    }

    // function code 4 (Read Input Registers) answers identically to 3
    std::vector<uint8_t> resp3;
    std::atomic<bool> client3_done{false};
    std::thread client3([&]() {
        resp3 = QueryRegisters(kPort, 0, 2, 0x04);
        client3_done = true;
    });
    for(int i = 0; i < 200 && !client3_done; i++) {
        srv.Poll(err);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    client3.join();
    Check(resp3.size() == 9 + 4 && resp3[7] == 0x04, "function code 4 also answers Read Registers");

    srv.Stop();

    // BuildModbusRegisters packing
    Method m;
    m.components.push_back(Component());
    m.components[0].name = "A";
    m.components.push_back(Component());
    m.components[1].name = "B";
    std::vector<ReportRow> rows(2);
    rows[0].component = 0; rows[0].calibrated = true; rows[0].concentration = 5.0;
    rows[1].component = 1; rows[1].calibrated = false; rows[1].concentration = 99.0;   // not calibrated -> 0
    std::vector<uint16_t> packed = BuildModbusRegisters(m, rows);
    Check(packed.size() == 4, "BuildModbusRegisters packs 2 registers per component");
    float c0, c1;
    uint32_t b0 = ((uint32_t)packed[0] << 16) | packed[1];
    uint32_t b1 = ((uint32_t)packed[2] << 16) | packed[3];
    std::memcpy(&c0, &b0, 4);
    std::memcpy(&c1, &b1, 4);
    Check(c0 == 5.0f, "calibrated component packs its concentration");
    Check(c1 == 0.0f, "uncalibrated component packs zero");

    std::printf("\n%s\n", g_fail ? "SOME CHECKS FAILED" : "all checks passed");
    return g_fail ? 1 : 0;
}
