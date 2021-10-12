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
    if (!name.is_equal("core0")) {
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
        && !par1.is_equal("step")) {
        return CMD_INVALID;
    }
    return CMD_VALID;
}

void CmdDmiRunControl::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    AttributeType &par1 = (*args)[1];

    DCSR_TYPE::ValueType dcsr;
    DMCONTROL_TYPE::ValueType dmcontrol;
    dmcontrol.val = 0;
    if (par1.is_equal("halt") || par1.is_equal("stop") || par1.is_equal("break")) {
        dmcontrol.bits.haltreq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 8, dmcontrol.u8);
    } else if (par1.is_equal("go") || par1.is_equal("run") || par1.is_equal("c")) {
        dmcontrol.bits.resumereq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 8, dmcontrol.u8);
    } else if (par1.is_equal("step")) {
        dcsr.val = 0;
        dcsr.bits.ebreakm = 1;
        dcsr.bits.step = 1;
        // TODO: write step in dcsr using abstract command

        dmcontrol.bits.resumereq = 1;
        tap_->write(dmibaseaddr_ + 4*0x10, 8, dmcontrol.u8);
    }
}

}  // namespace debugger
