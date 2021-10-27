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

#include "dmifunc.h"
#include <riscv-isa.h>

namespace debugger {

DmiFunctional::DmiFunctional(const char *name)
    : RegMemBankGeneric(name),
    data0(this, "data0", 0, 0x04*sizeof(uint32_t)),
    data1(this, "data1", 1, 0x05*sizeof(uint32_t)),
    data2(this, "data2", 2, 0x06*sizeof(uint32_t)),
    data3(this, "data3", 3, 0x07*sizeof(uint32_t)),
    dmcontrol(this, "dmcontrol", 0x10*sizeof(uint32_t)),
    command(this, "command", 0x17*sizeof(uint32_t)),
    haltsum0(this, "haltsum0", 0x40*sizeof(uint32_t)) {
}

DmiFunctional::~DmiFunctional() {
}

}  // namespace debugger

