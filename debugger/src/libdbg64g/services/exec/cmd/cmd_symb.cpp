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
    bool sequence;
    const char *pf, *ps;
    while ((*filt) && (*symbname)) {
        if (filt[0] == '*') {
            filt++;
            pf = filt;
            sequence = false;
            if (pf[0] == '\0') {
                return true;
            }
            while (symbname[0] != '\0') {
                if (pf[0] == '\0') {
                    return true;
                } else if (pf[0] == '*') {
                    return filt_pass(pf, symbname);
                } else if (pf[0] == symbname[0]) {
                    pf++;
                    if (!sequence) {
                        sequence = true;
                        ps = symbname;
                    }
                } else if (sequence) {
                    sequence = false;
                    pf = filt;
                    symbname = ps;
                }
                symbname++;
            }
            if ((pf[0] != 0) && (pf[0] != '*') && (pf[0] != '?')) {
                sequence = false;
            }
            return sequence;
        } else if (filt[0] == '?' || symbname[0] == filt[0]) {
            filt++;
        } else if (symbname[0] != filt[0]) {
            return false;
        }

        symbname++;
    }
    return false;
}

}  // namespace debugger
