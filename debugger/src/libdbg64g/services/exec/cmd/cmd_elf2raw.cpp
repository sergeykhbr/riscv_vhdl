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
#include "cmd_elf2raw.h"
#include "coreservices/ielfreader.h"
#include <iostream>

namespace debugger {

CmdElf2Raw::CmdElf2Raw(IService *parent)
    : ICommand(parent, "elf2raw") {

    briefDescr_.make_string("Load ELF-file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Create Raw-image file from the Elf-file\n"
        "Usage:\n"
        "    elf2raw elf-file raw-file [Size KB]\n"
        "Example:\n"
        "    elf2raw /home/riscv/image.elf /home/riscv/image.raw\n"
        "    elf2raw /home/riscv/image.elf /home/riscv/image.raw 128\n");
}

int CmdElf2Raw::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 3 || args->size() == 4) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdElf2Raw::exec(AttributeType *args, AttributeType *res) {
    AttributeType imageSize;
    res->attr_free();
    res->make_nil();

    imageSize.make_int64(1024 * 128);
    if (args->size() == 4) {
        imageSize.make_int64(1024 * (*args)[3].to_int());
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


    uint8_t *image = new uint8_t[imageSize.to_int()];
    uint64_t waddr;
    size_t wsz;
    for (unsigned i = 0; i < elf->loadableSectionTotal(); i++) {
        waddr = elf->sectionAddress(i);
        wsz = static_cast<size_t>(elf->sectionSize(i));
        if ((waddr + wsz) >= imageSize.to_uint32()) {
            continue;
        }
        memcpy(&image[waddr], elf->sectionData(i), wsz);
    }
    FILE *fp = fopen((*args)[2].to_string(), "w");
    fwrite(image, 1, imageSize.to_int(), fp);
    fclose(fp);
    delete [] image;
}

}  // namespace debugger
