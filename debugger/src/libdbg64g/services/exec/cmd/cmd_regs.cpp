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

    res->make_dict();
    for (unsigned i = 0; i < soclst.size(); i++) {
        const char *name = soclst[i].to_string();
        tap_->read(info_->reg2addr(name), 8, u.buf);
        (*res)[name].make_uint64(u.val);
    }
}

}  // namespace debugger
