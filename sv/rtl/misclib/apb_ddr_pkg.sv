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
package apb_ddr_pkg;

import types_amba_pkg::*;

typedef struct {
    logic pll_locked;
    logic init_calib_done;
    logic [11:0] device_temp;
    logic sr_active;
    logic ref_ack;
    logic zq_ack;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_ddr_registers;

const apb_ddr_registers apb_ddr_r_reset = '{
    1'b0,                               // pll_locked
    1'b0,                               // init_calib_done
    '0,                                 // device_temp
    1'b0,                               // sr_active
    1'b0,                               // ref_ack
    1'b0,                               // zq_ack
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

endpackage: apb_ddr_pkg
