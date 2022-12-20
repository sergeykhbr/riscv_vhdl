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
package apb_prci_pkg;

import types_amba_pkg::*;

typedef struct {
    logic sys_nrst;
    logic dbg_nrst;
    logic sys_locked;
    logic ddr_locked;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
    logic [3:0] delayed_lock;
} apb_prci_registers;

const apb_prci_registers apb_prci_r_reset = '{
    1'b0,                               // sys_nrst
    1'b0,                               // dbg_nrst
    1'b0,                               // sys_locked
    1'b0,                               // ddr_locked
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0,                               // resp_err
    '0                                  // delayed_lock
};

endpackage: apb_prci_pkg
