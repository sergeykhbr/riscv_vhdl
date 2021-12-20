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

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iirq.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class GPIO : public RegMemBankGeneric {
 public:
    explicit GPIO(const char *name);

    /** IService interface */
    virtual void postinitService() override;

    /** Common methods */
    uint32_t readInputs();
    void requestInterrupt(int pinidx);

 private:
    class GPIO_INPUT_VAL_TYPE : public MappedReg32Type {
     public:
        GPIO_INPUT_VAL_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg32Type(parent, name, addr) {}

     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    class GPIO_INPUT_EN_TYPE : public MappedReg32Type {
     public:
        GPIO_INPUT_EN_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0xFFFF;
            value_.val = hard_reset_value_;
        }
    };


    AttributeType irqctrl_;
    AttributeType irqid_;
    AttributeType dip_;

    IIrqController *iirq_;

    GPIO_INPUT_VAL_TYPE input_val;  // Pin value
    GPIO_INPUT_EN_TYPE input_en;    // Pin input enable
    MappedReg32Type output_en;      // Pin output enable
    MappedReg32Type output_val;     // Output value
    MappedReg32Type pue;        // internal pull-up enable
    MappedReg32Type ds;         // Pin drive strength
    MappedReg32Type rise_ie;    // Rise interrupt enable
    MappedReg32Type rise_ip;    // Rise interrupt pending
    MappedReg32Type fall_ie;    // Fall interrupt enable
    MappedReg32Type fall_ip;    // Fall interrupt pending
    MappedReg32Type high_ie;    // High interrupt enable
    MappedReg32Type high_ip;    // High interrupt pending
    MappedReg32Type low_ie;     // Low interrupt enable
    MappedReg32Type low_ip;     // Low interrupt pending
    MappedReg32Type iof_en;     // IO function enable
    MappedReg32Type iof_sel;    // IO function select
    MappedReg32Type out_xor;    // Output XOR (invert)
};

DECLARE_CLASS(GPIO)

}  // namespace debugger
