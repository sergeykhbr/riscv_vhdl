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

#include <attribute.h>
#include <ithread.h>
#include <icommand.h>
#include "c_dpi.h"

class DpiClient : public IThread,
                  public ICommand {
 public:
    explicit DpiClient(socket_def skt);
    virtual ~DpiClient();

    void postinit(const AttributeType &config);

    /** ICommand */
    virtual const AttributeType &getRequest() { return request_; }
    virtual void setResponse(const AttributeType &resp);

 protected:
    virtual void busyLoop();

 protected:
    void processRequest();
    void closeSocket();
    void sendBuffer(const char *buf, unsigned size);

 private:
    AttributeType config_;
    AttributeType request_;
    AttributeType response_;
    AttributeType keepAlive_;
    AttributeType wrongFormat_;

    socket_def hsock_;
    event_def event_cmd_;
    char rcvbuf[4096];
    char cmdbuf_[4096];
    int cmdcnt_;
};

