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

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "tcpcmd_gen.h"
#include "coreservices/ithread.h"
#include "coreservices/ijtagbitbang.h"

namespace debugger {

class TcpJtagBitBangClient : public IService,
                             public IThread {
 public:
    explicit TcpJtagBitBangClient(const char *name);
    virtual ~TcpJtagBitBangClient();

    /** IService interface */
    virtual void postinitService() override;

 protected:
    /** IThread interface */
    virtual void busyLoop();
    virtual void setExtArgument(void *args) {
        hsock_ = *reinterpret_cast<socket_def *>(args);
    }

 protected:
    int sendData(char *buf, int sz);
    void closeSocket();

 private:
    AttributeType isEnable_;
    AttributeType jtagtap_;

    IJtagBitBang *ijtagbb_;

    socket_def hsock_;
    mutex_def mutexTx_;
    char rcvbuf[4096];
    char txbuf_[1<<20];
    int txcnt_;
    union reg8_type {
        char ibyte;
        uint8_t ubyte;
    } asyncbuf_[1 << 20];
};

DECLARE_CLASS(TcpJtagBitBangClient)

}  // namespace debugger

