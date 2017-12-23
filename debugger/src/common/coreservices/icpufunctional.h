/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Functional CPU model interface.
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

static const char *const IFACE_CPU_FUNCTIONAL = "ICpuFunctional";

class ICpuFunctional : public IFace {
 public:
    ICpuFunctional() : IFace(IFACE_CPU_FUNCTIONAL) {}

    virtual void raiseSoftwareIrq() = 0;
    virtual uint64_t getPC() = 0;
    virtual void setBranch(uint64_t npc) = 0;
    virtual void pushStackTrace() = 0;
    virtual void popStackTrace() = 0;
    virtual uint64_t getPrvLevel() = 0;
    virtual void setPrvLevel(uint64_t lvl) = 0;
    virtual void dma_memop(Axi4TransactionType *tr) = 0;
    virtual bool isOn() = 0;
    virtual bool isHalt() = 0;
    virtual bool isSwBreakpoint() = 0;
    virtual bool isHwBreakpoint() = 0;
    virtual void go() = 0;
    virtual void halt(const char *descr) = 0;
    virtual void step() = 0;
    virtual void addHwBreakpoint(uint64_t addr) = 0;
    virtual void removeHwBreakpoint(uint64_t addr) = 0;
    virtual void skipBreakpoint() = 0;

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
