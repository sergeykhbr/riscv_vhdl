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

#include "arm-isa.h"
#include "srcproc.h"
#include <iostream>

namespace debugger {

static const char *const *RN = ARM_IREGS_NAMES;

int disasm_thumb(uint64_t pc,
                uint32_t ti,
                char *disasm,
                size_t sz);


int parseUndefinedInstruction(uint64_t pc, uint32_t instr,
                              char *mnemonic, size_t mnemonicsz,
                              char *comment, size_t commentsz) {
    if (((instr >> 20) & 0xFF) == 0x34) {
        uint32_t imm16 = instr & 0xFFF;
        imm16 |= ((instr & 0xF0000) >> 12);
        RISCV_sprintf(mnemonic, mnemonicsz, "movt     %s,#%04x",
            RN[(instr >> 12) & 0xf], imm16);
    } else if (((instr >> 20) & 0xFF) == 0x30) {
        DataProcessingType u;
        u.value = instr;
        uint32_t imm16 = (u.mov_bits.imm4 << 12) | u.mov_bits.imm12;
        RISCV_sprintf(mnemonic, mnemonicsz, "movw     %s,#%04x",
            RN[u.mov_bits.rd], imm16);
        
    }
    return 4;
}


int parseSingleDataTransfer(uint64_t pc, uint32_t instr,
                            char *mnemonic, size_t mnemonicsz,
                            char *comment, size_t commentsz) {
    SingleDataTransferType u;
    const char *strdown[2] = {"-", ""};
    u.value = instr;
    if (u.imm_bits.L) {
        if (!u.imm_bits.I) {
            if (u.imm_bits.rd == Reg_pc && u.imm_bits.rn == Reg_pc
                && (u.imm_bits.imm & 0x1)) {
                if (u.imm_bits.imm == 0x4F) {
                    RISCV_sprintf(mnemonic, mnemonicsz, "%s", "dsb");
                } else if (u.imm_bits.imm == 0x6F) {
                    RISCV_sprintf(mnemonic, mnemonicsz, "%s", "isb");
                } else {
                    RISCV_sprintf(mnemonic, mnemonicsz, "%s", "?sb");
                }
            } else {
                RISCV_sprintf(mnemonic, mnemonicsz, "ldr      %s,[%s,#%s%d]",
                    RN[u.imm_bits.rd],
                    RN[u.imm_bits.rn],
                    strdown[u.imm_bits.U],
                    u.imm_bits.imm
                    );
            }
        } else {
            RISCV_sprintf(mnemonic, mnemonicsz, "ldr      %s,[%s,%s,LSL#%d]",
                RN[u.reg_bits.rd],
                RN[u.reg_bits.rn],
                RN[u.reg_bits.rm],
                u.reg_bits.sh_sel
                );
        }
    } else {
        const char *instrname[2] = {"str     ", "strb    "};
        if (!u.imm_bits.I) {
                RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,[%s,#%s%d]",
                    instrname[u.imm_bits.B],
                    RN[u.imm_bits.rd],
                    RN[u.imm_bits.rn],
                    strdown[u.imm_bits.U],
                    u.imm_bits.imm
                    );
        } else {
            RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,[%s,%s,LSL#%d]",
                instrname[u.imm_bits.B],
                RN[u.reg_bits.rd],
                RN[u.reg_bits.rn],
                RN[u.reg_bits.rm],
                u.reg_bits.sh_sel
                );
        }
    }
    return 4;
}

int parseBlockDataTransfer(uint64_t pc, uint32_t instr,
                            char *mnemonic, size_t mnemonicsz,
                            char *comment, size_t commentsz) {
    BlockDataTransferType u;
    const char *instrname[2] = {"stm     ", "ldm     "};
    const char *spname[2] =    {"push    ", "pop     "};
    const char *sflag[2] = {"", "^"};
    char rnames[512] = "";
    int sz = 0;
    u.value = instr;
    for (int i = 0; i < 16; i++) {
        if ((instr & (1 << i )) == 0) {
            continue;
        }
        if (sz) {
            sz = RISCV_sprintf(&rnames[sz], sizeof(rnames) - sz,
                                ",%s", RN[i]);
        } else {
            sz = RISCV_sprintf(&rnames[sz], sizeof(rnames) - sz,
                                "%s", RN[i]);
        }
    }

    if (u.bits.rn == Reg_sp) {
        RISCV_sprintf(mnemonic, mnemonicsz, "%s {%s}%s",
            spname[u.bits.L],
            rnames,
            sflag[u.bits.S]
            );
    } else {
        RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,{%s}%s",
            instrname[u.bits.L],
            RN[u.bits.rn],
            rnames,
            sflag[u.bits.S]
            );
    }
    return 4;
}


