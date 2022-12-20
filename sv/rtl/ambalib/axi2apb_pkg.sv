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
package axi2apb_pkg;

import types_amba_pkg::*;
import types_bus1_pkg::*;

localparam bit [1:0] State_Idle = 2'h0;
localparam bit [1:0] State_setup = 2'h1;
localparam bit [1:0] State_access = 2'h2;
localparam bit [1:0] State_out = 2'h3;

typedef struct {
    logic [2:0] state;
    logic [2:0] selidx;                                     // TODO: clog2 depending slaves number
    logic pvalid;
    logic [31:0] paddr;
    logic [CFG_SYSBUS_DATA_BITS-1:0] pwdata;
    logic [CFG_SYSBUS_DATA_BITS-1:0] prdata;
    logic pwrite;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] pstrb;
    logic [2:0] pprot;
    logic pselx;
    logic penable;
    logic pslverr;
    logic [7:0] size;
} axi2apb_registers;

const axi2apb_registers axi2apb_r_reset = '{
    State_Idle,                         // state
    '0,                                 // selidx
    1'b0,                               // pvalid
    '0,                                 // paddr
    '0,                                 // pwdata
    '0,                                 // prdata
    1'b0,                               // pwrite
    '0,                                 // pstrb
    '0,                                 // pprot
    1'b0,                               // pselx
    1'b0,                               // penable
    1'b0,                               // pslverr
    '0                                  // size
};

endpackage: axi2apb_pkg
