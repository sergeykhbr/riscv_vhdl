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

#include "cmd_stack.h"
#include "iservice.h"
#include "coreservices/isrccode.h"

namespace debugger {

CmdStack::CmdStack(ITap *tap, ISocInfo *info) 
    : ICommand ("stack", tap, info) {

    briefDescr_.make_string("Read CPU Stack Trace buffer");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read CPU stack trace buffer. Buffer's size is defined\n"
        "    by VHDL configuration parameter:\n"
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
    res->make_list(0);

    Reg64Type t1;
    DsuMapType *pdsu = info_->getpDsu();
    uint64_t addr = reinterpret_cast<uint64_t>(&pdsu->ureg.v.stack_trace_cnt);
    t1.val = 0;
    tap_->read(addr, 8, t1.buf);

    addr = reinterpret_cast<uint64_t>(&pdsu->ureg.v.stack_trace_buf[0]);
    unsigned trace_sz = t1.buf32[0];
    if (args->size() == 2 && args[1].to_uint64() < trace_sz) {
        addr += 8 * (trace_sz - args[1].to_uint32());
        trace_sz = args[1].to_uint32();
    }

    if (trace_sz == 0) {
        return;
    }

    AttributeType tbuf, lstServ;
    uint64_t *p_data;
    ISourceCode *isrc = 0;
    uint64_t from_addr, to_addr;
    tbuf.make_data(16*trace_sz);
    tap_->read(addr, tbuf.size(), tbuf.data());

    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    if (lstServ.size() > 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc = static_cast<ISourceCode *>(
                iserv->getInterface(IFACE_SOURCE_CODE));
    }

    res->make_list(t1.buf32[0]);
    p_data = reinterpret_cast<uint64_t *>(tbuf.data());
    for (unsigned i = 0; i < trace_sz; i++) {
        AttributeType &item = (*res)[i];
        from_addr = p_data[2*(trace_sz - i) - 2];
        to_addr = p_data[2*(trace_sz - i) - 1];
        // [from, ['symb_name',symb_offset], to, ['symb_name',symb_offset]]
        item.make_list(4);
        item[0u].make_uint64(from_addr);
        item[2].make_uint64(to_addr);
        if (isrc) {
            isrc->addressToSymbol(from_addr, &item[1]);
            isrc->addressToSymbol(to_addr, &item[3]);
        }
    }
}

}  // namespace debugger
