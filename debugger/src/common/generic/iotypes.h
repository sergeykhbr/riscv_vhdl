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
    virtual void reset(bool active);

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

class IOReg32Type : public IORegType {
public:
    IOReg32Type(IService *parent, const char *name,
        uint32_t addr, uint32_t len, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(bool active);

    /** General access methods: */
    uint32_t getValue() { return value.buf32[0]; }
    void setValue(uint32_t v) { value.buf32[0] = v; }

protected:
    virtual uint32_t get_direction() { return 0u; }
    virtual uint32_t read();
    virtual void write(uint32_t val);

protected:
    Reg64Type value;
    uint32_t hard_reset_value_;
};

class IOPinType : public IIOPortListener {
 public:
    IOPinType(IService *parent, const char *name);
    IOPinType(IService *parent, const char *name, AttributeType &pincfg);

    /** IIOPortListener */
    virtual void readData(uint8_t *val, uint8_t mask);
    virtual void writeData(uint8_t val, uint8_t mask);
    virtual void latch();

    /** generic accessors */
    void postinit();
    void connectToBit(const AttributeType &cfg);
    void connectToWire(const AttributeType &cfg);
    uint8_t get_bit();
    void set_bit(uint8_t v);

 protected:
    virtual uint8_t aboutToRead(uint8_t val) { return val; }
    virtual void aboutToWrite(uint8_t /*prev*/, uint8_t /*val*/) {}

 protected:
    static const int READ_MASK = 0x1;
    static const int WRITE_MASK = 0x2;

    AttributeType pinName_;
    AttributeType IOPinTypeCfg_;
    IService *parent_;
    IWire *iwire_;
    uint8_t value_;     // triggered
    uint8_t prelatch_;  // not triggered value
    int bitIdx_;
    int access_;
};

class IOPinTypeDebug : public IOPinType {
 public:
    IOPinTypeDebug(IService *parent, const char *name)
        : IOPinType(parent, name) {}
    IOPinTypeDebug(IService *parent, const char *name, AttributeType &pincfg)
        : IOPinType(parent, name, pincfg) {}
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_GENERIC_IOTYPES_H__
