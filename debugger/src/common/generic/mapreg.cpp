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

#include <api_core.h>
#include "mapreg.h"

namespace debugger {

MappedReg64Type::MappedReg64Type(IService *parent, const char *name,
                  uint64_t addr, int priority) {
    if (parent == NULL) {
    } else {
        parent->registerPortInterface(name,
                static_cast<IMemoryOperation *>(this));
        parent->registerPortInterface(name,
                static_cast<IResetListener *>(this));
    }
    parent_ = parent;
    portListeners_.make_list(0);
    regname_.make_string(name);
    baseAddress_.make_uint64(addr);
    length_.make_uint64(8);
    priority_.make_int64(priority);
    value_.val = 0;
    hard_reset_value_ = 0;
}

IFace *MappedReg64Type::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

ETransStatus MappedReg64Type::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    if (trans->action == MemAction_Read) {
        Reg64Type cur;
        cur.val = aboutToRead(value_.val);
        memcpy(trans->rpayload.b8, &cur.buf[off], trans->xsize);
        RISCV_debug("Read %s [%02" RV_PRI64 "x] => %016" RV_PRI64 "x",
                    regName(), trans->addr, trans->rpayload.b64[0]);
    } else {
        Reg64Type new_val = value_;
        memcpy(&new_val.buf[off], trans->wpayload.b8, trans->xsize);
        new_val.val = aboutToWrite(new_val.val);
        value_ = new_val;
        RISCV_debug("Write %s [%02" RV_PRI64 "x] <= %016" RV_PRI64 "x",
                    regName(), trans->addr, trans->wpayload.b64[0]);
    }
    return TRANS_OK;
}

/** 32-bits register */
MappedReg32Type::MappedReg32Type(IService *parent, const char *name,
                  uint64_t addr, int priority) {
    if (parent == NULL) {
    } else {
        parent->registerPortInterface(name,
                static_cast<IMemoryOperation *>(this));
        parent->registerPortInterface(name,
                static_cast<IResetListener *>(this));
    }
    parent_ = parent;
    portListeners_.make_list(0);
    regname_.make_string(name);
    baseAddress_.make_uint64(addr);
    length_.make_uint64(4);
    priority_.make_int64(priority);
    value_.val = 0;
    hard_reset_value_ = 0;
}

IFace *MappedReg32Type::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

ETransStatus MappedReg32Type::b_transport(Axi4TransactionType *trans) {
    // ARM supports word, half-word and byte access to registers
    uint64_t off = trans->addr - getBaseAddress();
    int xsz = trans->xsize >= 4 ? 4 : static_cast<int>(trans->xsize);
    int roff = static_cast<int>(off & 0x3);
    int rsz = xsz - roff;
    Reg32Type cur;
    if (trans->action == MemAction_Read) {
        cur.val = aboutToRead(value_.val);
        trans->rpayload.b32[0] = 0;
        memcpy(trans->rpayload.b8, &cur.buf[roff], rsz);
        RISCV_debug("Read %s [%02" RV_PRI64 "x] => %08x",
                    regName(), trans->addr, trans->rpayload.b32[0]);
    } else {
        cur.val = value_.val;
        memcpy(&cur.buf[roff], trans->wpayload.b8, rsz);
        value_.val = aboutToWrite(cur.val);
        RISCV_debug("Write %s [%02" RV_PRI64 "x] <= %08x",
                    regName(), trans->addr, trans->wpayload.b32[0]);
    }
    return TRANS_OK;
}


/** 16-bits register */
MappedReg16Type::MappedReg16Type(IService *parent, const char *name,
                  uint64_t addr, int len, int priority) {
    if (parent == NULL) {
    } else {
        parent->registerPortInterface(name,
                static_cast<IMemoryOperation *>(this));
        parent->registerPortInterface(name,
                static_cast<IResetListener *>(this));
    }
    parent_ = parent;
    portListeners_.make_list(0);
    regname_.make_string(name);
    baseAddress_.make_uint64(addr);
    length_.make_uint64(len);
    priority_.make_int64(priority);
    value_.word = 0;
    hard_reset_value_ = 0;
}

IFace *MappedReg16Type::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

ETransStatus MappedReg16Type::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    if (trans->action == MemAction_Read) {
        Reg16Type cur;
        cur.word = aboutToRead(value_.word);
        if (trans->addr & 0x1) {
            trans->rpayload.b8[0] = cur.buf[off];
            trans->rpayload.b8[1] = 0;
        } else {
            trans->rpayload.b8[0] = cur.buf[off];
            trans->rpayload.b8[1] = cur.buf[off + 1];
        }
        RISCV_debug("Read %s [%02" RV_PRI64 "x] => %04x",
                    regName(), trans->addr, trans->rpayload.b16[0]);
    } else {
        Reg16Type new_val = value_;
        if (trans->xsize == 1) {
            new_val.r8[trans->addr & 0x1].byte = trans->wpayload.b8[0];
        } else {
            new_val.word = trans->wpayload.b16[0];
        }
        new_val.word = aboutToWrite(new_val.word);
        value_ = new_val;
        RISCV_debug("Write %s [%02" RV_PRI64 "x] <= %04x",
                    regName(), trans->addr, trans->wpayload.b16[0]);
    }
    return TRANS_OK;
}

