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
package int_mul_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic busy;
    logic [3:0] ena;
    logic [RISCV_ARCH-1:0] a1;
    logic [RISCV_ARCH-1:0] a2;
    logic unsign;
    logic high;
    logic rv32;
    logic zero;
    logic inv;
    logic [127:0] result;
    logic [RISCV_ARCH-1:0] a1_dbg;
    logic [RISCV_ARCH-1:0] a2_dbg;
    logic [RISCV_ARCH-1:0] reference_mul;                   // Used for run-time comparision
    logic [68:0] lvl1[0: 16 - 1];
    logic [82:0] lvl3[0: 4 - 1];
} IntMul_registers;

endpackage: int_mul_pkg
