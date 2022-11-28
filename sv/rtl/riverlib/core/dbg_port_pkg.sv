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
package dbg_port_pkg;

import river_cfg_pkg::*;

localparam bit [3:0] idle = 4'h0;
localparam bit [3:0] csr_region = 4'h1;
localparam bit [3:0] reg_bank = 4'h2;
localparam bit [3:0] reg_stktr_cnt = 4'h3;
localparam bit [3:0] reg_stktr_buf_adr = 4'h4;
localparam bit [3:0] reg_stktr_buf_dat = 4'h5;
localparam bit [3:0] exec_progbuf_start = 4'h6;
localparam bit [3:0] exec_progbuf_next = 4'h7;
localparam bit [3:0] exec_progbuf_waitmemop = 4'h8;
localparam bit [3:0] abstract_mem_request = 4'h9;
localparam bit [3:0] abstract_mem_response = 4'ha;
localparam bit [3:0] wait_to_accept = 4'hb;

typedef struct {
    logic dport_write;
    logic [RISCV_ARCH-1:0] dport_addr;
    logic [RISCV_ARCH-1:0] dport_wdata;
    logic [RISCV_ARCH-1:0] dport_rdata;
    logic [1:0] dport_size;
    logic [3:0] dstate;
    logic [RISCV_ARCH-1:0] rdata;
    logic [CFG_LOG2_STACK_TRACE_ADDR-1:0] stack_trace_cnt;  // Stack trace buffer counter
    logic req_accepted;
    logic resp_error;
    logic progbuf_ena;
    logic [RISCV_ARCH-1:0] progbuf_pc;
    logic [63:0] progbuf_instr;
} DbgPort_registers;

const DbgPort_registers DbgPort_r_reset = '{
    1'b0,                               // dport_write
    '0,                                 // dport_addr
    '0,                                 // dport_wdata
    '0,                                 // dport_rdata
    '0,                                 // dport_size
    idle,                               // dstate
    '0,                                 // rdata
    '0,                                 // stack_trace_cnt
    1'b0,                               // req_accepted
    1'b0,                               // resp_error
    1'b0,                               // progbuf_ena
    '0,                                 // progbuf_pc
    '0                                  // progbuf_instr
};

endpackage: dbg_port_pkg
