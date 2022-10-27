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
package l2_amba_pkg;

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;

localparam bit [1:0] idle = 2'h0;
localparam bit [1:0] reading = 2'h1;
localparam bit [1:0] writing = 2'h2;
localparam bit [1:0] wack = 2'h3;

typedef struct {
    logic [1:0] state;
} L2Amba_registers;

const L2Amba_registers L2Amba_r_reset = '{
    idle                                // state
};

endpackage: l2_amba_pkg
