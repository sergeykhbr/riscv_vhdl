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
package pmp_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic [RISCV_ARCH-1:0] start_addr;
    logic [RISCV_ARCH-1:0] end_addr;
    logic [CFG_PMP_FL_TOTAL-1:0] flags;
} PmpTableItemType;


typedef struct {
    PmpTableItemType tbl[0: CFG_PMP_TBL_SIZE - 1];
} PMP_registers;

endpackage: pmp_pkg
