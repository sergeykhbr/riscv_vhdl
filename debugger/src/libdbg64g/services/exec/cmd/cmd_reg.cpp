/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "cmd_reg.h"
#include "riscv-isa.h"

namespace debugger {
struct reg_default_list_type {
    char name[8];
};
static const reg_default_list_type RISCV_INTEGER_REGLIST[] = {
    "pc", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    0
};

CmdReg::CmdReg(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "reg", ijtag) {

    briefDescr_.make_string("Read or modify CPU registers values");
    detailedDescr_.make_string(
        "Description:\n"
        "    Print values of CPU registers if no arguments are provided. Otherwise,\n"
        "    read or write specified registers.\n"
        "Return:\n"
        "    Dictionary format: {'name1':rdval1, 'name2':rdval2, ..}.\n"
        "Usage:\n"
        "    reg\n"
        "    reg name1 name2 ..\n"
        "    reg name1 value1 name2 value2 ..\n"
        "Example:\n"
        "    reg\n"
        "    reg a0 s0 sp\n"
        "    reg pc 0x1000\n"
        "    reg ra 0x200 pc 0x1000\n");
}

int CmdReg::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    } 
    if (args->size() >= 1) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdReg::exec(AttributeType *args, AttributeType *res) {
    Reg64Type u;
    res->make_dict();

    if (args->size() == 1) {
        // Read Integer registers
        const reg_default_list_type *preg = RISCV_INTEGER_REGLIST;
        while (*preg->name) {
            if (get_reg(preg->name, &u)) {
                generateError(res, "Cannot read registers");
                break;
            }
            (*res)[preg->name].make_uint64(u.val);
            preg++;
        }
        return;
    }

    const char *regname;
    int err;
    for (unsigned i = 1; i < args->size(); i++) {
        regname = (*args)[i].to_string();

        if ((i + 1) < args->size() && (*args)[i+1].is_integer()) {
            u.val = (*args)[i+1].to_uint64();
            err = set_reg(regname, &u);
            i++;
        } else {
            err = get_reg(regname, &u);
            (*res)[regname].make_uint64(u.val);
        }

        if (err) {
            generateError(res, "Cannot read registers");
            break;
        }
    }
}

}  // namespace debugger
