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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_RCC_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_RCC_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class STM32L4_RCC : public RegMemBankGeneric {
 public:
    explicit STM32L4_RCC(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 protected:

    class RCC_CR_TYPE : public MappedReg32Type {
     public:
        RCC_CR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t MSION : 1;         // [0] rw
                uint32_t MSIRDY: 1;         // [1] r
                uint32_t MSIPLLEN: 1;       // [2] rw
                uint32_t MSIRGLEN: 1;       // [3] rs
                uint32_t MSIRANGE: 4;       // [7:4] rw
                uint32_t HSION : 1;         // [8] rw
                uint32_t HSIKERON : 1;      // [9] rw
                uint32_t HSIRDY : 1;        // [10] r
                uint32_t HSIASFS : 1;       // [11] rw
                uint32_t res15_12 : 4;      // [15:12]
                uint32_t HSEON : 1;         // [16] rw
                uint32_t HSERDY : 1;        // [17] r
                uint32_t HSEBP : 1;         // [18] rw
                uint32_t CSSON : 1;         // [19] rw
                uint32_t res23_20 : 4;      // [23:20]
                uint32_t PLLON : 1;         // [24] rw
                uint32_t PLLRDY : 1;        // [25] r
                uint32_t PLLSAI1ON : 1;     // [26] rw
                uint32_t PLLSAI1RDY : 1;    // [27] r
                uint32_t PLLSAI2ON : 1;     // [28] rw
                uint32_t PLLSAI2RDY : 1;    // [29] r
                uint32_t res31_30 : 2;      // [23:20]
            } b;
        };

        RCC_CR_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    class RCC_CFGR_TYPE : public MappedReg32Type {
     public:
        RCC_CFGR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000063;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t SW : 2;            // [1:0] rw
                uint32_t SWS: 2;            // [3:2] r
                uint32_t HPRE: 4;           // [7:4] rw
                uint32_t PPRE1: 3;          // [10:8] rw
                uint32_t PPRE2: 3;          // [13:11] rw
                uint32_t res14: 1;          // [14]
                uint32_t STOPWUCK : 1;      // [15] rw
                uint32_t res23_16 : 8;      // [23:16] rw
                uint32_t MCOSEL : 4;        // [27:24] rw
                uint32_t MCOPRE : 3;        // [30:28] rw
                uint32_t res31: 1;          // [31]
            } b;
        };

        RCC_CFGR_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    class RCC_PLLCFGR_TYPE : public MappedReg32Type {
     public:
        RCC_PLLCFGR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00001000;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t PLLSRC : 2;        // [1:0] rw
                uint32_t res3_2 : 2;        // [3:2]
                uint32_t PLLM : 4;          // [7:4] rw
                uint32_t PLLN : 7;          // [14:8] rw
                uint32_t res15 : 1;         // [15]
                uint32_t PLLPEN : 1;        // [16] rw
                uint32_t PLLP : 1;          // [17] rw
                uint32_t res19_18 : 2;      // [19:18]
                uint32_t PLLQEN : 1;        // [20] rw
                uint32_t PLLQ : 2;          // [22:21] rw
                uint32_t res23 : 1;         // [23]
                uint32_t PLLREN : 1;        // [24] rw
                uint32_t PLLR : 2;          // [26:25] rw
                uint32_t PLLPDIV : 5;       // [31:27] rw
            } b;
        };

        RCC_PLLCFGR_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Clock Interrupt enable register
    class RCC_CIER_TYPE : public MappedReg32Type {
     public:
        RCC_CIER_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t LSIRDUIE : 1;          // [0] rw
                uint32_t LSERDYIE : 1;          // [1] rw
                uint32_t MSIRDYIE : 1;          // [2] rw
                uint32_t HSIRDYIE : 1;          // [3] rw
                uint32_t HSERDYIE : 1;          // [4] rw
                uint32_t PLLRDYIE : 1;          // [5] rw
                uint32_t PLLSAI1RDYIE : 1;      // [6] rw
                uint32_t PLLSAI2RDYIE : 1;      // [7] rw
                uint32_t res8 : 1;              // [8]
                uint32_t LSECSSIE : 1;          // [9] rw
                uint32_t HSI48RDYIE : 1;        // [10] rw
                uint32_t res31_11 : 21;         // [31:11]
            } b;
        };

        RCC_CIER_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };


    RCC_CR_TYPE CR;             // 0x00
    MappedReg32Type ICSCR;      // 0x04
    RCC_CFGR_TYPE CFGR;         // 0x08
    RCC_PLLCFGR_TYPE PLLCFGR;   // 0x0c
    RCC_CIER_TYPE CIER;         // 0x18
};

DECLARE_CLASS(STM32L4_RCC)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_RCC_H__
