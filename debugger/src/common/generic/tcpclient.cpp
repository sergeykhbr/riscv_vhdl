/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "tcpclient.h"

namespace debugger {

TcpClient::TcpClient(IService *parent, const char *name) : IService(parent, name) {
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("TargetIP", &targetIP_);
    registerAttribute("TargetPort", &targetPort_);
    registerAttribute("RecvTimeout", &recvTimeout_);
    RISCV_mutex_init(&mutexTx_);
    hsock_ = -1;
    txcnt_ = 0;
    recvTimeout_.make_int64(500);   //default 500 ms timeout for recv() function
}

TcpClient::~TcpClient() {
    RISCV_mutex_destroy(&mutexTx_);
}

void TcpClient::postinitService() {
    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
    }
}

void TcpClient::busyLoop() {
    int rxbytes;
    afterThreadStarted();

    while (isEnabled()) {
        rxbytes = recv(hsock_, rxbuf_, sizeof(rxbuf_), 0);
        if (rxbytes == 0) {
            // connection closed
            break;
        }  else if (rxbytes < 0) {
            // timeout or use WSAGetLastError() to confirm it
        }
        rxbuf_[rxbytes] = 0;
        if (processRxBuffer(rxbuf_, rxbytes) < 0) {
            break;
        }
        if (sendData() < 0) {
            break;
        }
    }
    stop();

    beforeThreadClosing();
    closeSocket();
}

int TcpClient::writeTxBuffer(const char *buf, int sz) {
    int ret = 0;
    if (!isEnabled()) {
        return ret;
    }

    RISCV_mutex_lock(&mutexTx_);
    if (txcnt_ + sz >= sizeof(txbuf_)) {
        RISCV_error("%s", "Tx buffer overflow");
    } else {
        memcpy(&txbuf_[txcnt_], buf, sz);
        txcnt_ += sz;
        ret = sz;
    }
    RISCV_mutex_unlock(&mutexTx_);
    return ret;
}

int TcpClient::sendData() {
    const char *ptx = txbuf2_;
    int total;
    int txbytes;

    // Use double buffering    
    RISCV_mutex_lock(&mutexTx_);
    memcpy(txbuf2_, txbuf_, txcnt_);
    total = txcnt_;
    txcnt_ = 0;
    RISCV_mutex_unlock(&mutexTx_);

#if 1
    if (strcmp(getObjName(), "openocd0") == 0 && total) {
        bool st = true;
    }
#endif

    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes <= 0) {
            RISCV_error("Send error: txcnt=%d", txcnt_);
            return -1;
        }
        total -= txbytes;
        ptx += txbytes;
    }
    return 0;
}

int TcpClient::connectToServer() {
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) < 0) {
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /**
     * Check availability of IPv4 address assigned via attribute 'hostIP'.
     * If it woudn't be found use the last avaialble IP address.
     */
    int retval;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    retval = getaddrinfo(hostName, "0", &hints, &result);
    if (retval != 0) {
        return -1;
    }

    hsock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        RISCV_error("%s", "Error: socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)");
        return -1;
    }

    memset(&sockaddr_ipv4_, 0, sizeof(struct sockaddr_in));
    sockaddr_ipv4_.sin_family = AF_INET;
    sockaddr_ipv4_.sin_addr.s_addr = inet_addr(targetIP_.to_string());
    sockaddr_ipv4_.sin_port = htons(static_cast<uint16_t>(targetPort_.to_uint32()));

    int enable = 1;
    if (setsockopt(hsock_, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char *>(&enable), sizeof(int)) < 0) {
        RISCV_error("%s", "setsockopt(TCP_NODELAY) failed");
        return -1;
    }

    if (setsockopt(hsock_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&enable), sizeof(int)) < 0) {
        RISCV_error("%s", "setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    if (setRecvTimeout(recvTimeout_.to_int())) {
        RISCV_error("%s", "setsockopt(SO_RCVTIMEO) failed");
        return -1;
    }

    int res = connect(hsock_,
                   reinterpret_cast<struct sockaddr *>(&sockaddr_ipv4_),
                   sizeof(sockaddr_ipv4_));
    if (res != 0) {
        RISCV_error("Error: connect(hsock_, \"%s\", ...)", targetIP_.to_string());
        return -1;
    }

    RISCV_info("IPv4 address %s:%d . . . connected",
                inet_ntoa(sockaddr_ipv4_.sin_addr),
                ntohs(sockaddr_ipv4_.sin_port));

    return 0;
}

int TcpClient::setRecvTimeout(int timeout_ms) {
    if (!timeout_ms) {
        return 0;
    }
    struct timeval tv;
#if defined(_WIN32) || defined(__CYGWIN__)
    /** On windows timeout of the setsockopt() function is the DWORD
        * size variable in msec, so we use only the first field in timeval
        * struct and directly assgign argument.
        */
    tv.tv_sec = timeout_ms;
    tv.tv_usec = 0;
#else
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    tv.tv_sec = timeout_ms / 1000;
#endif
     return setsockopt(hsock_, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<char *>(&tv), sizeof(struct timeval));
}


void TcpClient::closeSocket() {
    if (hsock_ < 0) {
        return;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(hsock_);
#else
    shutdown(hsock_, SHUT_RDWR);
    close(hsock_);
#endif
    hsock_ = -1;
}

}  // namespace debugger
