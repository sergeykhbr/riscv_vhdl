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
#include "stm32l4_rcc.h"

namespace debugger {

STM32L4_RCC::STM32L4_RCC(const char *name) : RegMemBankGeneric(name),
    CR(this, "CR", 0x0),
    ICSCR(this, "ICSCR", 0x4),
    CFGR(this, "CFGR", 0x08),
    PLLCFGR(this, "PLLCFGR", 0x0C),
    CIER(this, "CIER", 0x18) {
}

void STM32L4_RCC::postinitService() {
    RegMemBankGeneric::postinitService();
}

uint32_t STM32L4_RCC::RCC_CR_TYPE::aboutToRead(uint32_t cur_val) {
    value_type ret;
    ret.v = cur_val;
    ret.b.MSIRDY = ret.b.MSION;
    ret.b.HSIRDY = ret.b.HSION;
    ret.b.HSERDY = ret.b.HSEON;
    ret.b.PLLRDY = ret.b.PLLON;
    ret.b.PLLSAI1RDY = ret.b.PLLSAI1ON;
    ret.b.PLLSAI2RDY = ret.b.PLLSAI2ON;
    return ret.v;
}

uint32_t STM32L4_RCC::RCC_CFGR_TYPE::aboutToRead(uint32_t cur_val) {
    value_type ret;
    ret.v = cur_val;
    ret.b.SWS = ret.b.SW;
    return ret.v;
}

uint32_t STM32L4_RCC::RCC_PLLCFGR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

uint32_t STM32L4_RCC::RCC_CIER_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

}  // namespace debugger

