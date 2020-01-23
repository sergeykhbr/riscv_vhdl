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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class STM32L4_NVIC : public IService {
 public:
    explicit STM32L4_NVIC(const char *name);
    virtual ~STM32L4_NVIC();

    /** IService interface */
    virtual void postinitService() override;

    /** Common methods */
    void enableInterrupt(int idx);
    void disableInterrupt(int idx);

 protected:
    class ONEBITS_TYPE : public MappedReg32Type {
     public:
        ONEBITS_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    MappedReg32Type(parent, name, addr) {
            startidx_ = startidx;
        }
     protected:
        int startidx_;
    };

    // Interrupt set-enable
    class ISER_TYPE : public ONEBITS_TYPE {
     public:
        ISER_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    ONEBITS_TYPE(parent, name, addr, startidx) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Interrupt clear-enable
    class ICER_TYPE : public ONEBITS_TYPE {
     public:
        ICER_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    ONEBITS_TYPE(parent, name, addr, startidx) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Interrupt set-pending
    class ISPR_TYPE : public ONEBITS_TYPE {
     public:
        ISPR_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    ONEBITS_TYPE(parent, name, addr, startidx) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Interrupt clear-pending
    class ICPR_TYPE : public ONEBITS_TYPE {
     public:
        ICPR_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    ONEBITS_TYPE(parent, name, addr, startidx) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Interrupt active
    class IABR_TYPE : public ONEBITS_TYPE {
     public:
        IABR_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) :
                    ONEBITS_TYPE(parent, name, addr, startidx) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };

    // Interrupt Priority. 8 bits per one interrupt (255 priorities)
    class IPR_TYPE : public MappedReg32Type {
     public:
        IPR_TYPE(IService *parent, const char *name, uint64_t addr,
                    int startidx) : MappedReg32Type(parent, name, addr) {
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
     protected:
        int startidx_;
    };

    // Software trigger interrupt
    class STIR_TYPE : public MappedReg32Type {
     public:
        STIR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t INTID : 9;         // [8:0] w. SW generated Interrupt ID
                uint32_t res31_9: 23;       // [31:7]
            } b;
        };

        STIR_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };


    ISER_TYPE *NVIC_ISERx[8];     // interrupt 0 to 239
    ICER_TYPE *NVIC_ICERx[8];     // interrupt 0 to 239
    ISPR_TYPE *NVIC_ISPRx[8];     // interrupt 0 to 239
    ICPR_TYPE *NVIC_ICPRx[8];     // interrupt 0 to 239
    IABR_TYPE *NVIC_IABRx[8];     // interrupt 0 to 239
    IPR_TYPE * NVIC_IPRx[61];
    STIR_TYPE NVIC_STIR;
};

DECLARE_CLASS(STM32L4_NVIC)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__
