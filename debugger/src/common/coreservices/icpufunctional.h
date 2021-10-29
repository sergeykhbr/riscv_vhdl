/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICPUFUNCTIONAL_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICPUFUNCTIONAL_H__

#include <inttypes.h>
#include <iface.h>
#include <api_types.h>
#include "coreservices/imemop.h"

namespace debugger {

static const char *const IFACE_INSTRUCTION = "IInstruction";

class IInstruction : public IFace {
 public:
    IInstruction() : IFace(IFACE_INSTRUCTION) {}

    virtual const char *name() = 0;
    /** Return instruction size */
    virtual int exec(Reg64Type *payload) = 0;
};

class GenericInstruction : public IInstruction {
 public:
    GenericInstruction() : IInstruction() {}
};

enum EEndianessType {
    LittleEndian,
    BigEndian,
};

/*enum EHaltCause {
    HaltDoNotChange,
    HaltSwBreakpoint,
    HaltHwTrigger,
    HaltExternal,
    HaltStepping
};*/

static const char *const IFACE_CPU_FUNCTIONAL = "ICpuFunctional";

class ICpuFunctional : public IFace {
 public:
    ICpuFunctional() : IFace(IFACE_CPU_FUNCTIONAL) {}

    virtual void raiseSoftwareIrq() = 0;
    virtual uint64_t *getpRegs() = 0;
    virtual uint64_t getPC() = 0;
    virtual void setPC(uint64_t v) = 0;
    virtual uint64_t getNPC() = 0;
    virtual void setNPC(uint64_t v) = 0;

    virtual void setBranch(uint64_t npc) = 0;
    virtual void pushStackTrace() = 0;
    virtual void popStackTrace() = 0;
    virtual uint64_t getPrvLevel() = 0;
    virtual void setPrvLevel(uint64_t lvl) = 0;
    virtual ETransStatus dma_memop(Axi4TransactionType *tr) = 0;
    virtual void exceptionLoadInstruction(Axi4TransactionType *tr) = 0;
    virtual void exceptionLoadData(Axi4TransactionType *tr) = 0;
    virtual void exceptionStoreData(Axi4TransactionType *tr) = 0;
    virtual bool isOn() = 0;
    virtual void halt(uint32_t cause, const char *descr) = 0;
    virtual void addHwBreakpoint(uint64_t addr) = 0;
    virtual void removeHwBreakpoint(uint64_t addr) = 0;
    virtual void flush(uint64_t addr) = 0;
    virtual void doNotCache(uint64_t addr) = 0;

  protected:
    virtual uint64_t getResetAddress() = 0;
    virtual uint64_t getIrqAddress(int idx) = 0;
    virtual EEndianessType endianess() = 0;
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache) = 0;
    virtual void generateIllegalOpcode() = 0;
    virtual void handleTrap() = 0;
    /** Tack Registers changes during execution */
    virtual void trackContextStart() = 0;
    /** Stop tracking and write trace file */
    virtual void trackContextEnd() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICPUFUNCTIONAL_H__
