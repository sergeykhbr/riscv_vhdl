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

module PMA(
    input logic i_clk,                                      // CPU clock
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_iaddr,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_daddr,
    output logic o_icached,                                 // Hardcoded cached memory range for I$
    output logic o_dcached                                  // Hardcoded cached memory range for D$
);

import river_cfg_pkg::*;
import pma_pkg::*;

always_comb
begin: comb_proc
    logic v_icached;
    logic v_dcached;

    v_icached = 0;
    v_dcached = 0;

    v_icached = 1'b1;
    if ((i_iaddr & (~CLINT_MASK)) == CLINT_BAR) begin
        v_icached = 1'b0;
    end else if ((i_iaddr & (~PLIC_MASK)) == PLIC_BAR) begin
        v_icached = 1'b0;
    end else if ((i_iaddr & (~IO1_MASK)) == IO1_BAR) begin
        v_icached = 1'b0;
    end

    v_dcached = 1'b1;
    if ((i_daddr & (~CLINT_MASK)) == CLINT_BAR) begin
        v_dcached = 1'b0;
    end else if ((i_daddr & (~PLIC_MASK)) == PLIC_BAR) begin
        v_dcached = 1'b0;
    end else if ((i_daddr & (~IO1_MASK)) == IO1_BAR) begin
        v_dcached = 1'b0;
    end


    o_icached = v_icached;
    o_dcached = v_dcached;
end: comb_proc

endmodule: PMA
