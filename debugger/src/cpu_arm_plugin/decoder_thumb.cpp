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

static EIsaArmV7 decoder_w(uint32_t ti, uint32_t *tio,
                         char *errmsg, size_t errsz) {
    EIsaArmV7 ret = ARMV7_Total;
    if ((ti & 0xFFF0FFF0) == 0xF000E8D0) {
        ret = T1_TBB;
    } else if ((ti & 0xF0F0FFF0) == 0xF0F0FB90) {
        ret = T1_SDIV;
    } else if ((ti & 0xF0F0FFF0) == 0xF0F0FBB0) {
        ret = T1_UDIV;
    } else if ((ti & 0xF0F0FFF0) == 0xF000FB00) {
        ret = T2_MUL;
    } else if ((ti & 0x8020FFF0) == 0x0000F340) {
        ret = T1_SBFX;
    } else if ((ti & 0x8020FFF0) == 0x0000F3C0) {
        ret = T1_UBFX;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F110) {
        // T3_ADD_I => T1_CMN_I
    } else if ((ti & 0x8F00FFF0) == 0x0F00EB10) {
        // T3_ADD_R => T2_CMN_R
    } else if ((ti & 0xF0F0FFEF) == 0x0000EA4F) {
        // T3_MOV_R
    } else if ((ti & 0x8000FFEF) == 0x0000EB0D) {
        // T3_ADD_R => T3_ADDSP_R
    } else if ((ti & 0x8000FBEF) == 0x0000F10D) {
        // T3_ADD_I => T3_ADDSP_I
    } else if ((ti & 0x8000FBEF) == 0x0000F1AD) {
        // T2_SUBSP_I
    } else if ((ti & 0x0000FF7F) == 0x0000F85F) { // highest
        ret = T2_LDR_L;
    } else if ((ti & 0x0F00FFF0) == 0x0E00F850) { // < T2_LDR_L
        // T1_LDRT
    } else if ((ti & 0x0D00FFF0) == 0x0800F850) { // < T2_LDR_L
        // T4_LDR_I: undefined
    } else if ((ti & 0xF000FF7F) == 0xF000F81F) { // highest
        // T3_PLD_I
    } else if ((ti & 0xFF00FFF0) == 0xFC00F810) { // < T3_PLD_I
        // T2_PLD_I
    } else if ((ti & 0xF000FFF0) == 0xF000F890) { // < T3_PLD_I
        // T1_PLD_I
    } else if ((ti & 0xFFC0FFF0) == 0xF000F810) { // < T3_PLD_I
        // T1_PLD_R
    } else if ((ti & 0xF000FF7F) == 0xF000F91F) { // highest
        // T3_PLI_I
    } else if ((ti & 0xFF00FFF0) == 0xFC00F910) { // < T3_PLI_I
        // T2_PLI_I
    } else if ((ti & 0xF000FFF0) == 0xF000F990) { // < T3_PLI_I
        // T1_PLI_I
    } else if ((ti & 0xFFC0FFF0) == 0xF000F910) { // < T3_PLI_I
        // T1_PLI_R
    } else if ((ti & 0x0FC0FF7F) == 0x0000F81F) { // < T3_PLD_I
        // T1_LDRB_L
    } else if ((ti & 0x0000FF7F) == 0x0000F91F) { // < T3_PLI_I
        // T1_LDRSB_L
    } else if ((ti & 0x2000FFFF) == 0x0000E8BD) { // highest
        ret = T2_POP;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F800) { // highest
        ret = T2_STRB_R;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F810) { // < T1_PLD_R, T1_LDRB_L
        ret = T2_LDRB_R;
    } else if ((ti & 0x0000FF7F) == 0x0000F83F) { // < Memory hints
        // T1_LDRH_L;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F830) { // < T1_LDRH_L, Memory hints
        ret = T2_LDRH_R;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F840) { // Highest
        ret = T2_STR_R;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F910) { // < T1_PLI_R, T1_LDRSB_L
        ret = T2_LDRSB_R;
    } else if ((ti & 0x0F00FFF0) == 0x0E00F800) { // Highest
        // T1_STRBT
    } else if ((ti & 0xF0C0FFFF) == 0xF080FA1F) { // Highest
        // T2_UXTH;
    } else if ((ti & 0xF0C0FFFF) == 0xF080FA4F) { // Highest
        // T2_SXTB;
    } else if ((ti & 0xF0C0FFFF) == 0xF080FA5F) { // Highest
        // T2_UXTB;
    } else if ((ti & 0x8F00FFF0) == 0x0F00EA10) { // Highest
        // T2_TST_R
    } else if ((ti & 0x8F00FFF0) == 0x0F00EBB0) { // Highest
        // T3_CMP_R
    } else if ((ti & 0xF0C0FFF0) == 0xF080FA10) { // < T2_UXTH
        ret = T1_UXTAH;
    } else if ((ti & 0xF0C0FFF0) == 0xF080FA40) { // < T2_SXTB
        ret = T1_SXTAB;
    } else if ((ti & 0xF0C0FFF0) == 0xF080FA50) { // < T2_UXTB
        ret = T1_UXTAB;
    } else if ((ti & 0x00F0FFF0) == 0x0010FB00) { // Highest
        ret = T1_MLS;
    } else if ((ti & 0x0F00FFF0) == 0x0E00F810) { // < T1_LDRB_L
        // T1_LDRBT
    } else if ((ti & 0x0F00FFF0) == 0x0E00F820) { // Highest
        // T1_STRHT;
    } else if ((ti & 0x0800FFF0) == 0x0800F800) { // < T1_STRBT
        ret = T3_STRB_I;
    } else if ((ti & 0x0800FFF0) == 0x0800F810) { // < T1_LDRB_L, T3_PLD_I, T1_LDRBT
        ret = T3_LDRB_I;
    } else if ((ti & 0x0800FFF0) == 0x0800F820) { // < T1_STRHT
        ret = T3_STRH_I;
    } else if ((ti & 0x0800FFF0) == 0x0800F850) { // < T2_LDR_L, T1_LDRT
        ret = T4_LDR_I;
    } else if ((ti & 0x0FC0FFF0) == 0x0000F850) { // < T2_LDR_L
        ret = T2_LDR_R;
    } else if ((ti & 0x00F0FFF0) == 0x0000FB00) {
        ret = T1_MLA;
    } else if ((ti & 0x00F0FFF0) == 0x0000FB80) {
        ret = T1_SMULL;
    } else if ((ti & 0x00F0FFF0) == 0x0000FBA0) {
        ret = T1_UMULL;
    } else if ((ti & 0xF0F0FFE0) == 0xF000FA00) {
        ret = T2_LSL_R;
    } else if ((ti & 0xF0F0FFE0) == 0xF000FA20) {
        ret = T2_LSR_R;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F010) {
        ret = T1_TST_I;
    } else if ((ti & 0x8F00FBF0) == 0x0F00F1B0) {
        ret = T2_CMP_I;
    } else if ((ti & 0x2000FFD0) == 0x0000E890) {
        ret = T2_LDMIA;
    } else if ((ti & 0xA000FFD0) == 0x0000E900) {
        ret = T1_STMDB;
    } else if ((ti & 0x0000FFF0) == 0x0000F880) { // Highest
        ret = T2_STRB_I;
    } else if ((ti & 0x0000FFF0) == 0x0000F890) { // < T3_PLD_I, T1_LDRB_L
        ret = T2_LDRB_I;
    } else if ((ti & 0x0000FFF0) == 0x0000F8A0) { // Highest
        ret = T2_STRH_I;
    } else if ((ti & 0x0000FFF0) == 0x0000F8D0) { // < T2_LDR_L
        ret = T3_LDR_I;
    } else if ((ti & 0x0000FFF0) == 0x0000F990) { // < T3_PLI_I, T1_LDRSB_L
        ret = T1_LDRSB_I;
    } else if ((ti & 0x8000FFE0) == 0x0000EA00) {
        ret = T2_AND_R;
    } else if ((ti & 0x8000FFE0) == 0x0000EA40) {
        ret = T2_ORR_R;
    } else if ((ti & 0x8000FFE0) == 0x0000EB00) {
        ret = T3_ADD_R;
    } else if ((ti & 0x8000FFEF) == 0x0000EBAD) {
        // T1_SUBSP_R;
    } else if ((ti & 0x8000FFE0) == 0x0000EBA0) {
        ret = T2_SUB_R;
    } else if ((ti & 0x8000FFE0) == 0x0000EBC0) {
        ret = T1_RSB_R;
    } else if ((ti & 0x8000FBF0) == 0x0000F240) {
        ret = T3_MOV_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F000) {
        ret = T1_AND_I;
    } else if ((ti & 0x8F00FBE0) == 0x0F00F080) {
        // T1_TEQ_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F080) {
        ret = T1_EOR_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F100) {
        ret = T3_ADD_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F140) {
        ret = T1_ADC_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F1A0) {
        ret = T3_SUB_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F1C0) {
        ret = T2_RSB_I;
    } else if ((ti & 0x8000FBEF) == 0x0000F04F) {
        ret = T2_MOV_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F040) {
        ret = T1_ORR_I;
    } else if ((ti & 0x8000FBE0) == 0x0000F020) {
        ret = T1_BIC_I;
    } else if ((ti & 0x0000FF70) == 0x0000E840) {
        // see Load/Store double and exclusive, and table branch on page 3-28
    } else if ((ti & 0x0000FE50) == 0x0000E840) {
        ret = T1_STRD_I;
    } else if ((ti & 0xD000F800) == 0x8000F000) {
        ret = T3_B;
    } else if ((ti & 0xD000F800) == 0x9000F000) {
        ret = T4_B;
    }
    return ret;
}

