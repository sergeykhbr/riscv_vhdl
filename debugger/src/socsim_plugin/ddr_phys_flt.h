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
#include "coreservices/imemop.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class DdrPhysFilter : public RegMemBankGeneric {
 public:
    explicit DdrPhysFilter(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 private:

    class PMP_TYPE : public MappedReg64Type {
     public:
        PMP_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg64Type(parent, name, addr) {
            value_type t;
            t.v = 0;
            idx_ = addr / sizeof(uint64_t);
            if (idx_ == 0) {
                t.b.addr_hi = 0x80000;
                t.b.r = 1;
                t.b.w = 1;
                t.b.a = 1;
            } else if (idx_ == 1) {
            } else if (idx_ == 2) {
                t.b.addr_hi = 0x880000;
            } else if (idx_ == 3) {
                t.b.addr_hi = 0x2000000;
                t.b.r = 1;
                t.b.w = 1;
                t.b.a = 1;
            }
            hard_reset_value_ = t.v;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint64_t v;
            struct bits_type {
                uint64_t rsrv9_0 : 10;  // [9:0]
                uint64_t addr_hi : 26;  // [35:10] RW Page address. Specified top-of-range
                                        //        page address for this PMP and bottom-of-range
                                        //        for following PMP. Cannot be modified if
                                        //        l-bit is set on this or subsequent PMP
                uint64_t rsrv55_36 : 20;// [55:36]
                uint64_t r : 1;         // [56] RW Read bit grants read access
                uint64_t w : 1;         // [57] RW Write bit grants write access
                uint64_t rsrv58 : 1;    // [58]
                uint64_t a : 1;         // [59] RW Access bit enables filtering
                uint64_t rsrv62_60 : 3; // [62:60]
                uint64_t l : 1;         // [63] RW Lock bit
            } b;
        };

        PMP_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }

     protected:
        virtual uint64_t aboutToWrite(uint64_t nxt_val) override;

     private:
        uint64_t idx_;
    };

    AttributeType ddrctrl_;     // List of context names: [MCore0, MCore1, SCore1, MCore2, ...]

    PMP_TYPE devicepmp0;            // [0x00] Physical Filter Device PMP Regsiter 0
    PMP_TYPE devicepmp1;            // [0x08] Physical Filter Device PMP Regsiter 1
    PMP_TYPE devicepmp2;            // [0x10] Physical Filter Device PMP Regsiter 2
    PMP_TYPE devicepmp3;            // [0x18] Physical Filter Device PMP Regsiter 3
};

DECLARE_CLASS(DdrPhysFilter)

}  // namespace debugger
