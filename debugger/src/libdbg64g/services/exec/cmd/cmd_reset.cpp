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

#include "cmd_reset.h"

namespace debugger {

CmdReset::CmdReset(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "reset", ijtag) {

    briefDescr_.make_string("Reset, Un-reset or Reboot target");
    detailedDescr_.make_string(
        "Description:\n"
        "    Reset, Un-reset or Reboot target.\n"
        "Warning:\n"
        "    When reset is HIGH CPU debug port is resetting also and cannot\n"
        "    response on debugger requests.\n"
        "Example:\n"
        "    reset\n"
        "    reset 1\n"
        "    reset 0\n");
}

int CmdReset::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() <= 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdReset::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    IJtag::dmi_dmcontrol_type dmcontrol;
    dmcontrol.u32 = 0;
    dmcontrol.bits.ndmreset = 1;

    if (args->size() == 2) {
        dmcontrol.bits.ndmreset = (*args)[1].to_uint64();       // [1] ndmreset
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
    } else {
        // Reboot
        dmcontrol.bits.ndmreset = 1;
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
        RISCV_sleep_ms(10);
        dmcontrol.bits.ndmreset = 0;
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
    }
}

}  // namespace debugger
