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

#include <api_core.h>
#include <iservice.h>
#include <ihap.h>
#include "coreservices/icommand.h"
#include "coreservices/isrccode.h"

namespace debugger {

class CmdBr : public ICommand,
              public IHap  {
 public:
    explicit CmdBr(IService *parent, IJtag *ijtag);
    virtual ~CmdBr();

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

    /** IHap */
    virtual void hapTriggered(EHapType type,
                              uint64_t param,
                              const char *descr) {
        RISCV_event_set(&eventHalted_);
    }

 protected:
    virtual bool isHalted();
    virtual bool isHardware(uint64_t flags) {
        return (flags & BreakFlag_HW) ? true: false;
    }
    virtual void getSwBreakpointInstr(Reg64Type *instr, uint32_t *len) = 0;

 protected:
    ISourceCode *isrc_;
    event_def eventHalted_;
};

}  // namespace debugger

