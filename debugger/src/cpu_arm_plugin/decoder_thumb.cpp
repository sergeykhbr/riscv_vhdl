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

EIsaArmV7 decoder_thumb(uint32_t ti, uint32_t *tio,
                         char *errmsg, size_t errsz) {
    EIsaArmV7 ret = ARMV7_Total;
    switch ((ti >> 13) & 0x7) {
    case 0:  // [15:13] == 3b'000
        /** 
           000?_????_????_???? - Shift by immediate
           0001_10??_????_???? - Add/subtract register
           0001_11??_????_???? - Add/subtract immediate
        */
        if ((ti & 0xFC00) == 0x1800) {
            DataProcessingType op;
            op.value = 0;
            op.reg_bits.rm = (ti >> 6) & 0x7;
            op.reg_bits.rd = ti & 0x7;
            op.reg_bits.rn = (ti >> 3) & 0x7;
            op.reg_bits.S = 1;
            op.reg_bits.cond = 0xE;
            if (((ti >> 9) & 0x1) == 0) {
                op.reg_bits.opcode = 0x4;
                ret = ARMV7_ADD;
            } else {
                op.reg_bits.opcode = 0x2;
                ret = ARMV7_SUB;        // SUB(3)
            }
            *tio = op.value;
        } else if ((ti & 0xFC00) == 0x1C00) {
            DataProcessingType op;
            op.value = 0;
            op.imm_bits.imm = (ti >> 6) & 0x7;
            op.imm_bits.rd = ti & 0x7;
            op.imm_bits.rn = (ti >> 3) & 0x7;
            op.imm_bits.S = 1;
            op.imm_bits.I = 1;
            op.imm_bits.cond = 0xE;
            if (((ti >> 9) & 0x1) == 0) {
                op.imm_bits.opcode = 0x4;
                ret = ARMV7_ADD;    // ADD(1)/SUB(1)/MOV(2)
            } else {
                op.imm_bits.opcode = 0x2;
                ret = ARMV7_SUB;
            }
            *tio = op.value;
        } else {
            DataProcessingType op;
            op.value = 0;
            op.reg_bits.rm = (ti >> 3) & 0x7;
            op.reg_bits.shift = (ti >> 6) & 0x1f;
            op.reg_bits.rd = ti & 0x7;
            op.reg_bits.S = 1;
            op.reg_bits.opcode = 0xD;
            op.reg_bits.cond = 0xE;
            switch ((ti >> 11) & 0x3) {
            case 0:
                op.reg_bits.sh_type = 0x0;
                ret = ARMV7_MOV;  // LSL(1)
                break;
            case 1:
                op.reg_bits.sh_type = 0x1;
                ret = ARMV7_MOV;  // LSR(1)
                break;
            case 2:
                op.reg_bits.sh_type = 0x2;
                ret = ARMV7_MOV;  // ASR(1)
                break;
            case 3:
                RISCV_sprintf(errmsg, errsz,
                    "undefined instruction %04x", ti & 0xFFFF);
                break;
            }
            *tio = op.value;
        }
        break;
    case 1:  // [15:13] == 3b'001
        /** Add/subtract/compare/move immediate
                001?_????_????_????
         */
        DataProcessingType op;
        op.value = 0;
        op.imm_bits.imm = ti & 0xFF;
        op.imm_bits.rd = (ti >> 8) & 0x7;
        op.imm_bits.rn = op.imm_bits.rd;
        op.imm_bits.S = 1;
        op.imm_bits.I = 1;
        op.imm_bits.cond = 0xE;
        switch ((ti >> 11) & 0x3) {
        case 0:
            op.imm_bits.rn = 0;
            op.imm_bits.opcode = 0xD;
            ret = ARMV7_MOV;        // MOV(1)
            break;
        case 1:
            op.imm_bits.rd = 0;
            op.imm_bits.opcode = 0xA;
            ret = ARMV7_CMP;
            break;
        case 2:
            op.imm_bits.opcode = 0x4;
            ret = ARMV7_ADD;
            break;
        case 3:
            op.imm_bits.opcode = 0x2;
            ret = ARMV7_SUB;        // SUB(2)
            break;
        }
        *tio = op.value;
        break;
    case 2:  // [15:13] == 3b'010
        /** 
           0100_00??_????_???? - Data-processing register
           0100_01??_????_???? - Special data processing
           0100_0111_????_???? - Branch/Exchange instruction set
           0100_1???_????_???? - Load from literal pool
           0101_????_????_???? - Load/Store register offset
        */
        if ((ti & 0xFC00) == 0x4000) {
            DataProcessingType op;
            op.value = 0;
            op.reg_bits.S = 1;
            op.reg_bits.opcode = (ti >> 6) & 0xF;
            op.reg_bits.cond = 0xE;
            switch (op.reg_bits.opcode) {
            case 0x0:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_AND;
                break;
            case 0x1:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_EOR;
                break;
            case 0x2:
                op.reg_bits.rm = ti & 0x7;
                op.reg_bits.sh_sel = 1;
                op.reg_bits.sh_type = 0x0;
                op.imm_bits.rotate = (ti >> 3) & 0x7;  // rs
                op.imm_bits.rd = ti & 0x7;
                op.imm_bits.rn = 0;
                op.imm_bits.opcode = 0xD;
                ret = ARMV7_MOV;    // LSL(2)
                break;
            case 0x3:
                op.reg_bits.rm = ti & 0x7;
                op.reg_bits.sh_sel = 1;
                op.reg_bits.sh_type = 0x1;
                op.imm_bits.rotate = (ti >> 3) & 0x7;  // rs
                op.imm_bits.rd = ti & 0x7;
                op.imm_bits.rn = 0;
                op.imm_bits.opcode = 0xD;
                ret = ARMV7_MOV;    // LSR(2)
                break;
            case 0x4:
                op.reg_bits.rm = ti & 0x7;
                op.reg_bits.sh_sel = 1;
                op.reg_bits.sh_type = 0x2;
                op.imm_bits.rotate = (ti >> 3) & 0x7;  // rs
                op.imm_bits.rd = ti & 0x7;
                op.imm_bits.rn = 0;
                op.imm_bits.opcode = 0xD;
                ret = ARMV7_MOV;    // ASR(2)
                break;
            case 0x5:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_ADC;
                break;
            case 0x6:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_SBC;
                break;
            case 0x7:
                op.reg_bits.rm = ti & 0x7;
                op.reg_bits.sh_sel = 1;
                op.reg_bits.sh_type = 0x3;
                op.imm_bits.rotate = (ti >> 3) & 0x7;  // rs
                op.imm_bits.rd = ti & 0x7;
                op.imm_bits.rn = 0;
                op.imm_bits.opcode = 0xD;
                ret = ARMV7_MOV;    // ROR
                break;
            case 0x8:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.imm_bits.rd = 0;
                op.imm_bits.rn = ti & 0x7;
                ret = ARMV7_TST;
                break;
            case 0x9:
                op.reg_bits.rm = 0;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = (ti >> 3) & 0x7;
                op.reg_bits.opcode = 0x3;
                op.reg_bits.I = 1;
                ret = ARMV7_RSB;    // NEG
                break;
            case 0xA:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = 0;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_CMP;
                break;
            case 0xB:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = 0;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_CMN;
                break;
            case 0xC:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_ORR;
                break;
            case 0xD:
                {
                    MulType mulop;
                    mulop.value = 0xE0100090;
                    mulop.bits.rm = (ti >> 3) & 0x7;
                    mulop.bits.rs = ti & 0x7; // rd
                    mulop.bits.rn = 0;
                    mulop.bits.rd = ti & 0x7;
                    op.value = mulop.value;
                }
                ret = ARMV7_MUL;
                break;
            case 0xE:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = op.reg_bits.rd;
                ret = ARMV7_BIC;
                break;
            case 0xF:
                op.reg_bits.rm = (ti >> 3) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = 0;
                ret = ARMV7_MVN;
                break;
            default:;
            }
            *tio = op.value;
        } else if ((ti & 0xFF00) == 0x4700) {
            BranchExchangeIndirectType op;
            op.value = 0;
            op.bits.rm = (ti >> 3) & 0xF;
            op.bits.SBO = ~0ul;
            op.bits.opcode2 = 0x12;
            op.bits.cond = 0xE;
            if ((ti & 0x87) == 0x80) {
                // BLX(2) for ARMv5T and above
                op.bits.opcode0 = 0x3;
                ret = ARMV7_BLX;
            } else if ((ti & 0x87) == 0x00) {
                // BX all Thumb types
                op.bits.opcode0 = 0x1;
                ret = ARMV7_BX;
            }
            *tio = op.value;
        } else if ((ti & 0xFC00) == 0x4400) {
            uint32_t h1 = ((ti >> 7) & 1) << 3;
            uint32_t h2 = ((ti >> 6) & 1) << 3;
            DataProcessingType op;
            op.value = 0;
            op.reg_bits.rm = h2 | ((ti >> 3) & 0x7);
            op.reg_bits.rd = h1 | (ti & 0x7);
            op.reg_bits.rn = op.imm_bits.rd;
            op.reg_bits.cond = 0xE;
            switch ((ti >> 8) & 0x3) {
            case 0:  // ADD(4)
                op.imm_bits.opcode = 0x4;
                ret = ARMV7_ADD;
                break;
            case 1:  // CMP(3)
                op.reg_bits.rd = 0;
                op.reg_bits.S = 1;
                op.imm_bits.opcode = 0xA;
                ret = ARMV7_CMP;
                break;
            case 2:  // CPY/MOV(3)
                op.reg_bits.rn = 0;
                op.imm_bits.opcode = 0xD;
                ret = ARMV7_MOV;
                break;
            case 3:
                RISCV_sprintf(errmsg, errsz,
                    "undefined instruction %04x", ti & 0xFFFF);
                break;
            }
            *tio = op.value;
        } else if ((ti & 0xF800) == 0x4800) {
            // LDR(3)
            SingleDataTransferType op;
            op.value = 0;
            op.imm_bits.imm = (ti & 0xFF) << 2;
            op.reg_bits.rd = (ti >> 8) & 0x7;
            op.reg_bits.rn = Reg_pc;
            op.reg_bits.L = 1;
            op.reg_bits.U = 1;
            op.reg_bits.P = 1;
            op.reg_bits.zeroone = 0x1;
            op.reg_bits.cond = 0xE;
            *tio = op.value;
        } else {
            /**
               STR(2)    0101_000?_????_????
               LDR(2)    0101_100?_????_????
               STRH(2)   0101_001?_????_????
               LDRH(2)   0101_101?_????_????
               STRB(2)   0101_010?_????_????
               LDRB(2)   0101_110?_????_????
               LDRSB     0101_011?_????_????
               LDRSH     0101_111?_????_????
            */
            SingleDataTransferType op;
            op.value = 0;
            if ((ti & 0xF600) == 0x5000) {
                const EIsaArmV7 MTYPE[2] = {ARMV7_STR, ARMV7_LDR};
                op.reg_bits.rm = (ti >> 6) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = (ti >> 3) & 0x7;
                op.reg_bits.L = (ti >> 11) & 0x1;
                op.reg_bits.U = 1;
                op.reg_bits.P = 1;
                op.reg_bits.I = 1;                  // BUG? in spec?
                op.reg_bits.zeroone = 0x1;
                op.reg_bits.cond = 0xE;
                ret = MTYPE[op.reg_bits.L];        // LDR(2)/STR(2)
            } else if ((ti & 0xF600) == 0x5200) {
                const EIsaArmV7 MTYPE[2] = {ARMV7_STRH, ARMV7_LDRH};
                op.reg_bits.rm = (ti >> 6) & 0x7;
                op.reg_bits.sh_sel = 0xB;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = (ti >> 3) & 0x7;
                op.reg_bits.L = (ti >> 11) & 0x1;
                op.reg_bits.U = 1;
                op.reg_bits.P = 1;
                op.reg_bits.zeroone = 0x1;
                op.reg_bits.cond = 0xE;
                ret = MTYPE[op.reg_bits.L];        // LDRH(2)/STRH(2)
            } else if ((ti & 0xF600) == 0x5400) {
                const EIsaArmV7 MTYPE[2] = {ARMV7_STRB, ARMV7_LDRB};
                op.reg_bits.rm = (ti >> 6) & 0x7;
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = (ti >> 3) & 0x7;
                op.reg_bits.L = (ti >> 11) & 0x1;
                op.reg_bits.B = 1;
                op.reg_bits.U = 1;
                op.reg_bits.P = 1;
                op.reg_bits.I = 1;                  // BUG? in spec p.572.
                op.reg_bits.zeroone = 0x1;
                op.reg_bits.cond = 0xE;
                ret = MTYPE[op.reg_bits.L];        // LDRB(2)/STRB(2)
            } else if ((ti & 0xF600) == 0x5600) {
                const EIsaArmV7 MTYPE[2] = {ARMV7_LDRSB, ARMV7_LDRSH};
                uint32_t b = (ti >> 11) & 0x1;
                op.reg_bits.rm = (ti >> 6) & 0x7;
                op.reg_bits.sh_sel = 0xD | (ti << 1);
                op.reg_bits.rd = ti & 0x7;
                op.reg_bits.rn = (ti >> 3) & 0x7;
                op.reg_bits.L = 1;
                op.reg_bits.U = 1;
                op.reg_bits.P = 1;
                op.reg_bits.cond = 0xE;
                ret = MTYPE[b];                     // LDRSB/LDRSH
            }
            *tio = op.value;
        }
    case 3:  // [15:13] == 3b'011
        /** 
           011?_????_????_???? - Load/Store word/byte immediate offset
           LDR(1)  0110_1???_????_????
           STR(1)  0110_0???_????_????
           LDRB(1) 0111_1???_????_????
           STRB(1) 0111_0???_????_????
        */
        {
            SingleDataTransferType op;
            op.value = 0;
            op.imm_bits.imm = (ti >> 6) & 0x1F;
            op.imm_bits.rd = ti & 0x7;
            op.imm_bits.rn = (ti >> 3) & 0x7;
            op.imm_bits.L = (ti >> 11) & 0x1;
            op.imm_bits.U = 1;
            op.imm_bits.P = 1;
            op.imm_bits.zeroone = 0x1;
            op.imm_bits.cond = 0xE;
            if ((ti & (1 << 12)) == 0) {
                // STR(1)/LDR(1)
                const EIsaArmV7 MTYPE[2] = {ARMV7_STR, ARMV7_LDR};
                op.imm_bits.imm <<= 2;
                ret = MTYPE[op.imm_bits.L];
            } else {
                // STRB(1)/LDRB(1)
                const EIsaArmV7 MTYPE[2] = {ARMV7_STRB, ARMV7_LDRB};
                op.imm_bits.B = 1;
                ret = MTYPE[op.imm_bits.L];
            }
            *tio = op.value;
        }
        break;
    case 4:  // [15:13] == 3b'100
        /** 
           1000_????_????_???? - Load/Store halfword immediate offset
           1001_????_????_???? - Load/Store to/from stack
        */
        if ((ti & 0xF000) == 0x8000) {
            // STRH(1)/LDRH(1)
            const EIsaArmV7 MTYPE[2] = {ARMV7_STRH, ARMV7_LDRH};
            SingleDataTransferType op;
            op.value = 0;
            uint32_t imm2_0 = (ti >> 6) & 0x7;
            uint32_t imm4_3 = (ti >> 9) & 0x3;
            op.imm_bits.imm = (imm4_3 << 8) | 0xB0 | (imm2_0 << 1);
            op.imm_bits.rd = ti & 0x7;
            op.imm_bits.rn = (ti >> 3) & 0x7;
            op.imm_bits.L = (ti >> 11) & 0x1;
            op.imm_bits.B = 1;
            op.imm_bits.U = 1;
            op.imm_bits.P = 1;
            op.imm_bits.cond = 0xE;
            ret = MTYPE[op.imm_bits.L];
            *tio = op.value;
        } else {
            if (ti & (1 << 12)) {
                // LDR(4)/STR(3)
                const EIsaArmV7 MTYPE[2] = {ARMV7_STR, ARMV7_LDR};
                SingleDataTransferType op;
                op.value = 0;
                op.imm_bits.imm = (ti & 0xFF) << 2;
                op.imm_bits.rd = (ti >> 8) & 0x7;
                op.imm_bits.rn = Reg_sp;
                op.imm_bits.L = (ti >> 11) & 0x1;
                op.imm_bits.U = 1;
                op.imm_bits.P = 1;
                op.imm_bits.zeroone = 0x1;
                op.imm_bits.cond = 0xE;
                ret = MTYPE[op.imm_bits.L];
                *tio = op.value;
            } else {
                RISCV_sprintf(errmsg, errsz,
                    "undefined instruction %04x", ti & 0xFFFF);
            }
        }
        break;
    case 5:  // [15:13] == 3b'101
        /** 
           1010_????_????_???? - Add to SP or PC
           1011_????_????_???? - Miscellaneous
        */
        if ((ti & 0xF000) == 0xA000) {
            DataProcessingType op;
            op.value = 0;
            op.imm_bits.imm = ti & 0xFF;
            op.imm_bits.rotate = 0xF;
            op.imm_bits.rd = (ti >> 8) & 0x7;
            op.imm_bits.I = 1;
            op.imm_bits.cond = 0xE;
            op.imm_bits.opcode = 0x4;
            ret = ARMV7_ADD;
            if (((ti >> 11) & 0x1) == 0) {
                op.imm_bits.rn = Reg_pc;
            } else {
                op.imm_bits.rn = Reg_sp;
            }
            *tio = op.value;
        } else {
            /**
               1011_0000_0???_???? - Add to SP imm7*4
               1011_0000_1???_???? - Sub from SP imm7*4
               1011_1110_????_???? - Sub from SP imm7*4
            */
            if ((ti & 0xFF00) == 0xB000) {
                DataProcessingType op;
                op.value = 0;
                op.imm_bits.imm = ti & 0x7F;
                op.imm_bits.rotate = 0xF;
                op.imm_bits.rd = Reg_sp;
                op.imm_bits.rn = Reg_sp;
                op.imm_bits.I = 1;
                op.imm_bits.cond = 0xE;
                if ((ti >> 7) & 0x1) {
                    op.imm_bits.opcode = 0x2;
                    ret = ARMV7_SUB;        // SUB(4)
                } else {
                    op.imm_bits.opcode = 0x4;
                    ret = ARMV7_ADD;
                }
                *tio = op.value;
            } else if ((ti & 0xFFE8) == 0xB660) {
                RISCV_sprintf(errmsg, errsz,
                    "ARMv5T CPS %04x not implemented", ti & 0xFFFF);
            } else if ((ti & 0xFF00) == 0xBE00) {
                RISCV_sprintf(errmsg, errsz,
                    "instruction BKPT %04x not implemented", ti & 0xFFFF);
            } else if ((ti & 0xFF00) == 0xB200) {
                SignExtendType op;
                switch ((ti >> 6) & 0x3) {
                case 0:
                    RISCV_sprintf(errmsg, errsz,
                        "instruction SXTH %04x not implemented", ti & 0xFFFF);
                    break;
                case 1:
                    RISCV_sprintf(errmsg, errsz,
                        "instruction SXTB %04x not implemented", ti & 0xFFFF);
                    break;
                case 2:
                    op.value = 0xE6FF0070;
                    op.bits.rm = (ti >> 3) & 0x7;
                    op.bits.rd = ti & 0x7;
                    *tio = op.value;
                    ret = ARMV7_UXTH;
                    break;
                case 3:
                    op.value = 0xE6EF0070;
                    op.bits.rm = (ti >> 3) & 0x7;
                    op.bits.rd = ti & 0x7;
                    *tio = op.value;
                    ret = ARMV7_UXTB;
                    break;
                }
            } else {
                RISCV_sprintf(errmsg, errsz,
                    "undefined instruction %04x", ti & 0xFFFF);
            }
        }
        break;
    case 6:  // [15:13] == 3b'110
        /** 
           1100_????_????_???? - Load/Store multiple
           1101_????_????_???? - Conditional branch
           1101_1110_????_???? - Undefined instruction
           1101_1111_????_???? - Software interrupt
        */
        if ((ti & 0xFF00) == 0xDF00) {
            *tio = 0xEF000000;
            *tio |= (ti & 0xFF);
            ret = ARMV7_SWI;
        } else if ((ti & 0xFF00) == 0xDE00) {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        } else if ((ti & 0xF000) == 0xD000) {
            BranchType op;
            op.value = 0;
            op.bits.offset = ti & 0xFF;
            if (ti & 0x80) {
                op.bits.offset |= 0xFFFF00;
            }
            op.bits.opcode = 0x5;
            op.bits.cond = (ti >> 8) & 0xF;
            ret = ARMV7_B;
            *tio = op.value;
        } else {
            BlockDataTransferType op;
            op.value = 0;
            op.bits.reglist = ti & 0xFF;
            op.bits.rn = (ti >> 8) & 0x7;
            op.bits.U = 1;
            op.bits.b27_25 = 0x4;
            op.bits.cond = 0xE;
            if ((ti >> 11) & 0x1) {
                if (op.bits.reglist & (1 << op.bits.rn)) {
                    op.bits.W = 1;
                }
                op.bits.L = 1;
                ret = ARMV7_LDM;
            } else {
                op.bits.W = 1;
                ret = ARMV7_STM;
            }
            *tio = op.value;
        }
        break;
    case 7:  // [15:13] == 3b'111
        /** 
           1110_0???_????_???? - Unconditional branch
           1110_1???_????_???0 - BLX suffix
           1110_1???_????_???1 - Undefined instruction
           1111_0???_????_???? - BL/BLX prefix
           1111_1???_????_???? - BL suffix
        */
        if ((ti & 0xF800) == 0xE000) {
            BranchType op;
            op.value = 0;
            op.bits.offset = ti & 0x7FF;
            if (ti & 0x400) {
                op.bits.offset |= 0xFFF800;
            }
            op.bits.opcode = 0x5;
            op.bits.cond = 0xE;
            ret = ARMV7_B;
            *tio = op.value;
        } else if ((ti & 0xF801) == 0xE800) {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        } else if ((ti & 0xF801) == 0xE801) {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        } else if ((ti & 0xF800) == 0xF000) {
            BranchType op;
            op.value = 0;
            op.bits.offset = ti & 0x7FF;
            if (ti & 0x4) {
                op.bits.offset |= 0xfff800;
            }
            op.bits.offset <<= 12;  // prefix
            op.bits.opcode = 0x5;

            // The next Thumb instruction MUST BE suffix
            uint32_t ti2 = ti >> 16;
            if ((ti2 & 0xF800) == 0xF800) {
                // BL call Thumb routine
                op.bits.offset |= ((ti & 0x7FF) << 1);
                op.bits.L = 1;
                op.bits.cond = 0xE;
                ret = ARMV7_BL;
            } else if ((ti2 & 0xF800) == 0xE800) {
                // BLX call ARM routine
                RISCV_sprintf(errmsg, errsz,
                    "ARMV5T BLX(1) unsupported: %04x", ti2 & 0xFFFF);
            } else {
                RISCV_sprintf(errmsg, errsz,
                    "Wrong BL/BLX suffix %04x", ti2 & 0xFFFF);
            }
            *tio = op.value;
        } else {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        }
        break;
    default:;
    }
    return ret;
}

}  // debugger
