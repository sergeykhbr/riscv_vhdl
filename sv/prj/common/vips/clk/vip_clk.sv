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

module vip_clk #(
    parameter realtime period = 1.0
)
(
    output logic o_clk
);

import vip_clk_pkg::*;

logic pll;

initial begin
    pll = 0;
end
always begin
    #(0.5 * 1000000000 * period) pll = ~pll;
end


always_comb
begin: comb_proc
    o_clk = pll;
end: comb_proc

endmodule: vip_clk
