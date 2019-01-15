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

EIsaArmV7 decoder_thumb(uint32_t ti, char *errmsg, size_t errsz) {
    EIsaArmV7 ret = ARMV7_Total;
    switch ((ti >> 13) & 0x7) {
    case 0:  // [15:13] == 3b'000
        /** 
           000?_????_????_???? - Shift by immediate
           0001_10??_????_???? - Add/subtract register
           0001_11??_????_???? - Add/subtract immediate
        */
        if ((ti & 0xFC00) == 0x1800) {
        } else if ((ti & 0xFC00) == 0x1C00) {
        } else {
        }
        break;
    case 1:  // [15:13] == 3b'001
        /** Add/subtract/compare/move immediate
                001?_????_????_????
         */
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
        } else if ((ti & 0xFF00) == 0x4700) {
        } else if ((ti & 0xFC00) == 0x4400) {
        } else if ((ti & 0xF800) == 0x4800) {
        } else {
        }
    case 3:  // [15:13] == 3b'011
        /** 
           011?_????_????_???? - Load/Store word/byte immediate offset
        */
        break;
    case 4:  // [15:13] == 3b'100
        /** 
           1000_????_????_???? - Load/Store halfword immediate offset
           1001_????_????_???? - Load/Store to/from stack
        */
        if ((ti & 0xF000) == 0x8000) {
        } else {
        }
        break;
    case 5:  // [15:13] == 3b'101
        /** 
           1010_????_????_???? - Add to SP or PC
           1011_????_????_???? - Miscellaneous
        */
        if ((ti & 0xF000) == 0xA000) {
        } else {
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
        } else if ((ti & 0xFF00) == 0xDE00) {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        } else if ((ti & 0xF000) == 0xD000) {
        } else {
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
        } else if ((ti & 0xF801) == 0xE800) {
        } else if ((ti & 0xF801) == 0xE801) {
            RISCV_sprintf(errmsg, errsz,
                "undefined instruction %04x", ti & 0xFFFF);
        } else if ((ti & 0xF800) == 0xF000) {
        } else {
        }
        break;
    default:;
    }
    return ret;
}

}  // debugger
