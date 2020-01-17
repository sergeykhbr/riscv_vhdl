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

#include "../arm-isa.h"
#include "srcproc.h"
#include "thumb_disasm.h"
#include <iostream>

namespace debugger {

/** Class registration in the Core */
enum ESymbInfo {
    SymbInfo_Name,
    SymbInfo_Address,
    SymbInfo_Total,
};

const char *const *RN = IREGS_NAMES;


ArmSourceService::ArmSourceService(const char *name) : IService(name) {
    registerInterface(static_cast<ISourceCode *>(this));
    registerAttribute("CPU", &cpu_);
    registerAttribute("Endianess", &endianess_);

    brList_.make_list(0);
    symbolListSortByName_.make_list(0);
    symbolListSortByAddr_.make_list(0);
}

ArmSourceService::~ArmSourceService() {
}

void ArmSourceService::postinitService() {
    iarm_ = static_cast<ICpuArm *>(
        RISCV_get_service_iface(cpu_.to_string(), IFACE_CPU_ARM));
    if (!iarm_) {
        RISCV_error("Can't get ICpuArm interface in %s", cpu_.to_string());
    }
}

void ArmSourceService::addFileSymbol(const char *name, uint64_t addr,
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

void ArmSourceService::addFunctionSymbol(const char *name,
                                      uint64_t addr, int sz) {
    addFileSymbol(name, addr, sz);
}

void ArmSourceService::addDataSymbol(const char *name, uint64_t addr,
                                       int sz) {
    addFileSymbol(name, addr, sz);
}

void ArmSourceService::clearSymbols() {
    symbolListSortByName_.make_list(0);
    symbolListSortByAddr_.make_list(0);
}

void ArmSourceService::addSymbols(AttributeType *list) {
    for (unsigned i = 0; i < list->size(); i++) {
        AttributeType &item = (*list)[i];
        symbolListSortByName_.add_to_list(&item);
        symbolListSortByAddr_.add_to_list(&item);
    }
    symbolListSortByName_.sort(Symbol_Name);
    symbolListSortByAddr_.sort(Symbol_Addr);
}

void ArmSourceService::addressToSymbol(uint64_t addr, AttributeType *info) {
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

int ArmSourceService::symbol2Address(const char *name, uint64_t *addr) {
    for (unsigned i = 0; i < symbolListSortByName_.size(); i++) {
        AttributeType &item = symbolListSortByName_[i];
        if (item[Symbol_Name].is_equal(name)) {
            *addr = item[Symbol_Addr].to_uint64();
            return 0;
        }
    }
    return -1;
}

void ArmSourceService::registerBreakpoint(uint64_t addr, uint64_t flags,
                                       uint64_t instr) {
    AttributeType item;
    item.make_list(BrkList_Total);
    item[BrkList_address].make_uint64(addr);
    item[BrkList_flags].make_uint64(flags);
    item[BrkList_instr].make_uint64(instr);

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

int ArmSourceService::unregisterBreakpoint(uint64_t addr, uint64_t *flags,
                                       uint64_t *instr) {
    for (unsigned i = 0; i < brList_.size(); i++) {
        AttributeType &br = brList_[i];
        if (addr == br[BrkList_address].to_uint64()) {
            *flags = br[BrkList_flags].to_uint64();
            *instr = br[BrkList_instr].to_uint64();
            brList_.remove_from_list(i);
            return 0;
        }
    }
    return 1;
}

void ArmSourceService::getBreakpointList(AttributeType *list) {
    if (!list->is_list() || list->size() != brList_.size()) {
        list->make_list(brList_.size());
    }

    for (unsigned i = 0; i < brList_.size(); i++) {
        AttributeType &item = (*list)[i];
        AttributeType &br = brList_[i];
        if (!item.is_list() || item.size() != 3) {
            item.make_list(BrkList_Total);
        }
        item[BrkList_address] = br[BrkList_address];
        item[BrkList_flags] = br[BrkList_flags];
        item[BrkList_instr] = br[BrkList_instr];
    }
}

bool ArmSourceService::isBreakpoint(uint64_t addr, AttributeType *outbr) {
    for (unsigned i = 0; i < brList_.size(); i++) {
        uint64_t bradr = brList_[i][BrkList_address].to_uint64();
        if (addr == bradr) {
            *outbr = brList_[i];
            return true;
        }
    }
    return false;
}

int ArmSourceService::disasm(uint64_t pc,
                       uint8_t *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment) {
    uint32_t instr;
    memcpy(&instr, &data[offset], 4);
    if (iarm_->getInstrMode() == THUMB_mode) {
        char tstr[1024];
        int ret = disasm_thumb(pc, instr, tstr, sizeof(tstr));
        mnemonic->make_string(tstr);
        comment->make_string("");
        return ret;
    }


    if (instr == 0xFEDEFFE7) {
        mnemonic->make_string("und");
        comment->make_string("");
        return 4;
    } else if ((instr & 0x0FB00000) == 0x03000000) {
        return parseUndefinedInstruction(pc, instr, mnemonic, comment);
    } else if ((instr & 0x0FD0F0F0) == 0x0710f010) {
        // sdiv ****0111_0001****_1111****_0001****
        // udiv ****0111_0011****_1111****_0001****
        return parseDivide(pc, instr, mnemonic, comment);
    } else if ((instr & 0x0F0000F0) == 0x90
        && (((instr >> 22) & 0x3F) == 0 || ((instr >> 23) & 0x1F) == 1)) {
        // mla  ****0000_001*****_********_1001****
        // mls  ****0000_0110****_********_1001****
        // mul  ****0000_000*****_0000****_1001****
        return parseMultiply(pc, instr, mnemonic, comment);
    } else if (((instr >> 4) & 0x00FFFFFF) == 0x12FFF1) {
        return parseBranchExchange(pc, instr, mnemonic, comment);
    } else if (((instr >> 4) & 0x00FFFFFF) == 0x12FFF3) {
        return parseBranchLinkExchange(pc, instr, mnemonic, comment);
    } else if ((instr & 0x0F0000F0) == 0x06000070) {
        // ARM V6 operation: uxtab, uxtb, uxah, uxth..
        return parseBytesExtending(pc, instr, mnemonic, comment);
    } else if (((instr >> 26) & 0x3) == 0x1) {
        return parseSingleDataTransfer(pc, instr, mnemonic, comment);
    } else if (((instr >> 25) & 0x7) == 0x4) {
        return parseBlockDataTransfer(pc, instr, mnemonic, comment);
    } else if (((instr >> 24) & 0xF) == 0xE && (instr & 0x10)) {
        return parseCoprocRegTransfer(pc, instr, mnemonic, comment);
    } else if (((instr >> 26) & 0x3) == 0x0) {
        return parseDataProcessing(pc, instr, mnemonic, comment);
    } else if (((instr >> 25) & 0x7) == 0x5) {
        return parseBranch(pc, instr, mnemonic, comment);
    }
    mnemonic->make_string("unimpl");
    comment->make_string("");
    return 4;
}

void ArmSourceService::disasm(uint64_t pc,
                          AttributeType *idata,
                          AttributeType *asmlist) {
    asmlist->make_list(0);
    if (!idata->is_data()) {
        return;
    }
    uint8_t *data = idata->data();

    AttributeType asm_item, symb_item, info, brpoint;
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

        if (isBreakpoint(pc + off, &brpoint)) {
            asm_item[ASM_breakpoint].make_boolean(true);
            if (!(brpoint[BrkList_flags].to_uint64() & BreakFlag_HW)) {
                code.val = brpoint[BrkList_instr].to_uint32();
            }
        }
        codesz = disasm(pc + off,
                        code.buf,
                        0,
                        &asm_item[ASM_mnemonic],
                        &asm_item[ASM_comment]);

        uint64_t swap = code.val;
        if (codesz == 2) {
            swap = code.buf16[0];
        }
        if (endianess_.to_int() == 1) {
            swap = 0;
            for (int i = 0; i < codesz; i++) {
                swap = (swap << 8) | code.buf[i];
            }
        }
        asm_item[ASM_code].make_uint64(swap);
        asm_item[ASM_codesize].make_uint64(codesz);
        asmlist->add_to_list(&asm_item);
        off += codesz;
    }
}

int ArmSourceService::parseUndefinedInstruction(uint64_t pc, uint32_t instr,
                                                AttributeType *mnemonic,
                                                AttributeType *comment) {
    char tstr[64] = "unimpl";
    if (((instr >> 20) & 0xFF) == 0x34) {
        uint32_t imm16 = instr & 0xFFF;
        imm16 |= ((instr & 0xF0000) >> 12);
        RISCV_sprintf(tstr, sizeof(tstr), "movt     %s,#%04x",
            IREGS_NAMES[(instr >> 12) & 0xf], imm16);
    } else if (((instr >> 20) & 0xFF) == 0x30) {
        DataProcessingType u;
        u.value = instr;
        uint32_t imm16 = (u.mov_bits.imm4 << 12) | u.mov_bits.imm12;
        RISCV_sprintf(tstr, sizeof(tstr), "movw     %s,#%04x",
            IREGS_NAMES[u.mov_bits.rd], imm16);
        
    }
    mnemonic->make_string(tstr);
    comment->make_string("");
    return 4;
}


int ArmSourceService::parseSingleDataTransfer(uint64_t pc, uint32_t instr,
                                              AttributeType *mnemonic,
                                              AttributeType *comment) {
    char tstr[64];
    SingleDataTransferType u;
    const char *strdown[2] = {"-", ""};
    u.value = instr;
    if (u.imm_bits.L) {
        if (!u.imm_bits.I) {
            if (u.imm_bits.rd == Reg_pc && u.imm_bits.rn == Reg_pc
                && (u.imm_bits.imm & 0x1)) {
                if (u.imm_bits.imm == 0x4F) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%s", "dsb");
                } else if (u.imm_bits.imm == 0x6F) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%s", "isb");
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%s", "?sb");
                }
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "ldr      %s,[%s,#%s%d]",
                    IREGS_NAMES[u.imm_bits.rd],
                    IREGS_NAMES[u.imm_bits.rn],
                    strdown[u.imm_bits.U],
                    u.imm_bits.imm
                    );
            }
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "ldr      %s,[%s,%s,LSL#%d]",
                IREGS_NAMES[u.reg_bits.rd],
                IREGS_NAMES[u.reg_bits.rn],
                IREGS_NAMES[u.reg_bits.rm],
                u.reg_bits.sh_sel
                );
        }
    } else {
        const char *instrname[2] = {"str     ", "strb    "};
        if (!u.imm_bits.I) {
                RISCV_sprintf(tstr, sizeof(tstr), "%s %s,[%s,#%s%d]",
                    instrname[u.imm_bits.B],
                    IREGS_NAMES[u.imm_bits.rd],
                    IREGS_NAMES[u.imm_bits.rn],
                    strdown[u.imm_bits.U],
                    u.imm_bits.imm
                    );
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "%s %s,[%s,%s,LSL#%d]",
                instrname[u.imm_bits.B],
                IREGS_NAMES[u.reg_bits.rd],
                IREGS_NAMES[u.reg_bits.rn],
                IREGS_NAMES[u.reg_bits.rm],
                u.reg_bits.sh_sel
                );
        }
    }
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseBlockDataTransfer(uint64_t pc, uint32_t instr,
                                             AttributeType *mnemonic,
                                             AttributeType *comment) {
    BlockDataTransferType u;
    const char *instrname[2] = {"stm     ", "ldm     "};
    const char *spname[2] =    {"push    ", "pop     "};
    const char *sflag[2] = {"", "^"};
    char rnames[512] = "";
    char tstr[512];
    int sz = 0;
    u.value = instr;
    for (int i = 0; i < 16; i++) {
        if ((instr & (1 << i )) == 0) {
            continue;
        }
        if (sz) {
            sz = RISCV_sprintf(&rnames[sz], sizeof(rnames) - sz,
                                ",%s", IREGS_NAMES[i]);
        } else {
            sz = RISCV_sprintf(&rnames[sz], sizeof(rnames) - sz,
                                "%s", IREGS_NAMES[i]);
        }
    }

    if (u.bits.rn == Reg_sp) {
        RISCV_sprintf(tstr, sizeof(tstr), "%s {%s}%s",
            spname[u.bits.L],
            rnames,
            sflag[u.bits.S]
            );
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "%s %s,{%s}%s",
            instrname[u.bits.L],
            IREGS_NAMES[u.bits.rn],
            rnames,
            sflag[u.bits.S]
            );
    }
    mnemonic->make_string(tstr);
    return 4;
}


