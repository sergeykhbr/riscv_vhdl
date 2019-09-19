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

#include "c_dpi.h"
#include <iface.h>
#include <attribute.h>

typedef struct ServerDataType {
    volatile int enable;
    dpi_data_t dpi_data;
    request_t request;
    char txbuf[1 << 16];
    volatile int txcnt;
} ServerDataType;

static const char *DPI_SERVER_IFACE = "DpiServer";

class DpiServer : public IFace {
 public:
    DpiServer();

    /** create and start seperate thread */
    virtual bool run();

    /** @brief Stop and join thread */
    virtual void stop();

    /** check thread status */
    virtual bool isEnabled() { return loopEnable_.state; }

    /** Pass data from the parent thread */
    virtual void setExtArgument(void *args) {}

    int getRequest(AttributeType &req);
    void sendResponse(AttributeType &resp);

 protected:
    /** working cycle function */
    virtual void busyLoop();

    static void runThread(void *arg) {
        reinterpret_cast<DpiServer *>(arg)->busyLoop();
#if defined(_WIN32) || defined(__CYGWIN__)
        _endthreadex(0);
#else
        pthread_exit(0);
#endif
    }

 protected:
    event_def loopEnable_;
    LibThreadType threadInit_;
    request_t request_;
};

