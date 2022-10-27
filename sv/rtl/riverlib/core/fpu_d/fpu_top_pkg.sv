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
package fpu_top_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic [Instr_FPU_Total-1:0] ivec;
    logic busy;
    logic ready;
    logic [63:0] a;
    logic [63:0] b;
    logic [63:0] result;
    logic ex_invalidop;                                     // Exception: invalid operation
    logic ex_divbyzero;                                     // Exception: divide by zero
    logic ex_overflow;                                      // Exception: overflow
    logic ex_underflow;                                     // Exception: underflow
    logic ex_inexact;                                       // Exception: inexact
    logic ena_fadd;
    logic ena_fdiv;
    logic ena_fmul;
    logic ena_d2l;
    logic ena_l2d;
    logic ena_w32;
} FpuTop_registers;

const FpuTop_registers FpuTop_r_reset = '{
    '0,                                 // ivec
    1'b0,                               // busy
    1'b0,                               // ready
    '0,                                 // a
    '0,                                 // b
    '0,                                 // result
    1'b0,                               // ex_invalidop
    1'b0,                               // ex_divbyzero
    1'b0,                               // ex_overflow
    1'b0,                               // ex_underflow
    1'b0,                               // ex_inexact
    1'b0,                               // ena_fadd
    1'b0,                               // ena_fdiv
    1'b0,                               // ena_fmul
    1'b0,                               // ena_d2l
    1'b0,                               // ena_l2d
    1'b0                                // ena_w32
};

endpackage: fpu_top_pkg
