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
    registerAttribute("Target", &target_);
    ibitbang_ = 0;
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
    if (target_.is_list() && target_.size() == 2) {
        ibitbang_ = static_cast<IJtagTap *>(
            RISCV_get_service_port_iface(target_[0u].to_string(),
                                         target_[1].to_string(),
                                         IFACE_JTAG_TAP));
    }
    if (ibitbang_ == 0) {
        RISCV_error("Cannot get IJtagTap interface");
    }
}

void JTAG::resetAsync() {
    srst_ = 1;
    trst_ = 1;
    ibitbang_->resetTAP(trst_, srst_);
    srst_ = 0;
    trst_ = 0;
    ibitbang_->resetTAP(trst_, srst_);
}

void JTAG::resetSync() {
    scanSize_ = 0;
    for (int i = 0; i < 10; i++) {
        addToScanSequence(1, 1);
    }
    ir_ = IR_IDCODE;
    etapstate_ = IJtag::RESET;

    // IDLE state
    addToScanSequence(0, 1);
    transmitScanSequence();
    etapstate_ = IJtag::IDLE;
}

uint32_t JTAG::scanIdCode() {
    uint32_t ret = 0;

    scan(IR_IDCODE, 0xFFFFFFFFFFFFFFFFull, 32);

    for (int i = 0; i < 32; i++) {
        ret |= (static_cast<uint32_t>(tdi_[i]) << i);
    }
    RISCV_info("dr = %08x", ret);
    return ret;
}

uint32_t JTAG::scanDtmControl() {
    return 0;
}

uint64_t JTAG::scanDmiBus() {
    return 0;
}



uint64_t JTAG::scan(uint32_t ir, uint64_t dr, int drlen) {
    char tms = 0;
    initScanSequence(ir);

    for (int i = 0; i < drlen; i++) {
        tms = i == drlen - 1 ? 1: 0;
        addToScanSequence(tms, static_cast<char>((dr >> i) & 0x1));
    }

    addToScanSequence(1, 1);    // DREXIT1 -> DRUPDATE
    addToScanSequence(0, 1);    // DRUPDATE -> Idle

    transmitScanSequence();
    return 0;
}


void JTAG::initScanSequence(uint32_t ir) {
    scanSize_ = 0;
    addToScanSequence(1, 1);    // Idle -> DRSELECT

    // Need to capture IR first if this is another IR request
    if (ir_ != ir) {
        ir_ = ir;
        addToScanSequence(1, 1);    // DRSELECT -> IRSELECT
        addToScanSequence(0, 1);    // IRSELECT -> IRCAPTURE
        addToScanSequence(0, 1);    // IRCAPTURE -> IRSHIFT
        char tms = 0;
        for (int i = 0; i < IRLEN; i++) {
            tms = i == IRLEN - 1 ? 1: 0;
            addToScanSequence(tms, ir & 1);
            ir >>= 1;
        }
        addToScanSequence(1, 1);    // IREXIT1 -> IRUPDATE
        addToScanSequence(1, 1);    // IRUPDATE -> DRSELECT
    }
    addToScanSequence(0, 1);    // DRSELECT -> DRCAPTURE
    addToScanSequence(0, 1);    // DRCAPTURE -> DRSHIFT
}

void JTAG::addToScanSequence(char tms, char tdo) {
    out_[scanSize_].tms = tms;
    out_[scanSize_].tdo = tdo;
    ++scanSize_;
}

void JTAG::transmitScanSequence() {
    for (int i = 0; i < scanSize_; i++) {
        ibitbang_->setPins(0, out_[i].tms, out_[i].tdo);
        ibitbang_->setPins(1, out_[i].tms, out_[i].tdo);
        tdi_[i] = ibitbang_->getTDO();
    }
    ibitbang_->setPins(1, 0, 1);
}

}  // namespace debugger