/** 8-bits register */
MappedReg8Type::MappedReg8Type(IService *parent, const char *name,
                  uint64_t addr, int len, int priority) {
    if (parent == NULL) {
    } else {
        parent->registerPortInterface(name,
                static_cast<IMemoryOperation *>(this));
        parent->registerPortInterface(name,
                static_cast<IResetListener *>(this));
    }
    parent_ = parent;
    portListeners_.make_list(0);
    regname_.make_string(name);
    baseAddress_.make_uint64(addr);
    length_.make_uint64(len);
    priority_.make_int64(priority);
    value_.byte = 0;
    hard_reset_value_ = 0;
}

IFace *MappedReg8Type::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

ETransStatus MappedReg8Type::b_transport(Axi4TransactionType *trans) {
    if (trans->action == MemAction_Read) {
        trans->rpayload.b8[0] = aboutToRead(value_.byte);
        RISCV_debug("Read %s [%02" RV_PRI64 "x] => %02x",
                    regName(), trans->addr, trans->rpayload.b8[0]);
    } else {
        value_.byte = aboutToWrite(trans->wpayload.b8[0]);
        RISCV_debug("Write %s [%02" RV_PRI64 "x] <= %04x",
                    regName(), trans->addr, trans->wpayload.b16[0]);
    }
    return TRANS_OK;
}

ETransStatus GenericReg64Bank::b_transport(Axi4TransactionType *trans) {
    int idx = static_cast<int>((trans->addr - getBaseAddress()) >> 3);
    if (trans->action == MemAction_Read) {
        trans->rpayload.b64[0] = read(idx).val;
    } else {
        write(idx, trans->wpayload.b64[0]);
    }
    return TRANS_OK;
}

void GenericReg64Bank::reset() {
    memset(regs_, 0, length_.to_int());
}

void GenericReg64Bank::setRegTotal(int len) {
    if (len * static_cast<int>(sizeof(Reg64Type)) == length_.to_int()) {
        return;
    }
    if (regs_) {
        delete [] regs_;
    }
    length_.make_int64(len * sizeof(Reg64Type));
    regs_ = new Reg64Type[len];
    reset();
}


ETransStatus GenericReg32Bank::b_transport(Axi4TransactionType *trans) {
    int idx = static_cast<int>((trans->addr - getBaseAddress()) >> 2);
    if (trans->action == MemAction_Read) {
        trans->rpayload.b64[0] = read(idx);
    } else {
        write(idx, trans->wpayload.b32[0]);
    }
    return TRANS_OK;
}

void GenericReg32Bank::reset() {
    memset(regs_, 0, length_.to_int());
}

void GenericReg32Bank::setRegTotal(int len) {
    if (len * static_cast<int>(sizeof(Reg32Type)) == length_.to_int()) {
        return;
    }
    if (regs_) {
        delete [] regs_;
    }
    length_.make_int64(len * sizeof(Reg32Type));
    regs_ = new Reg32Type[len];
    reset();
}


ETransStatus GenericReg16Bank::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    int idx = static_cast<int>(off >> 1);
    Reg16Type t;
    if (trans->action == MemAction_Read) {
        t = read(idx);
        trans->rpayload.b64[0] = aboutToRead(idx, t.word);
        RISCV_debug("Read %s[%04" RV_PRI64 "x] => %04x",
            bankName_.to_string(), off/2, t.word);
    } else {
        if (trans->xsize == 1) {
            t = getp()[idx];
            t.buf[off & 0x1] = trans->wpayload.b8[0];
            t.word = aboutToWrite(idx, t.word);
            write(idx, t.word);
            RISCV_debug("Write %s[%04" RV_PRI64 "x] <= %04x",
                bankName_.to_string(), off/2, t.word);
        } else {
            t.word = aboutToWrite(idx, trans->wpayload.b16[0]);
            write(idx, t.word);
            RISCV_debug("Write %s[%04" RV_PRI64 "x] <= %04x",
                bankName_.to_string(), off/2, t.word);
        }
    }
    return TRANS_OK;
}

void GenericReg16Bank::reset() {
    memset(regs_, 0, length_.to_int());
}

void GenericReg16Bank::setRegTotal(int len) {
    if (len * static_cast<int>(sizeof(Reg16Type)) == length_.to_int()) {
        return;
    }
    if (regs_) {
        delete [] regs_;
    }
    length_.make_int64(len * sizeof(Reg16Type));
    regs_ = new Reg16Type[len];
    reset();
}

uint16_t GenericReg16Bank::aboutToRead(int idx, uint16_t cur_val) {
    return cur_val;
}

uint16_t GenericReg16Bank::aboutToWrite(int idx, uint16_t new_val) {
    return new_val;
}


IFace *GenericReg16Bank::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

}  // namespace debugger