int ArmSourceService::parseDataProcessing(uint64_t pc, uint32_t instr,
                                          AttributeType *mnemonic,
                                          AttributeType *comment) {
    char tstr[64];
    char op2[32];
    DataProcessingType u;
    u.value = instr;
    const char instr_name[][16] = {
        "and     ", "eor     ", "sub     ", "rsb     ",
        "add     ", "adc     ", "sbc     ", "rsc     ",
        "tst     ", "teq     ", "cmp     ", "cmn     ",
        "orr     ", "mov     ", "bic     ", "mvn     "

    };
    const char psrname[2][16] = {"CPSR", "SPSR"};
    // N Z C V
    const char flagmask[][16] = {
    "",    "v",    "c",   "cv",
    "z",  "zv",   "zc",  "zcv",
    "n",  "nv",   "nc",  "ncv",
    "nz", "nzv", "nzc", "nzcv"
    };

    if (u.imm_bits.I) {
        uint32_t rsh = 2*u.imm_bits.rotate;
        uint32_t imm = (u.imm_bits.imm >> rsh)
                     | (u.imm_bits.imm << (32 - rsh));
        RISCV_sprintf(op2, sizeof(op2), "#%d", imm);
    } else {
        if (u.reg_bits.shift) {
            RISCV_sprintf(op2, sizeof(op2), "%s{,%d}",
                IREGS_NAMES[u.reg_bits.rm],
                u.reg_bits.shift);
        } else {
            RISCV_sprintf(op2, sizeof(op2), "%s",
                IREGS_NAMES[u.reg_bits.rm],
                u.reg_bits.shift);
        }
    }
    if (instr == 0xe320f000) {
        RISCV_sprintf(tstr, sizeof(tstr), "nop      {%d}", 0);
    } else if (u.mrs_bits.b27_23 == 0x2 && u.mrs_bits.b21_20 == 0
        && u.mrs_bits.mask == 0xF && u.mrs_bits.zero12 == 0) {
        RISCV_sprintf(tstr, sizeof(tstr), "mrs      %s,%s",
            IREGS_NAMES[u.mrs_bits.rd],
            psrname[u.mrs_bits.ps]);
    } else if ((u.mrs_bits.b27_23 == 0x2 || u.mrs_bits.b27_23 == 0x6)
        && u.mrs_bits.b21_20 == 0x2
        && u.mrs_bits.rd == 0xf) {
        RISCV_sprintf(tstr, sizeof(tstr), "msr      %s_%s,%s",
            psrname[u.mrs_bits.ps],
            flagmask[u.mrs_bits.mask+1],
            op2
            );
    } else if (u.imm_bits.opcode == 13 || u.imm_bits.opcode == 15) {
        // MOV, MVN
        RISCV_sprintf(tstr, sizeof(tstr), "%s %s,%s",
            instr_name[u.imm_bits.opcode],
            IREGS_NAMES[u.imm_bits.rd],
            op2);
    } else if (u.imm_bits.opcode == 8 || u.imm_bits.opcode == 9
        || u.imm_bits.opcode == 10 || u.imm_bits.opcode == 11) {
        // CMP, CMN, TEQ, TST
        RISCV_sprintf(tstr, sizeof(tstr), "%s %s,%s",
            instr_name[u.imm_bits.opcode],
            IREGS_NAMES[u.imm_bits.rn],
            op2);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "%s %s,%s,%s",
            instr_name[u.imm_bits.opcode],
            IREGS_NAMES[u.imm_bits.rd],
            IREGS_NAMES[u.imm_bits.rn],
            op2);
    }
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseMultiply(uint64_t pc, uint32_t instr,
                                    AttributeType *mnemonic,
                                    AttributeType *comment) {
    char tstr[64] = "unimpl";
    instr &= 0x0FFFFFFF;
    if ((instr >> 23) == 0) {
        MulType u;
        u.value = instr;
        if (u.bits.A) {
            RISCV_sprintf(tstr, sizeof(tstr), "mla      %s,%s,%s,%s",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rm],
                IREGS_NAMES[u.bits.rs],
                IREGS_NAMES[u.bits.rn]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "mul      %s,%s,%s",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rm],
                IREGS_NAMES[u.bits.rs]);
        }
    } else if ((instr >> 23) == 1) {
        const char *str_s[2] = {"u", "s"};
        const char *str_instr[2] = {"mull    ", "mlal    "};
        MulLongType u;
        u.value = instr;
        RISCV_sprintf(tstr, sizeof(tstr), "%s%s %s,%s,%s,%s",
            str_s[u.bits.S],
            str_instr[u.bits.A],
            IREGS_NAMES[u.bits.rdhi],
            IREGS_NAMES[u.bits.rdlo],
            IREGS_NAMES[u.bits.rm],
            IREGS_NAMES[u.bits.rs]);
    }
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseDivide(uint64_t pc, uint32_t instr,
    AttributeType *mnemonic,
    AttributeType *comment) {
    char tstr[64] = "unimpl";
    instr &= 0x0FFFFFFF;
    DivType u;

    u.value = instr;
    if (u.bits.S) {
        RISCV_sprintf(tstr, sizeof(tstr), "udiv     %s,%s,%s",
            IREGS_NAMES[u.bits.rd],
            IREGS_NAMES[u.bits.rn],
            IREGS_NAMES[u.bits.rm]);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "sdiv     %s,%s,%s",
            IREGS_NAMES[u.bits.rd],
            IREGS_NAMES[u.bits.rn],
            IREGS_NAMES[u.bits.rm]);
    }
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseBytesExtending(uint64_t pc, uint32_t instr,
                                          AttributeType *mnemonic,
                                          AttributeType *comment) {
    char tstr[64] = "unimpl";
    SignExtendType u;
    u.value = instr;
    if (u.bits.rn == 0xF) {
        if (u.bits.b27_20 == 0x6E) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxtb     %s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6C) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxtb16   %s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6F) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxth     %s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        }
    } else {
        if (u.bits.b27_20 == 0x6E) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxtab    %s,%s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rn],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6C) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxtab16  %s,%s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rn],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6F) {
            RISCV_sprintf(tstr, sizeof(tstr), "uxtah    %s,%s,%s,sh#%d",
                IREGS_NAMES[u.bits.rd],
                IREGS_NAMES[u.bits.rn],
                IREGS_NAMES[u.bits.rm],
                8*u.bits.rotate);
        }
    }
    mnemonic->make_string(tstr);
    return 4;
}


