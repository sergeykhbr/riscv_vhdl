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
    registerAttribute("CPU", &cpu_);
    registerAttribute("IrqController", &irqctrl_);
    registerAttribute("IrqId", &irqid_);

    lastReloadTime_ = 0;
}

void STM32L4_SysTick::postinitService() {
    uint64_t baseaddr = 0xE000E010;
    STK_CTRL.setBaseAddress(baseaddr + 0x00);
    STK_LOAD.setBaseAddress(baseaddr + 0x04);
    STK_VAL.setBaseAddress(baseaddr + 0x08);
    STK_CALIB.setBaseAddress(baseaddr + 0x0C);

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't find IClock interface %s", cpu_.to_string());
        return;
    }

    iirq_ = static_cast<IIrqController *>(
        RISCV_get_service_iface(irqctrl_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirq_) {
        RISCV_error("Can't find IIrqController interface %s", irqctrl_.to_string());
        return;
    }
}

void STM32L4_SysTick::stepCallback(uint64_t t) {
    STK_CTRL_TYPE::value_type ctrl = STK_CTRL.getTyped();
    uint64_t dt = getReloadSteps();
    lastReloadTime_ = t;

    ctrl.b.COUNTFLAG = 1;
    STK_CTRL.setValue(ctrl.v);
    if (ctrl.b.TICKINT) {
        iirq_->requestInterrupt(static_cast<IService *>(this),
                                irqid_.to_int());
    }
    if (ctrl.b.ENABLE) {
        iclk_->moveStepCallback(static_cast<IClockListener *>(this), t + dt);
    }
}

uint64_t STM32L4_SysTick::getReloadSteps() {
    STK_CTRL_TYPE::value_type ctrl = STK_CTRL.getTyped();
    uint64_t ret = (STK_LOAD.getValue().val & 0x00FFFFFF) + 1;
    if (ctrl.b.CLKSOURCE == 0) {
        ret *= 8;
    }
    return ret;
}

void STM32L4_SysTick::enableCounter() {
    uint64_t dt = getReloadSteps();
    lastReloadTime_ = iclk_->getStepCounter();
    iclk_->moveStepCallback(static_cast<IClockListener *>(this),
         lastReloadTime_ + dt);

    RISCV_info("Enable Counter %.1f ms",
                static_cast<double>(dt) / (0.001*iclk_->getFreqHz()));
}

void STM32L4_SysTick::disableCounter() {
    RISCV_info("%s", "Disable Counter");
}

void STM32L4_SysTick::enableInterrupt() {
    RISCV_info("Enable Interrupt %d", irqid_.to_int());
}

void STM32L4_SysTick::disableInterrupt() {
    RISCV_info("Disable Interrupt %d", irqid_.to_int());
}


uint32_t STM32L4_SysTick::STK_CTRL_TYPE::aboutToRead(uint32_t cur_val) {
    value_type t = getTyped();
    uint32_t ret = t.v;
    t.b.COUNTFLAG = 0;
    setValue(t.v);
    return ret;
}

uint32_t STM32L4_SysTick::STK_CTRL_TYPE::aboutToWrite(uint32_t cur_val) {
    value_type next;
    value_type old = getTyped();
    STM32L4_SysTick *p = static_cast<STM32L4_SysTick *>(parent_);
    next.v = cur_val;
    setValue(cur_val);
    if (!old.b.ENABLE && next.b.ENABLE) {
        p->enableCounter();
    } else if (old.b.ENABLE && !next.b.ENABLE) {
        p->disableCounter();
    }

    if (!old.b.TICKINT && next.b.TICKINT) {
        p->enableInterrupt();
    } else if (old.b.TICKINT && !next.b.TICKINT) {
        p->disableInterrupt();
    }
    return cur_val;
}


}  // namespace debugger

