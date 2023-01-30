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
    ICommand(this, name),
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
    registerPortInterface("ODR", static_cast<IIOPort *>(&ODR));
    registerPortInterface("IDR", static_cast<IIOPort *>(&IDR));
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("HardResetMode", &hardResetMode_);
    direction_ = 0;

    briefDescr_.make_string("IO Port of the CPU.");
    detailedDescr_.make_string(
        "Description:\n"
        "    This command allows to change GPIO behavior via CLI\n"
        "Usage:\n"
        "    <name>\n"
        "    <name> log-level\n"
        "    <key_name> log-level 4\n"
        "Example (maximum debug information):\n"
        "    keyboard0 log-level 4\n");
}

void STM32L4_GPIO::postinitService() {
    RegMemBankGeneric::postinitService();
    
    MODER.setHardResetValue(hardResetMode_.to_uint32());

    iexec_ = static_cast<ICmdExecutor *>
        (RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
    } else {
        iexec_->registerCommand(static_cast<ICommand *>(this));
    }
}

void STM32L4_GPIO::predeleteService() {
    if (iexec_) {
        iexec_->unregisterCommand(static_cast<ICommand *>(this));
    }
}

int STM32L4_GPIO::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() >= 2 && ((*args)[1].is_equal("log-level"))) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void STM32L4_GPIO::exec(AttributeType *args, AttributeType *res) {
    AttributeType &type = (*args)[1];
    res->make_nil();
    if (type.is_equal("log-level")) {
        if (args->size() == 2) {
            res->make_int64(getLogLevel());
        } else {
            setLogLevel((*args)[2].to_int());
        }
    }
}

void STM32L4_GPIO::setMode(uint32_t v) {
    for (int i = 0; i < 16; i++) {
        switch ((v >> 2*i) & 0x3) {
        case 0: // Input Mode
            direction_ &= ~(1 << i);
            break;
        case 1: // General purpose Output Mode
            direction_ |= 1 << i;
            break;
        case 2: // Alternate function mode
            direction_ &= ~(1 << i);
            break;
        case 3: // Analog mode (reset state)
            direction_ &= ~(1 << i);
            break;
        default:;
        }
    }
}

void STM32L4_GPIO::setOpenDrain(uint32_t v) {
    openDrain_ = v & 0xFFFF;
}

uint32_t STM32L4_GPIO::preOutput(uint32_t v) {
    // TODO: open-drain, pull-up, pull-down config
    return v & 0xFFFF;
}

uint32_t STM32L4_GPIO::getDirection() {
    return direction_;
}

uint32_t STM32L4_GPIO::getOutput() {
    return ODR.getValue().val;
}

void STM32L4_GPIO::modifyOutput(uint32_t v) {
    ODR.aboutToWrite(v);
}

uint32_t STM32L4_GPIO::MODER_TYPE::aboutToWrite(uint32_t nxt_val) {
    STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
    p->setMode(nxt_val);
    return nxt_val;
}

uint32_t STM32L4_GPIO::OTYPER_TYPE::aboutToWrite(uint32_t nxt_val) {
    STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
    p->setOpenDrain(nxt_val);
    return nxt_val;
}

uint32_t STM32L4_GPIO::IDR_TYPE::aboutToRead(uint32_t val) {
    STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
    IIOPortListener32 *lstn;
    uint32_t rdata = val;
    for (unsigned i = 0; i < portListeners_.size(); i++) {
        lstn = static_cast<IIOPortListener32 *>(portListeners_[i].to_iface());
        lstn->readData(&rdata, p->getDirection());
    }
    for (unsigned i = 0; i < portListeners_.size(); i++) {
        lstn = static_cast<IIOPortListener32 *>(portListeners_[i].to_iface());
        lstn->latch();
    }
    return rdata;
}

uint32_t STM32L4_GPIO::ODR_TYPE::aboutToWrite(uint32_t nxt_val) {
    STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
    IIOPortListener32 *lstn;
    for (unsigned i = 0; i < portListeners_.size(); i++) {
        lstn = static_cast<IIOPortListener32 *>(portListeners_[i].to_iface());
        lstn->writeData(nxt_val, p->getDirection());
    }
    for (unsigned i = 0; i < portListeners_.size(); i++) {
        lstn = static_cast<IIOPortListener32 *>(portListeners_[i].to_iface());
        lstn->latch();
    }
    return nxt_val;
}

uint32_t STM32L4_GPIO::BSRR_TYPE::aboutToWrite(uint32_t newval) {
    STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
    uint32_t obits = p->getOutput();
    uint32_t br = newval & 0xFFFF;
    uint32_t bs = newval >> 16;
    obits &= ~br;
    obits |= bs;
    p->modifyOutput(obits);
    return newval;
}

}  // namespace debugger

