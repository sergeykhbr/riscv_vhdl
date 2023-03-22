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

#include "iservice.h"
#include "cmd_symb.h"
#include "coreservices/isrccode.h"

namespace debugger {

CmdSymb::CmdSymb(IService *parent)
    : ICommand(parent, "symb") {

    briefDescr_.make_string("Get symbols list");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read symbols list. Command 'loadelf' or 'loadmap' must be "
        "    applied first to make available debug information.\n"
        "Usage:\n"
        "    symb filter\n"
        "Example:\n"
        "    symb\n"
        "    symb *main*\n");
}

int CmdSymb::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdSymb::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    if (lstServ.size() == 0) {
        generateError(res, "SourceCode service not found");
        return;
    }
    IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
    ISourceCode *isrc = static_cast<ISourceCode *>(
                        iserv->getInterface(IFACE_SOURCE_CODE));

    if (args->size() == 2 && (*args)[1].is_string()) {
        AttributeType allSymb;
        isrc->getSymbols(&allSymb);
        applyFilter((*args)[1].to_string(), &allSymb, res);
    } else {
        isrc->getSymbols(res);
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
