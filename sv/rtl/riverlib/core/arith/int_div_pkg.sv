// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
package int_div_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic rv32;
    logic resid;
    logic invert;
    logic div_on_zero;
    logic overflow;
    logic busy;
    logic [9:0] ena;
    logic [63:0] divident_i;
    logic [119:0] divisor_i;
    logic [63:0] bits_i;
    logic [RISCV_ARCH-1:0] result;
    logic [RISCV_ARCH-1:0] reference_div;
    logic [63:0] a1_dbg;                                    // Store this value for output in a case of error
    logic [63:0] a2_dbg;
} IntDiv_registers;

const IntDiv_registers IntDiv_r_reset = '{
    1'b0,                               // rv32
    1'b0,                               // resid
    1'b0,                               // invert
    1'b0,                               // div_on_zero
    1'b0,                               // overflow
    1'b0,                               // busy
    '0,                                 // ena
    '0,                                 // divident_i
    '0,                                 // divisor_i
    '0,                                 // bits_i
    '0,                                 // result
    '0,                                 // reference_div
    '0,                                 // a1_dbg
    '0                                  // a2_dbg
};

endpackage: int_div_pkg
