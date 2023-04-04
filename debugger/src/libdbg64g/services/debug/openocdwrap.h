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

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "coreservices/ithread.h"
#include "coreservices/ijtag.h"
#include "coreservices/icmdexec.h"
#include "generic/tcpclient.h"
#include <string>

namespace debugger {

class OpenOcdWrapper : public TcpClient {
 public:
    explicit OpenOcdWrapper(const char *name);
    virtual ~OpenOcdWrapper();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** TcpClient */
    virtual int processRxBuffer(const char *buf, int sz) { return 0; }

 protected:
    /** IThread interface */
    virtual void busyLoop();

 private:
    class ExternalProcessThread : public IThread {
     public:
        ExternalProcessThread(IService *parent) : IThread() {
            opened_ = true;
        }
        bool isOpened() { return opened_; }
     protected:
        virtual void busyLoop();
     private:
        event_def eventLoopStarted_;
        bool opened_;
    };


 private:
    AttributeType isEnable_;
    AttributeType jtag_;
    AttributeType cmdexec_;
    AttributeType pollingMs_;
 
    IJtag *ijtag_;
    ICmdExecutor *icmdexec_;

    event_def config_done_;
    ExternalProcessThread openocd_;
};

DECLARE_CLASS(OpenOcdWrapper)

}  // namespace debugger
