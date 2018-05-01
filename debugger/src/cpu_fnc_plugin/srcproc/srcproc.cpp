/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Source code processor class declaration.
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

int opcode_0x00(ISourceCode *isrc, uint64_t pc, uint32_t code,
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
int opcode_0x0C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0D(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0E(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x18(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x19(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1B(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1C(ISourceCode *isrc, uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);


RiscvSourceService::RiscvSourceService(const char *name) : IService(name) {
    registerInterface(static_cast<ISourceCode *>(this));
    memset(tblOpcode1_, 0, sizeof(tblOpcode1_));
    tblOpcode1_[0x00] = &opcode_0x00;
    tblOpcode1_[0x03] = &opcode_0x03;
    tblOpcode1_[0x04] = &opcode_0x04;
    tblOpcode1_[0x05] = &opcode_0x05;
    tblOpcode1_[0x06] = &opcode_0x06;
    tblOpcode1_[0x08] = &opcode_0x08;
    tblOpcode1_[0x0C] = &opcode_0x0C;
    tblOpcode1_[0x0D] = &opcode_0x0D;
    tblOpcode1_[0x0E] = &opcode_0x0E;
    tblOpcode1_[0x18] = &opcode_0x18;
    tblOpcode1_[0x19] = &opcode_0x19;
    tblOpcode1_[0x1B] = &opcode_0x1B;
    tblOpcode1_[0x1C] = &opcode_0x1C;

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

void RiscvSourceService::registerBreakpoint(uint64_t addr, uint64_t flags,
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

int RiscvSourceService::unregisterBreakpoint(uint64_t addr, uint64_t *flags,
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

void RiscvSourceService::getBreakpointList(AttributeType *list) {
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

bool RiscvSourceService::isBreakpoint(uint64_t addr, AttributeType *outbr) {
    for (unsigned i = 0; i < brList_.size(); i++) {
        uint64_t bradr = brList_[i][BrkList_address].to_uint64();
        if (addr == bradr) {
            *outbr = brList_[i];
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
    if ((data[offset] & 0x3) < 3) {
        mnemonic->make_string("compr");
        comment->make_string("");
        return 2;
    } else if ((data[offset] & 0x3) == 0x3) {
        uint32_t val = *reinterpret_cast<uint32_t*>(&data[offset]);
        uint32_t opcode1 = (val >> 2) & 0x1f;
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
    return 4;
}

void RiscvSourceService::disasm(uint64_t pc,
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
    RISCV_sprintf(tstr, sizeof(tstr), "auipc   %s,%" RV_PRI64 "x",
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
        RISCV_sprintf(tstr, sizeof(tstr), "jalr    %s,%08" RV_PRI64 "x",
            RN[i.bits.rs1], pc + imm64);
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
        RISCV_sprintf(tstr, sizeof(tstr), "jal     %08" RV_PRI64 "x",
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

}  // namespace debugger
