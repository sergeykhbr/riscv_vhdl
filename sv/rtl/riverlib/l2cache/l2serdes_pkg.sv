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
package l2serdes_pkg;

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;

localparam int linew = L2CACHE_LINE_BITS;
localparam int busw = CFG_SYSBUS_DATA_BITS;
localparam int lineb = (linew / 8);
localparam int busb = (busw / 8);
localparam int SERDES_BURST_LEN = (lineb / busb);
localparam bit [1:0] State_Idle = 2'h0;
localparam bit [1:0] State_Read = 2'h1;
localparam bit [1:0] State_Write = 2'h2;

typedef struct {
    logic [1:0] state;
    logic [7:0] req_len;
    logic b_wait;
    logic [linew-1:0] line;
    logic [lineb-1:0] wstrb;
    logic [SERDES_BURST_LEN-1:0] rmux;
} L2SerDes_registers;

const L2SerDes_registers L2SerDes_r_reset = '{
    State_Idle,                         // state
    '0,                                 // req_len
    1'b0,                               // b_wait
    '0,                                 // line
    '0,                                 // wstrb
    '0                                  // rmux
};

endpackage: l2serdes_pkg
