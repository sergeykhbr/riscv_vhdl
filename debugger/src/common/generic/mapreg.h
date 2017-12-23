/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Templates for the memory mapped registers and banks.
 */

#ifndef __DEBUGGER_COMMON_GENERIC_MAPREG_H__
#define __DEBUGGER_COMMON_GENERIC_MAPREG_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/ireset.h"

namespace debugger {

class MappedReg64Type : public IMemoryOperation,
                        public IResetListener {
 public:
    MappedReg64Type(IService *parent, const char *name,
                    uint64_t addr, int len = 8, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(bool active);

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    Reg64Type getValue() { return value_; }
    void setValue(Reg64Type v) { value_ = v; }
    void setValue(uint64_t v) { value_.val = v; }

 protected:
    /** Possible side effects handlers:  */
    virtual uint64_t aboutToRead(uint64_t cur_val) {
        return cur_val;
    }
    virtual uint64_t aboutToWrite(uint64_t new_val) {
        return new_val;
    }
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
    Reg64Type value_;
    uint64_t hard_reset_value_;
};


class GenericReg64Bank : public IMemoryOperation {
 public:
    GenericReg64Bank(IService *parent, const char *name,
                    uint64_t addr, int len) {
        parent->registerPortInterface(name,
                                      static_cast<IMemoryOperation *>(this));
        regs_ = 0;
        bankName_.make_string(name);
        baseAddress_.make_uint64(addr);
        setLength(len);
    }
    virtual ~GenericReg64Bank() {
        if (regs_) {
            delete [] regs_;
        }
    }

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset();

    /** General access methods: */
    void setLength(int len);
    Reg64Type read(int idx) { return regs_[idx]; }
    void write(int idx, Reg64Type val) { regs_[idx] = val; }
    void write(int idx, uint64_t val) { regs_[idx].val = val; }
    Reg64Type *getp() { return regs_; }
    uint64_t *getpR64() { return &regs_[0].val; }

 protected:
    AttributeType bankName_;
    Reg64Type *regs_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_GENERIC_MAPREG_H__
