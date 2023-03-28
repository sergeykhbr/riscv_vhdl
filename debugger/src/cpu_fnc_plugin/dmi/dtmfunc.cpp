/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "dtmfunc.h"
#include <riscv-isa.h>
#include "coreservices/ijtag.h"

namespace debugger {

DtmFunctional::DtmFunctional(const char * name)
    : IService(name) {
    registerInterface(static_cast<IJtagBitBang *>(this));

    registerAttribute("Version", &version_);
    registerAttribute("IdCode", &idcode_);
    registerAttribute("irlen", &irlen_);
    registerAttribute("abits", &abits_);
    registerAttribute("Dmi", &dmi_);

    estate_ = RESET_TAP;
    ir_ = IR_IDCODE;
    dr_ = 0;
    dr_length_ = 0;
    bypass_ = 0;
    dmi_addr_ = 0;
    dmi_data_ = 0;
}

DtmFunctional::~DtmFunctional() {
}

void DtmFunctional::postinitService() {
    idmi_ = static_cast<IDmi *>(
        RISCV_get_service_iface(dmi_.to_string(), IFACE_DMI));

    if (!idmi_) {
        RISCV_error("Cannot get IDmi interface at %s",
                    dmi_.to_string());
    }
}

void DtmFunctional::resetTAP(char trst, char srst) {
    if (trst == 1) {
        estate_ = RESET_TAP;
        ir_ = IR_IDCODE;
    }
}

void DtmFunctional::setPins(char tck, char tms, char tdi) {
    bool tck_posedge = false;

    if (!tck_ && tck) {
        tck_posedge = true;
    }

    if (!tck_posedge) {
        tck_ = tck;
        tdi_ = tdi;
        tms_ = tms;
        return;
    }

    switch (estate_) {
    case RESET_TAP:
        ir_ = IR_IDCODE;
        break;
    case CAPTURE_DR:
        if (ir_ == IR_IDCODE) {
            dr_ = idcode_.to_uint32();
            dr_length_ = 32;
        } else if (ir_ == IR_DTMCONTROL) {
            IJtag::DtmcsType dtmcs;
            dtmcs.u32 = 0;
            dtmcs.bits.version = version_.to_uint32();
            dtmcs.bits.abits = abits_.to_uint32();
            dtmcs.bits.dmistat = idmi_->dmi_status();
            dr_ = dtmcs.u32;
            dr_length_ = 32;
        } else if (ir_ == IR_DBUS) {
            IJtag::DmiType dmi;
            dmi.u64 = 0;
            dmi.bits.status = dmi_status_;
            dmi.bits.data = dmi_data_;
            dmi.bits.addr = dmi_addr_;
            dr_ = dmi.u64;
            dr_length_ = abits_.to_int() + 34;
        } else if (ir_ == IR_BYPASS) {
            dr_ = bypass_;
            dr_length_ = 1;
        }
        break;
    case SHIFT_DR:
        dr_ >>= 1;
        dr_ |= (static_cast<uint64_t>(tdi_) << (dr_length_ - 1));
        break;
    case UPDATE_DR:
        if (ir_ == IR_BYPASS) {
            bypass_ = dr_ & 0x1;
        } else if (ir_ == IR_DTMCONTROL) {
            IJtag::DtmcsType dtmcs;
            dtmcs.u32 = static_cast<uint32_t>(dr_);

            if (dtmcs.bits.dmihardreset) {  // W1: Only 1 takes effect
                idmi_->dtm_dmihardreset();
            }
            if (dtmcs.bits.dmireset) {      // W1: Only 1 takes effect
                // Writing 1 to this bit clears the sticky error state,
                // but does not affect outstanding DMI transactions
                dmi_status_ = 0;
            }
        } else if (ir_ == IR_DBUS) {
            IJtag::DmiType dmi;
            dmi.u64 = dr_;
            dmi_addr_ = static_cast<uint32_t>(dmi.bits.addr);
            dmi_data_ = static_cast<uint32_t>(dmi.bits.data);
            if (dr_ & 0x2) {
                RISCV_debug("DMI: [0x%02x] <= %08x", dmi_addr_, dmi_data_);
                idmi_->dmi_write(dmi_addr_, dmi_data_);
            } else if (dr_ & 0x1) {
                idmi_->dmi_read(dmi_addr_, &dmi_data_);
                RISCV_debug("DMI: [0x%02x] => %08x", dmi_addr_, dmi_data_);
            }
            dmi_status_ = idmi_->dmi_status();
        }
        break;
    case CAPTURE_IR:
        dr_ = ir_ & ((1ul << irlen_.to_int()) - 1);
        dr_ = (dr_ >> 2) << 2;
        dr_ |= 0x1;
        break;
    case SHIFT_IR:
        dr_ >>= 1;
        dr_ |= (tdi_ << (irlen_.to_int() - 1));
        break;
    case UPDATE_IR:
        ir_ = dr_ & ((1ull << irlen_.to_int()) - 1);
        break;
    default:;
    }
    estate_ = next[estate_][static_cast<int>(tms_)];

    tck_ = tck;
    tdi_ = tdi;
    tms_ = tms;
}

bool DtmFunctional::getTDO() {
    return dr_ & 0x1 ? true : false;
}

}  // namespace debugger

