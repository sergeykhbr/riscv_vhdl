/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <ithread.h>
#include <attribute.h>
#include <icommand.h>
#include "c_dpi.h"
#include "client.h"

class DpiServer : public IThread,
                  public IThreadListener,
                  public ICommand {
 public:
    explicit DpiServer();
    virtual ~DpiServer();

    void postinit(const AttributeType &config);

    /** IThreadListener */
    virtual void threadExit(IFace *iface);

    /** ICommand */
    virtual const AttributeType &getRequest() { return request_; }
    virtual void setResponse(const AttributeType &resp);

 protected:
    /** working cycle function */
    virtual void busyLoop();

    int createServerSocket();
    void closeServerSocket();
    bool setBlockingMode(bool mode);

    DpiClient *addClient(socket_def skt);

    void message_hartbeat();
    void message_client_connected();
    void message_client_disconnected();

 protected:
    AttributeType config_;
    AttributeType listClient_;
    AttributeType request_;

    mutex_def mutex_clients_;
    event_def event_cmd_;

    struct sockaddr_in sockaddr_ipv4_;
    socket_def hsock_;
    char rcvbuf[4096];
};

