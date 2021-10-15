/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "debug/dmi_regs.h"
#include "cmd_dmi_runcontrol.h"

namespace debugger {

CmdDmiRunControl::CmdDmiRunControl(IFace *parent, uint64_t dmibaseaddr, ITap *tap)
    : ICommand (static_cast<IService *>(parent)->getObjName(), tap),
    dmibaseaddr_(dmibaseaddr) {

    briefDescr_.make_string("Core run control command");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run, halt or run with enabled stepping command for a selected CPU.\n"
        "Example:\n"
        "    <objname> command\n"
        "    Commands:\n"
        "       halt, stop, break - are the commands to stop the CPU\n"
        "       go, run, c - are the command to start the CPU\n"
        "       step - execute one instruction and return into Debug Mode\n"
        "Example:\n"
        "    core0 halt\n"
        "    core0 stop\n"
        "    core0 go\n"
        "    core0 step\n");
}


int CmdDmiRunControl::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (name.is_equal("status")) {
        return CMD_VALID;
    }
    if (!cmdName_.is_equal(name.to_string())) {
        return CMD_INVALID;
    }
    if (args->size() < 2) {
        return CMD_WRONG_ARGS;
    }
    AttributeType &par1 = (*args)[1];
    if (!par1.is_equal("halt")
        && !par1.is_equal("stop")
        && !par1.is_equal("break")
        && !par1.is_equal("go")
        && !par1.is_equal("run")
        && !par1.is_equal("c")
        && !par1.is_equal("step")
        && !par1.is_equal("reg")
        && !par1.is_equal("regs")) {
        return CMD_WRONG_ARGS;
    }
    if (!par1.is_equal("reg") && args->size() != 3) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdDmiRunControl::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    AttributeType &name = (*args)[0u];
    AttributeType &par1 = (*args)[1];
    Reg64Type reg = {0};

    if (name.is_equal("status")) {
        // Read arg0
        tap_->read(dmibaseaddr_ + 4*0x40, 4, reg.buf);    // haltsum0
        res->make_uint64(reg.val);
        return;
    }

    DCSR_TYPE::ValueType dcsr;
    DMCONTROL_TYPE::ValueType dmcontrol;
    COMMAND_TYPE::ValueType command;
    dmcontrol.val = 0;
    if (par1.is_equal("halt") || par1.is_equal("stop") || par1.is_equal("break")) {
        dmcontrol.bits.haltreq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 4, dmcontrol.u8);
    } else if (par1.is_equal("go") || par1.is_equal("run") || par1.is_equal("c")) {
        dmcontrol.bits.resumereq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 4, dmcontrol.u8);
    } else if (par1.is_equal("step")) {
        // enabe stepping = 1
        dcsr.val = 0;
        dcsr.bits.ebreakm = 1;
        dcsr.bits.step = 1;
        tap_->write(dmibaseaddr_ + 4*0x04, 4, dcsr.u8);    // arg0 = [data1,data0]
        // transfer dcsr,arg0
        command.val = 0;
        command.bits.transfer = 1;
        command.bits.aarsize = 2;
        command.bits.regno = 0x7b0;//CSR_dcsr;
        tap_->write(dmibaseaddr_ + 4*0x17, 4, command.u8);    // arg0 = [data1,data0]
        // resume with step enabled
        dmcontrol.val = 0;
        dmcontrol.bits.resumereq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 4, dmcontrol.u8);
    } else if (par1.is_equal("reg")) {
        AttributeType &regname = (*args)[2];
        if (regname.is_equal("npc")) {
            // transfer dpc,arg0
            command.val = 0;
            command.bits.transfer = 1;
            command.bits.aarsize = 3;
            command.bits.regno = 0x7b1;//CSR_dpc;
            tap_->write(dmibaseaddr_ + 4*0x17, 4, command.u8);    // arg0 = [data1,data0]
            // Read arg0
            tap_->read(dmibaseaddr_ + 4*0x4, 4, reg.buf);    // [data1,data0]
            tap_->read(dmibaseaddr_ + 4*0x5, 4, &reg.buf[4]);
        } else if (regname.is_equal("steps")) {
            // transfer dpc,arg0
            command.val = 0;
            command.bits.transfer = 1;
            command.bits.aarsize = 3;
            command.bits.regno = 0xC02;//CSR_insret;
            tap_->write(dmibaseaddr_ + 4*0x17, 4, command.u8);    // arg0 = [data1,data0]
            // Read arg0
            tap_->read(dmibaseaddr_ + 4*0x4, 4, reg.buf);    // [data1,data0]
            tap_->read(dmibaseaddr_ + 4*0x5, 4, &reg.buf[4]);
        }
        res->make_uint64(reg.val);
    }
}

}  // namespace debugger
