/**
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

#include "api_core.h"
#include "otp.h"

namespace debugger {

OTP::OTP(const char *name) : RegMemBankGeneric(name),
    PA(static_cast<IService *>(this), "PA", 0x00),
    PAI0(static_cast<IService *>(this), "PAI0", 0x04),
    PAS(static_cast<IService *>(this), "PAS", 0x08),
    PCE(static_cast<IService *>(this), "PCE", 0x0C),
    PCLK(static_cast<IService *>(this), "PCLK", 0x10),
    PDIN(static_cast<IService *>(this), "PDIN", 0x14),
    PDOUT(static_cast<IService *>(this), "PDOUT", 0x18),
    PDSTB(static_cast<IService *>(this), "PDSTB", 0x1C),
    PPROG(static_cast<IService *>(this), "PPROG", 0x20),
    PTC(static_cast<IService *>(this), "PTC", 0x24),
    PTM(static_cast<IService *>(this), "PTM", 0x28),
    PTM_REP(static_cast<IService *>(this), "PTM_REP", 0x2C),
    PTR(static_cast<IService *>(this), "PTR", 0x30),
    PTRIM(static_cast<IService *>(this), "PTRIM", 0x34),
    PWE(static_cast<IService *>(this), "PWE", 0x38) {
    registerAttribute("InitData", &initData_);
    data_ = 0;
}

OTP::~OTP() {
    if (data_) {
        delete [] data_;
    }
}

void OTP::postinitService() {
    RegMemBankGeneric::postinitService();

    // 4X x 32bits in FU740
    data_ = new uint32_t[getLength()/sizeof(uint32_t)];

    for (unsigned i = 0; i < initData_.size(); i++) {
        AttributeType &item = initData_[i];
        uint32_t a = item[0u].to_uint32();
        uint32_t d = item[1].to_uint32();
        data_[a] = d;
    }
}

void OTP::pclk_posedge() {
    uint32_t addr = PA.getValue().val;
    addr %= getLength()/sizeof(uint32_t);
    PDOUT.setValue(data_[addr]);
}

void OTP::pclk_negedge() {
}

uint32_t OTP::OTP_PCLK_TYPE::aboutToWrite(uint32_t new_val) {
    OTP *p = static_cast<OTP *>(parent_);
    uint32_t v = getValue().val;
    if (!v && new_val) {
        p->pclk_posedge();
    } else if (v && !new_val) {
        p->pclk_negedge();
    }
    return new_val;
}

}  // namespace debugger