int parseDataProcessing(uint64_t pc, uint32_t instr,
                        char *mnemonic, size_t mnemonicsz,
                        char *comment, size_t commentsz) {
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
                RN[u.reg_bits.rm],
                u.reg_bits.shift);
        } else {
            RISCV_sprintf(op2, sizeof(op2), "%s",
                RN[u.reg_bits.rm],
                u.reg_bits.shift);
        }
    }
    if (instr == 0xe320f000) {
        RISCV_sprintf(mnemonic, mnemonicsz, "nop      {%d}", 0);
    } else if (u.mrs_bits.b27_23 == 0x2 && u.mrs_bits.b21_20 == 0
        && u.mrs_bits.mask == 0xF && u.mrs_bits.zero12 == 0) {
        RISCV_sprintf(mnemonic, mnemonicsz, "mrs      %s,%s",
            RN[u.mrs_bits.rd],
            psrname[u.mrs_bits.ps]);
    } else if ((u.mrs_bits.b27_23 == 0x2 || u.mrs_bits.b27_23 == 0x6)
        && u.mrs_bits.b21_20 == 0x2
        && u.mrs_bits.rd == 0xf) {
        RISCV_sprintf(mnemonic, mnemonicsz, "msr      %s_%s,%s",
            psrname[u.mrs_bits.ps],
            flagmask[u.mrs_bits.mask+1],
            op2
            );
    } else if (u.imm_bits.opcode == 13 || u.imm_bits.opcode == 15) {
        // MOV, MVN
        RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,%s",
            instr_name[u.imm_bits.opcode],
            RN[u.imm_bits.rd],
            op2);
    } else if (u.imm_bits.opcode == 8 || u.imm_bits.opcode == 9
        || u.imm_bits.opcode == 10 || u.imm_bits.opcode == 11) {
        // CMP, CMN, TEQ, TST
        RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,%s",
            instr_name[u.imm_bits.opcode],
            RN[u.imm_bits.rn],
            op2);
    } else {
        RISCV_sprintf(mnemonic, mnemonicsz, "%s %s,%s,%s",
            instr_name[u.imm_bits.opcode],
            RN[u.imm_bits.rd],
            RN[u.imm_bits.rn],
            op2);
    }
    return 4;
}

int parseMultiply(uint64_t pc, uint32_t instr,
                  char *mnemonic, size_t mnemonicsz,
                  char *comment, size_t commentsz) {
    instr &= 0x0FFFFFFF;
    if ((instr >> 23) == 0) {
        MulType u;
        u.value = instr;
        if (u.bits.A) {
            RISCV_sprintf(mnemonic, mnemonicsz, "mla      %s,%s,%s,%s",
                RN[u.bits.rd],
                RN[u.bits.rm],
                RN[u.bits.rs],
                RN[u.bits.rn]);
        } else {
            RISCV_sprintf(mnemonic, mnemonicsz, "mul      %s,%s,%s",
                RN[u.bits.rd],
                RN[u.bits.rm],
                RN[u.bits.rs]);
        }
    } else if ((instr >> 23) == 1) {
        const char *str_s[2] = {"u", "s"};
        const char *str_instr[2] = {"mull    ", "mlal    "};
        MulLongType u;
        u.value = instr;
        RISCV_sprintf(mnemonic, mnemonicsz, "%s%s %s,%s,%s,%s",
            str_s[u.bits.S],
            str_instr[u.bits.A],
            RN[u.bits.rdhi],
            RN[u.bits.rdlo],
            RN[u.bits.rm],
            RN[u.bits.rs]);
    }
    return 4;
}

int parseDivide(uint64_t pc, uint32_t instr,
                char *mnemonic, size_t mnemonicsz,
                char *comment, size_t commentsz) {
    instr &= 0x0FFFFFFF;
    DivType u;

    u.value = instr;
    if (u.bits.S) {
        RISCV_sprintf(mnemonic, mnemonicsz, "udiv     %s,%s,%s",
            RN[u.bits.rd],
            RN[u.bits.rn],
            RN[u.bits.rm]);
    } else {
        RISCV_sprintf(mnemonic, mnemonicsz, "sdiv     %s,%s,%s",
            RN[u.bits.rd],
            RN[u.bits.rn],
            RN[u.bits.rm]);
    }
    return 4;
}

