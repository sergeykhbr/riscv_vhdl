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

namespace debugger {

class TcpClient : public IService,
                  public IThread {
 public:
    explicit TcpClient(IService *parent, const char *name);
    virtual ~TcpClient();

    /** IService interface */
    virtual void postinitService() override;

 protected:
    /** IThread interface */
    virtual void busyLoop();

    /** TcpClient generic methods: */
    virtual int processRxBuffer(const char *ibuf, int ilen) = 0;
    virtual int writeTxBuffer(const char *buf, int sz);

 protected:
    virtual int connectToServer();       // create socket only if Server doesn't create it
    virtual int setRecvTimeout(int timeout_ms);
    virtual int sendData();
    virtual void afterThreadStarted() {};
    virtual void beforeThreadClosing() {};
    virtual void closeSocket();

 protected:
    AttributeType targetIP_;
    AttributeType targetPort_;
    AttributeType recvTimeout_;

    struct sockaddr_in sockaddr_ipv4_;
    socket_def hsock_;

    mutex_def mutexTx_;
    char rxbuf_[4096];
    char txbuf_[1<<20];
    char txbuf2_[1<<20];    // double buffering
    int txcnt_;
};

}  // namespace debugger
