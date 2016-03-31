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

    virtual void halt() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
