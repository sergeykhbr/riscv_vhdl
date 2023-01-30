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

#include "cmd_init.h"

namespace debugger {

CmdInit::CmdInit(IService *parent, IJtag *ijtag) 
    : ICommandRiscv(parent, "init", ijtag) {

    briefDescr_.make_string("Read platform configuration and capabilities");
    detailedDescr_.make_string(
        "Description:\n"
        "    See RISC-V Debug specification to clarify OpenOCD initialization sequence\n"
        "Output format:\n"
        "   JSON dictionary with the read information {'codeid':0x11223344,....}\n"
        "Example:\n"
        "    init\n");

}

int CmdInit::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() != 1) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdInit::exec(AttributeType *args, AttributeType *res) {
    IJtag::DtmcsType dtmcs;
    IJtag::dmi_abstractcs_type abstractcs;
    IJtag::dmi_dmcontrol_type dmcontrol;
    uint32_t hartsnum;
    res->make_dict();
    ijtag_->resetAsync();
    ijtag_->resetSync();
    (*res)["idcode"].make_uint64(ijtag_->scanIdCode());

    dtmcs = ijtag_->scanDtmcs();
    if (dtmcs.bits.version == 0) {
        (*res)["version"].make_string("0.11");
    } else if (dtmcs.bits.version == 1) {
        (*res)["version"].make_string("0.13 and 1.0");
    } else {
        (*res)["version"].make_string("unknown");
    }
    (*res)["abits"].make_uint64(dtmcs.bits.abits);


    // Reset TAP setting dmactive to 0 -> 1
    dmcontrol.u32 = 0;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
    dmcontrol.bits.dmactive = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    // Read Prog Buffer size and number of Abstract Data registers
    abstractcs.u32 = read_dmi(IJtag::DMI_ABSTRACTCS);
    (*res)["datacount"].make_uint64(abstractcs.bits.datacount);
    (*res)["progbufsize"].make_uint64(abstractcs.bits.progbufsize);

    // Read number of Harts:
    dmcontrol.bits.hartsello = 0x3FF;
    dmcontrol.bits.hartselhi = 0x3FF;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
    dmcontrol.u32 = read_dmi(IJtag::DMI_DMCONTROL);
    hartsnum = dmcontrol.bits.hartsello + 1;
    (*res)["hartsnum"].make_uint64(hartsnum);
    (*res)["hart"].make_list(hartsnum);

    // Select hart:
    for (unsigned i = 0; i < hartsnum; i++) {
        AttributeType &hartinfo = (*res)["hart"][i];
        hartinfo.make_dict();
        dmcontrol.bits.hartsello = i;
        dmcontrol.bits.hartselhi = i >> 10;
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
        hartinfo["type"].make_string("rv64");
        hartinfo["misa"].make_uint64(0x10110d);
    }

    // Select hart0:
    dmcontrol.bits.hartsello = 0x0;
    dmcontrol.bits.hartselhi = 0x0;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
}

}  // namespace debugger
