/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Browse symbols command.
 */

#include "iservice.h"
#include "cmd_symb.h"
#include "coreservices/ielfreader.h"

namespace debugger {

CmdSymb::CmdSymb(ITap *tap, ISocInfo *info) 
    : ICommand ("symb", tap, info) {

    briefDescr_.make_string("Get symbols list");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read symbols list. Command 'loadelf' must be applied first to\n"
        "    make available debug information.\n"
        "Usage:\n"
        "    symb filter\n"
        "Example:\n"
        "    symb\n"
        "    symb *main*\n");
}

bool CmdSymb::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal("symb") 
        && (args->size() == 1 || args->size() == 2)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdSymb::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    /**
     *  @todo Elf Loader service change on elf-reader
     */
    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_ELFREADER, &lstServ);
    if (lstServ.size() == 0) {
        generateError(res, "Elf-service not found");
        return;
    }
    IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
    IElfReader *elf = static_cast<IElfReader *>(
                        iserv->getInterface(IFACE_ELFREADER));

    elf->getSymbols(res);
}


}  // namespace debugger
