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

#include <api_core.h>
#include "thumb_disasm.h"
#include "../arm-isa.h"

namespace debugger {

const char CONDITION[][16] = {
    "eq", "ne", "cs", "cc",
    "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt",
    "gt", "le", "al", "-"
};

const char SHIFT_TYPE[][4] = {
    "-", "lsl", "lsr", "asr", "ror", "rrx"
};

const char *WBACK_SYMB[2] = {
    "", "!"
};

static uint32_t ThumbExpandImmWith(uint32_t imm12) {
    uint32_t imm32;
    uint32_t t8 = imm12 & 0xFF;
    if ((imm12 & 0xC00) == 0) {
        switch ((imm12 >> 8) & 0x3) {
        case 0:
            imm32 = t8;
            break;
        case 1:
            imm32 = (t8 << 16) | t8;
            break;
        case 2:
            imm32 = (t8 << 24) | (t8 << 8);
            break;
        case 3:
            imm32 = (t8 << 24) | (t8 << 16) | (t8 << 8) | t8;
            break;
        default:;
        }
    } else {
        uint32_t unrotated_value = (1 << 7) | (imm12 & 0x7F);
        uint32_t shift = (imm12 >> 7);
        // ror
        uint32_t lsr = unrotated_value >> shift;
        uint32_t lsl = unrotated_value << (32 - shift);
        imm32 = lsr | lsl;
    }
    return imm32;
}

const char *ImmShiftType(uint32_t type, uint32_t imm5) {
    switch (type) {
    case 0: return SHIFT_TYPE[1];
    case 1: return SHIFT_TYPE[2];
    case 2: return SHIFT_TYPE[3];
    case 3:
        if (imm5 == 0) {
            return SHIFT_TYPE[5];
        } else {
            return SHIFT_TYPE[4];
        }
    default:;
    }
    return SHIFT_TYPE[0];
}

uint32_t ImmShiftValue(uint32_t type, uint32_t imm5) {
    uint32_t ret = imm5;
    switch (type) {
    case 1:
        if (imm5 == 0) {
            ret = 32;
        }
        break;
    case 2:
        if (imm5 == 0) {
            ret = 32;
        }
        break;
    case 3:
        if (imm5 == 0) {
            ret = 1;
        }
        break;
    default:;
    }
    return ret;
}

int disasm_thumb(uint64_t pc,
                 uint32_t ti,
                 char *disasm,
                 size_t sz) {
    int len = 2;
    uint32_t ti1 = ti >> 16;
    int tsz;
    RISCV_sprintf(disasm, sz, "%s", "unknown");

    if ((ti & 0xFFFF) == 0xBF00) {
        RISCV_sprintf(disasm, sz, "%s", "nop");
    } else if ((ti & 0xFF87) == 0x4700) {
        uint32_t m = (ti >> 3) & 0xF;
        RISCV_sprintf(disasm, sz, "bx       %s",
                IREGS_NAMES[m]);
    } else if ((ti & 0xF0F0FFF0) == 0xF0F0FB90) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "sdiv     %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0xF0F0FFF0) == 0xF0F0FBB0) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "udiv     %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0xF0F0FFF0) == 0xF000FB00) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "mul.w    %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0xFFF0FFF0) == 0xF000E8D0) {
        uint32_t n = ti & 0xF;
        uint32_t m = (ti >> 16) & 0xF;
        RISCV_sprintf(disasm, sz, "tbb      %s, %s",
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0x8020FFF0) == 0x0000F3C0) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t lsbit = (imm3 << 2) | imm2;
        uint32_t widthm1 = ti1 & 0x1F;
        RISCV_sprintf(disasm, sz, "ubfx     %s, %s, #%d, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                lsbit, widthm1 + 1);
        len = 4;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F110) {
        // T1_CMN_I
        len = 4;
    } else if ((ti & 0x8F00FFF0) == 0x0F00EB10) {
        // T2_CMN_R
        len = 4;
    } else if ((ti & 0x8000FFEF) == 0x0000EB0D) {
        // T3_ADDSP_R
        len = 4;
    } else if ((ti & 0x8000FBEF) == 0x0000F10D) {
        // T3_ADDSP_I
        len = 4;
    } else if ((ti & 0x8000FBEF) == 0x0000F1AD) {
        // T2_SUBSP_I
        len = 4;
    } else if ((ti & 0x0800FFFF) == 0x0800F85F) {
        // T4_LDR_I: see LDR
        len = 4;
    } else if ((ti & 0x0F00FFF0) == 0x0E00F850) {
        // T4_LDR_I: see LDRT
        len = 4;
    } else if ((ti & 0x0D00FFF0) == 0x0800F850) {
        // T4_LDR_I: undefined
        len = 4;
    } else if ((ti & 0xF000FF7F) == 0xF000F81F) {
        // T3_PLD_I
        len = 4;
    } else if ((ti & 0xFF00FFF0) == 0xFC00F810) {
        // T2_PLD_I
        len = 4;
    } else if ((ti & 0xFFC0FFF0) == 0xF000F810) {
        // T1_PLD_R
        len = 4;
    } else if ((ti & 0x0FC0FF7F) == 0x0000F81F) {
        // T1_LDRB_L
        len = 4;
    } else if ((ti & 0x2000FFFF) == 0x0000E8BD) {
        // ldmia sp! equivalent
        uint32_t register_list = ti1 & 0xDFFF;
        int itotal = 0;
        tsz = RISCV_sprintf(disasm, sz, "pop.w    sp!, %s", "{");
        for (int i = 0; i < 16; i++) {
            if (ti1 & (1 << i)) {
                if (itotal) {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            ", %s", IREGS_NAMES[i]);
                } else {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            "%s", IREGS_NAMES[i]);
                }
                itotal++;
            }
        }
        tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                "%s", "}");
        len = 4;
    } else if ((ti & 0xF000FF7F) == 0xF000F91F) {
        // T3_PLI_I
        len = 4;
    } else if ((ti & 0xFF00FFF0) == 0xFC00F910) {
        // T2_PLI_I
        len = 4;
    } else if ((ti & 0xF000FFF0) == 0xF000F990) {
        // T1_PLI_I
        len = 4;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F800) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        RISCV_sprintf(disasm, sz, "strb.w   %s, [%s, %s, lsl #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                IREGS_NAMES[m],
                shift_n);
        len = 4;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F810) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        RISCV_sprintf(disasm, sz, "ldrb.w   %s, [%s, %s, lsl #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                IREGS_NAMES[m],
                shift_n);
        len = 4;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F840) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        RISCV_sprintf(disasm, sz, "str.w    %s, [%s, %s, lsl #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                IREGS_NAMES[m],
                shift_n);
        len = 4;
    } else if ((ti & 0x0F00FFF0) == 0x0E00F800) {
        // T1_STRBT
        len = 4;
    } else if ((ti & 0x0800FFF0) == 0x0800F800) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint64_t imm32 = ti1 & 0xFF;
        uint32_t index = (ti1 >> 10) & 1;
        uint32_t wback = (ti1 >> 8) & 1;
        RISCV_sprintf(disasm, sz, "strb.w   %s, [%s, #%d]%s",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                imm32,
                WBACK_SYMB[index & wback]);
        len = 4;
    } else if ((ti & 0x0800FFF0) == 0x0800F850) {
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFF;
        bool index = (ti1 >> 10) & 1;
        bool wback = (ti1 >> 8) & 1;

        if (index && !wback){
            RISCV_sprintf(disasm, sz, "ldr.w    %s, [%s, #%d]",
                    IREGS_NAMES[t], IREGS_NAMES[n], imm32);
        } else if (index && wback) {
            // pre-indexed
            RISCV_sprintf(disasm, sz, "ldr.w    %s, [%s, #%d]!",
                    IREGS_NAMES[t], IREGS_NAMES[n], imm32);
        } else if (!index && wback) {
            // post-indexed
            RISCV_sprintf(disasm, sz, "ldr.w    %s, [%s], #%d",
                    IREGS_NAMES[t], IREGS_NAMES[n], imm32);
        }
        len = 4;
    } else if ((ti & 0x0000FF7F) == 0x0000F85F) {
        uint32_t t = (ti1 >> 12) & 0xF;
        int32_t imm32 = static_cast<int32_t>(ti1 & 0xFFF);
        bool add = (ti >> 7) & 1;
        RISCV_sprintf(disasm, sz, "ldr.w    %s, [pc, #%d]",
                IREGS_NAMES[t], 
                add ? imm32: -imm32);
        len = 4;
    } else if ((ti & 0x0000FFF0) == 0x0000F8D0) {
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        RISCV_sprintf(disasm, sz, "ldr.w    %s, [%s, #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                imm32);
    } else if ((ti & 0x0FC0FFF0) == 0x0000F850) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int shift_n = (ti1 >> 4) & 0x3;
        RISCV_sprintf(disasm, sz, "ldr.w    %s, [%s, %s, lsl #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                IREGS_NAMES[m],
                shift_n);
        len = 4;
    } else if ((ti & 0x00F0FFF0) == 0x0000FBA0) {
        uint32_t dLo = (ti1 >> 12) & 0xF;
        uint32_t dHi = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "umull    %s, %s, %s, %s",
                IREGS_NAMES[dLo],
                IREGS_NAMES[dHi],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0xF0F0FFE0) == 0xF000FA00) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "lsl.w    %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0xF0F0FFE0) == 0xF000FA20) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        RISCV_sprintf(disasm, sz, "lsr.w    %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
        len = 4;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F010) {
        uint32_t ti0 = ti & 0xFFFF;
        uint32_t n = ti0  & 0xF;
        uint32_t i = (ti0 >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "tst.w    %s, #%xh",
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F1B0) {
        uint32_t n = ti & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "cmp.w    %s, #%xh",
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x2000FFD0) == 0x0000E890) {
        uint32_t n = ti & 0xF;
        uint32_t register_list = ti1 & 0xDFFF;
        uint32_t wback = (ti >> 5) & 1;
        int itotal = 0;
        tsz = RISCV_sprintf(disasm, sz, "ldmia.w  %s%s {",
            IREGS_NAMES[n],
            WBACK_SYMB[wback]);
        for (int i = 0; i < 16; i++) {
            if (ti1 & (1 << i)) {
                if (itotal) {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            ", %s", IREGS_NAMES[i]);
                } else {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            "%s", IREGS_NAMES[i]);
                }
                itotal++;
            }
        }
        tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                "%s", "}");
        len = 4;
    } else if ((ti & 0xA000FFD0) == 0x0000E900) {
        uint32_t n = ti & 0xF;
        uint32_t wback = (ti >> 5) & 1;
        int itotal = 0;
        tsz = RISCV_sprintf(disasm, sz, "stmdb    %s%s, {",
            IREGS_NAMES[n],
            WBACK_SYMB[wback]
            );
        for (int i = 0; i < 15; i++) {
            if (ti1 & (1 << i)) {
                if (itotal) {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            ", %s", IREGS_NAMES[i]);
                } else {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            "%s", IREGS_NAMES[i]);
                }
                itotal++;
            }
        }
        tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                "%s", "}");
        len = 4;
    } else if ((ti & 0x0000FFF0) == 0x0000F880) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint64_t imm32 = ti1 & 0xFFF;
        RISCV_sprintf(disasm, sz, "strb.w   %s, [%s, #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x0000FF7F) == 0x0000F91F) {
        // T1_LDRSB_L
        len = 4;
    } else if ((ti & 0x0000FFF0) == 0x0000F990) {
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        RISCV_sprintf(disasm, sz, "ldrsb.w  %s, [%s, #%d]",
                IREGS_NAMES[t],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FFE0) == 0x0000EB00) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        RISCV_sprintf(disasm, sz, "add.w    %s, %s, %s, %s #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m],
                ImmShiftType(type, (imm3 << 2) | imm2),
                ImmShiftValue(type, (imm3 << 2) | imm2));
        len = 4;
    } else if ((ti & 0x8000FBF0) == 0x0000F240) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm4 = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm32 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;
        RISCV_sprintf(disasm, sz, "movw     %s, #%xh",
                IREGS_NAMES[d],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBE0) == 0x0000F000) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "and.w    %s, #%xh",
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBE0) == 0x0000F100) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "add.w    %s, %s, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBE0) == 0x0000F1A0) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "sub.w    %s, %s, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBEF) == 0x0000F04F) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "mov.w    %s, #%d",
                IREGS_NAMES[d],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBE0) == 0x0000F040) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);
        RISCV_sprintf(disasm, sz, "orr.w    %s, %s, #%xh",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0x8000FBE0) == 0x0000F020) {
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWith(imm12);

        RISCV_sprintf(disasm, sz, "bic.w    %s, %s, #%xh",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm32);
        len = 4;
    } else if ((ti & 0xD000F800) == 0x8000F000) {
        uint32_t cond = (ti >> 6) & 0xF;
        uint32_t imm6 = ti & 0x3F;
        uint32_t imm11 = ti1 & 0x7FF;
        uint32_t S = (ti >> 10) & 1;
        uint32_t J1 = (ti1 >> 13) & 1;
        uint32_t J2 = (ti1 >> 11) & 1;
        uint32_t imm32 = (J2 << 19) | (J1 << 18) | (imm6 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 20;
        }
        RISCV_sprintf(disasm, sz, "b%s.n    %08x",
            CONDITION[cond],
            static_cast<uint32_t>(pc) + 4 + imm32
            );
        len = 4;
    } else if ((ti & 0xFFE8) == 0xB660) {
        uint32_t disable = (ti >> 4) & 1;
        uint32_t A = (ti >> 2) & 1;
        uint32_t I = (ti >> 1) & 1;
        uint32_t F = (ti >> 0) & 1;
        char iflags[4] = {0};
        char *effect[2] = {"e", "d"};
        int tcnt = 0;
        if (A) {
            iflags[tcnt++] = 'a';
        }
        if (I) {
            iflags[tcnt++] = 'i';
        }
        if (F) {
            iflags[tcnt++] = 'f';
        }
        RISCV_sprintf(disasm, sz, "cps%s%s    %s",
                iflags,
                effect[disable],
                iflags);
    } else if ((ti & 0xFF78) == 0x4468) {
        //ret = T1_ADDSP_R;
    } else if ((ti & 0xFF87) == 0x4485) {
        //ret = T2_ADDSP_R;
    } else if ((ti & 0xFF87) == 0x4780) {
        uint32_t m = (ti >> 3) & 0xF;
        RISCV_sprintf(disasm, sz, "blx      %s",
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x0000) {
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "mov      %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4000) {
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "ands     %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4040) {
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "eors     %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4080) {
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "lsls     %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x40C0) {
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "lsrs     %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4200) {
        uint32_t n = ti  & 0x7;
        uint32_t m = (ti >> 3)  & 0x7;
        RISCV_sprintf(disasm, sz, "tsts     %s, %s",
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4280) {
        uint32_t n = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "cmp      %s, %s",
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4300) {
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "orrs     %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0x4340) {
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t dm = ti & 0x7;
        RISCV_sprintf(disasm, sz, "mul      %s, %s, %s",
                IREGS_NAMES[dm],
                IREGS_NAMES[n],
                IREGS_NAMES[dm]);
    } else if ((ti & 0xFFC0) == 0x43C0) {
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "mvns     %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFFC0) == 0xB2C0) {
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        RISCV_sprintf(disasm, sz, "uxtb     %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFF80) == 0xB080) {
        uint32_t imm = (ti & 0x7F) << 2;
        RISCV_sprintf(disasm, sz, "sub      sp, #%d",
                static_cast<int32_t>(imm));
    } else if ((ti & 0xFF00) == 0x4400) {
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t DN = (ti >> 7) & 0x1;
        uint32_t dn = (DN << 3) | (ti & 0x7);
        RISCV_sprintf(disasm, sz, "add      %s, %s",
                IREGS_NAMES[dn],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFF00) == 0x4500) {
        uint32_t N = (ti >> 7) & 0x1;
        uint32_t n = (N << 3) | (ti  & 0x7);
        uint32_t m = (ti >> 3) & 0xF;
        RISCV_sprintf(disasm, sz, "cmp      %s, %s",
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFF00) == 0x4600) {
        uint32_t D = (ti >> 7) & 1;
        uint32_t d = (D << 3) | (ti & 0x7);
        uint32_t m = (ti >> 3) & 0xF;
        RISCV_sprintf(disasm, sz, "mov      %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFF00) == 0xBF00) {
        uint32_t firstcond = (ti >> 4) & 0xF;
        uint32_t mask = ti & 0xF;
        RISCV_sprintf(disasm, sz, "it%x      %s",
                mask,
                CONDITION[firstcond]);
    } else if ((ti & 0xFE00) == 0x1800) {
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "add      %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFE00) == 0x1A00) {
        uint32_t d = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "sub      %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFE00) == 0x1C00) {
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "adds     %s, %s, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm32);
    } else if ((ti & 0xFE00) == 0x1E00) {
        uint32_t d = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "sub      %s, %s, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm);
    } else if ((ti & 0xFE00) == 0x5000) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "str      %s, [%s, %s]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            IREGS_NAMES[m]
            );
    } else if ((ti & 0xFE00) == 0x5800) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "ldr      %s, [%s, %s]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            IREGS_NAMES[m]
            );
    } else if ((ti & 0xFE00) == 0x5C00) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "ldrb     %s, [%s, %s]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            IREGS_NAMES[m]
            );
    } else if ((ti & 0xFE00) == 0xB400) {
        int itotal = 0;
        tsz = RISCV_sprintf(disasm, sz, "push     %s", "{");
        for (int i = 0; i < 8; i++) {
            if (ti & (1 << i)) {
                if (itotal) {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            ", %s", IREGS_NAMES[i]);
                } else {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            "%s", IREGS_NAMES[i]);
                }
                itotal++;
            }
        }
        if (ti & 0x100) {
            if (itotal) {
                tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                        ", %s}", IREGS_NAMES[Reg_lr]);
            } else {
                tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                        "%s}", IREGS_NAMES[Reg_lr]);
            }
        } else {
            tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                    "%s", "}");
        }
    } else if ((ti & 0xFE00) == 0xBC00) {
        int itotal = 0;
        tsz = RISCV_sprintf(disasm, sz, "pop      %s", "{");
        for (int i = 0; i < 8; i++) {
            if (ti & (1 << i)) {
                if (itotal) {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            ", %s", IREGS_NAMES[i]);
                } else {
                    tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                            "%s", IREGS_NAMES[i]);
                }
                itotal++;
            }
        }
        if (ti & 0x100) {
            if (itotal) {
                tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                        ", %s}", IREGS_NAMES[Reg_pc]);
            } else {
                tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                        "%s}", IREGS_NAMES[Reg_pc]);
            }
        } else {
            tsz += RISCV_sprintf(&disasm[tsz], sz - tsz,
                    "%s", "}");
        }
    } else if ((ti & 0xFD00) == 0xB900) {
        // 4.6.22 CBNZ
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint64_t imm32 = (i << 6) | (imm5 << 1);
        RISCV_sprintf(disasm, sz, "cbnz     %s, %08x",
            IREGS_NAMES[n],
            static_cast<uint32_t>(pc) + 4 + imm32
            );
    } else if ((ti & 0xFD00) == 0xB100) {
        // 4.6.23 CBZ
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint64_t imm32 = (i << 6) | (imm5 << 1);
        RISCV_sprintf(disasm, sz, "cbz      %s, %08x",
            IREGS_NAMES[n],
            static_cast<uint32_t>(pc) + 4 + imm32
            );
    } else if ((ti & 0xF800) == 0x0000) {
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t imm5 = (ti >> 6) & 0x1F;
        RISCV_sprintf(disasm, sz, "lsls     %s, %s, #%d",
            IREGS_NAMES[d],
            IREGS_NAMES[m],
            imm5
            );
    } else if ((ti & 0xF800) == 0x0800) {
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t imm5 = (ti >> 6) & 0x1F;
        RISCV_sprintf(disasm, sz, "lsrs     %s, %s, #%d",
            IREGS_NAMES[d],
            IREGS_NAMES[m],
            imm5
            );
    } else if ((ti & 0xF800) == 0x1000) {
        uint32_t d = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t imm5 = (ti >> 6) & 0x1F;
        RISCV_sprintf(disasm, sz, "asrs     %s, %s, #%d",
            IREGS_NAMES[d],
            IREGS_NAMES[m],
            imm5
            );
    } else if ((ti & 0xF800) == 0x2000) {
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        RISCV_sprintf(disasm, sz, "movs     %s, #%d",
            IREGS_NAMES[d],
            imm32
            );
    } else if ((ti & 0xF800) == 0x2800) {
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        RISCV_sprintf(disasm, sz, "cmp      %s, #%d",
            IREGS_NAMES[d],
            imm32
            );
    } else if ((ti & 0xF800) == 0x3000) {
        uint32_t dn = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        RISCV_sprintf(disasm, sz, "adds     %s, #%d",
            IREGS_NAMES[dn],
            imm32
            );
    } else if ((ti & 0xF800) == 0x3800) {
        uint32_t dn = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        RISCV_sprintf(disasm, sz, "subs     %s, #%d",
            IREGS_NAMES[dn],
            imm32
            );
    } else if ((ti & 0xF800) == 0x4800) {
        uint32_t imm = (ti & 0xFF) << 2;
        RISCV_sprintf(disasm, sz, "ldr      %s, [pc, #%d]",
            IREGS_NAMES[(ti >> 8) & 0x7],
            imm
            );
    } else if ((ti & 0xF800) == 0x6000) {
        uint32_t t = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm = ((ti >> 6) & 0x1F) << 2;
        RISCV_sprintf(disasm, sz, "str      %s, [%s, #%d]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            imm
            );
    } else if ((ti & 0xF800) == 0x6800) {
        uint32_t t = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm = ((ti >> 6) & 0x1F) << 2;
        RISCV_sprintf(disasm, sz, "ldr      %s, [%s, #%d]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            imm
            );
    } else if ((ti & 0xF800) == 0x7000) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = (ti >> 6) & 0x1F;
        RISCV_sprintf(disasm, sz, "strb     %s, [%s, #%d]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            imm32
            );
    } else if ((ti & 0xF800) == 0x7800) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x1F;
        RISCV_sprintf(disasm, sz, "ldrb     %s, [%s, #%d]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            imm32
            );
    } else if ((ti & 0xF800) == 0x8000) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = ((ti >> 6) & 0x1F) << 1;
        RISCV_sprintf(disasm, sz, "strh     %s, [%s, #%d]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            imm32
            );
    } else if ((ti & 0xF800) == 0x9800) {
        uint32_t t = (t >> 8) & 0x7;
        uint64_t imm32 = (ti & 0xFF) << 2;
        RISCV_sprintf(disasm, sz, "ldr      %s, [sp, #%d]",
            IREGS_NAMES[t],
            imm32
            );
    } else if ((ti & 0xF800) == 0xA000) {
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        RISCV_sprintf(disasm, sz, "adr      %s, pc, #%d",
            IREGS_NAMES[d],
            imm32
            );
    } else if ((ti & 0xF800) == 0xA800) {
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        RISCV_sprintf(disasm, sz, "add      %s, sp, #%d",
            IREGS_NAMES[d],
            imm32
            );
    } else if ((ti & 0xF800) == 0xE000) {
        uint32_t imm11 = ti & 0x7FF;
        uint64_t imm32 = imm11 << 1;
        if (ti & 0x400) {
            imm32 |= (~0ull) << 12;
        }
        RISCV_sprintf(disasm, sz, "b.n      %08x",
            static_cast<uint32_t>(pc) + 4 + imm32
            );
    } else if ((ti & 0xD000F800) == 0xD000F000) {
        // 4.6.18 BL, BLX
        uint32_t instr0 = ti & 0xFFFF;
        uint32_t instr1 = ti >> 16;
        uint32_t S = (instr0 >> 10) & 1;
        uint32_t I1 = (((instr1 >> 13) & 0x1) ^ S) ^ 1;
        uint32_t I2 = (((instr1 >> 11) & 0x1) ^ S) ^ 1;
        uint32_t imm11 = instr1 & 0x7FF;
        uint32_t imm10 = instr0 & 0x3FF;
        uint32_t imm = 
        imm = (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
        if (S) {
            imm |= (~0ul) << 24;
        }
        RISCV_sprintf(disasm, sz, "bl       %08x",
            static_cast<uint32_t>(pc) + 4 + imm
            );
        len = 4;
    } else if ((ti & 0xFF00) == 0xDE00) {
        // See permanently undefined space
    } else if ((ti & 0xFF00) == 0xDF00) {
        // See SVC (formely SWI)
    } else if ((ti & 0xF000) == 0xD000) {
        uint32_t cond = (ti >> 8) & 0xF;
        uint32_t imm8 = ti & 0xFF;
        uint64_t imm32 = imm8 << 1;
        if (ti & 0x80) {
            imm32 |= (~0ull) << 9;
        }
        RISCV_sprintf(disasm, sz, "b%s.n    %08x",
            CONDITION[cond],
            static_cast<uint32_t>(pc) + 4 + imm32
            );
    }
    return len;
}

}  // namespace debugger
