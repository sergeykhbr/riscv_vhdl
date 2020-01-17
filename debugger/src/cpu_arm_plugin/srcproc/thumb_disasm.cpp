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

namespace debugger {

const char CONDITION[][16] = {
    "eq", "ne", "cs", "cc",
    "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt",
    "gt", "le", "al", "-"
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


int disasm_thumb(uint64_t pc,
                 uint32_t ti,
                 char *disasm,
                 size_t sz) {
    int len = 2;
    int tsz;
    RISCV_sprintf(disasm, sz, "%s", "unknown");

    if ((ti & 0xFF87) == 0x4700) {
        uint32_t m = (ti >> 3) & 0xF;
        RISCV_sprintf(disasm, sz, "bx       %s",
                IREGS_NAMES[m]);
    } else if ((ti & 0xFF80) == 0xB080) {
        uint32_t imm = (ti & 0x7F) << 2;
        RISCV_sprintf(disasm, sz, "sub      sp, #%d",
                static_cast<int32_t>(imm));
    } else if ((ti & 0xFBE0) == 0xF040) {
        uint32_t ti1 = (ti >> 16);
        if ((ti1 & 0x8000) == 0) {
            // 4.6.91 ORR (immediate)
            uint32_t d = (ti1 >> 8) & 0xF;
            uint32_t n = ti  & 0xF;
            uint32_t i = (ti >> 10) & 1;
            uint32_t imm3 = (ti1 >> 12) & 0x7;
            uint32_t imm8 = ti1 & 0xFF;
            uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
            uint32_t imm32 = ThumbExpandImmWith(imm12);
            if (n == Reg_pc) {
                // 4.6.76 MOV (immediate), T2
                RISCV_sprintf(disasm, sz, "mov.w    %s, #%d",
                        IREGS_NAMES[d],
                        imm32);
            } else {
                RISCV_sprintf(disasm, sz, "orr.w    %s, %s, #%xh",
                        IREGS_NAMES[d],
                        IREGS_NAMES[n],
                        imm32);
            }
        }
        len = 4;
    } else if ((ti & 0xFBE0) == 0xF020) {
        uint32_t ti1 = (ti >> 16);
        if ((ti1 & 0x8000) == 0) {
            // 4.6.15 BIC (immediate)
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
        }
        len = 4;
    } else if ((ti & 0xFFC0) == 0x4280) {
        // 4.6.30 CMP (register)
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t n = ti & 0x3;
        RISCV_sprintf(disasm, sz, "cmp      %s, %s",
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFE00) == 0x1E00) {
        uint32_t d = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "sub      %s, %s, #%d",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                imm);
    } else if ((ti & 0xFE00) == 0x1A00) {
        //ret = T1_SUB_R;
        uint32_t d = ti  & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "sub      %s, %s, %s",
                IREGS_NAMES[d],
                IREGS_NAMES[n],
                IREGS_NAMES[m]);
    } else if ((ti & 0xFE00) == 0xBC00) {
        // 4.6.98 POP
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
    } else if ((ti & 0xFE00) == 0xB400) {
        // 4.6.99 PUSH
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
    } else if ((ti & 0xFE00) == 0x5800) {
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t m = (ti >> 6) & 0x7;
        RISCV_sprintf(disasm, sz, "ldr      %s, [%s, %s]",
            IREGS_NAMES[t],
            IREGS_NAMES[n],
            IREGS_NAMES[m]
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
    } else if ((ti & 0xF800) == 0x4800) {
        uint32_t imm = (ti & 0xFF) << 2;
        RISCV_sprintf(disasm, sz, "ldr      %s, [pc, #%d]",
            IREGS_NAMES[(ti >> 8) & 0x7],
            imm
            );
    } else if ((ti & 0xF800) == 0x2000) {
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        RISCV_sprintf(disasm, sz, "movs     %s, #%d",
            IREGS_NAMES[d],
            imm32
            );
    } else if ((ti & 0xF800) == 0xF000) {
        uint32_t ti2 = ti >> 16;
        if ((ti2 & 0xD000) == 0xD000) {
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
        } else if ((ti2 & 0xD000) == 0xC000) {
            // BLX call ARM routine
            len = 4;
        }
    } else if ((ti & 0xF000) == 0xD000) {
        if ((ti & 0x0F00) == 0xE) {
            // See permanently undefined space
        } else if ((ti & 0x0F00) == 0xE) {
            // See SVC (formely SWI)
        } else {
            uint32_t cond = (ti >> 8) & 0xF;
            uint32_t imm8 = ti & 0xFF;
            uint64_t imm32 = imm8 << 1;
            if (ti & 0x80) {
                imm32 |= (~0ull) << 9;
            }
            RISCV_sprintf(disasm, sz, "b%s      %08x",
                CONDITION[cond],
                static_cast<uint32_t>(pc) + 4 + imm32
                );
        }
    }
    return len;
}

}  // namespace debugger
