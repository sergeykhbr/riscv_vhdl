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

#include "cpu_riscv_func.h"
#include "cpu_stub_fpga.h"
#include "icache_func.h"
#include "dmi/dmifunc.h"
#include "dmi/dtmfunc.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS_IDX(CpuRiver_Functional, 1);
    REGISTER_CLASS_IDX(CpuStubRiscVFpga, 2);
    REGISTER_CLASS_IDX(ICacheFunctional, 3);
    REGISTER_CLASS_IDX(DmiFunctional, 4);
    REGISTER_CLASS_IDX(DtmFunctional, 5);
}

}  // namespace debugger
