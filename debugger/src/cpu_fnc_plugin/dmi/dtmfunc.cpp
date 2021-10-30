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

namespace debugger {

DtmFunctional::DtmFunctional(const char * name)
    : IService(name) {
    registerInterface(static_cast<IJtagTap *>(this));

    registerAttribute("SysBus", &sysbus_);
    registerAttribute("DmiBAR", &dmibar_);
    registerAttribute("SysBusMasterID", &busid_);

    estate_ = RESET_TAP;
    ir_ = IR_IDCODE;
    dr_ = 0;
    dr_length_ = 0;
    bypass_ = 0;
    dmi_addr_ = 0;
    memset(&trans_, 0, sizeof(trans_));
}

DtmFunctional::~DtmFunctional() {
}

void DtmFunctional::postinitService() {
    ibus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(sysbus_.to_string(), IFACE_MEMORY_OPERATION));

    if (!ibus_) {
        RISCV_error("Cannot get IMemoryOperation interface at %s",
                    sysbus_.to_string());
    }
}

void DtmFunctional::resetTAP() {
    estate_ = RESET_TAP;
    ir_ = IR_IDCODE;
}

void DtmFunctional::setPins(char tck, char tms, char tdi) {
    bool tck_posedge = false;
    bool tck_negedge = false;
    uint64_t stat = DMISTAT_SUCCESS;

    if (!tck_ && tck) {
        tck_posedge = true;
    }
    if (tck_ && !tck) {
        tck_negedge = true;
    }

    if (!tck_posedge) {
        tck_ = tck;
        tdi_ = tdi;
        tms_ = tms;
        return;
    }

    switch (estate_) {
    case CAPTURE_DR:
        if (ir_ == IR_IDCODE) {
            dr_ = idcode;
            dr_length_ = 32;
        } else if (ir_ == IR_DTMCONTROL) {
            dr_ = 0x1;      // version
            dr_ |= abits << 4;    // the size of the address
            dr_ |= stat << 10;
            dr_length_ = 32;
        } else if (ir_ == IR_DBUS) {
            dr_ = stat;
            dr_ |= (trans_.rpayload.b64[0] << 2);
            dr_ |= dmi_addr_ << 34;
            dr_length_ = abits + 34;
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
        if (ir_ == IR_DTMCONTROL) {
            //v_dmi_reset = r.dr.read()[DTMCONTROL_DMIRESET];
            //v_dmi_hardreset = r.dr.read()[DTMCONTROL_DMIHARDRESET];
        } else if (ir_ == IR_BYPASS) {
            bypass_ = dr_ & 0x1;
        } else if (ir_ == IR_DBUS) {
            dmi_addr_ = (dr_ >> 34) & ((1ull << abits) - 1);
            trans_.rpayload.b64[0] = (dr_ >> 2) & 0xFFFFFFFFul;
            if (dr_ & 0x3) {        // read | write
                if ((dr_ >> 1) & 0x1) {
                    trans_.action = MemAction_Write;
                    RISCV_debug("DMI: [0x%02x] <= %08x",
                                 dmi_addr_, trans_.rpayload.b32[0]);
                } else {
                    trans_.action = MemAction_Read;
                }
                trans_.source_idx = busid_.to_int();
                trans_.addr = dmibar_.to_uint64() + (dmi_addr_ << 2);
                trans_.xsize = 4;
                trans_.wstrb = (1u << trans_.xsize) - 1;
                trans_.wpayload.b64[0] = trans_.rpayload.b64[0];
                ibus_->b_transport(&trans_);

                if (trans_.action == MemAction_Read && dmi_addr_ != 0x11) {
                    // Exclude polling DMI status from the debug output
                    RISCV_debug("DMI: [0x%02x] => %08x",
                                 dmi_addr_, trans_.rpayload.b32[0]);
                }
            }
        }
        break;
    case CAPTURE_IR:
        dr_ = ir_ & ((1ul << irlen) - 1);
        dr_ = (dr_ >> 2) << 2;
        dr_ |= 0x1;
        break;
    case SHIFT_IR:
        dr_ >>= 1;
        dr_ |= (tdi_ << (irlen - 1));
        break;
    case UPDATE_IR:
        ir_ = dr_ & ((1ull << irlen) - 1);
        break;
    default:;
    }
    estate_ = next[estate_][tms_];

    tck_ = tck;
    tdi_ = tdi;
    tms_ = tms;
}

bool DtmFunctional::getTDO() {
    return dr_ & 0x1 ? true : false;
}

}  // namespace debugger

