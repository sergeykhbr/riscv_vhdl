/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassemble data block command.
 */

#include "cmd_disas.h"

namespace debugger {

CmdDisas::CmdDisas(ITap *tap, ISocInfo *info) 
    : ICommand ("disas", tap, info) {

    briefDescr_.make_string("Disassemble block of data.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Disassemble memory range or a block of data. Convert result\n"
        "    to a list of assembler items where each of them has\n"
        "    the following format:\n"
        "Response:\n"
        "    List of lists [[iii]*] if breakpoint list was requested, where:\n"
        "        i    - uint64_t address value\n"
        "        i    - uint32_t instruction value\n"
        "        i    - uint64_t Breakpoint flags: hardware,...\n"
        "    Nil in a case of add/rm breakpoint\n"
        "Usage:\n"
        "    disas <addr> <byte>\n"
        "    disas <addr> <data>\n"
        "Example:\n"
        "    data 0x40000000 100\n"
        "    data 0x1000 (0x73,0x00,0x10,0x00)\n"
        "    ['data',0x1000,(0x73,0x00,0x10,0x00)]\n");

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    isrc_ = 0;
    if (lstServ.size() != 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc_ = static_cast<ISourceCode *>(
                            iserv->getInterface(IFACE_SOURCE_CODE));
    }
}

bool CmdDisas::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
        && args->size() >= 3 && (*args)[1].is_integer()
        && ((*args)[2].is_data() || (*args)[2].is_integer())
        ) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdDisas::exec(AttributeType *args, AttributeType *res) {
    if (!isrc_ || !isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }
    res->make_list(0);

    uint64_t addr = (*args)[1].to_uint64();
    AttributeType *data_ibuf;
    AttributeType tbuf;
    if ((*args)[2].is_integer()) {
        uint32_t sz = (*args)[2].to_uint32();
        tbuf.make_data(sz);
        data_ibuf = &tbuf;
        tap_->read(addr, sz, tbuf.data());
    } else {
        data_ibuf = &(*args)[2];
    }

    isrc_->getBreakpointList(res);

    unsigned off = 0;
    AttributeType asm_item;
    asm_item.make_list(ASM_Total);
    while (off < data_ibuf->size()) {
        off += isrc_->disasm(addr, data_ibuf->data(), off,
                &asm_item[ASM_mnemonic], &asm_item[ASM_comment]);
        res->add_to_list(&asm_item);
    }

}

}  // namespace debugger
