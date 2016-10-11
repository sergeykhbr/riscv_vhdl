/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Dump memory range into file.
 */

#include "cmd_memdump.h"
#include <string>

namespace debugger {

CmdMemDump::CmdMemDump(ITap *tap, ISocInfo *info) 
    : ICommand ("memdump", tap, info) {

    briefDescr_.make_string("Dump memory to file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Dump memory to file (default in Binary format).\n"
        "Usage:\n"
        "    memdump <addr> <bytes> [filepath] [bin|hex]\n"
        "Example:\n"
        "    memdump 0x0 8192 dump.bin\n"
        "    memdump 0x40000000 524288 dump.hex hex\n"
        "    memdump 0x10000000 128 \"c:/My Documents/dump.bin\"\n");
}

bool CmdMemDump::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
     && (args->size() == 4 || args->size() == 5)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdMemDump::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    const char *filename = (*args)[3].to_string();
    FILE *fd = fopen(filename, "w");
    if (fd == NULL) {
        char tst[256];
        RISCV_sprintf(tst, sizeof(tst), "Can't open '%s' file", filename);
        generateError(res, tst);
        return;
    }
    uint64_t addr = (*args)[1].to_uint64();
    int len = static_cast<int>((*args)[2].to_uint64());
    res->make_data(len);

    tap_->read(addr, len, res->data());
    uint8_t *dumpbuf = res->data();

    if (args->size() == 5 && (*args)[4].is_equal("hex")) {
        char t1[256];
        int t1_cnt = 0;
        for (int i = 0; i < len; i++) {
            t1_cnt += RISCV_sprintf(&t1[t1_cnt], sizeof(t1) - t1_cnt, "%02x",
                                    dumpbuf[(i & ~0xf) | (0xf - (i & 0xf))]);
            if ((i & 0xf) != 0xf) {
                continue;
            }
            t1_cnt += RISCV_sprintf(&t1[t1_cnt], sizeof(t1) - t1_cnt, "\n");
            fwrite(t1, t1_cnt, 1, fd);
            t1_cnt = 0;
        }
        if (len & 0xf) {
            fwrite(t1, t1_cnt, 1, fd);
        }
    } else {
        fwrite(dumpbuf, len, 1, fd);
    }
    fclose(fd);
}

}  // namespace debugger
