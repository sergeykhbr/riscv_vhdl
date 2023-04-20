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
#include "coreservices/ijtagbitbang.h"
#include "generic/tcpserver.h"

namespace debugger {

class TcpServerJtagBitBang : public TcpServer {
 public:
    TcpServerJtagBitBang(const char *name) : TcpServer(name) {
        registerAttribute("JtagTap", &jtagtap_);
    }
 protected:
    virtual IThread *createClientThread(const char *name, socket_def skt);

 private:
    class ClientThread : public TcpServer::ClientThreadGeneric {
     public:
        explicit ClientThread(TcpServerJtagBitBang *parent,
                              const char *name,
                              socket_def skt,
                              int recvTimeout,
                              const char *jtagtap);
        virtual ~ClientThread();

     protected:
        virtual int processRxBuffer(const char *cmdbuf, int bufsz);

     private:
        IJtagBitBang *ijtagbb_;
    };

 private:
    AttributeType jtagtap_;
};


DECLARE_CLASS(TcpServerJtagBitBang)

}  // namespace debugger

