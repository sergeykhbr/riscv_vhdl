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

#include "tcpsrv_jtagbb.h"

namespace debugger {

IThread *TcpServerJtagBitBang::createClientThread(const char *name, socket_def skt) {

    ClientThread *cln = new ClientThread(this,
                                         name,
                                         skt,
                                         recvTimeout_.to_int(),
                                         jtagtap_.to_string());
    cln->run();
    return cln;
}


TcpServerJtagBitBang::ClientThread::ClientThread(TcpServerJtagBitBang *parent,
                                                const char *name,
                                                socket_def skt,
                                                int recvTimeout,
                                                const char *jtagtap)
    : TcpServer::ClientThreadGeneric(parent, name, skt, recvTimeout) {
    ijtagbb_ = static_cast<IJtagBitBang *>(
        RISCV_get_service_iface(jtagtap, IFACE_JTAG_BITBANG));
}

TcpServerJtagBitBang::ClientThread::~ClientThread() {
}

int TcpServerJtagBitBang::ClientThread::processRxBuffer(const char *cmdbuf, int bufsz) {
    int ret = 0;

    for (int i = 0; i < bufsz; i++) {
        switch (cmdbuf[i]) {
        case 'B':
            RISCV_debug("%s", "Blink on");
            break;
        case 'b':
            RISCV_debug("%s", "Blink off");
            break;
        case 'r':
            ijtagbb_->resetTAP(0, 0);
            break;
        case 's':
            ijtagbb_->resetTAP(0, 1);
            break;
        case 't':
            ijtagbb_->resetTAP(1, 0);
            break;
        case 'u':
            ijtagbb_->resetTAP(1, 1);
            break;
        case '0':
            ijtagbb_->setPins(0, 0, 0);
            break;
        case '1':
            ijtagbb_->setPins(0, 0, 1);
            break;
        case '2':
            ijtagbb_->setPins(0, 1, 0);
            break;
        case '3':
            ijtagbb_->setPins(0, 1, 1);
            break;
        case '4':
            ijtagbb_->setPins(1, 0, 0);
            break;
        case '5':
            ijtagbb_->setPins(1, 0, 1);
            break;
        case '6':
            ijtagbb_->setPins(1, 1, 0);
            break;
        case '7':
            ijtagbb_->setPins(1, 1, 1);
            break;
        case 'R':
            writeTxBuffer(ijtagbb_->getTDO() ? "1" : "0", 1);
            break;
        case 'Q':
            ret = -1;
            break;
        default:
            RISCV_error("Unsupported command '%c'\n", cmdbuf[i]);
            ret = -1;
        }
    }

    return ret;
}

}  // namespace debugger
