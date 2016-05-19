/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RISC-V simulating CPU interface.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
#define __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_CPU_RISCV = "ICpuRiscV";

class ICpuRiscV : public IFace {
public:
    ICpuRiscV() : IFace(IFACE_CPU_RISCV) {}

    virtual bool isHalt() =0;
    virtual void halt() =0;
    virtual void go() =0;
    virtual void step(uint64_t cnt) =0;
    virtual uint64_t getReg(uint64_t idx) =0;
    virtual void setReg(uint64_t idx, uint64_t val) =0;
    virtual uint64_t getPC() =0;
    virtual void setPC(uint64_t val) =0;
    virtual uint64_t getNPC() =0;
    virtual void setNPC(uint64_t val) =0;
    virtual void addBreakpoint(uint64_t addr) =0;
    virtual void removeBreakpoint(uint64_t addr) =0;
    virtual void hitBreakpoint(uint64_t addr) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
