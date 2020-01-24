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

#ifndef __DEBUGGER_COMMON_GENERIC_IOTYPES_H__
#define __DEBUGGER_COMMON_GENERIC_IOTYPES_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/iioport.h"
#include "coreservices/ireset.h"
#include "coreservices/iwire.h"

namespace debugger {

class IORegType : public IMemoryOperation,
                  public IResetListener,
                  public IIOPort {
 public:
    IORegType(IService *parent, const char *name,
        uint32_t addr, uint32_t len, int priority = 1);

    /** IIOPort */
    virtual void registerPortListener(IFace *listener);
    virtual void unregisterPortListener(IFace *listener);

    /** General access methods: */
    virtual const char *regName() { return regname_.to_string(); }

 protected:
    // Debug output compatibility
    virtual IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
};

class IOReg8Type : public IORegType {
 public:
    IOReg8Type(IService *parent, const char *name,
               uint32_t addr, uint32_t len, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { write(hard_reset_value_); }

    /** General access methods: */
    uint8_t getValue() { return value.byte; }
    void setValue(uint8_t v) { value.byte = v; }

 protected:
    virtual uint8_t get_direction() { return 0u; }
    virtual uint8_t read();
    virtual void write(uint8_t val);

 protected:
    Reg8Type value;
    uint8_t hard_reset_value_;
};

class IOReg16Type : public IORegType {
 public:
    IOReg16Type(IService *parent, const char *name,
               uint32_t addr, uint32_t len, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { write(hard_reset_value_); }

    /** General access methods: */
    uint16_t getValue() { return value.word; }
    void setValue(uint16_t v) { value.word = v; }

 protected:
    virtual uint16_t get_direction() { return 0u; }
    virtual uint16_t read();
    virtual void write(uint16_t val);

 protected:
    Reg16Type value;
    uint16_t hard_reset_value_;
};

class IOReg32Type : public IORegType {
public:
    IOReg32Type(IService *parent, const char *name,
        uint32_t addr, uint32_t len, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { write(hard_reset_value_); }

    /** General access methods: */
    uint32_t getValue() { return value.val; }
    void setValue(uint32_t v) { value.val = v; }

protected:
    virtual uint32_t get_direction() { return 0u; }
    virtual uint32_t read();
    virtual void write(uint32_t val);

protected:
    Reg32Type value;
    uint32_t hard_reset_value_;
};

class IOPinType {
 public:
    IOPinType(IService *parent, const char *name);
    IOPinType(IService *parent, const char *name, AttributeType &pincfg);

    virtual void regiterAsListener(IIOPort *port) = 0;

    /** generic accessors */
    void postinit();
    void connectToBit(const AttributeType &cfg);
    void connectToWire(const AttributeType &cfg);
    virtual bool get_bit();
    virtual void set_bit(bool v);

 protected:
    virtual void memread(uint64_t *val, uint64_t mask);
    virtual void memwrite(uint64_t val, uint64_t mask);
    virtual bool aboutToRead(bool val) { return val; }
    virtual bool aboutToWrite(bool prev, bool val) { return val; }

 protected:
    static const int READ_MASK = 0x1;
    static const int WRITE_MASK = 0x2;

    AttributeType pinName_;
    AttributeType IOPinTypeCfg_;
    IService *parent_;
    IWire *iwire_;
    int bitIdx_;
    int access_;
    volatile bool value_;     // triggered
    volatile bool prelatch_;  // not triggered value
};

class IOPinType8 : public IOPinType,
                   public IIOPortListener8 {
 public:
    IOPinType8(IService *parent, const char *name) :
        IOPinType(parent, name) {}
    IOPinType8(IService *parent, const char *name, AttributeType &pincfg) :
        IOPinType(parent, name, pincfg) {}

    virtual void regiterAsListener(IIOPort *port) {
        port->registerPortListener(static_cast<IIOPortListener8 *>(this));
    }

    /** IIOPortListener8 */
    virtual void readData(uint8_t *val, uint8_t mask) {
        Reg64Type t;
        t.val = *val;
        memread(&t.val, mask);
        *val = t.buf[0];
    }

    virtual void writeData(uint8_t val, uint8_t mask) {
        memwrite(val, mask);
    }

    virtual void latch() { value_ = prelatch_; }

 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }
};

class IOPinType16 : public IOPinType,
                    public IIOPortListener16 {
 public:
    IOPinType16(IService *parent, const char *name) :
        IOPinType(parent, name) {}
    IOPinType16(IService *parent, const char *name, AttributeType &pincfg) :
        IOPinType(parent, name, pincfg) {}

    virtual void regiterAsListener(IIOPort *port) {
        port->registerPortListener(static_cast<IIOPortListener16 *>(this));
    }

    /** IIOPortListener16 */
    virtual void readData(uint16_t *val, uint16_t mask) {
        Reg64Type t;
        t.val = *val;
        memread(&t.val, mask);
        *val = t.buf16[0];
    }

    virtual void writeData(uint16_t val, uint16_t mask) {
        memwrite(val, mask);
    }

    virtual void latch() {
        value_ = prelatch_;
    }
};

/** TODO: remove IOPinType from classes like in the following */
class IOPinType32 : public IIOPortListener32 {
 public:
    IOPinType32(IService *parent, const char *name) : parent_(parent) {
        parent->registerPortInterface(name,
                static_cast<IIOPortListener32 *>(this));
        pinidx_ = 0;
        bitLatched_ = false;
        bitPreLatched_ = false;
    }

    /** IIOPortListener32 */
    virtual void readData(uint32_t *val, uint32_t mask) {
        *val |= bitLatched_ << pinidx_;
    }
    virtual void writeData(uint32_t val, uint32_t mask) {
        //if (mask & (1ul << pinidx_)) {
            bitPreLatched_ = (val >> pinidx_) & 1;
        //}
    }
    virtual void latch() override {
        if (!bitLatched_ && bitPreLatched_) {
            posedge();
        } else if (bitLatched_ && !bitPreLatched_) {
            negedge();
        }
        bitLatched_ = bitPreLatched_;
    }

    /** Common methods: */
    void setPinIdx(int pinidx) { pinidx_ = pinidx; }
    uint32_t getLevel() { return bitLatched_; }

 protected:
    virtual void posedge() {}
    virtual void negedge() {}

 protected:
    IService *parent_;
    int pinidx_;
    uint32_t bitLatched_;
    uint32_t bitPreLatched_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_GENERIC_IOTYPES_H__
