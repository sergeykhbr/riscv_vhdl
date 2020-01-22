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
#include "stm32l4_nvic.h"

namespace debugger {

STM32L4_NVIC::STM32L4_NVIC(const char *name) : IService(name),
    NVIC_ISER0(this, "NVIC_ISER0", 0x100),
    NVIC_ISER1(this, "NVIC_ISER1", 0x104),
    NVIC_ISER2(this, "NVIC_ISER2", 0x108),
    NVIC_ISER3(this, "NVIC_ISER3", 0x10C),
    NVIC_ISER4(this, "NVIC_ISER4", 0x110),
    NVIC_ISER5(this, "NVIC_ISER5", 0x114),
    NVIC_ISER6(this, "NVIC_ISER6", 0x118),
    NVIC_ISER7(this, "NVIC_ISER7", 0x11C) {
}

void STM32L4_NVIC::postinitService() {
    uint64_t baseaddr = 0xe000e000;
    NVIC_ISER0.setBaseAddress(baseaddr + 0x100);
    NVIC_ISER1.setBaseAddress(baseaddr + 0x104);
    NVIC_ISER2.setBaseAddress(baseaddr + 0x108);
    NVIC_ISER3.setBaseAddress(baseaddr + 0x10C);
    NVIC_ISER4.setBaseAddress(baseaddr + 0x110);
    NVIC_ISER5.setBaseAddress(baseaddr + 0x114);
    NVIC_ISER6.setBaseAddress(baseaddr + 0x118);
    NVIC_ISER7.setBaseAddress(baseaddr + 0x11C);
}

}  // namespace debugger

