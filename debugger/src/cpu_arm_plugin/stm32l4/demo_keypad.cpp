/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "demo_keypad.h"

namespace debugger {

DemoKeypad::DemoKeypad(const char *name) : IService(name) {
    registerInterface(static_cast<IKeyboard *>(this));
    registerInterface(static_cast<IIOPortListener32 *>(this));
    registerAttribute("Port", static_cast<IAttribute *>(&port_));
    registerAttribute("Keys", static_cast<IAttribute *>(&keys_));
    registerAttribute("CmdExecutor", static_cast<IAttribute *>(&cmdexec_));
    col_ = 0;
}

DemoKeypad::~DemoKeypad() {
    for (unsigned i = 0; i < keys_.size(); i++) {
        delete BTN[i];
    }
    delete BTN;
}

void DemoKeypad::postinitService() {
    iport_ = static_cast<IIOPort *>(
        RISCV_get_service_port_iface(port_[0u].to_string(),
                    port_[1].to_string(), IFACE_IOPORT));
    if (!iport_) {
        RISCV_error("Can't find IOPort interface %s",
            port_.to_string());
        return;
    }
    iport_->registerPortListener(static_cast<IIOPortListener32 *>(this));

    iexec_ = static_cast<ICmdExecutor *>
        (RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
            cmdexec_.to_string());
        return;
    }

    BTN = new KeyGeneric32 *[keys_.size()];
    for (unsigned i = 0; i < keys_.size(); i++) {
        BTN[i] = new KeyGeneric32(static_cast<IService *>(this), keys_[i]);
        BTN[i]->postinit();
        iexec_->registerCommand(static_cast<ICommand *>(BTN[i]));
    }
}

void DemoKeypad::predeleteService() {
    for (unsigned i = 0; i < keys_.size(); i++) {
        iexec_->unregisterCommand(static_cast<ICommand *>(BTN[i]));
    }
}

void DemoKeypad::readData(uint32_t *val, uint32_t mask) {
    *val &= (~0x03) << 3;  // [4:3]
    *val |= (col_ << 3);
}

void DemoKeypad::writeData(uint32_t val, uint32_t mask) {
    RISCV_debug("gpiob: %02x mask: %02x", val, mask);
    col_ = (val >> 3) & 0x03;
}

int DemoKeypad::getRow() {
    int ret = -1;
    uint8_t t = col_;
    while (t) {
        ++ret;
        if (t & 0x1) {
            RISCV_debug("Select col %d", ret);
            return ret;
        }
        t >>= 1;
    }
    return ret;
}

}  // namespace debugger
