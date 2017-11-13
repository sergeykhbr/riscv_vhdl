/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Add or remove memory breakpoint.
 */

#include "cmd_br.h"

namespace debugger {

CmdBr::CmdBr(ITap *tap, ISocInfo *info) 
    : ICommand ("br", tap, info) {

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

bool CmdBr::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
        && (args->size() == 1 || (args->size() >= 3 && (*args)[1].is_string()))
        ) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdBr::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isrc_ || !isValid(args)) {
        generateError(res, "Wrong argument list");
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

    DsuMapType *pdsu = info_->getpDsu();

    Reg64Type braddr;
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
        uint64_t dsuaddr = 
            reinterpret_cast<uint64_t>(&pdsu->udbg.v.add_breakpoint);
        isrc_->registerBreakpoint(braddr.val, flags);
        tap_->write(dsuaddr, 8, braddr.buf);
        return;
    } 
    
    if ((*args)[1].is_equal("rm")) {
        uint64_t dsuaddr = 
            reinterpret_cast<uint64_t>(&pdsu->udbg.v.remove_breakpoint);
        isrc_->unregisterBreakpoint(braddr.val, &flags);
        tap_->write(dsuaddr, 8, braddr.buf);
    }
}

}  // namespace debugger
