/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "jsoncmd.h"
#include "gdbcmd.h"

namespace debugger {

TcpClient::TcpClient(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("PlatformConfig", &platformConfig_);
    registerAttribute("Type", &type_);
    registerAttribute("ListenDefaultOutput", &listenDefaultOutput_);
    RISCV_mutex_init(&mutexTx_);
    tcpcmd_ = 0;
}

TcpClient::~TcpClient() {
    RISCV_mutex_destroy(&mutexTx_);
    if (tcpcmd_) {
        delete tcpcmd_;
    }
}

void TcpClient::postinitService() {
    if (type_.is_equal("json")) {
        tcpcmd_ = new JsonCommands(static_cast<IService *>(this));
    } else if (type_.is_equal("gdb")) {
        tcpcmd_ = new GdbCommands(static_cast<IService *>(this));
    } else {
        RISCV_error("Unsupported command type %s.", type_.to_string());
        return;
    }

    tcpcmd_->setPlatformConfig(&platformConfig_);
    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

int TcpClient::updateData(const char *buf, int buflen) {
    int tsz = RISCV_sprintf(&asyncbuf_[0].ibyte, sizeof(asyncbuf_),
                    "['%s',", "Console");

    memcpy(&asyncbuf_[tsz], buf, buflen);
    tsz += buflen;
    asyncbuf_[tsz++].ubyte = ']';
    asyncbuf_[tsz++].ubyte = '\0';

    sendData(&asyncbuf_[0].ubyte, tsz);
    return buflen;
}

void TcpClient::busyLoop() {
    int rxbytes;
    if (listenDefaultOutput_.to_bool()) {
        RISCV_add_default_output(static_cast<IRawListener *>(this));
    }

    txcnt_ = 0;
    while (isEnabled()) {
        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);
        if (rxbytes <= 0) {
            // Timeout:
            continue;
        } 

        rcvbuf[rxbytes] = '\0';
        RISCV_debug("i=>[%d]: %s", rxbytes, rcvbuf);
        tcpcmd_->updateData(rcvbuf, rxbytes);
        int tsz = tcpcmd_->response_size();
        if (tsz != 0) {
            sendData(tcpcmd_->response_buf(), tsz);
        }
        tcpcmd_->done();
    }

    if (listenDefaultOutput_.to_bool()) {
        RISCV_remove_default_output(static_cast<IRawListener *>(this));
    }
    closeSocket();
}

int TcpClient::sendData(uint8_t *buf, int sz) {
    RISCV_mutex_lock(&mutexTx_);
    memcpy(txbuf_, buf, sz);
    txbuf_[sz] = '\0';
    txcnt_ = sz;
    RISCV_mutex_unlock(&mutexTx_);

    //RISCV_debug("o<=[%d]: %s", txcnt_, txbuf_);

    int total = txcnt_;
    char *ptx = txbuf_;
    int txbytes;

    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes <= 0) {
            RISCV_error("Send error: txcnt=%d", txcnt_);
            loopEnable_.state = false;
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
