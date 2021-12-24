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

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class OTP : public RegMemBankGeneric {
 public:
    explicit OTP(const char *name);
    virtual ~OTP();

    /** IService interface */
    virtual void postinitService() override;

    // Common methods
    void pclk_posedge();
    void pclk_negedge();

 private:
    // Clock input
    class OTP_PCLK_TYPE : public MappedReg32Type {
     public:
        OTP_PCLK_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg32Type(parent, name, addr) {}
     protected:
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    AttributeType initData_;    // [[addr,data32],[addr,data32],..]

    MappedReg32Type PA;         // [0x00] Address input
    MappedReg32Type PAI0;       // [0x04] Programming address input
    MappedReg32Type PAS;        // [0x08] Program redundency cell selection input
    MappedReg32Type PCE;        // [0x0C] OTP macro enable input
    OTP_PCLK_TYPE PCLK;         // [0x10] Clock input
    MappedReg32Type PDIN;       // [0x14] Write data input
    MappedReg32Type PDOUT;      // [0x18] Read data input
    MappedReg32Type PDSTB;      // [0x1C] Deep standby mode enable input (active low)
    MappedReg32Type PPROG;      // [0x20] Program mode enable input
    MappedReg32Type PTC;        // [0x24] Test column enable input
    MappedReg32Type PTM;        // [0x28] Test mode enable input
    MappedReg32Type PTM_REP;    // [0x2C] Repair function test mode enable input
    MappedReg32Type PTR;        // [0x30] Test row enable input
    MappedReg32Type PTRIM;      // [0x34] Repair function enable input
    MappedReg32Type PWE;        // [0x38] Write enable input (defines program cycle)

    uint32_t *data_;
};

DECLARE_CLASS(OTP)

}  // namespace debugger
