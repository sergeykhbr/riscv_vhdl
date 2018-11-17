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

#include "cmd_write.h"

namespace debugger {

CmdWrite::CmdWrite(ITap *tap, ISocInfo *info) 
    : ICommand ("write", tap, info) {

    briefDescr_.make_string("Write memory");
    detailedDescr_.make_string(
        "Description:\n"
        "    Write memory.\n"
        "Usage:\n"
        "    write <addr> <bytes> [value 64bits]\n"
        "Example:\n"
        "    write 0xfffff004 4 0x20160323\n"
        "    write 0x10040000 16 [0xaabbccdd00112233, 0xaabbccdd00112233]\n");
}

bool CmdWrite::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() == 4) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdWrite::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    uint64_t addr = (*args)[1].to_uint64();
    uint64_t val = (*args)[3].to_uint64();
    unsigned bytes = static_cast<unsigned>((*args)[2].to_uint64());

    /** each value 8-bytes (64 bits) add 8 for bullet proofness: */
    if (wrData_.size() < (bytes + 8)) {
        wrData_.make_data(bytes + 8);
    }

    if ((*args)[3].is_integer()) {
        reinterpret_cast<uint64_t *>(wrData_.data())[0] = val;
    } else if ((*args)[3].is_list()) {
        const AttributeType &lst = (*args)[3];
        uint64_t *tmpbuf = reinterpret_cast<uint64_t *>(wrData_.data());
        for (unsigned i = 0; i < lst.size(); i++) {
            val = lst[i].to_uint64();
            tmpbuf[i] = val;
        }
    } else {
        generateError(res, "Write value must be i or [i*]");
        return;
    }
    tap_->write(addr, bytes, wrData_.data());
}

}  // namespace debugger
