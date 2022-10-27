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

module ram_cache_bwe_tech #(
    parameter int abits = 6,
    parameter int dbits = 128
)
(
    input logic i_clk,                                      // CPU clock
    input logic [abits-1:0] i_addr,
    input logic [(dbits / 8)-1:0] i_wena,
    input logic [dbits-1:0] i_wdata,
    output logic [dbits-1:0] o_rdata
);

logic wb_we[0: (dbits / 8) - 1];
logic [7:0] wb_wdata[0: (dbits / 8) - 1];
logic [7:0] wb_rdata[0: (dbits / 8) - 1];

for (genvar i = 0; i < (dbits / 8); i++) begin: rxgen
    ram_tech #(
        .abits(abits),
        .dbits(8)
    ) rx (
        .i_clk(i_clk),
        .i_addr(i_addr),
        .i_wena(wb_we[i]),
        .i_wdata(wb_wdata[i]),
        .o_rdata(wb_rdata[i])
    );

end: rxgen

always_comb
begin: comb_proc
    logic [dbits-1:0] vb_rdata;

    vb_rdata = 0;

    for (int i = 0; i < (dbits / 8); i++) begin
        wb_we[i] = i_wena[i];
        wb_wdata[i] = i_wdata[(8 * i) +: 8];
        vb_rdata[(8 * i) +: 8] = wb_rdata[i];
    end
    o_rdata = vb_rdata;
end: comb_proc

endmodule: ram_cache_bwe_tech
