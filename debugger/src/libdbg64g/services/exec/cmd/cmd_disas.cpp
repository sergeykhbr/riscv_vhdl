/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassemble data block command.
 */

#include <string>
#include "cmd_disas.h"

namespace debugger {

CmdDisas::CmdDisas(ITap *tap, ISocInfo *info) 
    : ICommand ("disas", tap, info) {

    briefDescr_.make_string("Disassemble block of data.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Disassemble memory range or a block of data. Convert result\n"
        "    to a list of assembler items where each of them has\n"
        "    ASM_Total sub-items. You can add 'str' flag to receive\n"
        "    formatted text output.\n"
        "Usage:\n"
        "    disas <addr> <byte> [str]\n"
        "    disas <addr> <data> [str]\n"
        "Example:\n"
        "    data 0x40000000 100\n"
        "    data 0x40000000 100 str\n"
        "    data 0x1000 (73,00,10,00)\n"
        "    ['data',0x1000,(73,00,10,00)]\n");

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
    AttributeType *mem_data, *asm_data;
    AttributeType membuf, asmbuf;
    if ((*args)[2].is_integer()) {
        // 4-bytes alignment
        uint32_t sz = ((*args)[2].to_uint32() + 3) & ~0x3;
        membuf.make_data(sz);
        mem_data = &membuf;
        if (tap_->read(addr, sz, membuf.data()) == -1) {
            return;
        }
    } else {
        mem_data = &(*args)[2];
    }

    asm_data = res;
    bool do_format = false;
    if (args->size() == 4 && (*args)[3].is_equal("str")) {
        asm_data = &asmbuf;
        do_format = true;
    }

    isrc_->disasm(addr, mem_data, asm_data);

    if (do_format) {
        format(asm_data, res);
    }
}

void CmdDisas::format(AttributeType *asmbuf, AttributeType *fmtstr) {
    char tstr[128];
    std::string tout;
    for (unsigned i = 0; i < asmbuf->size(); i++) {
        const AttributeType &line = (*asmbuf)[i];
        if (line[ASM_list_type].to_int() != AsmList_disasm) {
            continue;
        }
        RISCV_sprintf(tstr, sizeof(tstr), "%016" RV_PRI64 "x: %08x    %s\n",
                line[ASM_addrline].to_uint64(),
                line[ASM_code].to_uint32(),
                line[ASM_mnemonic].to_string()
                );
        tout += tstr;
    }
    fmtstr->make_string(tout.c_str());
}

}  // namespace debugger