int ArmSourceService::parseCoprocRegTransfer(uint64_t pc, uint32_t instr,
                                             AttributeType *mnemonic,
                                             AttributeType *comment) {
    char tstr[64] = "unimpl";
    CoprocessorTransferType u;
    u.value = instr;
    const char instr_name[2][16] = {"mcr     ", "mrc     "};
    RISCV_sprintf(tstr, sizeof(tstr), "%s p%d,%d,%s,c%d,c%d,%d",
        instr_name[u.bits.L],
        u.bits.cp_num,
        u.bits.mode,
        IREGS_NAMES[u.bits.rd],
        u.bits.crn,
        u.bits.crm,
        u.bits.cp_nfo
        );
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseBranch(uint64_t pc, uint32_t instr,
                                  AttributeType *mnemonic,
                                  AttributeType *comment) {
    char tstr[64];
    BranchType u;
    u.value = instr;
    const char *flags[16] = {
        "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
        "hi", "ls", "ge", "lt", "gt", "le", "", ""
    };

    uint32_t off = u.bits.offset;
    if ((u.value >> 23) & 0x1) {
        off |= 0xFF000000;
    }
    off = static_cast<uint32_t>(pc + (off << 2) + 8);

    if (u.bits.L) {
        RISCV_sprintf(tstr, sizeof(tstr), "bl       %x",
            off);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "b%2s      %x",
            flags[u.bits.cond],
            off);
    }
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseBranchExchange(uint64_t pc, uint32_t instr,
                                          AttributeType *mnemonic,
                                          AttributeType *comment) {
    char tstr[64];
    BranchType u;
    u.value = instr;

    RISCV_sprintf(tstr, sizeof(tstr), "bx       %s",
            IREGS_NAMES[u.bits.offset & 0xF]);
    mnemonic->make_string(tstr);
    return 4;
}

int ArmSourceService::parseBranchLinkExchange(uint64_t pc, uint32_t instr,
    AttributeType *mnemonic,
    AttributeType *comment) {
    char tstr[64];
    BranchExchangeIndirectType u;
    u.value = instr;

    RISCV_sprintf(tstr, sizeof(tstr), "blx      %s",
        IREGS_NAMES[u.bits.rm]);
    mnemonic->make_string(tstr);
    return 4;
}

}  // namespace debugger
