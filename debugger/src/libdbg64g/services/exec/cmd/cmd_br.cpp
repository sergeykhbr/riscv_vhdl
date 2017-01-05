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
        "    Add or remove memory breakpoint (software/hardware).\n"
        "Usage:\n"
        "    br add <addr>\n"
        "    br rm <addr>\n"
        "    br add <addr> hw\n"
        "    br rm <addr> hw\n"
        "Example:\n"
        "    br add 0x10000000\n"
        "    br add 0x00020040 hw\n"
        "    br rm 0x10000000\n");

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
        && (args->size() >= 3 && (*args)[1].is_string())) {
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
    uint64_t flags = 0;
    if (args->size() == 4 && (*args)[3].is_equal("hw")) {
        flags |= BreakFlag_HW;
    }

    Reg64Type instr;
    uint64_t addr = (*args)[2].to_uint64();
    tap_->read(addr, 4, instr.buf);
    isrc_->registerBreakpoint(addr, instr.buf32[0], flags);

    instr.buf32[0] = 0x00100073;   // EBREAK instruction

    if ((*args)[1].is_equal("add")) {
        tap_->write(addr, 4, instr.buf);
        return;
    } 
    
    if ((*args)[1].is_equal("rm")) {
        if (!isrc_->unregisterBreakpoint(addr, instr.buf32, &flags)) {
            tap_->write(addr, 4, instr.buf);
        }
    }
}

}  // namespace debugger
