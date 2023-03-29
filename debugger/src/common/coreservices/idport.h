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

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_DPORT = "IDPort";

class IDPort : public IFace {
 public:
    IDPort() : IFace(IFACE_DPORT) {}

    virtual int resumereq() = 0;
    virtual int haltreq() = 0;
    virtual bool isHalted() = 0;
    virtual bool isResumeAck() = 0;
    virtual void setHaltOnReset() = 0;
    virtual void clrHaltOnReset() = 0;

    virtual int dportReadReg(uint32_t regno, uint64_t *val) = 0;
    virtual int dportWriteReg(uint32_t regno, uint64_t val) = 0;

    virtual int dportReadMem(uint64_t addr, uint32_t virt, uint32_t sz, uint64_t *payload) = 0;
    virtual int dportWriteMem(uint64_t addr, uint32_t virt, uint32_t sz, uint64_t payload) = 0;

    virtual bool executeProgbuf(uint32_t *progbuf) = 0;
    virtual bool isExecutingProgbuf() = 0;
    virtual void setResetPin(bool val) = 0;
};

}  // namespace debugger

