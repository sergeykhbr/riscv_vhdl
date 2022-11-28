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
package pma_pkg;

import river_cfg_pkg::*;

localparam bit [CFG_CPU_ADDR_BITS-1:0] CLINT_BAR = 48'h000002000000;
localparam bit [CFG_CPU_ADDR_BITS-1:0] CLINT_MASK = 48'h00000000ffff;// 64 KB
localparam bit [CFG_CPU_ADDR_BITS-1:0] PLIC_BAR = 48'h00000c000000;
localparam bit [CFG_CPU_ADDR_BITS-1:0] PLIC_MASK = 48'h000003ffffff;// 64 MB
localparam bit [CFG_CPU_ADDR_BITS-1:0] IO1_BAR = 48'h000010000000;
localparam bit [CFG_CPU_ADDR_BITS-1:0] IO1_MASK = 48'h0000000fffff;// 1 MB

endpackage: pma_pkg
