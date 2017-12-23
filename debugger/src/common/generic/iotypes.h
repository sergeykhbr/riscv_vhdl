/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Input/output registers and line implementation.
 */

#ifndef __DEBUGGER_COMMON_GENERIC_IOTYPES_H__
#define __DEBUGGER_COMMON_GENERIC_IOTYPES_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/iioport.h"
#include "coreservices/ireset.h"
#include "coreservices/iwire.h"

namespace debugger {

class IOReg8Type : public IMemoryOperation,
                   public IIOPort,
                   public IResetListener {
 public:
    IOReg8Type(IService *parent, const char *name,
               uint16_t addr, uint16_t len, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IIOPort */
    virtual void registerPortListener(IFace *listener);
    virtual void unregisterPortListener(IFace *listener);

    /** IResetListener interface */
    virtual void reset(bool active);

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    uint8_t getValue() { return value.byte; }
    void setValue(uint8_t v) { value.byte = v; }

 protected:
    virtual uint8_t read();
    virtual void write(uint8_t val);
    virtual uint8_t get_direction() {return 0u; }

 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
    Reg8Type value;
    uint8_t hard_reset_value_;
};


class IOPinType : public IIOPortListener {
 public:
    IOPinType(IService *parent, const char *name);

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
    virtual void aboutToWrite(uint8_t prev, uint8_t val) {}

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
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_GENERIC_IOTYPES_H__
