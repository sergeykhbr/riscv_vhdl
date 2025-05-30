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
package bp_pkg;

import river_cfg_pkg::*;

typedef struct {
    logic c_valid;
    logic [RISCV_ARCH-1:0] addr;
    logic [31:0] data;
    logic jmp;
    logic [RISCV_ARCH-1:0] pc;
    logic [RISCV_ARCH-1:0] npc;
} PreDecType;


endpackage: bp_pkg
