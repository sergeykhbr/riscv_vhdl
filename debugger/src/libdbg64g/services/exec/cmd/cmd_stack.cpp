/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read CPU stack trace buffer.
 */

#include "cmd_stack.h"
#include "iservice.h"
#include "coreservices/ielfreader.h"

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

bool CmdStack::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && (args->size() == 1
       || (args->size() == 2 && (*args)[1].is_integer()))) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdStack::exec(AttributeType *args, AttributeType *res) {
    res->make_list(0);
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

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
    IElfReader *elf = 0;
    uint64_t from_addr, to_addr;
    tbuf.make_data(16*trace_sz);
    tap_->read(addr, tbuf.size(), tbuf.data());

    RISCV_get_services_with_iface(IFACE_ELFREADER, &lstServ);
    if (lstServ.size() >= 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        elf = static_cast<IElfReader *>(iserv->getInterface(IFACE_ELFREADER));
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
        if (elf) {
            elf->addressToSymbol(from_addr, &item[1]);
            elf->addressToSymbol(to_addr, &item[3]);
        }
    }
}

}  // namespace debugger
