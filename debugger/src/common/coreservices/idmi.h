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

static const char *const IFACE_DMI = "IDmi";

enum ECmdErrType {
    CMDERR_NONE = 0,
    CMDERR_BUSY = 1,
    CMDERR_NOTSUPPROTED = 2,
    CMDERR_EXCEPTION = 3,
    CMDERR_WRONGSTATE = 4,
    CMDERR_BUSERROR = 5,
    CMDERR_OTHERS = 7,
};


class IDmi : public IFace {
 public:
    IDmi() : IFace(IFACE_DMI) {}

    // Must be 2*n value
    virtual int getCpuMax() = 0;
    virtual int getRegTotal() = 0;
    virtual int getProgbufTotal() = 0;
    virtual uint32_t getHartSelected() = 0;
    // Returns resumeack on success
    virtual void set_resumereq(uint32_t hartsel) = 0;
    virtual bool get_resumeack(uint32_t hartsel) = 0;
    virtual void set_haltreq(uint32_t hartsel) = 0;
    virtual void set_hartreset(uint32_t hartsel) = 0;
    virtual void clear_hartreset(uint32_t hartsel) = 0;
    // Halt on reset request
    virtual void set_resethaltreq(uint32_t hartsel) = 0;
    virtual void clear_resethaltreq(uint32_t hartsel) = 0;
    // Full system reset
    virtual void set_ndmreset() = 0;
    virtual void clear_ndmreset() = 0;

    virtual void set_cmderr(ECmdErrType cmderr) = 0;
    virtual ECmdErrType get_cmderr() = 0;

    virtual bool isHalted(uint32_t hartsel) = 0;
    virtual bool isAvailable(uint32_t hartsel) = 0;

    virtual void readTransfer(uint32_t regno, uint32_t size) = 0;
    virtual void writeTransfer(uint32_t regno, uint32_t size) = 0;
    virtual bool isAutoexecData(int idx) = 0;
    virtual bool isAutoexecProgbuf(int idx) = 0;
    virtual void executeCommand() = 0;
};

}  // namespace debugger

