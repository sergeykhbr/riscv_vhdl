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

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_ELFREADER, &lstServ);
    if (lstServ.size() == 0) {
        generateError(res, "Elf-service not found");
        return;
    }
    IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
    IElfReader *elf = static_cast<IElfReader *>(
                        iserv->getInterface(IFACE_ELFREADER));

    if (args->size() == 2 && (*args)[1].is_string()) {
        AttributeType allSymb;
        elf->getSymbols(&allSymb);
        applyFilter((*args)[1].to_string(), &allSymb, res);
    } else {
        elf->getSymbols(res);
    }
}

void CmdSymb::applyFilter(const char *filt, AttributeType *in,
                          AttributeType *out) {
    const char *symbname;
    out->make_list(0);
    for (unsigned i = 0; i < in->size(); i++) {
        AttributeType &item = (*in)[i];
        symbname = item[Symbol_Name].to_string();
        if (filt_pass(filt, symbname)) {
            out->add_to_list(&item);
        }
    }
}

bool CmdSymb::filt_pass(const char *filt, const char *symbname) {
    while ((*filt) && (*symbname)) {
        if (filt[0] == '*') {
            filt++;
            if (filt[0] == '\0') {
                return true;
            }
            while (symbname[0] && symbname[0] != filt[0]) {
                symbname++;
            }
            if (symbname[0] != filt[0]) {
                return false;
            }
        } else if (filt[0] != '?' && symbname[0] != filt[0]) {
            return false;
        } else {
            filt++;
            symbname++;
        }
    }
    if (symbname[0] == 0 && 
        (filt[0] == 0 || (filt[0] == '*' && filt[1] == 0))) {
        return true;
    }
    return false;
}

}  // namespace debugger
