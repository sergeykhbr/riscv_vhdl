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
    NVIC_STIR(this, "NVIC_STIR", 0xF00) {
    char tstr[64];
    for (int i = 0; i < 8; i++) {
        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_ISER%d", i);
        NVIC_ISERx[i] = new ISER_TYPE(this, tstr, 0x100 + 4*i, 32*i);

        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_ICER%d", i);
        NVIC_ICERx[i] = new ICER_TYPE(this, tstr, 0x180 + 4*i, 32*i);

        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_ISPR%d", i);
        NVIC_ISPRx[i] = new ISPR_TYPE(this, tstr, 0x200 + 4*i, 32*i);

        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_ICPR%d", i);
        NVIC_ICPRx[i] = new ICPR_TYPE(this, tstr, 0x280 + 4*i, 32*i);

        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_IABR%d", i);
        NVIC_IABRx[i] = new IABR_TYPE(this, tstr, 0x300 + 4*i, 32*i);
    }
    for (int i = 0; i < 61; i++) {
        RISCV_sprintf(tstr, sizeof(tstr), "NVIC_IPR%d", i);
        NVIC_IPRx[i] = new IPR_TYPE(this, tstr, 0x400 + 4*i, 4*i);
    }
}

STM32L4_NVIC::~STM32L4_NVIC() {
    for (int i = 0; i < 8; i++) {
        delete NVIC_ISERx[i];
        delete NVIC_ICERx[i];
        delete NVIC_ISPRx[i];
        delete NVIC_ICPRx[i];
        delete NVIC_IABRx[i];
    }
    for (int i = 0; i < 61; i++) {
        delete NVIC_IPRx[i];
    }
}

void STM32L4_NVIC::postinitService() {
    uint64_t baseaddr = 0xe000e000;
    for (int i = 0; i < 8; i++) {
        NVIC_ISERx[i]->setBaseAddress(baseaddr + 0x100 + 4*i);
        NVIC_ICERx[i]->setBaseAddress(baseaddr + 0x180 + 4*i);
        NVIC_ISPRx[i]->setBaseAddress(baseaddr + 0x200 + 4*i);
        NVIC_ICPRx[i]->setBaseAddress(baseaddr + 0x280 + 4*i);
        NVIC_IABRx[i]->setBaseAddress(baseaddr + 0x300 + 4*i);
    }
    for (int i = 0; i < 61; i++) {
        NVIC_IPRx[i]->setBaseAddress(baseaddr + 0x400 + 4*i);
    }
}

void STM32L4_NVIC::enableInterrupt(int idx) {
    int regidx = idx >> 5;
    int regoff = idx & 0x1F;
    uint32_t t = NVIC_IABRx[regidx]->getValue().val;

    // 29  TIM3_IRQn
    // 54  TIM6_DAC_IRQn
    RISCV_info("Enabling Interrupt %d", idx);
    t |= (1ul << regoff);
    NVIC_IABRx[regidx]->setValue(t);
}

void STM32L4_NVIC::disableInterrupt(int idx) {
    int regidx = idx >> 5;
    int regoff = idx & 0x1F;
    uint32_t t = NVIC_IABRx[regidx]->getValue().val;

    RISCV_info("Disabling Interrupt %d", idx);
    t &= ~(1ul << regoff);
    NVIC_IABRx[regidx]->setValue(t);
}


uint32_t STM32L4_NVIC::ISER_TYPE::aboutToWrite(uint32_t cur_val) {
    STM32L4_NVIC *p = static_cast<STM32L4_NVIC *>(parent_);
    for (int i = 0; i < 32; i++) {
        if (cur_val & (1 << i)) {
            p->enableInterrupt(startidx_ + i);
        }
    }
    return cur_val;
}

uint32_t STM32L4_NVIC::ICER_TYPE::aboutToWrite(uint32_t cur_val) {
    STM32L4_NVIC *p = static_cast<STM32L4_NVIC *>(parent_);
    for (int i = 0; i < 32; i++) {
        if (cur_val & (1 << i)) {
            p->disableInterrupt(startidx_ + i);
        }
    }
    return cur_val;
}

uint32_t STM32L4_NVIC::ISPR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

uint32_t STM32L4_NVIC::ICPR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

uint32_t STM32L4_NVIC::IABR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

uint32_t STM32L4_NVIC::IPR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

uint32_t STM32L4_NVIC::STIR_TYPE::aboutToWrite(uint32_t cur_val) {
    return cur_val;
}

}  // namespace debugger

