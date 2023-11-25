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
package axi2apb_bus1_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;
import types_bus1_pkg::*;

localparam bit [1:0] State_Idle = 2'd0;
localparam bit [1:0] State_setup = 2'd1;
localparam bit [1:0] State_access = 2'd2;
localparam bit [1:0] State_out = 2'd3;

typedef struct {
    logic [2:0] state;
    logic [CFG_BUS1_PSLV_LOG2_TOTAL-1:0] selidx;
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
} axi2apb_bus1_registers;

const axi2apb_bus1_registers axi2apb_bus1_r_reset = '{
    State_Idle,                         // state
    3'd0,                               // selidx
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

endpackage: axi2apb_bus1_pkg
