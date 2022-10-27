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
package jtagcdc_pkg;


localparam int CDC_REG_WIDTH = (1  // i_dmi_hardreset
        + 1  // i_dmi_reset
        + 7  // i_dmi_req_addr
        + 32  // i_dmi_req_data
        + 1  // i_dmi_req_write
        + 1  // i_dmi_req_valid
);

typedef struct {
    logic [CDC_REG_WIDTH-1:0] l1;
    logic [CDC_REG_WIDTH-1:0] l2;
    logic req_valid;
    logic req_accepted;
    logic req_write;
    logic [6:0] req_addr;
    logic [31:0] req_data;
    logic req_reset;
    logic req_hardreset;
} jtagcdc_registers;

const jtagcdc_registers jtagcdc_r_reset = '{
    '1,                                 // l1
    '0,                                 // l2
    1'b0,                               // req_valid
    1'b0,                               // req_accepted
    1'b0,                               // req_write
    '0,                                 // req_addr
    '0,                                 // req_data
    1'b0,                               // req_reset
    1'b0                                // req_hardreset
};

endpackage: jtagcdc_pkg
