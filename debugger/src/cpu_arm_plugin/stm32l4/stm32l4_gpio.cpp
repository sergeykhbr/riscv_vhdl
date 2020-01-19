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
#include "stm32l4_gpio.h"

namespace debugger {

STM32L4_GPIO::STM32L4_GPIO(const char *name) : RegMemBankGeneric(name),
    MODER(this, "MODER", 0x00),
    OTYPER(this, "OTYPER", 0x04),
    OSPEEDR(this, "OSPEEDR", 0x08),
    PUPDR(this, "PUPDR", 0x0c),
    IDR(this, "IDR", 0x10),
    ODR(this, "ODR", 0x14),
    BSRR(this, "BSRR", 0x18),
    LCKR(this, "LCKR", 0x1c),
    AFRL(this, "AFRL", 0x20),
    AFRH(this, "AFRH", 0x24),
    BRR(this, "BRR", 0x28) {
}

void STM32L4_GPIO::postinitService() {
    RegMemBankGeneric::postinitService();

    MODER.setBaseAddress(baseAddress_.to_uint64() + 0x00);
    OTYPER.setBaseAddress(baseAddress_.to_uint64() + 0x04);
    OSPEEDR.setBaseAddress(baseAddress_.to_uint64() + 0x08);
    PUPDR.setBaseAddress(baseAddress_.to_uint64() + 0x0c);
    IDR.setBaseAddress(baseAddress_.to_uint64() + 0x10);
    ODR.setBaseAddress(baseAddress_.to_uint64() + 0x14);
    BSRR.setBaseAddress(baseAddress_.to_uint64() + 0x18);
    LCKR.setBaseAddress(baseAddress_.to_uint64() + 0x1c);
    AFRL.setBaseAddress(baseAddress_.to_uint64() + 0x20);
    AFRH.setBaseAddress(baseAddress_.to_uint64() + 0x24);
    BRR.setBaseAddress(baseAddress_.to_uint64() + 0x28);
}

}  // namespace debugger

