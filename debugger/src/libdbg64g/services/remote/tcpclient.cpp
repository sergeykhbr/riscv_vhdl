/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Remote access to debugger via TCP connection. Client thread.
 */

#include "tcpclient.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(TcpClient)

TcpClient::TcpClient(const char *name) : IService(name),
    tcpcmd_(static_cast<IService *>(this)) {
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("Enable", &isEnable_);
    RISCV_mutex_init(&mutexTx_);
}

TcpClient::~TcpClient() {
    RISCV_mutex_destroy(&mutexTx_);
}

void TcpClient::postinitService() {
    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void TcpClient::updateData(const char *buf, int buflen) {
    AttributeType console;
    console.make_list(2);
    console[0u].make_string("Console");
    console[1].make_string(buf);
    console.to_config();

    RISCV_mutex_lock(&mutexTx_);
    memcpy(&txbuf_[txcnt_], console.to_string(), console.size() + 1);
    txcnt_ += console.size() + 1;
    RISCV_mutex_unlock(&mutexTx_);
}

void TcpClient::busyLoop() {
    int rxbytes;
    int sockerr;
    addr_size_t sockerr_len = sizeof(sockerr);
    RISCV_add_default_output(static_cast<IRawListener *>(this));

    cmdcnt_ = 0;
    txcnt_ = 0;
    while (isEnabled()) {
        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);
        getsockopt(hsock_, SOL_SOCKET, SO_ERROR,
                    reinterpret_cast<char *>(&sockerr), &sockerr_len);

        if (rxbytes == 0) {
            RISCV_error("Socket error: rxbytes=%d, sockerr=%d",
                        rxbytes, sockerr);
            loopEnable_.state = false;
        } else if (rxbytes < 0) {
            // Timeout:
        } else if (rxbytes > 0) {
            for (int i = 0; i < rxbytes; i++) {
                cmdbuf_[cmdcnt_++] = rcvbuf[i];
                if (rcvbuf[i] == '\0') {
                    processRxString();
                    cmdcnt_ = 0;
                }
            }
        }
        if (sendTxBuf() < 0) {
            RISCV_error("Send error: txcnt=%d", txcnt_);
            loopEnable_.state = false;
        }
    }
    closeSocket();
    RISCV_remove_default_output(static_cast<IRawListener *>(this));
}

void TcpClient::processRxString() {
    tcpcmd_.updateData(cmdbuf_, cmdcnt_);
    AttributeType *resp = tcpcmd_.response();
    RISCV_mutex_lock(&mutexTx_);
    memcpy(&txbuf_[txcnt_], resp->to_string(), resp->size() + 1);
    txcnt_ += resp->size() + 1;
    RISCV_mutex_unlock(&mutexTx_);
}

int TcpClient::sendTxBuf() {
    int total = txcnt_;
    char *ptx = txbuf_;
    int txbytes;
    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes == 0) {
            return -1;
        }
        total -= txbytes;
        ptx += txbytes;
    }
    RISCV_mutex_lock(&mutexTx_);
    txcnt_ = 0;
    RISCV_mutex_unlock(&mutexTx_);
    return 0;
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