EIsaArmV7 decoder_thumb(uint32_t ti, uint32_t *tio,
                         char *errmsg, size_t errsz) {
    EIsaArmV7 tret;
    EIsaArmV7 ret = ARMV7_Total;
    if ((ti & 0xFFFF) == 0xBF00) {
        ret = T1_NOP;
    } else if ((ti & 0xFF87) == 0x4700) {
        ret = T1_BX;
    } else if ((tret = decoder_w(ti, tio, errmsg, errsz)) != ARMV7_Total) {
        ret = tret;
    } else if ((ti & 0xFFE8) == 0xB660) {
        ret = T1_CPS;
    } else if ((ti & 0xFF78) == 0x4468) {
        //ret = T1_ADDSP_R;
    } else if ((ti & 0xFF87) == 0x4485) {
        //ret = T2_ADDSP_R;
    } else if ((ti & 0xFF87) == 0x4780) {
        ret = T1_BLX_R;
    } else if ((ti & 0xFFC0) == 0x0000) {
        ret = T2_MOV_R;
    } else if ((ti & 0xFFC0) == 0x4000) {
        ret = T1_AND_R;
    } else if ((ti & 0xFFC0) == 0x4040) {
        ret = T1_EOR_R;
    } else if ((ti & 0xFFC0) == 0x4080) {
        ret = T1_LSL_R;
    } else if ((ti & 0xFFC0) == 0x40C0) {
        ret = T1_LSR_R;
    } else if ((ti & 0xFFC0) == 0x4200) {
        ret = T1_TST_R;
    } else if ((ti & 0xFFC0) == 0x4240) {
        ret = T1_RSB_I;
    } else if ((ti & 0xFFC0) == 0x4280) {
        ret = T1_CMP_R;
    } else if ((ti & 0xFFC0) == 0x4300) {
        ret = T1_ORR_R;
    } else if ((ti & 0xFFC0) == 0x4340) {
        ret = T1_MUL;
    } else if ((ti & 0xFFC0) == 0x43C0) {
        ret = T1_MVN_R;
    } else if ((ti & 0xFFC0) == 0xB240) {
        ret = T1_SXTB;
    } else if ((ti & 0xFFC0) == 0xB280) {
        ret = T1_UXTH;
    } else if ((ti & 0xFFC0) == 0xB2C0) {
        ret = T1_UXTB;
    } else if ((ti & 0xFF80) == 0xB000) {
        ret = T2_ADDSP_I;
    } else if ((ti & 0xFF80) == 0xB080) {
        ret = T1_SUBSP_I;
    } else if ((ti & 0xFF00) == 0x4400) {
        ret = T2_ADD_R;
    } else if ((ti & 0xFF00) == 0x4500) {
        ret = T2_CMP_R;
    } else if ((ti & 0xFF00) == 0x4600) {
        ret = T1_MOV_R;
    } else if ((ti & 0xFF00) == 0xBE00) {
        ret = T1_BKPT;
    } else if ((ti & 0xFF00) == 0xBF00) {
        ret = T1_IT;
    } else if ((ti & 0xFE00) == 0x1800) {
        ret = T1_ADD_R;
    } else if ((ti & 0xFE00) == 0x1A00) {
        ret = T1_SUB_R;
    } else if ((ti & 0xFE00) == 0x1C00) {
        ret = T1_ADD_I;
    } else if ((ti & 0xFE00) == 0x1E00) {
        ret = T1_SUB_I;
    } else if ((ti & 0xFE00) == 0x5000) {
        ret = T1_STR_R;
    } else if ((ti & 0xFE00) == 0x5400) {
        ret = T1_STRB_R;
    } else if ((ti & 0xFE00) == 0x5600) {
        ret = T1_LDRSB_R;
    } else if ((ti & 0xFE00) == 0x5800) {
        ret = T1_LDR_R;
    } else if ((ti & 0xFE00) == 0x5C00) {
        ret = T1_LDRB_R;
    } else if ((ti & 0xFE00) == 0xB400) {
        ret = T1_PUSH;
    } else if ((ti & 0xFE00) == 0xBC00) {
        ret = T1_POP;
    } else if ((ti & 0xFD00) == 0xB900) {
        ret = T1_CBNZ;
    } else if ((ti & 0xFD00) == 0xB100) {
        ret = T1_CBZ;
    } else if ((ti & 0xF800) == 0x0000) {
        ret = T1_LSL_I;
    } else if ((ti & 0xF800) == 0x0800) {
        ret = T1_LSR_I;
    } else if ((ti & 0xF800) == 0x1000) {
        ret = T1_ASR_I;
    } else if ((ti & 0xF800) == 0x2000) {
        ret = T1_MOV_I;
    } else if ((ti & 0xF800) == 0x2800) {
        ret = T1_CMP_I;
    } else if ((ti & 0xF800) == 0x3000) {
        ret = T2_ADD_I;
    } else if ((ti & 0xF800) == 0x3800) {
        ret = T2_SUB_I;
    } else if ((ti & 0xF800) == 0x4800) {
        ret = T1_LDR_L;
    } else if ((ti & 0xF800) == 0x6000) {
        ret = T1_STR_I;
    } else if ((ti & 0xF800) == 0x6800) {
        ret = T1_LDR_I;
    } else if ((ti & 0xF800) == 0x7000) {
        ret = T1_STRB_I;
    } else if ((ti & 0xF800) == 0x7800) {
        ret = T1_LDRB_I;
    } else if ((ti & 0xF800) == 0x8000) {
        ret = T1_STRH_I;
    } else if ((ti & 0xF800) == 0x8800) {
        ret = T1_LDRH_I;
    } else if ((ti & 0xF800) == 0x9000) {
        ret = T2_STR_I;
    } else if ((ti & 0xF800) == 0x9800) {
        ret = T2_LDR_I;
    } else if ((ti & 0xF800) == 0xA000) {
        ret = T1_ADR;
    } else if ((ti & 0xF800) == 0xA800) {
        ret = T1_ADDSP_I;
    } else if ((ti & 0xF800) == 0xC000) {
        ret = T1_STMIA;
    } else if ((ti & 0xF800) == 0xE000) {
        ret = T2_B;
    } else if ((ti & 0xD000F800) == 0xD000F000) {
        // 4.6.18 BL, BLX
        ret = T1_BL_I;
    } else if ((ti & 0xFF00) == 0xDE00) {
        RISCV_sprintf(errmsg, errsz,
            "B: See permanently undefined space %04x", ti & 0xFFFF);
    } else if ((ti & 0xFF00) == 0xDF00) {
        RISCV_sprintf(errmsg, errsz,
            "B: See SVC (formely SWI) %04x", ti & 0xFFFF);
    } else if ((ti & 0xF000) == 0xD000) {
        ret = T1_B;
    } else {
        RISCV_sprintf(errmsg, errsz,
            "undefined instruction %04x", ti & 0xFFFF);
    }
    return ret;
}

}  // debugger
