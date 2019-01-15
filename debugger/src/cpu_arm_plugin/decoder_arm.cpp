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
#include <api_core.h>
#include <iservice.h>

namespace debugger {

EIsaArmV7 decoder_arm(uint32_t ti, char *errmsg, size_t errsz) {
    EIsaArmV7 ret = ARMV7_Total;
    switch ((ti >> 26) & 0x3) {
    case 0:  // [27:26] == 2b'00
        if ((ti & 0x0FC000F0) == 0x00000090) {
            /** Multiply
                    ????0000_00??????_????????_1001????
             */
            if ((ti >> 21) & 1) {
                ret = ARMV7_MLA;
            } else {
                ret = ARMV7_MUL;
            }
        } else if ((ti & 0x0F8000F0) == 0x00800090) {
            /** Multiply Long
                    ????0000_1???????_????????_1001????
             */
            switch ((ti >> 21) & 3) {
            case 0:
                ret = ARMV7_UMULL;
                break;
            case 1:
                ret = ARMV7_UMLAL;
                break;
            case 2:
                ret = ARMV7_SMULL;
                break;
            case 3:
                ret = ARMV7_SMLAL;
                break;
            default:;
            }
        } else if ((ti & 0x0FB00FF0) == 0x01000090) {
            /** Single Data Swap
                    ????0001_0?00????_????0000_1001????
             */
            ret = ARMV7_SWP;
        } else if ((ti & 0x0FFFFFF0) == 0x012FFF10) {
            /** Branch and Exchange
                    ????0001_00101111_11111111_1001????
             */
            ret = ARMV7_BX;
        } else if ((ti & 0x0FFFFFF0) == 0x012FFF30) {
            /**
                    ????0001_00101111_11111111_1011????
             */
            ret = ARMV7_BLX;
        } else if ((ti & 0x0E400F90) == 0x00000090) {
            /** HWord Data transfer: regiser offset
                    ????000?_?0??????_????0000_1??1????
             */
            uint32_t L = (ti >> 20) & 0x1;
            if (L) {
                // Load
                switch ((ti >> 5) & 0x3) {
                case 0x1:
                    ret = ARMV7_LDRH;
                    break;
                case 0x2:
                    ret = ARMV7_LDRSB;
                    break;
                case 0x3:
                    ret = ARMV7_LDRSH;
                    break;
                default:
                    RISCV_sprintf(errmsg, errsz,
                        "undefined LD_reg opcode %08x", ti);
                }
            } else {
                // Store
                if ((ti >> 5) & 0x1) {
                    ret = ARMV7_STRH;
                } else {
                    RISCV_sprintf(errmsg, errsz,
                        "undefined ST_reg opcode %08x", ti);
                }
            }
        } else if ((ti & 0x0E0000F0) == 0x000000D0) {
            /** DWord Data transfer
                LDRD (imm)      ????000?_?1?0????_????????_1101????
                LDRD (literal)  ????0001_?1001111_????????_1101????
                LDRD (reg)      ????000?_?0?0????_????0000_1101????
             */
             ret = ARMV7_LDRD;
        } else if ((ti & 0x0E0000F0) == 0x000000F0) {
            /** DWord Store Data transfer
                STRD (imm)      ????000?_?1?0????_????????_1111????
                STRD (reg)      ????000?_?0?0????_????0000_1111????
             */
              ret = ARMV7_STRD;
        } else if ((ti & 0x0E400090) == 0x00400090) {
            /** HWord Data transfer: immediate offset
                    ????000?_?1??????_????????_1??1????
             */
            uint32_t L = (ti >> 20) & 0x1;
            if (L) {
                // Load
                switch ((ti >> 5) & 0x3) {
                case 0x1:
                    ret = ARMV7_LDRH;
                    break;
                case 0x2:
                    ret = ARMV7_LDRSB;
                    break;
                case 0x3:
                    ret = ARMV7_LDRSH;
                    break;
                default:
                    RISCV_sprintf(errmsg, errsz,
                        "undefined LD_imm opcode %08x", ti);
                }
            } else {
                // Store
                if ((ti >> 5) & 0x1) {
                    ret = ARMV7_STRH;
                } else {
                    RISCV_sprintf(errmsg, errsz,
                        "undefined ST_imm opcode %08x", ti);
                }
            }
        } else {
            /** Data Processing / PSR Transfer:
                     ????00??_????????_????????_????????
             */
            if ((ti & 0x0FFFFFFF) == 0x0320f000) {
                /** "NOP", "????0011_00100000_11110000_00000000" */
                ret = ARMV7_NOP;
            } else if ((ti & 0x0FBF0FFF) == 0x010F0000) {
                /** "MRS", "????0001_0?001111_????0000_00000000" */
                ret = ARMV7_MRS;
            } else if ((ti & 0x0FBFFFF0) == 0x0129f000) {
                /** "MSR", "????0001_0?101001_11110000_0000????" */
                ret = ARMV7_MSR;
            } else if ((ti & 0x0DB0F000) == 0x0120f000) {
                /** "MSR", "????00?1_0?101000_11110000_0000????" */
                uint32_t M1 = (ti >> 16) & 0xF;
                if (M1 == 0x8) {
                    ret = ARMV7_MSR;
                } else {
                    RISCV_sprintf(errmsg, errsz,
                        "MSR unsupported opcode %08x", ti);
                    ret = ARMV7_MSR;
                }
            } else {
                uint32_t I = (ti >> 25) & 0x1;
                uint32_t OpCode = (ti >> 21) & 0xF;
                uint32_t S = (ti >> 20) & 0x1;
                switch (OpCode) {
                case 0x0:
                    ret = ARMV7_AND;
                    break;
                case 0x1:
                    ret = ARMV7_EOR;
                    break;
                case 0x2:
                    ret = ARMV7_SUB;
                    break;
                case 0x3:
                    ret = ARMV7_RSB;
                    break;
                case 0x4:
                    ret = ARMV7_ADD;
                    break;
                case 0x5:
                    ret = ARMV7_ADC;
                    break;
                case 0x6:
                    ret = ARMV7_SBC;
                    break;
                case 0x7:
                    ret = ARMV7_RSC;
                    break;
                case 0x8:
                    if (I && !S) {
                        ret = ARMV7_MOVW;
                    } else {
                        ret = ARMV7_TST;
                    }
                    break;
                case 0x9:
                    ret = ARMV7_TEQ;
                    break;
                case 0xA:
                    if (I && !S) {
                        ret = ARMV7_MOVT;
                    } else {
                        ret = ARMV7_CMP;
                    }
                    break;
                case 0xB:
                    ret = ARMV7_CMN;
                    break;
                case 0xC:
                    ret = ARMV7_ORR;
                    break;
                case 0xD:
                    ret = ARMV7_MOV;
                    break;
                case 0xE:
                    ret = ARMV7_BIC;
                    break;
                case 0xF:
                    ret = ARMV7_MVN;
                    break;
                default:;
                }
            }
        }
        break;
    case 1:  // [27:26] == 2b'01
        if ((ti & 0x0E000010) == 0x06000010) {
            /** Undefined
                    ????011?_????????_????????_???1????
             */
            if ((ti & 0x0FD0F0F0) == 0x0710F010) {
                /*
                    "UDIV",    "????0111_0011????_1111????_0001????"
                    "SDIV",    "????0111_0001????_1111????_0001????"
                */
                uint32_t U = (ti >> 21) & 0x1;
                if (U) {
                    ret = ARMV7_UDIV;
                } else {
                    ret = ARMV7_SDIV;
                }
            } else if ((ti & 0x0FE00070) == 0x07C00010) {
                /* BFC      ????0111_110?????_????????_?0011111
                   BFI      ????0111_110?????_????????_?001????
                */
                if ((ti & 0xF) == 0xF) {
                    ret = ARMV7_BFC;
                } else {
                    ret = ARMV7_BFI;
                }
            } else if ((ti & 0x0FC000F0) == 0x06C00070) {
                /*
                    "UXTB16",  "????0110_11001111_????????_0111????"
                    "UXTAB16", "????0110_1100????_????????_0111????"
                    "UXTB",    "????0110_11101111_????????_0111????"
                    "UXTAB",   "????0110_1110????_????????_0111????"
                    "UXTH",    "????0110_11111111_????????_0111????"
                    "UXTAH",   "????0110_1111????_????????_0111????"
                */
                uint32_t rn = (ti >> 16) & 0xF;
                switch ((ti >> 20) & 0x3) {
                case 0x0:
                    if (rn == 0xF) {
                        ret = ARMV7_UXTB16;
                    } else {
                        ret = ARMV7_UXTAB16;
                    }
                    break;
                case 0x2:
                    if (rn == 0xF) {
                        ret = ARMV7_UXTB;
                    } else {
                        ret = ARMV7_UXTAB;
                    }
                    break;
                case 0x3:
                    if (rn == 0xF) {
                        ret = ARMV7_UXTH;
                    } else {
                        ret = ARMV7_UXTAH;
                    }
                    break;
                default:
                    RISCV_sprintf(errmsg, errsz,
                        "undefined UX* opcode %08x", ti);
                }
            } else {
                RISCV_sprintf(errmsg, errsz, "undefined opcode %08x", ti);
            }
        } else {
            /** Single Data Transfer */
            {
                uint32_t L = (ti >> 20) & 0x1;
                uint32_t B = (ti >> 22) & 0x1;
                if (L) {
                    if (B) {
                        ret = ARMV7_LDRB;
                    } else {
                        ret = ARMV7_LDR;
                    }
                } else {
                    if (B) {
                        ret = ARMV7_STRB;
                    } else {
                        ret = ARMV7_STR;
                    }
                }
            }
        }
        break;
    case 2:  // [27:26] == 2b'10
        if ((ti & 0x0E000000) == 0x08000000) {
            /** Block Data Transfer */
            uint32_t L = (ti >> 20) & 0x1;
            if (L) {
                ret = ARMV7_LDM;
            } else {
                ret = ARMV7_STM;
            }
        } else {
            /** Branch */
            if ((ti & 0x0F000000) == 0x0A000000) {
                ret = ARMV7_B;
            } else if ((ti & 0x0F000000) == 0x0B000000) {
                ret = ARMV7_BL;
                if ((ti >> 28) == 0xF) {
                    RISCV_sprintf(errmsg, errsz,
                        "BLX A2: sign extending %08x", ti);
                }
            } else {
                RISCV_sprintf(errmsg, errsz,
                    "undefined opcode %08x", ti);
            }
        }
        break;
    case 3:  // [27:26] == 2b'11
        if ((ti & 0x0E000000) == 0x0C000000) {
            /** Coprocessor Data transfer */
            RISCV_sprintf(errmsg, errsz,
                "CDP opcode %08x not implemented", ti);
        } else if ((ti & 0x0F000010) == 0x0E000000) {
            /** Coprocessor data operation */
            RISCV_sprintf(errmsg, errsz,
                "LDC/STC opcode %08x not implemented", ti);
        } else if ((ti & 0x0F000010) == 0x0E000010) {
            /** Coprocessor register transfer */
            uint32_t L = (ti >> 20) & 0x1;
            if (L) {
                ret = ARMV7_MRC;
            } else {
                ret = ARMV7_MCR;
            }
        } else {
            /** Software interrupt */
            ret = ARMV7_SWI;
        }
        break;
    default:;
    }
    return ret;
}

}  // debugger
