/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "coreservices/icmdexec.h"
#include <string>

namespace debugger {

class CpuMonitor : public IService,
                   public IThread,
                   public IHap {
 public:
    explicit CpuMonitor(const char *name);
    virtual ~CpuMonitor();

    /** IService interface */
    virtual void postinitService() override;

    /** IHap */
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

 protected:
    /** IThread interface */
    virtual void busyLoop();

    uint64_t getStatus();
    // execute current breakpoint from progbuf
    void stepOverSwBreakpoint();
    void writeBreakpoints();
    void removeBreakpoints();

 private:
    AttributeType cmdexec_;
    AttributeType pollingMs_;
    AttributeType statusResponse_;
    AttributeType npcResponse_;
    AttributeType brList_;
    AttributeType writeMemResp_;
 
    ICmdExecutor *icmdexec_;

    event_def config_done_;
    mutex_def mutex_resume_;
    uint64_t hartsel_;      // context switched Hart index
    uint64_t haltsum_;
};

DECLARE_CLASS(CpuMonitor)

}  // namespace debugger
