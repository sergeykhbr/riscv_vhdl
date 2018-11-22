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

#include "cmd_br_generic.h"
#include "debug/dsumap.h"

namespace debugger {

CmdBrGeneric::CmdBrGeneric(ITap *tap) : ICommand ("br", tap) {

    briefDescr_.make_string("Add or remove breakpoint.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Get breakpoints list or add/remove breakpoint with specified\n"
        "    flags.\n"
        "Response:\n"
        "    List of lists [[iii]*] if breakpoint list was requested, where:\n"
        "        i|s  - uint64_t address value or 'string' symbol name\n"
        "        i    - uint32_t instruction value\n"
        "        i    - uint64_t Breakpoint flags: hardware,...\n"
        "    Nil in a case of add/rm breakpoint\n"
        "Usage:\n"
        "    br"
        "    br add <addr>\n"
        "    br rm <addr>\n"
        "    br rm 'symbol_name'\n"
        "    br add <addr> hw\n"
        "Example:\n"
        "    br add 0x10000000\n"
        "    br add 0x00020040 hw\n"
        "    br add 'func1'\n"
        "    br rm 0x10000000\n"
        "    br rm 'func1'\n");

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    isrc_ = 0;
    if (lstServ.size() != 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc_ = static_cast<ISourceCode *>(
                            iserv->getInterface(IFACE_SOURCE_CODE));
    }
}

int CmdBrGeneric::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || (args->size() >= 3 && (*args)[1].is_string())) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdBrGeneric::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    if (!isrc_) {
        generateError(res, "ISource interface not found");
        return;
    }
    if (args->size() == 1) {
        isrc_->getBreakpointList(res);
        return;
    }

    uint64_t flags = 0;
    if (args->size() == 4 && (*args)[3].is_equal("hw")) {
        flags |= BreakFlag_HW;
    }

    DsuMapType *pdsu = DSUBASE();

    Reg64Type braddr;
    Reg64Type brinstr;
    AttributeType &bpadr = (*args)[2];
    if (bpadr.is_integer()) {
        braddr.val = bpadr.to_uint64();
    } else if (bpadr.is_string()) {
        if (isrc_->symbol2Address(bpadr.to_string(), &braddr.val) < 0) {
            generateError(res, "Symbol not found");
            return;
        }
    } else {
        generateError(res, "Wrong command format");
        return;
    }
    
    if ((*args)[1].is_equal("add")) {
        brinstr.val = 0;
        tap_->read(braddr.val, 4, brinstr.buf);

        isrc_->registerBreakpoint(braddr.val, flags, brinstr.val);
        if (isHardware(flags)) {
            uint64_t dsuaddr = 
                reinterpret_cast<uint64_t>(&pdsu->udbg.v.add_breakpoint);
            tap_->write(dsuaddr, 8, braddr.buf);
        } else {
            getSwBreakpointInstr(&brinstr);
            tap_->write(braddr.val, 4, brinstr.buf);
        }
        return;
    } 
    
    if ((*args)[1].is_equal("rm")) {
        isrc_->unregisterBreakpoint(braddr.val, &flags, &brinstr.val);
        if (isHardware(flags)) {
            uint64_t dsuaddr = 
                reinterpret_cast<uint64_t>(&pdsu->udbg.v.remove_breakpoint);
            tap_->write(dsuaddr, 8, braddr.buf);
        } else {
            tap_->write(braddr.val, 4, brinstr.buf);
        }
    }
}

}  // namespace debugger