int parseBytesExtending(uint64_t pc, uint32_t instr,
                        char *mnemonic, size_t mnemonicsz,
                        char *comment, size_t commentsz) {
    SignExtendType u;
    u.value = instr;
    if (u.bits.rn == 0xF) {
        if (u.bits.b27_20 == 0x6E) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxtb     %s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6C) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxtb16   %s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6F) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxth     %s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rm],
                8*u.bits.rotate);
        }
    } else {
        if (u.bits.b27_20 == 0x6E) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxtab    %s,%s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rn],
                RN[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6C) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxtab16  %s,%s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rn],
                RN[u.bits.rm],
                8*u.bits.rotate);
        } else if (u.bits.b27_20 == 0x6F) {
            RISCV_sprintf(mnemonic, mnemonicsz, "uxtah    %s,%s,%s,sh#%d",
                RN[u.bits.rd],
                RN[u.bits.rn],
                RN[u.bits.rm],
                8*u.bits.rotate);
        }
    }
    return 4;
}


int parseCoprocRegTransfer(uint64_t pc, uint32_t instr,
                           char *mnemonic, size_t mnemonicsz,
                           char *comment, size_t commentsz) {
    CoprocessorTransferType u;
    u.value = instr;
    const char instr_name[2][16] = {"mcr     ", "mrc     "};
    RISCV_sprintf(mnemonic, mnemonicsz, "%s p%d,%d,%s,c%d,c%d,%d",
        instr_name[u.bits.L],
        u.bits.cp_num,
        u.bits.mode,
        RN[u.bits.rd],
        u.bits.crn,
        u.bits.crm,
        u.bits.cp_nfo
        );
    return 4;
}

int parseBranch(uint64_t pc, uint32_t instr,
                char *mnemonic, size_t mnemonicsz,
                char *comment, size_t commentsz) {
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
        RISCV_sprintf(mnemonic, mnemonicsz, "bl       %x",
            off);
    } else {
        RISCV_sprintf(mnemonic, mnemonicsz, "b%2s      %x",
            flags[u.bits.cond],
            off);
    }
    return 4;
}

int parseBranchExchange(uint64_t pc, uint32_t instr,
                        char *mnemonic, size_t mnemonicsz,
                        char *comment, size_t commentsz) {
    BranchType u;
    u.value = instr;

    RISCV_sprintf(mnemonic, mnemonicsz, "bx       %s",
            RN[u.bits.offset & 0xF]);
    return 4;
}

int parseBranchLinkExchange(uint64_t pc, uint32_t instr,
                            char *mnemonic, size_t mnemonicsz,
                            char *comment, size_t commentsz) {
    BranchExchangeIndirectType u;
    u.value = instr;

    RISCV_sprintf(mnemonic, mnemonicsz, "blx      %s",
        RN[u.bits.rm]);
    return 4;
}

int disasm_arm(int mode,
                uint64_t pc,
                uint8_t *data,
                int offset,
                char *mnemonic, size_t mnemonicsz,
                char *comment, size_t commentsz) {
    uint32_t instr;
    RISCV_sprintf(mnemonic, mnemonicsz, "%s", "unimpl");
    comment[0] = '\0';
    memcpy(&instr, &data[offset], 4);
    if (mode == THUMB_mode) {
        int ret = disasm_thumb(pc, instr, mnemonic, mnemonicsz);
        return ret;
    }


    if (instr == 0xFEDEFFE7) {
        RISCV_sprintf(mnemonic, mnemonicsz, "%s", "und");
        return 4;
    } else if ((instr & 0x0FB00000) == 0x03000000) {
        return parseUndefinedInstruction(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if ((instr & 0x0FD0F0F0) == 0x0710f010) {
        // sdiv ****0111_0001****_1111****_0001****
        // udiv ****0111_0011****_1111****_0001****
        return parseDivide(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if ((instr & 0x0F0000F0) == 0x90
        && (((instr >> 22) & 0x3F) == 0 || ((instr >> 23) & 0x1F) == 1)) {
        // mla  ****0000_001*****_********_1001****
        // mls  ****0000_0110****_********_1001****
        // mul  ****0000_000*****_0000****_1001****
        return parseMultiply(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 4) & 0x00FFFFFF) == 0x12FFF1) {
        return parseBranchExchange(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 4) & 0x00FFFFFF) == 0x12FFF3) {
        return parseBranchLinkExchange(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if ((instr & 0x0F0000F0) == 0x06000070) {
        // ARM V6 operation: uxtab, uxtb, uxah, uxth..
        return parseBytesExtending(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 26) & 0x3) == 0x1) {
        return parseSingleDataTransfer(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 25) & 0x7) == 0x4) {
        return parseBlockDataTransfer(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 24) & 0xF) == 0xE && (instr & 0x10)) {
        return parseCoprocRegTransfer(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 26) & 0x3) == 0x0) {
        return parseDataProcessing(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    } else if (((instr >> 25) & 0x7) == 0x5) {
        return parseBranch(pc, instr, mnemonic, mnemonicsz, comment, commentsz);
    }
    return 4;
}

}  // namespace debugger
