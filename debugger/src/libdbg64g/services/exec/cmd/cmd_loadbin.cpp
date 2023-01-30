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
#include "cmd_loadbin.h"
#include <iostream>

namespace debugger {

CmdLoadBin::CmdLoadBin(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "loadbin", ijtag) {

    briefDescr_.make_string("Load binary file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load BIN-file to SOC target memory with specified address.\n"
        "Example:\n"
        "    loadsrec /home/hc08/image.bin 0x04000\n");
}

int CmdLoadBin::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 3) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdLoadBin::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    const char *filename = (*args)[1].to_string();
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        generateError(res, "File not found");
        return;
    }
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    uint8_t *image = new uint8_t[sz];
    fread(image, 1, sz, fp);
    fclose(fp);

    uint64_t addr = (*args)[2].to_uint64();
    write_memory(addr, sz, image);
    delete [] image;
}

}  // namespace debugger
