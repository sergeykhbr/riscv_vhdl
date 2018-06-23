/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Binary-file loader command.
 */

#include "iservice.h"
#include "cmd_loadbin.h"
#include <iostream>

namespace debugger {

CmdLoadBin::CmdLoadBin(ITap *tap, ISocInfo *info) 
    : ICommand ("loadbin", tap, info) {

    briefDescr_.make_string("Load binary file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load BIN-file to SOC target memory with specified address.\n"
        "Example:\n"
        "    loadsrec /home/hc08/image.bin 0x04000\n");
}

bool CmdLoadBin::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal("loadbin") 
        && args->size() == 3) {
        return CMD_VALID;
    }
    return CMD_INVALID;
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
    tap_->write(addr, sz, image);
    delete [] image;
}

}  // namespace debugger
