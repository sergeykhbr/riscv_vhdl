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

#include "cmd_bt.h"
#include "iservice.h"
#include "coreservices/isrccode.h"

namespace debugger {

CmdStack::CmdStack(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "stack", ijtag) {

    briefDescr_.make_string("Read CPU Stack Trace buffer");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read CPU stack trace buffer. Buffer's size is defined\n"
        "    by RTL configuration parameter:\n"
        "Usage:\n"
        "    stack <depth>\n"
        "Example:\n"
        "    stack\n"
        "    stack 3\n");
}

int CmdStack::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1
       || (args->size() == 2 && (*args)[1].is_integer())) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdStack::exec(AttributeType *args, AttributeType *res) {
    Reg64Type stktr_cnt;
    uint32_t addr;
    res->make_list(0);

    get_reg("stktr_cnt", &stktr_cnt);

    addr = reg2addr("stktr_buf");
    unsigned trace_sz = stktr_cnt.buf32[0];
    if (args->size() == 2 && args[1].to_uint64() < trace_sz) {
        addr += 8 * (trace_sz - args[1].to_uint32());
        trace_sz = args[1].to_uint32();
    }

    if (trace_sz == 0) {
        return;
    }

    AttributeType lstServ;
    ISourceCode *isrc = 0;
    Reg64Type from_addr, to_addr;

    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    if (lstServ.size() > 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc = static_cast<ISourceCode *>(
                iserv->getInterface(IFACE_SOURCE_CODE));
    }

    res->make_list(stktr_cnt.buf32[0]);
    for (unsigned i = 0; i < trace_sz; i++) {
        AttributeType &item = (*res)[i];
        get_reg(addr, 8, &from_addr);
        get_reg(addr + 1, 8, &to_addr);
        // [from, ['symb_name',symb_offset], to, ['symb_name',symb_offset]]
        item.make_list(4);
        item[0u].make_uint64(from_addr.val);
        item[2].make_uint64(to_addr.val);
        if (isrc) {
            isrc->addressToSymbol(from_addr.val, &item[1]);
            isrc->addressToSymbol(to_addr.val, &item[3]);
        }
        addr += 2;
    }
}

}  // namespace debugger
