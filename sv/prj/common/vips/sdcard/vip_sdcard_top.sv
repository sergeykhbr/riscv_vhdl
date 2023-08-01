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

module vip_sdcard_top(
    input logic i_sclk,
    inout logic io_cmd,
    inout logic io_dat0,
    inout logic io_dat1,
    inout logic io_dat2,
    inout logic io_cd_dat3
);

import vip_sdcard_top_pkg::*;

logic w_clk;
logic [7:0] wb_rdata;

always_comb
begin: comb_proc
end: comb_proc

endmodule: vip_sdcard_top
