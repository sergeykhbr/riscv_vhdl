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

#include "iservice.h"
#include "cmd_loadelf.h"
#include "coreservices/ielfreader.h"

namespace debugger {

CmdLoadElf::CmdLoadElf(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "loadelf", ijtag) {

    briefDescr_.make_string("Load ELF-file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load ELF-file to SOC target memory. Optional key 'nocode'\n"
        "    allows to read debug information from the elf-file without\n"
        "    target programming.\n"
        "Usage:\n"
        "    loadelf filename [nocode]\n"
        "Example:\n"
        "    loadelf /home/riscv/image.elf\n"
        "    loadelf /home/riscv/image.elf nocode\n");
}

int CmdLoadElf::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 2 
        || (args->size() == 3 && (*args)[2].is_equal("nocode"))) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdLoadElf::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    bool program = true;
    if (args->size() == 3 
        && (*args)[2].is_string() && (*args)[2].is_equal("nocode")) {
        program = false;
    }

    /**
     *  @todo Elf Loader service change on elf-reader
     */
    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_ELFREADER, &lstServ);
    if (lstServ.size() == 0) {
        generateError(res, "Elf-service not found");
        return;
    }

    IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
    IElfReader *elf = static_cast<IElfReader *>(
                        iserv->getInterface(IFACE_ELFREADER));
    elf->readFile((*args)[1].to_string());

    if (!program) {
        return;
    }

    IJtag::dmi_dmcontrol_type dmcontrol;
    dmcontrol.u32 = 0;
    dmcontrol.bits.ndmreset = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    uint64_t sec_addr;
    int sec_sz;
    for (unsigned i = 0; i < elf->loadableSectionTotal(); i++) {
        sec_addr = elf->sectionAddress(i);
        sec_sz = static_cast<int>(elf->sectionSize(i));
        write_memory(sec_addr, sec_sz, elf->sectionData(i));
    }

    //soft_reset = 0;
    //tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&soft_reset));
}


}  // namespace debugger
