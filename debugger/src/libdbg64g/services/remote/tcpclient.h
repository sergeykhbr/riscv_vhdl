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

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ithread.h"
#include "coreservices/irawlistener.h"

namespace debugger {

class TcpClient : public IService,
                  public IThread,
                  public IRawListener {
 public:
    explicit TcpClient(const char *name);
    virtual ~TcpClient();

    /** IService interface */
    virtual void postinitService() override;
    virtual void setExtArgument(void *args) override {
        hsock_ = *reinterpret_cast<socket_def *>(args);
    }

    /** IRawListener interface */
    virtual int updateData(const char *buf, int buflen);

 protected:
    /** IThread interface */
    virtual void busyLoop();

    virtual void processData(const char *ibuf, int ilen,
                             const char *obuf, int *olen) = 0;

 protected:
    int sendData(const char *buf, int sz);
    int connectToServer();       // create socket only if Server doesn't create it
    void closeSocket();

 private:
    AttributeType isEnable_;
    AttributeType timeout_;
    AttributeType targetIP_;
    AttributeType targetPort_;

    struct sockaddr_in sockaddr_ipv4_;

    socket_def hsock_;
    mutex_def mutexTx_;
    char rxbuf_[4096];
    char txbuf_[1<<20];
    int txcnt_;
    union reg8_type {
        char ibyte;
        uint8_t ubyte;
    } asyncbuf_[1 << 20];
};

}  // namespace debugger
