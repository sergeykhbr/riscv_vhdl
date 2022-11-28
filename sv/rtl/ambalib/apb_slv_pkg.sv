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
package apb_slv_pkg;

import types_amba_pkg::*;

localparam bit [1:0] State_Idle = 2'h0;
localparam bit [1:0] State_Request = 2'h1;
localparam bit [1:0] State_WaitResp = 2'h2;
localparam bit [1:0] State_Resp = 2'h3;

typedef struct {
    logic [2:0] state;
    logic req_valid;
    logic [31:0] req_addr;
    logic req_write;
    logic [31:0] req_wdata;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_slv_registers;

const apb_slv_registers apb_slv_r_reset = '{
    State_Idle,                         // state
    1'b0,                               // req_valid
    '0,                                 // req_addr
    1'b0,                               // req_write
    '0,                                 // req_wdata
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

endpackage: apb_slv_pkg
