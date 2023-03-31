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

#include "cmd_halt.h"

namespace debugger {

CmdHalt::CmdHalt(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "halt", ijtag) {

    briefDescr_.make_string("Stop simulation");
    detailedDescr_.make_string(
        "Description:\n"
        "    Stop simulation.\n"
        "Example:\n"
        "    halt\n"
        "    stop\n"
        "    s\n");
}


int CmdHalt::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (!cmdName_.is_equal(name.to_string())
        && !name.is_equal("break")
        && !name.is_equal("stop")
        && !name.is_equal("s")) {
        return CMD_INVALID;
    }
    if (args->size() != 1) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdHalt::exec(AttributeType *args, AttributeType *res) {
    IJtag::dmi_dmstatus_type dmstatus;
    IJtag::dmi_dmcontrol_type dmcontrol;
    IJtag::dmi_abstractcs_type abstractcs;
    int watchdog = 0;
    res->attr_free();
    res->make_nil();

    // set halt request:
    dmcontrol.u32 = 0;
    dmcontrol.bits.dmactive = 1;
    dmcontrol.bits.haltreq = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    // dmstatus: allresumeack anyresumeack allhalted anyhalted authenticated hasresethaltreq version=2
    dmstatus.u32 = 0;
    do {
        dmstatus.u32 = read_dmi(IJtag::DMI_DMSTATUS);
    } while (dmstatus.bits.allhalted == 0 && watchdog++ < 1000);


    // clear halt request
    dmcontrol.u32 = 0;
    dmcontrol.bits.dmactive = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    abstractcs.u32 = read_dmi(IJtag::DMI_ABSTRACTCS);
    if (abstractcs.bits.cmderr) {
        clear_cmderr();
    } else if (dmstatus.bits.allhalted == 0) {
        generateError(res, "Cannot halt selected harts");
    }
}

}  // namespace debugger
