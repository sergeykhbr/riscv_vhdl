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

#include <api_core.h>
#include "jtag.h"

namespace debugger {

JTAG::JTAG(const char *name) : IService(name) {
    registerInterface(static_cast<IJtag *>(this));
    registerAttribute("TargetBitBang", &targetBitBang_);
    ibb_ = 0;
    trst_ = 0;
    srst_ = 0;
    memset(out_, 0, sizeof(out_));
    memset(tdi_, 0, sizeof(tdi_));
    etapstate_ = IJtag::RESET;
    tapid_ = 0;
    ir_ = IR_IDCODE;
}

JTAG::~JTAG() {
}

void JTAG::postinitService() {
    if (targetBitBang_.is_list() && targetBitBang_.size() == 2) {
        ibb_ = static_cast<IJtagBitBang *>(
            RISCV_get_service_port_iface(targetBitBang_[0u].to_string(),
                                         targetBitBang_[1].to_string(),
                                         IFACE_JTAG_BITBANG));
    } else {
        ibb_ = static_cast<IJtagBitBang *>(
            RISCV_get_service_iface(targetBitBang_.to_string(),
                                    IFACE_JTAG_BITBANG));
    }
    if (ibb_ == 0) {
        RISCV_error("Cannot get IJtagBitBang interface");
    }
}

void JTAG::resetAsync() {
    srst_ = 1;
    trst_ = 1;
    ibb_->resetTAP(trst_, srst_);
    srst_ = 0;
    trst_ = 0;
    ibb_->resetTAP(trst_, srst_);
}

void JTAG::resetSync() {
    scanSize_ = 0;
    for (int i = 0; i < 10; i++) {
        addToScanSequence(1, 1, IJtag::RESET);
    }
    ir_ = IR_IDCODE;

    // IDLE state
    addToScanSequence(0, 1, IJtag::IDLE);
    transmitScanSequence();
}

uint32_t JTAG::scanIdCode() {
    uint32_t ret = 0;

    scan(IR_IDCODE, 0xFFFFFFFFFFFFFFFFull, 32);
    ret = static_cast<uint32_t>(getRxData());
    RISCV_debug("TAP id = %08x", ret);
    return ret;
}

IJtag::DtmcsType JTAG::scanDtmcs() {
    DtmcsType ret = {0};
    scan(IR_DTMCS, 0, 32);
    ret.u32 = static_cast<uint32_t>(getRxData());
    RISCV_debug("DTMCS = %08x: ver:%d, abits:%d, stat:%d",
            ret.u32, ret.bits.version, ret.bits.abits, ret.bits.dmistat);
    return ret;
}

uint32_t JTAG::scanDmi(uint32_t addr, uint32_t data, IJtag::EDmiOperation op) {
    DmiType ret;
    uint64_t dr = addr;
    dr = (dr << 32) | data;
    dr = (dr << 2) | op;
    scan(IR_DMI, dr, 34 + ABITS);

    // Do the same but with rena=0 and wena=0
    dr = static_cast<uint64_t>(addr) << 34;
    scan(IR_DMI, dr, 34 + ABITS);
    ret.u64 = getRxData();

    RISCV_debug("DMI [%02x] %08x, stat:%d",
            static_cast<uint32_t>(ret.bits.addr),
            static_cast<uint32_t>(ret.bits.data),
            static_cast<uint32_t>(ret.bits.status));
    return static_cast<uint32_t>(ret.bits.data);
}



uint64_t JTAG::scan(uint32_t ir, uint64_t dr, int drlen) {
    initScanSequence(ir);
    for (int i = 0; i < drlen; i++) {
        if (i == drlen - 1) {
            addToScanSequence(1, static_cast<char>((dr >> i) & 0x1), IJtag::DREXIT1);
        } else {
            addToScanSequence(0, static_cast<char>((dr >> i) & 0x1), IJtag::DRSHIFT);
        }
    }
    endScanSequenceToIdle();

    transmitScanSequence();
    return 0;
}


void JTAG::initScanSequence(uint32_t ir) {
    scanSize_ = 0;
    addToScanSequence(1, 1, IJtag::DRSCAN);    // Idle -> DRSELECT

    // Need to capture IR first if this is another IR request
    if (ir_ != ir) {
        ir_ = ir;
        addToScanSequence(1, 1, IJtag::IRSCAN);     // DRSELECT -> IRSELECT
        addToScanSequence(0, 1, IJtag::IRCAPTURE);  // IRSELECT -> IRCAPTURE
        addToScanSequence(0, 1, IJtag::IRSHIFT);    // IRCAPTURE -> IRSHIFT
        for (int i = 0; i < IRLEN; i++) {
            if (i == IRLEN - 1) {
                addToScanSequence(1, ir & 1, IJtag::IREXIT1);
            } else {
                addToScanSequence(0, ir & 1, IJtag::IRSHIFT);
            }
            ir >>= 1;
        }
        addToScanSequence(1, 1, IJtag::IRUPDATE);    // IREXIT1 -> IRUPDATE
        addToScanSequence(1, 1, IJtag::DRSCAN);    // IRUPDATE -> DRSELECT
    }
    addToScanSequence(0, 1, IJtag::DRCAPTURE);    // DRSELECT -> DRCAPTURE
    addToScanSequence(0, 1, IJtag::DRSHIFT);    // DRCAPTURE -> DRSHIFT
}

void JTAG::addToScanSequence(char tms, char tdo, ETapState nextstate) {
    out_[scanSize_].tms = tms;
    out_[scanSize_].tdo = tdo;
    out_[scanSize_].state = etapstate_;
    // TODO: acceptance of the next state
    etapstate_ = nextstate;

    ++scanSize_;
}

void JTAG::endScanSequenceToIdle() {
    addToScanSequence(1, 1, IJtag::DRUPDATE);    // DREXIT1 -> DRUPDATE
    addToScanSequence(0, 1, IJtag::IDLE);        // DRUPDATE -> Idle
}

void JTAG::transmitScanSequence() {
    drshift_ = 0;
    for (int i = 0; i < scanSize_; i++) {
        ibb_->setPins(0, out_[i].tms, out_[i].tdo);
        tdi_[i] = ibb_->getTDO();
        if (out_[i].state == DRSHIFT) {
            drshift_ >>= 1;
            if (ir_ == IJtag::IR_DMI) {
                drshift_ |= static_cast<uint64_t>(tdi_[i]) << (ABITS + 33);
            } else {
                drshift_ |= static_cast<uint64_t>(tdi_[i]) << 31;
            }
        }
        ibb_->setPins(1, out_[i].tms, out_[i].tdo);
    }
    ibb_->setPins(1, 0, 1);
}

uint64_t JTAG::getRxData() {
    return drshift_;
}

}  // namespace debugger
