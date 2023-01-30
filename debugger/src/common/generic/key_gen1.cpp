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
#include "iservice.h"
#include "key_gen1.h"
#include "coreservices/ireset.h"

namespace debugger {

KeyGeneric::KeyGeneric(IService *parent, const char *name)
    : ICommand(parent, name) {
    keyName_.make_string(name);
    pressed_ = false;
    power_on_ = false;

    briefDescr_.make_string("Press or release button.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Press release button or get status\n"
        "Usage:\n"
        "    <key_name>\n"
        "    <key_name> press\n"
        "    <key_name> release\n"
        "Example:\n"
        "    YES_START_BTN press\n"
        "    YES_START_BTN release\n");
}

int KeyGeneric::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || (args->size() == 2 && (*args)[1].is_string())) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void KeyGeneric::exec(AttributeType *args, AttributeType *res) {
    res->make_boolean(pressed_);
    if (args->size() < 2) {
        return;
    }
    AttributeType &type = (*args)[1];
    if (type.is_equal("press") && !pressed_) {
        press();
    } else if (type.is_equal("release") && pressed_) {
        release();
    }
    res->make_boolean(pressed_);
}

void KeyGeneric::reset(IFace *isource) {
    release();
}

void KeyGeneric::press() {
    pressed_ = true;
    ikb_ = static_cast<IKeyboard *>(cmdParent_->getInterface(IFACE_KEYBOARD));
    ikb_->keyPress(keyName_.to_string());
    RISCV_info("%s pressed", keyName_.to_string());

    if (power_on_) {
        AttributeType res;
        AttributeType execlist;
        ICmdExecutor *iexec;
        RISCV_get_iface_list(IFACE_CMD_EXECUTOR, &execlist);
        for (unsigned i = 0; i < execlist.size(); i++) {
            iexec = static_cast<ICmdExecutor *>(execlist[i].to_iface());
            iexec->exec("POWER ON", &res, true);
        }
    }
}

void KeyGeneric::release() {
    pressed_ = false;
    ikb_ = static_cast<IKeyboard *>(cmdParent_->getInterface(IFACE_KEYBOARD));
    ikb_->keyRelease(keyName_.to_string());
    RISCV_info("%s released", keyName_.to_string());
}

/** 8-bit IO-listener */
void KeyGeneric8::postinit() {
    IIOPort *iport_ = static_cast<IIOPort *>(
        RISCV_get_service_iface(port_[0u].to_string(), IFACE_IOPORT));
    if (!iport_) {
        RISCV_error("Can't find IOPort interface %s",
            port_.to_string());
        return;
    }
    iport_->registerPortListener(static_cast<IIOPortListener8 *>(this));
    ikb_ = static_cast<IKeyboard *>(cmdParent_->getInterface(IFACE_KEYBOARD));
}

void KeyGeneric8::readData(uint8_t *val, uint8_t mask) {
    uint8_t bit = 1u << port_[1].to_int();
    if (ikb_->getRow() == row_) {
        // Button selected
        if (pressed_) {
            if (!inverse_) {
                *val |= bit;
            } else {
                *val &= ~bit;
            }
        } else {
            if (!inverse_) {
                *val &= ~bit;
            } else {
                *val |= bit;
            }
        }
    }
}

/** 32-bit IO-listener */
void KeyGeneric32::postinit() {
    IIOPort *iport_ = 0;
    AttributeType &io = port_[0u];
    if (io.is_string()) {
        iport_ = static_cast<IIOPort *>(
            RISCV_get_service_iface(io.to_string(), IFACE_IOPORT));
    } else if (io.is_list()) {
        iport_ = static_cast<IIOPort *>(
            RISCV_get_service_port_iface(
                io[0u].to_string(),
                io[1].to_string(),
                IFACE_IOPORT));
    }
    if (!iport_) {
        RISCV_error("Can't find IOPort interface %s",
            port_.to_string());
        return;
    }
    iport_->registerPortListener(static_cast<IIOPortListener32 *>(this));
    ikb_ = static_cast<IKeyboard *>(cmdParent_->getInterface(IFACE_KEYBOARD));
}

void KeyGeneric32::readData(uint32_t *val, uint32_t mask) {
    uint8_t bit = 1u << port_[1].to_int();
    if (ikb_->getRow() == row_ || row_ == -1) {
        // Button selected
        if (pressed_) {
            if (!inverse_) {
                *val |= bit;
            } else {
                *val &= ~bit;
            }
        } else {
            if (!inverse_) {
                *val &= ~bit;
            } else {
                *val |= bit;
            }
        }
    }
}

}  // namespace debugger

