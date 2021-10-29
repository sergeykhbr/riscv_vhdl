/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

    estate_ = RESET_TAP;
    dr_ = 0;
}

DtmFunctional::~DtmFunctional() {
}

void DtmFunctional::resetTAP() {
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
            dr_ |= (dmi_resp_data_ << 2);
            dr_ |= (dmi_addr_ << 34);
            dr_length_ = abits + 34;
        } else if (ir_ == IR_BYPASS) {
            dr_ = bypass_;
            dr_length_ = 1;
        }
        break;
    case SHIFT_DR:
        dr_ >>= 1;
        dr_ |= (tdi << (dr_length_ - 1));
        break;
    case UPDATE_DR:
        if (ir_ == IR_DTMCONTROL) {
            //v_dmi_reset = r.dr.read()[DTMCONTROL_DMIRESET];
            //v_dmi_hardreset = r.dr.read()[DTMCONTROL_DMIHARDRESET];
        } else if (ir_ == IR_BYPASS) {
            bypass_ = dr_ & 0x1;
        } else if (ir_ == IR_DBUS) {
            //v_dmi_req_valid = r.dr.read()(1, 0).or_reduce();
            //v_dmi_req_write = r.dr.read()[1];
            //vb_dmi_req_data = r.dr.read()(33, 2);
            dmi_addr_ = (dr_ >> 34) & ((1ull << abits) - 1);

            //nv.dmi_addr = r.dr.read()(34 + abits - 1, 34);
        }
        break;
    case CAPTURE_IR:
        dr_ = ir_ & ((1ul << irlen) - 1);
        dr_ = (dr_ >> 2) << 2;
        dr_ |= 0x1;
        break;
    case SHIFT_IR:
        dr_ >>= 1;
        dr_ |= (tdi << (irlen - 1));
        break;
    case UPDATE_IR:
        ir_ = dr_ & ((1ull << irlen) - 1);
        break;
    default:;
    }
    if (tck_posedge) {
        estate_ = next[estate_][tms];
    }

    tck_ = tck;
    tdi_ = tdi;
    tms_ = tms;
}

bool DtmFunctional::getTDO() {
    return dr_ & 0x1 ? true : false;
}

}  // namespace debugger

