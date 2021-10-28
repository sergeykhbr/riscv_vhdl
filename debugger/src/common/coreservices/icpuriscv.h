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

#ifndef __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
#define __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_CPU_RISCV = "ICpuRiscV";

/** Signal types */
//static const int CPU_SIGNAL_RESET   = 0;
//static const int CPU_SIGNAL_EXT_IRQ = 1;

class ICpuRiscV : public IFace {
public:
    ICpuRiscV() : IFace(IFACE_CPU_RISCV) {}

    // atomic instruction LR/SC reservation
    virtual void mmuAddrReserve(uint64_t addr) = 0;
    virtual bool mmuAddrRelease(uint64_t addr) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
