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

static const ECpuRegMapping RISCV_INTEGER_REG_MAP[] = {
    {"pc",    8, 0x7b1}, //CSR_dpc},
    {"ra",    8, 0x1001},
    {"sp",    8, 0x1002},
    {"gp",    8, 0x1003},
    {"tp",    8, 0x1004},
    {"t0",    8, 0x1005},
    {"t1",    8, 0x1006},
    {"t2",    8, 0x1007},
    {"s0",    8, 0x1008},
    {"s1",    8, 0x1009},
    {"a0",    8, 0x100A},
    {"a1",    8, 0x100B},
    {"a2",    8, 0x100C},
    {"a3",    8, 0x100D},
    {"a4",    8, 0x100E},
    {"a5",    8, 0x100F},
    {"a6",    8, 0x1010},
    {"a7",    8, 0x1011},
    {"s2",    8, 0x1012},
    {"s3",    8, 0x1013},
    {"s4",    8, 0x1014},
    {"s5",    8, 0x1015},
    {"s6",    8, 0x1016},
    {"s7",    8, 0x1017},
    {"s8",    8, 0x1018},
    {"s9",    8, 0x1019},
    {"s10",   8, 0x101A},
    {"s11",   8, 0x101B},
    {"t3",    8, 0x101C},
    {"t4",    8, 0x101D},
    {"t5",    8, 0x101E},
    {"t6",    8, 0x101F},
    {"",      0, 0}
};


CmdReg::CmdReg(IJtag *ijtag)
    : ICommand ("reg", ijtag) {

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
        const ECpuRegMapping *preg = getpMappedReg();
        while (preg->name[0]) {
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

uint32_t CmdReg::get_reg(const char *regname, Reg64Type *res) {
    IJtag::dmi_abstractcs_type abstractcs;
    IJtag::dmi_command_type command;

    command.u32 = 0;
    command.regaccess.cmdtype = 0;
    command.regaccess.aarsize = regsize(regname);
    command.regaccess.transfer = 1;
    command.regaccess.aarpostincrement = 1;
    command.regaccess.regno = reg2addr(regname);

    ijtag_->scanDmi(IJtag::DMI_COMMAND, command.u32, IJtag::DmiOp_Write);
    do {
        abstractcs.u32 = ijtag_->scanDmi(IJtag::DMI_ABSTRACTCS, 0, IJtag::DmiOp_Read);
    } while (abstractcs.bits.busy == 1);

    if (command.regaccess.aarpostincrement) {
        command.regaccess.regno++;
    }

    res->buf32[0] = ijtag_->scanDmi(IJtag::DMI_ABSTRACT_DATA0, 0, IJtag::DmiOp_Read);
    res->buf32[1] = ijtag_->scanDmi(IJtag::DMI_ABSTRACT_DATA1, 0, IJtag::DmiOp_Read);
    return abstractcs.bits.cmderr;
}

uint32_t CmdReg::set_reg(const char *regname, Reg64Type *val) {
    IJtag::dmi_abstractcs_type abstractcs;
    IJtag::dmi_command_type command;

    ijtag_->scanDmi(IJtag::DMI_ABSTRACT_DATA0, val->buf32[0], IJtag::DmiOp_Write);
    ijtag_->scanDmi(IJtag::DMI_ABSTRACT_DATA1, val->buf32[1], IJtag::DmiOp_Write);

    command.u32 = 0;
    command.regaccess.cmdtype = 0;
    command.regaccess.aarsize = regsize(regname);
    command.regaccess.write = 1;
    command.regaccess.transfer = 1;
    command.regaccess.aarpostincrement = 1;
    command.regaccess.regno = reg2addr(regname);

    ijtag_->scanDmi(IJtag::DMI_COMMAND, command.u32, IJtag::DmiOp_Write);
    do {
        abstractcs.u32 = ijtag_->scanDmi(IJtag::DMI_ABSTRACTCS, 0, IJtag::DmiOp_Read);
    } while (abstractcs.bits.busy == 1);

    if (command.regaccess.aarpostincrement) {
        command.regaccess.regno++;
    }
    return abstractcs.bits.cmderr;
}

uint64_t CmdReg::reg2addr(const char *name) {
    const ECpuRegMapping  *preg = getpMappedReg();
    while (preg->name[0]) {
        if (strcmp(name, preg->name) == 0) {
            return preg->offset;
        }
        preg++;
    }
    return REG_ADDR_ERROR;
}

const ECpuRegMapping *CmdReg::getpMappedReg() {
    return RISCV_INTEGER_REG_MAP;
}

}  // namespace debugger
