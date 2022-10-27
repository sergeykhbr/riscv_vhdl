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
package ic_dport_pkg;

import river_cfg_pkg::*;
import types_river_pkg::*;

localparam bit [CFG_CPU_MAX-1:0] ALL_CPU_MASK = '1;

typedef struct {
    logic [CFG_LOG2_CPU_MAX-1:0] hartsel;
} ic_dport_registers;

const ic_dport_registers ic_dport_r_reset = '{
    '0                                  // hartsel
};

endpackage: ic_dport_pkg
