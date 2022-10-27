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

`timescale 1ns/10ps

module DummyCpu(
    output types_river_pkg::axi4_l1_out_type o_msto,
    output types_river_pkg::dport_out_type o_dport,
    output logic o_flush_l2,                                // Flush L2 after D$ has been finished
    output logic o_halted,                                  // CPU halted via debug interface
    output logic o_available                                // CPU was instantitated of stubbed
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import dummycpu_pkg::*;

always_comb
begin: comb_proc
    o_msto = axi4_l1_out_none;
    o_dport = dport_out_none;
    o_flush_l2 = 1'h0;
    o_halted = 1'h0;
    o_available = 1'h0;
end: comb_proc

endmodule: DummyCpu
