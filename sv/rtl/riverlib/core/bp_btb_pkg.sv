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
package bp_btb_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic [RISCV_ARCH-1:0] pc;
    logic [RISCV_ARCH-1:0] npc;
    logic exec;                                             // 0=predec; 1=exec (high priority)
} BtbEntryType;


typedef struct {
    BtbEntryType btb[0: CFG_BTB_SIZE - 1];
} BpBTB_registers;

endpackage: bp_btb_pkg
