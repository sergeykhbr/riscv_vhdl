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
#include "coreservices/ithread.h"
#include "tcpclient.h"

namespace debugger {

class TcpServer : public IService,
                  public IThread {
 public:
    explicit TcpServer(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 protected:
    /** IThread interface */
    virtual void busyLoop();

    class ClientThreadGeneric : public TcpClient {
     public:
        ClientThreadGeneric(TcpServer *parent,
                            const char *name,
                            socket_def skt,
                            int recvTimeout)
        : TcpClient(parent, name) {
            hsock_ = skt;
            recvTimeout_.make_int64(recvTimeout);
            setRecvTimeout(recvTimeout);
        }
    };

 protected:
    virtual IThread *createClientThread(const char *name, socket_def skt) = 0;

    int createServerSocket();
    void closeServerSocket();
    bool setBlockingMode(bool mode);

 protected:
    AttributeType isEnable_;
    AttributeType blockmode_;
    AttributeType hostIP_;
    AttributeType hostPort_;
    AttributeType recvTimeout_;

    struct sockaddr_in sockaddr_ipv4_;
    socket_def hsock_;
    char rcvbuf[4096];
};

}  // namespace debugger
