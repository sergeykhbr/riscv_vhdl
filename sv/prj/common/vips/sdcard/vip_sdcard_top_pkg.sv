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
package vip_sdcard_top_pkg;


// Generic config parameters
localparam int CFG_SDCARD_POWERUP_DONE_DELAY = 450;         // Delay of busy bits in ACMD41 response
localparam bit CFG_SDCARD_HCS = 1'b1;                       // High Capacity Support
localparam bit [3:0] CFG_SDCARD_VHS = 4'h1;                 // CMD8 Voltage supply mask
localparam bit CFG_SDCARD_PCIE_1_2V = 1'b0;
localparam bit CFG_SDCARD_PCIE_AVAIL = 1'b0;
localparam bit [23:0] CFG_SDCARD_VDD_VOLTAGE_WINDOW = 24'hFF8000;

endpackage: vip_sdcard_top_pkg
