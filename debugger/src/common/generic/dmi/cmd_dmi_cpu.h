/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "coreservices/icommand.h"
#include <generic-isa.h>

namespace debugger {

class CmdDmiCpuGneric : public ICommand  {
 public:
    explicit CmdDmiCpuGneric(IFace *parent, uint64_t dmibar, ITap *tap);

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 protected:
    virtual const ECpuRegMapping *getpMappedReg() = 0;
    virtual const uint32_t reg2addr(const char *name);

 private:
    void clearcmderr();
    void resume();
    void halt();
    void waithalted();
    void waitbusy();
    void setStep(bool val);
    void readreg(uint32_t regno, uint8_t *buf8);
    void writereg(uint32_t regno, uint8_t *buf8);
};

class CmdDmiCpuRiscV : public CmdDmiCpuGneric {
 public:
    explicit CmdDmiCpuRiscV(IService *parent) : CmdDmiCpuGneric(parent, 0, 0) {}

 protected:
    virtual const ECpuRegMapping *getpMappedReg();
};

}  // namespace debugger

