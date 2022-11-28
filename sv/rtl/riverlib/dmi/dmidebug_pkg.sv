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
package dmidebug_pkg;

import types_amba_pkg::*;
import river_cfg_pkg::*;

localparam bit [2:0] CMDERR_NONE = 3'h0;
localparam bit [2:0] CMDERR_BUSY = 3'h1;
localparam bit [2:0] CMDERR_NOTSUPPROTED = 3'h2;
localparam bit [2:0] CMDERR_EXCEPTION = 3'h3;
localparam bit [2:0] CMDERR_WRONGSTATE = 3'h4;
localparam bit [2:0] CMDERR_BUSERROR = 3'h5;
localparam bit [2:0] CMDERR_OTHERS = 3'h7;
// Dedicated bit in the 'command' register
localparam int CmdPostexecBit = 18;
localparam int CmdTransferBit = 17;
localparam int CmdWriteBit = 16;
localparam int CmdPostincrementBit = 19;
// dmstate:
localparam bit DM_STATE_IDLE = 1'h0;
localparam bit DM_STATE_ACCESS = 1'h1;
// cmdstate:
localparam bit [2:0] CMD_STATE_IDLE = 3'h0;
localparam bit [2:0] CMD_STATE_INIT = 3'h1;
localparam bit [2:0] CMD_STATE_REQUEST = 3'h2;
localparam bit [2:0] CMD_STATE_RESPONSE = 3'h3;
localparam bit [2:0] CMD_STATE_WAIT_HALTED = 3'h4;

typedef struct {
    logic bus_jtag;
    logic [31:0] jtag_resp_data;
    logic [31:0] prdata;
    logic [6:0] regidx;
    logic [31:0] wdata;
    logic regwr;
    logic regrd;
    logic dmstate;
    logic [2:0] cmdstate;
    logic haltreq;
    logic resumereq;
    logic resumeack;
    logic hartreset;
    logic resethaltreq;                                     // halt on reset
    logic ndmreset;
    logic dmactive;
    logic [CFG_LOG2_CPU_MAX-1:0] hartsel;
    logic cmd_regaccess;
    logic cmd_quickaccess;
    logic cmd_memaccess;
    logic cmd_progexec;
    logic cmd_read;
    logic cmd_write;
    logic postincrement;
    logic aamvirtual;
    logic [31:0] command;
    logic [CFG_DATA_REG_TOTAL-1:0] autoexecdata;
    logic [CFG_PROGBUF_REG_TOTAL-1:0] autoexecprogbuf;
    logic [2:0] cmderr;
    logic [31:0] data0;
    logic [31:0] data1;
    logic [31:0] data2;
    logic [31:0] data3;
    logic [(32 * CFG_PROGBUF_REG_TOTAL)-1:0] progbuf_data;
    logic dport_req_valid;
    logic [RISCV_ARCH-1:0] dport_addr;
    logic [RISCV_ARCH-1:0] dport_wdata;
    logic [2:0] dport_size;
    logic dport_resp_ready;
    logic pready;
} dmidebug_registers;

const dmidebug_registers dmidebug_r_reset = '{
    1'b0,                               // bus_jtag
    '0,                                 // jtag_resp_data
    '0,                                 // prdata
    '0,                                 // regidx
    '0,                                 // wdata
    1'b0,                               // regwr
    1'b0,                               // regrd
    DM_STATE_IDLE,                      // dmstate
    CMD_STATE_IDLE,                     // cmdstate
    1'b0,                               // haltreq
    1'b0,                               // resumereq
    1'b0,                               // resumeack
    1'b0,                               // hartreset
    1'b0,                               // resethaltreq
    1'b0,                               // ndmreset
    1'b0,                               // dmactive
    '0,                                 // hartsel
    1'b0,                               // cmd_regaccess
    1'b0,                               // cmd_quickaccess
    1'b0,                               // cmd_memaccess
    1'b0,                               // cmd_progexec
    1'b0,                               // cmd_read
    1'b0,                               // cmd_write
    1'b0,                               // postincrement
    1'b0,                               // aamvirtual
    '0,                                 // command
    '0,                                 // autoexecdata
    '0,                                 // autoexecprogbuf
    CMDERR_NONE,                        // cmderr
    '0,                                 // data0
    '0,                                 // data1
    '0,                                 // data2
    '0,                                 // data3
    '0,                                 // progbuf_data
    1'b0,                               // dport_req_valid
    '0,                                 // dport_addr
    '0,                                 // dport_wdata
    '0,                                 // dport_size
    1'b0,                               // dport_resp_ready
    1'b0                                // pready
};

endpackage: dmidebug_pkg
