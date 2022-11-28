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
package fetch_pkg;

import river_cfg_pkg::*;

localparam int unsigned Idle = 0;
localparam int unsigned WaitReqAccept = 1;
localparam int unsigned WaitResp = 2;

typedef struct {
    logic [1:0] state;
    logic req_valid;
    logic resp_ready;
    logic [RISCV_ARCH-1:0] req_addr;
    logic [RISCV_ARCH-1:0] mem_resp_shadow;                 // the same as memory response but internal
    logic [RISCV_ARCH-1:0] pc;
    logic [63:0] instr;
    logic instr_load_fault;
    logic instr_page_fault_x;
    logic progbuf_ena;
} InstrFetch_registers;

const InstrFetch_registers InstrFetch_r_reset = '{
    Idle,                               // state
    1'b0,                               // req_valid
    1'b0,                               // resp_ready
    '1,                                 // req_addr
    '1,                                 // mem_resp_shadow
    '1,                                 // pc
    '0,                                 // instr
    1'b0,                               // instr_load_fault
    1'b0,                               // instr_page_fault_x
    1'b0                                // progbuf_ena
};

endpackage: fetch_pkg
