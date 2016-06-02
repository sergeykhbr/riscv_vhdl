/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UDP transport level implementation.
 */

#include "api_core.h"
#include "udp.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(UdpService)

UdpService::UdpService(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IUdp *>(this));
    registerAttribute("Timeout", &timeout_);
    registerAttribute("BlockingMode", &blockmode_);
    registerAttribute("HostIP", &hostIP_);
    registerAttribute("BoardIP", &boardIP_);

    timeout_.make_int64(0);
    blockmode_.make_boolean(true);
    hostIP_.make_string("192.168.0.53");
    boardIP_.make_string("192.168.0.51");
}

UdpService::~UdpService() {
    closeDatagramSocket();
}

void UdpService::postinitService() {
    createDatagramSocket();
    // define hardcoded remote address:
    remote_sockaddr_ipv4_ = sockaddr_ipv4_;
    remote_sockaddr_ipv4_.sin_addr.s_addr = inet_addr(boardIP_.to_string());

    if (timeout_.to_int64()) {
        struct timeval tv;
#if defined(_WIN32) || defined(__CYGWIN__)
        tv.tv_usec = 0;
        tv.tv_sec = static_cast<long>(timeout_.to_int64());
#else
        tv.tv_usec = (timeout_.to_int64() % 1000) * 1000;
        tv.tv_sec = static_cast<long>(timeout_.to_int64()/1000);
#endif

        setsockopt(hsock_, SOL_SOCKET, SO_RCVTIMEO, 
                            (char *)&tv, sizeof(struct timeval));
    }

    /** By default socket was created with Blocking mode */
    if (!blockmode_.to_bool()) {
        setBlockingMode(hsock_, false);
    }
}

int UdpService::registerListener(IRawListener *ilistener) { 
    vecListeners_.push_back(ilistener);
    return 0;
}

int UdpService::createDatagramSocket() {
    char hostName[256];
    if(gethostname(hostName, sizeof(hostName)) < 0) {
        return -1;
    }

    memset(&sockaddr_ipv4_, 0, sizeof (sockaddr_ipv4_));
    sockaddr_ipv4_.sin_family = AF_INET;
    inet_aton(hostIP_.to_string(), &(sockaddr_ipv4_.sin_addr));
    hsock_ = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hsock_ < 0) {
        perror("Fatal error: socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)");
        //setLibError(ERR_UDPSOCKET_CREATE);
        return -1;
    }

    int res = bind(hsock_, (struct sockaddr *)&sockaddr_ipv4_, 
                              sizeof(sockaddr_ipv4_));
    if (res != 0) {
      char err[99];
      sprintf(err, "Fatal error: bind(hsock_, \"%s\", ...)", hostIP_.to_string());
      perror(err);
        //setLibError(ERR_UDPSOCKET_BIND);
        return -1;
    }

    addr_size_t addr_sz = sizeof(sockaddr_ipv4_);
    res = getsockname(hsock_, (struct sockaddr *)&sockaddr_ipv4_, &addr_sz);
    sockaddr_ipv4_port_ = ntohs(sockaddr_ipv4_.sin_port);

    RISCV_info("\tIPv4 address %s:%d . . . opened", 
                sockaddr_ipv4_str_, sockaddr_ipv4_port_);

    return 0;
}

void UdpService::closeDatagramSocket() {
    if (hsock_ < 0)
        return;

#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(hsock_);
#else
    shutdown(hsock_, SHUT_RDWR);
    close(hsock_);
#endif
    hsock_ = -1;
}

bool UdpService::setBlockingMode(socket_def h, bool mode) {
    int ret;
#if defined(_WIN32) || defined(__CYGWIN__)
    u_long arg = mode ? 0 : 1;
    ret = ioctlsocket(h, FIONBIO, &arg);
#else
    int flags = fcntl(h, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    flags = mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    ret = fcntl(h, F_SETFL, flags);
#endif
    if (ret == 0) {
        // success
        blockmode_.make_boolean(mode);
        return true;
    }
    return false;
}

int UdpService::sendData(const uint8_t *msg, int len) {
    int tx_bytes = sendto(hsock_, reinterpret_cast<const char *>(msg), len, 0,
                  reinterpret_cast<struct sockaddr *>(&remote_sockaddr_ipv4_),
                  static_cast<int>(sizeof(remote_sockaddr_ipv4_)));

    if (tx_bytes < 0) {
#if defined(_WIN32) || defined(__CYGWIN__)
        RISCV_error("sendto() failed with error: %d\n", WSAGetLastError());
#else
        RISCV_error("sendto() failed\n", NULL);
#endif
        return 1;
    } else {
        char dbg[1024];
        int pos = RISCV_sprintf(dbg, sizeof(dbg), "send  %d bytes to %s:%d: ",
                                tx_bytes,
                                inet_ntoa(remote_sockaddr_ipv4_.sin_addr),
                                ntohs(remote_sockaddr_ipv4_.sin_port));

        if (tx_bytes < 64) {
            for (int i = 0; i < len; i++) {
                pos += RISCV_sprintf(&dbg[pos], sizeof(dbg) - pos, 
                                                "%02x", msg[i] & 0xFF);
            }
        }
        RISCV_debug("%s", dbg);
    }
    return tx_bytes;
}

int UdpService::readData(const uint8_t *buf, int maxlen) {
    int sockerr;
    addr_size_t sockerr_len = sizeof(sockerr);
    addr_size_t addr_sz = sizeof(sockaddr_ipv4_);

    int res = recvfrom(hsock_, rcvbuf, sizeof(rcvbuf),
                        0, (struct sockaddr *)&sockaddr_ipv4_, &addr_sz);
    getsockopt(hsock_, SOL_SOCKET, SO_ERROR, 
                            (char *)&sockerr, &sockerr_len);

    if (res < 0 && sockerr < 0) {
        RISCV_error("Socket error %x", sockerr);
        res = -1;
    } else if (res < 0 && sockerr == 0) {
        // Timeout:
        res = 0;
    } else if (res > 0) {
        if (maxlen < res) {
            res = maxlen;
            RISCV_error("Receiver's buffer overflow maxlen = %d", maxlen);
        }
        memcpy(const_cast<uint8_t *>(buf), rcvbuf, res);
        char dbg[1024];
        int pos = RISCV_sprintf(dbg, sizeof(dbg), "received  %d Bytes: ", res);
        if (res < 64) {
            for (int i = 0; i < res; i++) {
                pos += RISCV_sprintf(&dbg[pos], sizeof(dbg) - pos, 
                                    "%02x", rcvbuf[i] & 0xFF);
            }
        }
        RISCV_debug("%s", dbg);
    }
    return res;
}

}  // namespace debugger
