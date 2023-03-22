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

#include "srcproc.h"
#include <iostream>
#include <riscv-isa.h>
#include "coreservices/icpuriscv.h"

namespace debugger {

extern int disasm_riscv(uint64_t pc,
                        uint8_t *data,
                        int offset,
                        AttributeType *mnemonic,
                        AttributeType *comment);

/** Class registration in the Core */
enum ESymbInfo {
    SymbInfo_Name,
    SymbInfo_Address,
    SymbInfo_Total,
};


RiscvSourceService::RiscvSourceService(const char *name) : IService(name),
    IHap(HAP_All) {
    registerInterface(static_cast<ISourceCode *>(this));
    registerAttribute("CmdExecutor", &cmdexec_);

    pcmdBr_ = new CmdBrRiscv(this);


    brList_.make_list(0);
    symbolListSortByName_.make_list(0);
    symbolListSortByAddr_.make_list(0);
}

RiscvSourceService::~RiscvSourceService() {
    delete pcmdBr_;
}

void RiscvSourceService::postinitService() {
    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        icmdexec_->registerCommand(pcmdBr_);
    }
}

void RiscvSourceService::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(pcmdBr_);
    }
}

void RiscvSourceService::hapTriggered(EHapType type,
                                      uint64_t param,
                                      const char *descr) {
    if (type == HAP_Resume) {
        // TODO: set software breakpoints into memory
    } else if (type == HAP_Halt) {
        // TODO: remove breakpoint from memory
    }
}

void RiscvSourceService::addFileSymbol(const char *name, uint64_t addr,
                                       int sz) {
    AttributeType symb(Attr_List);
    symb.make_list(Symbol_Total);
    symb[Symbol_Name].make_string(name);
    symb[Symbol_Addr].make_uint64(addr);
    symb[Symbol_Size].make_int64(sz);

    symbolListSortByName_.add_to_list(&symb);
    symbolListSortByName_.sort(Symbol_Name);

    symbolListSortByAddr_.add_to_list(&symb);
    symbolListSortByAddr_.sort(Symbol_Addr);
}

void RiscvSourceService::addFunctionSymbol(const char *name,
                                      uint64_t addr, int sz) {
    addFileSymbol(name, addr, sz);
}

void RiscvSourceService::addDataSymbol(const char *name, uint64_t addr,
                                       int sz) {
    addFileSymbol(name, addr, sz);
}

void RiscvSourceService::clearSymbols() {
    symbolListSortByName_.make_list(0);
    symbolListSortByAddr_.make_list(0);
}

void RiscvSourceService::addSymbols(AttributeType *list) {
    for (unsigned i = 0; i < list->size(); i++) {
        AttributeType &item = (*list)[i];
        symbolListSortByName_.add_to_list(&item);
        symbolListSortByAddr_.add_to_list(&item);
    }
    symbolListSortByName_.sort(Symbol_Name);
    symbolListSortByAddr_.sort(Symbol_Addr);
}

void RiscvSourceService::addressToSymbol(uint64_t addr, AttributeType *info) {
    uint64_t sadr, send;
    int sz = static_cast<int>(symbolListSortByAddr_.size());

    info->make_list(SymbInfo_Total);
    (*info)[SymbInfo_Name].make_string("");
    (*info)[SymbInfo_Address].make_uint64(0);
    if (sz == 0) {
        return;
    }
    sadr = symbolListSortByAddr_[0u][Symbol_Addr].to_uint64();
    if (addr < sadr) {
        return;
    }

    bool search = true;
    int dist, pos = sz / 2;
    dist = pos;
    while (search) {
        AttributeType &symb = symbolListSortByAddr_[pos];
        sadr = symb[Symbol_Addr].to_uint64();
        if (pos < static_cast<int>(symbolListSortByAddr_.size()) - 1) {
            send = symbolListSortByAddr_[pos + 1][Symbol_Addr].to_uint64();
        } else {
            send = sadr + symb[Symbol_Size].to_uint64();
        }
        if (sadr <= addr && addr < send) {
            (*info)[SymbInfo_Name] = symb[Symbol_Name];
            (*info)[SymbInfo_Address].make_uint64(addr - sadr);
            return;
        }
        
        if (addr < sadr) {
            if (dist == 0 || pos == 0) {
                search = false;
            } else if (dist == 1) {
                dist = 0;
                pos--;
            } else {
                int incr = dist / 2;
                pos -= incr;
                dist = (dist / 2) + (dist & 0x1);
                if (pos < 0) {
                    pos = 0;
                }
            }
        } else {
            if (dist == 0 || pos == (sz - 1)) {
                search = false;
            } else if (dist == 1) {
                dist = 0;
                pos++;
            } else {
                int incr = dist / 2;
                pos += incr;
                dist = (dist / 2) + (dist & 0x1);
                if (pos >= sz) {
                    pos = sz - 1;
                }
            }
        }
    }
}

