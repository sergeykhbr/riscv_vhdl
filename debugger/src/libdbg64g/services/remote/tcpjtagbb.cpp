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

#include "tcpjtagbb.h"

namespace debugger {

TcpJtagBitBangClient::TcpJtagBitBangClient(const char *name)
    : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("JtagTap", &jtagtap_);

    RISCV_mutex_init(&mutexTx_);
}

TcpJtagBitBangClient::~TcpJtagBitBangClient() {
    RISCV_mutex_destroy(&mutexTx_);
}

void TcpJtagBitBangClient::postinitService() {
    if (jtagtap_.is_list()) {
        itap_ = static_cast<IJtagTap *>(
            RISCV_get_service_port_iface(jtagtap_[0u].to_string(),
                                         jtagtap_[1].to_string(),
                                         IFACE_JTAG_TAP));
    } else {
        itap_ = static_cast<IJtagTap *>(
            RISCV_get_service_iface(jtagtap_.to_string(), IFACE_JTAG_TAP));
    }
    if (!itap_) {
        RISCV_error("IJtagTap interface '%s' not found", 
                    jtagtap_.to_string());
        return;
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void TcpJtagBitBangClient::busyLoop() {
    int rxbytes;
    int tsz = 0;
    bool quit = false;

    while (isEnabled()) {
        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);
        if (rxbytes <= 0 || quit) {
            break;
        } 

        txcnt_ = 0;
        for (int i = 0; i < rxbytes; i++) {
            switch (rcvbuf[i]) {
            case 'B':
                //RISCV_debug("%s", "*BLINK*\n");
                break;
            case 'b':
                //RISCV_debug("%s", "_______\n");
                break;
            case 'r':
                itap_->resetTAP();
                break;
            case '0':
                itap_->setPins(0, 0, 0);
                break;
            case '1':
                itap_->setPins(0, 0, 1);
                break;
            case '2':
                itap_->setPins(0, 1, 0);
                break;
            case '3':
                itap_->setPins(0, 1, 1);
                break;
            case '4':
                itap_->setPins(1, 0, 0);
                break;
            case '5':
                itap_->setPins(1, 0, 1);
                break;
            case '6':
                itap_->setPins(1, 1, 0);
                break;
            case '7':
                itap_->setPins(1, 1, 1);
                break;
            case 'R':
                txbuf_[tsz++] = itap_->getTDO() ? '1' : '0';
                break;
            case 'Q':
                quit = true;
                break;
            default:
                RISCV_error("Unsupported command '%c'\n", rcvbuf[i]);
            }

            /*if (!in_rti && tap->state() == RUN_TEST_IDLE) {
                entered_rti = true;
                break;
            }
            in_rti = false;*/
        }

        if (tsz != 0) {
            sendData(txbuf_, tsz);
        }
        tsz = 0;
    }

    closeSocket();
}

int TcpJtagBitBangClient::sendData(char *buf, int sz) {
    int total = sz;
    char *ptx = buf;
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
    return 0;
}

void TcpJtagBitBangClient::closeSocket() {
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
