/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "stm32l4_systick.h"

namespace debugger {

STM32L4_SysTick::STM32L4_SysTick(const char *name) : IService(name),
    STK_CTRL(this, "STK_CTRL", 0x00),
    STK_LOAD(this, "STK_LOAD", 0x04),
    STK_VAL(this, "STK_VAL", 0x08),
    STK_CALIB(this, "STK_CALIB", 0x0C) {
}

void STM32L4_SysTick::postinitService() {
    uint64_t baseaddr = 0xE000E010;
    STK_CTRL.setBaseAddress(baseaddr + 0x00);
    STK_LOAD.setBaseAddress(baseaddr + 0x04);
    STK_VAL.setBaseAddress(baseaddr + 0x08);
    STK_CALIB.setBaseAddress(baseaddr + 0x0C);
}

}  // namespace debugger

