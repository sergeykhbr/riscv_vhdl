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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_SYSTICK_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_SYSTICK_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/iirq.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class STM32L4_SysTick : public IService,
                        public IClockListener {
 public:
    explicit STM32L4_SysTick(const char *name);

    /** IService interface */
    virtual void postinitService() override;

    /** IClockListener interface */
    virtual void stepCallback(uint64_t t);

    /** Common methods shared with registers */
    void enableCounter();
    void disableCounter();
    void enableInterrupt();
    void disableInterrupt();
    uint64_t getReloadSteps();

 protected:
    AttributeType cpu_;
    AttributeType irqctrl_;
    AttributeType irqid_;

    IIrqController *iirq_;
    IClock *iclk_;

    class STK_CTRL_TYPE : public MappedReg32Type {
     public:
        STK_CTRL_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t ENABLE : 1;        // [0] rw. 1=Counter enabled
                uint32_t TICKINT : 1;       // [1] rw. 0=irq disable; 1=irq enabled
                uint32_t CLKSOURCE : 1;     // [2] rw. 0=AHB/8 clock; 1=CPU clock (AHB)
                uint32_t res15_3 : 13;      // [15:3]
                uint32_t COUNTFLAG : 1;     // [7:4] rw
                uint32_t res31_17 : 15;     // [8] 1 if timer counted to 0 since last read
            } b;
        };

        STK_CTRL_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
        virtual uint32_t aboutToWrite(uint32_t cur_val) override;
    };


    STK_CTRL_TYPE STK_CTRL;          // 0x00
    MappedReg32Type STK_LOAD;          // 0x04
    MappedReg32Type STK_VAL;           // 0x08
    MappedReg32Type STK_CALIB;         // 0x0C
    uint64_t lastReloadTime_;
};

DECLARE_CLASS(STM32L4_SysTick)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_SYSTICK_H__
