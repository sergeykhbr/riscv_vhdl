/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read only registers values.
 */

#include "cmd_regs.h"

namespace debugger {

// Any value greater or equal than 32 + 2 (general iregs + pc + npc)
#define REGS_MAX 64

CmdRegs::CmdRegs(ITap *tap, ISocInfo *info) 
    : ICommand ("regs", tap, info) {

    briefDescr_.make_string("List of Core's registers values");
    detailedDescr_.make_string(
        "Description:\n"
        "    Print values of CPU's registers.\n"
        "Return:\n"
        "    Dictionary if no names specified, list of int64_t otherwise.\n"
        "Usage:\n"
        "    regs\n"
        "    regs name1 name2 ..\n"
        "Example:\n"
        "    regs\n"
        "    regs a0 s0 sp\n");
}

bool CmdRegs::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() >= 1) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdRegs::exec(AttributeType *args, AttributeType *res) {
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    Reg64Type u;
    if (args->size() != 1) {
        res->make_list(args->size() - 1);
        for (unsigned i = 1; i < args->size(); i++) {
            const char *name = (*args)[i].to_string();
            tap_->read(info_->reg2addr(name), 8, u.buf);
            (*res)[i - 1].make_uint64(u.val);
        }
        return;
    }

    AttributeType soclst;
    info_->getRegsList(&soclst);

    struct RegsArrType {
        Reg64Type reg[REGS_MAX];
    };
    union CpiRegionType {
        RegsArrType regarr;
        uint8_t buf[sizeof(RegsArrType)];
    } t1;
    DsuMapType *dsu = info_->getpDsu();
    uint64_t addr = reinterpret_cast<uint64_t>(dsu->ureg.v.iregs);
    addr &= 0xFFFFFFFFul;
    tap_->read(addr, 8 * soclst.size(), t1.buf);

    uint64_t idx;
    res->make_dict();
    for (unsigned i = 0; i < soclst.size(); i++) {
        const char *name = soclst[i].to_string();
        idx = (info_->reg2addr(name) - addr) / sizeof(uint64_t);
        (*res)[name].make_uint64(t1.regarr.reg[idx].val);
    }
}

}  // namespace debugger
