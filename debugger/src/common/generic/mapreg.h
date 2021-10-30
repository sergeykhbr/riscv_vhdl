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
                    uint64_t addr, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { value_.val = hard_reset_value_; }

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    Reg64Type getValue() { return value_; }
    void setValue(Reg64Type v) { value_ = v; }
    void setValue(uint64_t v) { value_.val = v; }
    void setHardResetValue(uint64_t v) {
        hard_reset_value_ = v;
        reset(0);
    } 

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

class MappedReg32Type : public IMemoryOperation,
                        public IResetListener {
 public:
    MappedReg32Type(IService *parent, const char *name,
                    uint64_t addr, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { value_.val = hard_reset_value_; }

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    Reg32Type getValue() { return value_; }
    virtual void setValue(Reg32Type v) { value_ = v; }
    virtual void setValue(uint32_t v) { value_.val = v; }
    virtual void setHardResetValue(uint32_t v) {
        hard_reset_value_ = v;
        reset(0);
    } 

 protected:
    /** Possible side effects handlers:  */
    virtual uint32_t aboutToRead(uint32_t cur_val) {
        return cur_val;
    }
    virtual uint32_t aboutToWrite(uint32_t new_val) {
        return new_val;
    }
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
    Reg32Type value_;
    uint32_t hard_reset_value_;
};

class MappedReg16Type : public IMemoryOperation,
                        public IResetListener {
 public:
    MappedReg16Type(IService *parent, const char *name,
                    uint64_t addr, int len = 2, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { value_.word = hard_reset_value_; }

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    Reg16Type getValue() { return value_; }
    void setValue(Reg16Type v) { value_ = v; }
    void setValue(uint16_t v) { value_.word = v; }
    void setHardResetValue(uint16_t v) {
        hard_reset_value_ = v;
        reset(0);
    } 

 protected:
    /** Possible side effects handlers:  */
    virtual uint16_t aboutToRead(uint16_t cur_val) {
        return cur_val;
    }
    virtual uint16_t aboutToWrite(uint16_t new_val) {
        return new_val;
    }
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
    Reg16Type value_;
    uint16_t hard_reset_value_;
};

class MappedReg8Type : public IMemoryOperation,
                       public IResetListener {
 public:
    MappedReg8Type(IService *parent, const char *name,
                    uint64_t addr, int len = 1, int priority = 1);

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset(IFace *isource) { value_.byte = hard_reset_value_; }

    /** General access methods: */
    const char *regName() { return regname_.to_string(); }
    Reg8Type getValue() { return value_; }
    void setValue(Reg8Type v) { value_ = v; }
    void setValue(uint8_t v) { value_.byte = v; }
    void setHardResetValue(uint8_t v) {
        hard_reset_value_ = v;
        reset(0);
    } 

 protected:
    /** Possible side effects handlers:  */
    virtual uint8_t aboutToRead(uint8_t cur_val) {
        return cur_val;
    }
    virtual uint8_t aboutToWrite(uint8_t new_val) {
        return new_val;
    }
 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType regname_;
    AttributeType portListeners_;
    Reg8Type value_;
    uint8_t hard_reset_value_;
};

class GenericReg64Bank : public IMemoryOperation {
 public:
    GenericReg64Bank(IService *parent, const char *name,
                    uint64_t addr, int len) {
        parent_ = parent;
        parent->registerPortInterface(name,
                    static_cast<IMemoryOperation *>(this));
        regs_ = 0;
        bankName_.make_string(name);
        baseAddress_.make_uint64(addr);
        setRegTotal(len);
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
    void setRegTotal(int len);
    virtual Reg64Type read(int idx) { return regs_[idx]; }
    virtual void write(int idx, Reg64Type val) { regs_[idx] = val; }
    virtual void write(int idx, uint64_t val) { regs_[idx].val = val; }
    Reg64Type *getp() { return regs_; }
    uint64_t *getpR64() { return &regs_[0].val; }

 protected:
    IService *parent_;
    AttributeType bankName_;
    Reg64Type *regs_;
};

class GenericReg32Bank : public IMemoryOperation {
 public:
    GenericReg32Bank(IService *parent, const char *name,
                    uint64_t addr, int len) {
        parent_ = parent;
        parent->registerPortInterface(name,
                    static_cast<IMemoryOperation *>(this));
        regs_ = 0;
        bankName_.make_string(name);
        baseAddress_.make_uint64(addr);
        setRegTotal(len);
    }
    virtual ~GenericReg32Bank() {
        if (regs_) {
            delete [] regs_;
        }
    }

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset();

    /** General access methods: */
    void setRegTotal(int len);
    virtual uint32_t read(int idx) { return regs_[idx].val; }
    virtual void write(int idx, uint32_t val) { regs_[idx].val = val; }
    Reg32Type *getp() { return regs_; }
    uint32_t *getpR32() { return &regs_[0].val; }

 protected:
    IService *parent_;
    AttributeType bankName_;
    Reg32Type *regs_;
};

class GenericReg16Bank : public IMemoryOperation {
 public:
    GenericReg16Bank(IService *parent, const char *name,
                    uint64_t addr, int len) {
        parent_ = parent;
        parent->registerPortInterface(name,
                static_cast<IMemoryOperation *>(this));
        regs_ = 0;
        bankName_.make_string(name);
        baseAddress_.make_uint64(addr);
        setRegTotal(len);
    }
    virtual ~GenericReg16Bank() {
        if (regs_) {
            delete [] regs_;
        }
    }

    /** IMemoryOperation methods */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IResetListener interface */
    virtual void reset();

    /** General access methods: */
    void setRegTotal(int len);
    virtual Reg16Type read(int idx) { return regs_[idx]; }
    virtual void write(int idx, Reg16Type val) { regs_[idx] = val; }
    virtual void write(int idx, uint16_t val) { regs_[idx].word = val; }
    Reg16Type *getp() { return regs_; }
    uint16_t *getpR16() { return &regs_[0].word; }

 protected:
    virtual uint16_t aboutToRead(int idx, uint16_t cur_val);
    virtual uint16_t aboutToWrite(int idx, uint16_t new_val);

 protected:
    // Debug output compatibility
    IFace *getInterface(const char *name);

 protected:
    IService *parent_;
    AttributeType bankName_;
    Reg16Type *regs_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_GENERIC_MAPREG_H__
