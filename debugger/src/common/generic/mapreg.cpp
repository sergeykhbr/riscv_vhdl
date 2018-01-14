/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Template for the Mapped/IO register logic.
 */

#include <api_core.h>
#include "mapreg.h"

namespace debugger {

MappedReg64Type::MappedReg64Type(IService *parent, const char *name,
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
    value_.val = 0;
    hard_reset_value_ = 0;
}

IFace *MappedReg64Type::getInterface(const char *name) {
    if (strcmp(name, IFACE_MEMORY_OPERATION) == 0) {
        return static_cast<IMemoryOperation *>(this);
    }
    return parent_->getInterface(name);
}

void MappedReg64Type::reset(bool active) {
    if (!active) {
        return;
    }
    value_.val = hard_reset_value_;
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

}  // namespace debugger

