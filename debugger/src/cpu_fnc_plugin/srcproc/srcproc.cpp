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

namespace debugger {

/** Class registration in the Core */
enum ESymbInfo {
    SymbInfo_Name,
    SymbInfo_Address,
    SymbInfo_Total,
};

const char *const *RN = IREGS_NAMES;
const char *const *RF = FREGS_NAMES;

int opcode_0x00(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x01(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x03(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x04(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x05(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x06(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x08(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x09(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0D(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0E(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x14(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x18(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x19(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1B(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);

int C_ADDI16SP_LUI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_ADDI4SPN(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_BEQZ(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_BNEZ(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_J(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_JAL_ADDIW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_JR_MV_EBREAK_JALR_ADD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LDSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LWSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_MATH(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_NOP_ADDI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SDSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SLLI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SWSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);


RiscvSourceService::RiscvSourceService(const char *name) : IService(name) {
    registerInterface(static_cast<ISourceCode *>(this));
    memset(tblOpcode1_, 0, sizeof(tblOpcode1_));
    tblOpcode1_[0x00] = &opcode_0x00;
    tblOpcode1_[0x01] = &opcode_0x01;
    tblOpcode1_[0x03] = &opcode_0x03;
    tblOpcode1_[0x04] = &opcode_0x04;
    tblOpcode1_[0x05] = &opcode_0x05;
    tblOpcode1_[0x06] = &opcode_0x06;
    tblOpcode1_[0x08] = &opcode_0x08;
    tblOpcode1_[0x09] = &opcode_0x09;
    tblOpcode1_[0x0C] = &opcode_0x0C;
    tblOpcode1_[0x0D] = &opcode_0x0D;
    tblOpcode1_[0x0E] = &opcode_0x0E;
    tblOpcode1_[0x14] = &opcode_0x14;
    tblOpcode1_[0x18] = &opcode_0x18;
    tblOpcode1_[0x19] = &opcode_0x19;
    tblOpcode1_[0x1B] = &opcode_0x1B;
    tblOpcode1_[0x1C] = &opcode_0x1C;

    memset(tblCompressed_, 0, sizeof(tblCompressed_));
    // page 82, table 12.5 " RISC-V spec. v2.2"
    // Compute index as hash = {[15:13],[1:0]}
    tblCompressed_[0x00] = &C_ADDI4SPN;
    tblCompressed_[0x01] = &C_NOP_ADDI;
    tblCompressed_[0x02] = &C_SLLI;
    tblCompressed_[0x05] = &C_JAL_ADDIW;
    tblCompressed_[0x08] = &C_LW;
    tblCompressed_[0x09] = &C_LI;
    tblCompressed_[0x0A] = &C_LWSP;
    tblCompressed_[0x0C] = &C_LD;
    tblCompressed_[0x0D] = &C_ADDI16SP_LUI;
    tblCompressed_[0x0E] = &C_LDSP;
    tblCompressed_[0x11] = &C_MATH;
    tblCompressed_[0x12] = &C_JR_MV_EBREAK_JALR_ADD;
    tblCompressed_[0x15] = &C_J;
    tblCompressed_[0x18] = &C_SW;
    tblCompressed_[0x19] = &C_BEQZ;
    tblCompressed_[0x1A] = &C_SWSP;
    tblCompressed_[0x1C] = &C_SD;
    tblCompressed_[0x1D] = &C_BNEZ;
    tblCompressed_[0x1E] = &C_SDSP;

    brList_.make_list(0);
    symbolListSortByName_.make_list(0);
    symbolListSortByAddr_.make_list(0);
}

RiscvSourceService::~RiscvSourceService() {
}

void RiscvSourceService::postinitService() {
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

void RiscvSourceService::registerBreakpoint(uint64_t addr,
                                            uint64_t flags,
                                            uint32_t instr,
                                            uint32_t opcode,
                                            uint32_t oplen) {
    AttributeType item;
    item.make_list(BrkList_Total);
    item[BrkList_address].make_uint64(addr);
    item[BrkList_flags].make_uint64(flags);
    item[BrkList_instr].make_uint64(instr);
    item[BrkList_opcode].make_uint64(opcode);
    item[BrkList_oplen].make_int64(oplen);

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

int RiscvSourceService::unregisterBreakpoint(uint64_t addr) {
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

int RiscvSourceService::disasm(uint64_t pc,
                       uint8_t *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment) {
    int oplen;
    if ((data[offset] & 0x3) < 3) {
        Reg16Type val;
        uint32_t hash;
        val.word = *reinterpret_cast<uint16_t*>(&data[offset]);
        hash = ((val.word >> 11) & 0x1C) | (val.word & 0x3);
        oplen = 2;
        if (tblCompressed_[hash]) {
            return tblCompressed_[hash](static_cast<ISourceCode *>(this),
                                        pc + static_cast<uint64_t>(offset),
                                        val,
                                        mnemonic,
                                        comment);
        }
    } else if ((data[offset] & 0x3) == 0x3) {
        uint32_t val = *reinterpret_cast<uint32_t*>(&data[offset]);
        uint32_t opcode1 = (val >> 2) & 0x1f;
        oplen = 4;
        if (tblOpcode1_[opcode1]) {
            return tblOpcode1_[opcode1](static_cast<ISourceCode *>(this),
                                        pc + static_cast<uint64_t>(offset),
                                        val,
                                        mnemonic,
                                        comment);
        }
    }

    mnemonic->make_string("unimpl");
    comment->make_string("");
    return oplen;
}

void RiscvSourceService::disasm(uint64_t pc,
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
        codesz = disasm(pc + off,
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

int opcode_0x00(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "lb      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "lh      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 4:
        RISCV_sprintf(tstr, sizeof(tstr), "lbu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 5:
        RISCV_sprintf(tstr, sizeof(tstr), "lhu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 6:
        RISCV_sprintf(tstr, sizeof(tstr), "lwu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x01(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "fld     %s,%d(%s)",
            RF[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x03(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;

    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "fence");
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "fence_i");
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x04(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        if (imm == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "mv      %s,%s",
                RN[i.bits.rd], RN[i.bits.rs1]);
        } else if (i.bits.rs1 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "li      %s,%d",
                RN[i.bits.rd], imm);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "slli    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "slti    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sltiu   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 4:
        RISCV_sprintf(tstr, sizeof(tstr), "xori    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 5:
        if ((code >> 26) == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srli    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        } else if ((code >> 26) == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "srai    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    case 6:
        RISCV_sprintf(tstr, sizeof(tstr), "ori     %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 7:
        RISCV_sprintf(tstr, sizeof(tstr), "andi    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x05(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_U_type u;
    uint64_t imm64;

    u.value = code;
    imm64 = u.bits.imm31_12 << 12;
    if (imm64 & (1LL << 31)) {
        imm64 |= EXT_SIGN_32;
    }
    RISCV_sprintf(tstr, sizeof(tstr), "auipc   %s,0x%" RV_PRI64 "x",
        RN[u.bits.rd], imm64);
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x06(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "addiw   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "slliw   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 5:
        if ((code >> 25) == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srliw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        } else if ((code >> 25) == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sraiw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x08(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_S_type s;
    int32_t imm;
    s.value = code;
    imm = (s.bits.imm11_5 << 5) | s.bits.imm4_0;
    if (imm & 0x800) {
        imm |= EXT_SIGN_12;
    }
    switch (s.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "sb      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sh      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x09(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_S_type s;
    int32_t imm;
    s.value = code;
    imm = (s.bits.imm11_5 << 5) | s.bits.imm4_0;
    if (imm & 0x800) {
        imm |= EXT_SIGN_12;
    }
    switch (s.bits.funct3) {
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "fsd     %s,%d(%s)",
            RF[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;
    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "add     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "mul     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sub     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sll     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "slt     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sltu     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 4:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "xor     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "div     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 5:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srl     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divu    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sra     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 6:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "or      %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "rem     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 7:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "and     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remu    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0D(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_U_type u;
    u.value = code;
    RISCV_sprintf(tstr, sizeof(tstr), "lui     %s,0x%x",
        RN[u.bits.rd], u.bits.imm31_12);
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0E(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;

    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "addw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "mulw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "subw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sllw    %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 4:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 5:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srlw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divuw   %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sraw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 6:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 7:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remuw   %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x14(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type u;
    u.value = code;

    switch (u.bits.funct7) {
    case 0x1:
        RISCV_sprintf(tstr, sizeof(tstr), "fadd.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x5:
        RISCV_sprintf(tstr, sizeof(tstr), "fsub.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x9:
        RISCV_sprintf(tstr, sizeof(tstr), "fmul.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0xd:
        RISCV_sprintf(tstr, sizeof(tstr), "fdiv.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x15:
        if (u.bits.funct3 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmin.d  %s,%s,%s",
                RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmax.d  %s,%s,%s",
                RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        }
        break;
    case 0x2d:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fsqrt.d %s,%s",
                RF[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x51:
        if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fle.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "flt.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "feq.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        }
        break;
    case 0x61:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.w.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.wu.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.l.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 3) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.lu.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x69:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.w  %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.wu %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.l  %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 3) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.lu %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        }
        break;
    case 0x71:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmv.x.d %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x79:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmv.d.x %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        }
        break;
    default:;
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x18(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_SB_type sb;
    uint64_t imm64;

    sb.value = code;
    imm64 = (sb.bits.imm12 << 12) | (sb.bits.imm11 << 11)
                | (sb.bits.imm10_5 << 5) | (sb.bits.imm4_1 << 1);
    if (sb.bits.imm12) {
        imm64 |= EXT_SIGN_12;
    }
    imm64 += pc;
    switch (sb.bits.funct3) {
    case 0:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "beqz    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "beq     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 1:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bnez    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bne     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 4:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bltz    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "blt     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 5:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bgez    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bge     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 6:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bltuz   %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bltu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 7:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bgeuz   %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bgeu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    default:;
    }

    if (isrc) {
        AttributeType info;
        isrc->addressToSymbol(imm64, &info);
        if (info[0u].size()) {
            if (info[1].to_uint32() == 0) {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s",
                        info[0u].to_string());
            } else {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s+%xh",
                        info[0u].to_string(), info[1].to_uint32());
            }
        }
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x19(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    uint64_t imm64;

    i.value = code;
    imm64 = static_cast<int32_t>(code) >> 20;
    if (imm64 == 0) {
        if (i.bits.rs1 == Reg_ra) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ret");
        } else if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "jr      %s",
                RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "jalr    %s",
                RN[i.bits.rs1]);
        }
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "jalr    %" RV_PRI64 "d,(%s)",
            imm64, RN[i.bits.rs1]);
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x1B(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_UJ_type uj;
    uint64_t imm64;

    uj.value = code;
    imm64 = 0;
    if (uj.bits.imm20) {
        imm64 = 0xfffffffffff00000LL;
    }
    imm64 |= (uj.bits.imm19_12 << 12);
    imm64 |= (uj.bits.imm11 << 11);
    imm64 |= (uj.bits.imm10_1 << 1);
    if (uj.bits.rd) {
        RISCV_sprintf(tstr, sizeof(tstr), "jal     ra,%08" RV_PRI64 "x",
            pc + imm64);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "j       %08" RV_PRI64 "x",
            pc + imm64);
    }

    if (isrc) {
        AttributeType info;
        isrc->addressToSymbol(pc + imm64, &info);
        if (info[0u].size()) {
            if (info[1].to_uint32() == 0) {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s",
                        info[0u].to_string());
            } else {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s+%xh",
                        info[0u].to_string(), info[1].to_uint32());
            }
        }
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x1C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    uint32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;

    switch (i.bits.funct3) {
    case 0:
        if (code == 0x00000073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ecall");
        } else if (code == 0x00100073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ebreak");
        } else if (code == 0x00200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "uret");
        } else if (code == 0x10200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "sret");
        } else if (code == 0x20200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "hret");
        } else if (code == 0x30200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "mret");
        }
        break;
    case 1:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrw    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrw   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 2:
        if (i.bits.rs1 == 0) {
            // Read
            RISCV_sprintf(tstr, sizeof(tstr), "csrr    %s,0x%x",
                RN[i.bits.rd], imm);
        } else if (i.bits.rd == 0) {
            // Set
            RISCV_sprintf(tstr, sizeof(tstr), "csrs    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            // Read and set
            RISCV_sprintf(tstr, sizeof(tstr), "csrrs   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 3:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrc    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrc   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 5:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrwi   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrwi  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    case 6:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrsi   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrsi  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    case 7:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrci   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrci  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

/**
 * C-extension  disassembler
 */
int C_JR_MV_EBREAK_JALR_ADD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_CR_type u;
    u.value = code.word;
    if (u.bits.funct4 == 0x8) {
        if (u.bits.rdrs1 && !u.bits.rs2) {
            if (u.bits.rdrs1 == 1) {
                RISCV_sprintf(tstr, sizeof(tstr), "%s", "ret");
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "jr      %s",
                    RN[u.bits.rdrs1]);
            }
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "mv      %s,%s",
                RN[u.bits.rdrs1], RN[u.bits.rs2]);
        }
    } else {
        if (!u.bits.rdrs1 && !u.bits.rs2) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ebreak");
        } else if (u.bits.rdrs1 && !u.bits.rs2) {
            RISCV_sprintf(tstr, sizeof(tstr), "jalr    ra,%s,0",
                RN[u.bits.rdrs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "add     %s,%s,%s",
                RN[u.bits.rdrs1], RN[u.bits.rdrs1], RN[u.bits.rs2]);
        }
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_NOP_ADDI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }
    if (u.value == 0x1) {
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "nop");
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,%s,%" RV_PRI64 "d",
            RN[u.bits.rdrs], RN[u.bits.rdrs], imm);
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_ADDI16SP_LUI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    if (u.spbits.sp == 0x2) {
        uint64_t imm = (u.spbits.imm8_7 << 3) | (u.spbits.imm6 << 2)
                    | (u.spbits.imm5 << 1) | u.spbits.imm4;
        if (u.spbits.imm9) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 4;
        RISCV_sprintf(tstr, sizeof(tstr), "addi    sp,sp,%" RV_PRI64 "d",
            imm);
    } else {
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 12;
        RISCV_sprintf(tstr, sizeof(tstr), "lui     %s,0x%x",
            RN[u.bits.rdrs], static_cast<uint32_t>(imm));

    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_ADDI4SPN(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimp";
    char tcomm[128] = "";
    ISA_CIW_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.imm9_6 << 4) | (u.bits.imm5_4 << 2)
                | (u.bits.imm3 << 1) | u.bits.imm2;
    imm <<= 2;
    if (code.word) {
        RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,sp,%" RV_PRI64 "d",
            RN[8 + u.bits.rd], imm);
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_JAL_ADDIW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    int64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }
    // JAL is th RV32C only instruction
    RISCV_sprintf(tstr, sizeof(tstr), "addiw   %s,%s,%" RV_PRI64 "d",
        RN[u.bits.rdrs], RN[u.bits.rdrs], imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CL_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(%s)",
        RN[8 + u.bits.rd], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LDSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint32_t off = (u.ldspbits.off8_6 << 3) | (u.ldspbits.off5 << 2)
                    | u.ldspbits.off4_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(sp)",
        RN[u.ldspbits.rd], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    int64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "li      %s,%" RV_PRI64 "d",
        RN[u.bits.rdrs], imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CL_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(%s)",
        RN[8 + u.bits.rd], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LWSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint32_t off = (u.lwspbits.off7_6 << 4) | (u.lwspbits.off5 << 3)
                     | u.lwspbits.off4_2;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(sp)",
        RN[u.lwspbits.rd], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_MATH(ISourceCode *isrc, uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
    uint32_t imm = (u.bits.off7_6 << 3) | (u.bits.off2_1 << 1)  | u.bits.off5;

    if (u.bits.off4_3 == 0) {
        RISCV_sprintf(tstr, sizeof(tstr), "srli    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], shamt);
    } else if (u.bits.off4_3 == 1) {
        RISCV_sprintf(tstr, sizeof(tstr), "srai    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], shamt);
    } else if (u.bits.off4_3 == 2) {
        RISCV_sprintf(tstr, sizeof(tstr), "andi    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], imm);
    } else if (u.bits.off8 == 0) {
        ISA_CS_type u2;
        u2.value = code.word;
        switch (u.bits.off7_6) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "sub     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "xor     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 2:
            RISCV_sprintf(tstr, sizeof(tstr), "or      %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        default:
            RISCV_sprintf(tstr, sizeof(tstr), "and     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
        }
    } else {
        ISA_CS_type u2;
        u2.value = code.word;
        switch (u.bits.off7_6) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "subw    %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "addw    %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        default:;
        }
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_J(ISourceCode *isrc, uint64_t pc, Reg16Type code,
        AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CJ_type u;
    u.value = code.word;
    uint64_t off = (u.bits.off10 << 9) | (u.bits.off9_8 << 7)
                    | (u.bits.off7 << 6) | (u.bits.off6 << 5)
                    | (u.bits.off5 << 4) | (u.bits.off4 << 3)
                    | u.bits.off3_1;
    off <<= 1;
    if (u.bits.off11) {
        off |= EXT_SIGN_11;
    }
    RISCV_sprintf(tstr, sizeof(tstr), "j       %" RV_PRI64 "x", pc + off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SD(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CS_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(%s)",
        RN[8 + u.bits.rs2], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SDSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CSS_type u;
    u.value = code.word;
    uint32_t off = (u.dbits.imm8_6 << 3) | u.dbits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(sp)",
        RN[u.dbits.rs2], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SW(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CS_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(%s)",
        RN[8 + u.bits.rs2], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SWSP(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CSS_type u;
    u.value = code.word;
    uint32_t off = (u.wbits.imm7_6 << 4) | u.wbits.imm5_2;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(sp)",
        RN[u.dbits.rs2], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_BEQZ(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
            | (u.bits.off4_3 << 2) | u.bits.off2_1;
    imm <<= 1;
    if (u.bits.off8) {
        imm |= EXT_SIGN_9;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "beqz    %s,%" RV_PRI64 "x",
        RN[8 + u.bits.rs1], pc + imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_BNEZ(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
            | (u.bits.off4_3 << 2) | u.bits.off2_1;
    imm <<= 1;
    if (u.bits.off8) {
        imm |= EXT_SIGN_9;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "bnez    %s,%" RV_PRI64 "x",
        RN[8 + u.bits.rs1], pc + imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SLLI(ISourceCode *isrc, uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
    uint32_t idx = (u.shbits.funct2 << 3) | u.shbits.rd;
    RISCV_sprintf(tstr, sizeof(tstr), "slli    %s,%s,%d",
        RN[idx], RN[idx], shamt);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

}  // namespace debugger
