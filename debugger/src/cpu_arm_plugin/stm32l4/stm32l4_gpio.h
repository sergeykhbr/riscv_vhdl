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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_GPIO_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_GPIO_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iioport.h"
#include "coreservices/icmdexec.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class STM32L4_GPIO : public RegMemBankGeneric,
                     public ICommand {
 public:
    explicit STM32L4_GPIO(const char *name);

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

    /** Common methods for registers: */
    void setMode(uint32_t v);
    void setOpenDrain(uint32_t v);
    uint32_t preOutput(uint32_t v);
    uint32_t getDirection();
    uint32_t getOutput();
    void modifyOutput(uint32_t v);

 protected:
    int getLogLevel() { return logLevel_.to_int(); }
    void setLogLevel(int level) { return logLevel_.make_int64(level); }

 protected:
    AttributeType cmdexec_;
    AttributeType hardResetMode_;

    ICmdExecutor *iexec_;

    uint32_t direction_;        // 0=input; 1=output
    uint32_t openDrain_;        // 0=pull up/down; 1=open-drain

    // Direction config
    class MODER_TYPE : public MappedReg32Type {
     public:
        MODER_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }

        virtual void setHardResetValue(uint32_t v) override {
            MappedReg32Type::setHardResetValue(v);
            STM32L4_GPIO *p = static_cast<STM32L4_GPIO *>(parent_);
            p->setMode(v);
        } 

     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
    };

    // Open-drain, pull-up/down config
    class OTYPER_TYPE : public MappedReg32Type {
     public:
        OTYPER_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
    };

    // Output Data Register
    class ODR_TYPE : public MappedReg32Type,
                         public IIOPort {
     public:
        ODR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
            portListeners_.make_list(0);
        }

        /** IIOPort interface */
        virtual void registerPortListener(IFace *listener) {
            AttributeType item;
            item.make_iface(listener);
            portListeners_.add_to_list(&item);
        }

        virtual void unregisterPortListener(IFace *listener) {
            for (unsigned i = 0; i < portListeners_.size(); i++) {
                if (listener == portListeners_[i].to_iface()) {
                    portListeners_.remove_from_list(i);
                    break;
                }
            }
        }
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
     protected:
        AttributeType portListeners_;
    };

    // Input Data Register
    class IDR_TYPE : public MappedReg32Type,
                     public IIOPort {
     public:
        IDR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
            portListeners_.make_list(0);
        }

        /** IIOPort interface */
        virtual void registerPortListener(IFace *listener) {
            AttributeType item;
            item.make_iface(listener);
            portListeners_.add_to_list(&item);
        }

        virtual void unregisterPortListener(IFace *listener) {
            for (unsigned i = 0; i < portListeners_.size(); i++) {
                if (listener == portListeners_[i].to_iface()) {
                    portListeners_.remove_from_list(i);
                    break;
                }
            }
        }
     protected:
        virtual uint32_t aboutToRead(uint32_t val) override;
     protected:
        AttributeType portListeners_;
    };

    // Clear/Set bit atomically
    class BSRR_TYPE : public MappedReg32Type {
     public:
        BSRR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x00000000;
            value_.val = hard_reset_value_;
        }
     protected:
        virtual uint32_t aboutToWrite(uint32_t newval) override;
    };


    MappedReg32Type MODER;          // 0x00
    OTYPER_TYPE OTYPER;             // 0x04
    MappedReg32Type OSPEEDR;        // 0x08
    MappedReg32Type PUPDR;          // 0x0c
    IDR_TYPE IDR;                   // 0x10
    ODR_TYPE ODR;                   // 0x14
    BSRR_TYPE BSRR;           // 0x18
    MappedReg32Type LCKR;           // 0x1c
    MappedReg32Type AFRL;           // 0x20
    MappedReg32Type AFRH;           // 0x24
    MappedReg32Type BRR;            // 0x28 Bit Reset Register
};

DECLARE_CLASS(STM32L4_GPIO)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_GPIO_H__
