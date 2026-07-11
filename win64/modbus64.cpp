// modbus64.cpp -- Modbus TCP slave. See modbus64.h.

#include "modbus64.h"
#include <cstring>
#include <cstdio>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  #define CLOSESOCK closesocket
  #define SOCK_INVALID INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <fcntl.h>
  typedef int sock_t;
  #define CLOSESOCK close
  #define SOCK_INVALID (-1)
#endif

namespace wpeak64 {

ModbusServer::ModbusServer() {}
ModbusServer::~ModbusServer() { Stop(); }

bool ModbusServer::Start(int port, std::string &err)
{
#ifdef _WIN32
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { err = "modbus: WSAStartup failed"; return false; }
#endif
    sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == SOCK_INVALID) { err = "modbus: socket() failed"; return false; }

    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof one);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if(bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0) {
        err = "modbus: bind() failed on port " + std::to_string(port);
        CLOSESOCK(fd);
        return false;
    }
    if(listen(fd, 4) != 0) {
        err = "modbus: listen() failed";
        CLOSESOCK(fd);
        return false;
    }
#ifdef _WIN32
    u_long nb = 1;
    ioctlsocket(fd, FIONBIO, &nb);
#else
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
#endif
    listen_fd_ = (int)fd;
    return true;
}

void ModbusServer::Stop()
{
    if(listen_fd_ >= 0) {
        CLOSESOCK((sock_t)listen_fd_);
        listen_fd_ = -1;
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

void ModbusServer::SetRegisters(const std::vector<uint16_t> &regs)
{
    regs_ = regs;
}

static bool RecvAll(sock_t fd, uint8_t *buf, int n, int timeout_ms)
{
    int got = 0;
    while(got < n) {
        fd_set rf; FD_ZERO(&rf); FD_SET(fd, &rf);
        struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
        int r = select((int)fd + 1, &rf, nullptr, nullptr, &tv);
        if(r <= 0) return false;
        int n2 = recv(fd, (char *)buf + got, n - got, 0);
        if(n2 <= 0) return false;
        got += n2;
    }
    return true;
}

// Services at most one client per call: accept, read one MBAP-framed
// request, answer, close. A stateless one-shot-per-connection design is
// enough for the polling SCADA/PLC masters this targets, and keeps the
// server free of per-client state between Poll() calls.
bool ModbusServer::Poll(std::string &err)
{
    (void)err;
    if(listen_fd_ < 0) return false;
    sock_t lfd = (sock_t)listen_fd_;

    fd_set rf; FD_ZERO(&rf); FD_SET(lfd, &rf);
    struct timeval tv = { 0, 0 };
    if(select((int)lfd + 1, &rf, nullptr, nullptr, &tv) <= 0) return false;

    sock_t cfd = accept(lfd, nullptr, nullptr);
    if(cfd == SOCK_INVALID) return false;

    // MBAP header: transaction id(2) protocol id(2, must be 0) length(2)
    // unit id(1); length counts unit id + PDU that follows it.
    uint8_t hdr[7];
    if(!RecvAll(cfd, hdr, 7, 500)) { CLOSESOCK(cfd); return false; }
    uint16_t txn  = (uint16_t)((hdr[0] << 8) | hdr[1]);
    uint16_t len  = (uint16_t)((hdr[4] << 8) | hdr[5]);
    uint8_t  unit = hdr[6];

    if(len < 2 || len > 253) { CLOSESOCK(cfd); return false; }
    std::vector<uint8_t> pdu(len - 1);
    if(!pdu.empty() && !RecvAll(cfd, pdu.data(), (int)pdu.size(), 500)) {
        CLOSESOCK(cfd); return false;
    }
    if(pdu.empty()) { CLOSESOCK(cfd); return false; }

    uint8_t func = pdu[0];
    std::vector<uint8_t> resp_pdu;
    // function codes 3 (Read Holding Registers) and 4 (Read Input Registers)
    // only -- the same two the legacy serial ModBusServer answered, and
    // answered identically (no holding/input distinction in the register
    // table), so both map onto the same handler here.
    if((func == 0x03 || func == 0x04) && pdu.size() >= 5) {
        uint16_t start = (uint16_t)((pdu[1] << 8) | pdu[2]);
        uint16_t qty   = (uint16_t)((pdu[3] << 8) | pdu[4]);
        if(qty < 1 || qty > 125 || (size_t)start + qty > regs_.size()) {
            resp_pdu = { (uint8_t)(func | 0x80), 0x02 };   // illegal data address
        }
        else {
            resp_pdu.push_back(func);
            resp_pdu.push_back((uint8_t)(qty * 2));
            for(uint16_t i = 0; i < qty; i++) {
                uint16_t v = regs_[(size_t)(start + i)];
                resp_pdu.push_back((uint8_t)(v >> 8));
                resp_pdu.push_back((uint8_t)(v & 0xFF));
            }
        }
    }
    else {
        resp_pdu = { (uint8_t)(func | 0x80), 0x01 };   // illegal function
    }

    uint16_t rlen = (uint16_t)(resp_pdu.size() + 1);
    uint8_t out[7 + 253];
    out[0] = (uint8_t)(txn >> 8);  out[1] = (uint8_t)(txn & 0xFF);
    out[2] = 0;                    out[3] = 0;
    out[4] = (uint8_t)(rlen >> 8); out[5] = (uint8_t)(rlen & 0xFF);
    out[6] = unit;
    std::memcpy(out + 7, resp_pdu.data(), resp_pdu.size());
    send(cfd, (const char *)out, 7 + (int)resp_pdu.size(), 0);
    CLOSESOCK(cfd);
    return true;
}

static void PackFloat(std::vector<uint16_t> &out, float v)
{
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    out.push_back((uint16_t)(bits >> 16));   // high word first (legacy packing)
    out.push_back((uint16_t)(bits & 0xFFFF));
}

std::vector<uint16_t> BuildModbusRegisters(const Method &m,
                                           const std::vector<ReportRow> &rows,
                                           const std::vector<ReportRow> *rows_b)
{
    std::vector<uint16_t> out;
    auto pack_block = [&](const std::vector<Component> &comps, const std::vector<ReportRow> &rws) {
        for(size_t i = 0; i < comps.size(); i++) {
            float conc = 0.0f;
            for(const ReportRow &r : rws)
                if(r.component == (int)i && r.calibrated) { conc = (float)r.concentration; break; }
            PackFloat(out, conc);
        }
    };
    pack_block(m.components, rows);
    if(rows_b) pack_block(m.components_b, *rows_b);
    return out;
}

} // namespace wpeak64
