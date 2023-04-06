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

#include "coreservices/ithread.h"
#include "coreservices/icmdexec.h"
#include "coreservices/irawlistener.h"
#include "generic/tcpserver.h"

namespace debugger {

class TcpServerRpc : public TcpServer {
 public:
    TcpServerRpc(const char *name) : TcpServer(name) {
        registerAttribute("CmdExecutor", &cmdexec_);
    }
 protected:
    virtual IThread *createClientThread(const char *name, socket_def skt);

 private:
    class ClientThread : public TcpServer::ClientThreadGeneric,
                         public IRawListener {
     public:
        explicit ClientThread(TcpServer *parent,
                              const char *name,
                              socket_def skt,
                              int recvTimeout,
                              const char *cmdexec);

        /** IRawListener */
        virtual int updateData(const char *buf, int buflen);

     protected:
        virtual void beforeThreadClosing() override;

     protected:
        virtual int processRxBuffer(const char *cmdbuf, int bufsz);
        virtual bool isStartMarker(char s) { return true; }
        virtual bool isEndMarker(const char *s, int sz) {
            return s[sz - 1] == '\0';
        }

     private:
        ICmdExecutor *iexec_;
        int respcnt_;
    };

 private:
    AttributeType cmdexec_;
};

DECLARE_CLASS(TcpServerRpc)

}  // namespace debugger