int RiscvSourceService::symbol2Address(const char *name, uint64_t *addr) {
    for (unsigned i = 0; i < symbolListSortByName_.size(); i++) {
        AttributeType &item = symbolListSortByName_[i];
        if (item[Symbol_Name].is_equal(name)) {
            *addr = item[Symbol_Addr].to_uint64();
            return 0;
        }
    }
    return -1;
}

void RiscvSourceService::addBreakpoint(uint64_t addr, uint64_t flags) {
    AttributeType item;
    item.make_list(BrkList_Total);
    item[BrkList_address].make_uint64(addr);
    item[BrkList_flags].make_uint64(flags);
    //item[BrkList_instr].make_uint64(instr);
    //item[BrkList_opcode].make_uint64(opcode);
    //item[BrkList_oplen].make_int64(oplen);

    bool not_found = true;
    for (unsigned i = 0; i < brList_.size(); i++) {
        AttributeType &br = brList_[i];
        if (addr == br[BrkList_address].to_uint64()) {
            not_found = false;
        }
    }
    if (not_found) {
        brList_.add_to_list(&item);
    }
}

int RiscvSourceService::removeBreakpoint(uint64_t addr) {
    for (unsigned i = 0; i < brList_.size(); i++) {
        AttributeType &br = brList_[i];
        if (addr == br[BrkList_address].to_uint64()) {
            brList_.remove_from_list(i);
            return 0;
        }
    }
    return 1;
}

void RiscvSourceService::getBreakpointList(AttributeType *list) {
    list->clone(&brList_);
}

bool RiscvSourceService::isBreakpoint(uint64_t addr) {
    for (unsigned i = 0; i < brList_.size(); i++) {
        uint64_t bradr = brList_[i][BrkList_address].to_uint64();
        if (addr == bradr) {
            return true;
        }
    }
    return false;
}

void RiscvSourceService::disasm(int mode,
                                uint64_t pc,
                                AttributeType *idata,
                                AttributeType *asmlist) {
    asmlist->make_list(0);
    if (!idata->is_data()) {
        return;
    }
    uint8_t *data = idata->data();

    AttributeType asm_item, symb_item, info;
    asm_item.make_list(ASM_Total);
    symb_item.make_list(3);
    asm_item[ASM_list_type].make_int64(AsmList_disasm);
    symb_item[ASM_list_type].make_int64(AsmList_symbol);
    uint64_t off = 0;
    Reg64Type code;
    int codesz;

    while (static_cast<unsigned>(off) < idata->size()) {
        code.val = *reinterpret_cast<uint32_t*>(&data[off]);

        addressToSymbol(pc + off, &info);
        if (info[SymbInfo_Name].size() != 0 && 
            info[SymbInfo_Address].to_int() == 0) {
            symb_item[1].make_uint64(pc + off);
            symb_item[2].make_string(info[SymbInfo_Name].to_string());
            asmlist->add_to_list(&symb_item);
        }
        asm_item[ASM_addrline].make_uint64(pc + off);
        asm_item[ASM_breakpoint].make_boolean(false);
        asm_item[ASM_label].make_string("");

        if (isBreakpoint(pc + off)) {
            asm_item[ASM_breakpoint].make_boolean(true);
        }
        codesz = disasm_riscv(pc + off,
                        code.buf,
                        0,
                        &asm_item[ASM_mnemonic],
                        &asm_item[ASM_comment]);

#if 1
        uint64_t swap = code.val;
        if (codesz == 2) {
            swap = code.buf16[0];
        }
#else
        uint64_t swap = 0;
        for (int i = 0; i < codesz; i++) {
            swap = (swap << 8) | code.buf[i];
        }
#endif
        asm_item[ASM_code].make_uint64(swap);
        asm_item[ASM_codesize].make_uint64(codesz);
        asmlist->add_to_list(&asm_item);
        off += codesz;
    }
}

}  // namespace debugger
