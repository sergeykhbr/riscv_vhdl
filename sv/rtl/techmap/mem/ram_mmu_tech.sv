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

module ram_mmu_tech #(
    parameter int abits = 6,
    parameter int dbits = 104
)
(
    input logic i_clk,                                      // CPU clock
    input logic [abits-1:0] i_addr,
    input logic i_wena,
    input logic [dbits-1:0] i_wdata,
    output logic [dbits-1:0] o_rdata
);

localparam int DEPTH = (2**abits);

logic [dbits-1:0] rdata;
logic [dbits-1:0] mem[0: DEPTH - 1];

always_ff @(posedge i_clk) begin: rg_proc

    if (i_wena == 1'b1) begin
        mem[int'(i_addr)] = i_wdata;
    end else begin
        rdata = mem[int'(i_addr)];
    end

    o_rdata = rdata;
end: rg_proc

endmodule: ram_mmu_tech
