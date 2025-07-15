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

module IntMul #(
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,                                      // Enable bit
    input logic i_unsigned,                                 // Unsigned operands
    input logic i_hsu,                                      // MULHSU instruction: signed * unsigned
    input logic i_high,                                     // High multiplied bits [127:64]
    input logic i_rv32,                                     // 32-bits operands enabled
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a1,       // Operand 1
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a2,       // Operand 2
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_res,     // Result
    output logic o_valid                                    // Result is valid
);

import river_cfg_pkg::*;
import int_mul_pkg::*;

IntMul_registers r;
IntMul_registers rin;


always_comb
begin: comb_proc
    IntMul_registers v;
    logic [RISCV_ARCH-1:0] vb_a1;
    logic [RISCV_ARCH-1:0] vb_a2;
    logic [1:0] wb_mux_lvl0;
    logic [65:0] wb_lvl0[0: 32-1];
    logic [73:0] wb_lvl2[0: 8-1];
    logic [99:0] wb_lvl4[0: 2-1];
    logic [127:0] wb_lvl5;
    logic [127:0] wb_res32;
    logic [63:0] wb_res;
    logic [63:0] vb_a1s;
    logic [63:0] vb_a2s;
    logic v_a1s_nzero;
    logic v_a2s_nzero;
    logic v_ena;

    v.busy = r.busy;
    v.ena = r.ena;
    v.a1 = r.a1;
    v.a2 = r.a2;
    v.unsign = r.unsign;
    v.high = r.high;
    v.rv32 = r.rv32;
    v.zero = r.zero;
    v.inv = r.inv;
    v.result = r.result;
    v.a1_dbg = r.a1_dbg;
    v.a2_dbg = r.a2_dbg;
    v.reference_mul = r.reference_mul;
    for (int i = 0; i < 16; i++) begin
        v.lvl1[i] = r.lvl1[i];
    end
    for (int i = 0; i < 4; i++) begin
        v.lvl3[i] = r.lvl3[i];
    end
    vb_a1 = '0;
    vb_a2 = '0;
    wb_mux_lvl0 = '0;
    for (int i = 0; i < 32; i++) begin
        wb_lvl0[i] = '0;
    end
    for (int i = 0; i < 8; i++) begin
        wb_lvl2[i] = '0;
    end
    for (int i = 0; i < 2; i++) begin
        wb_lvl4[i] = '0;
    end
    wb_lvl5 = '0;
    wb_res32 = '0;
    wb_res = '0;
    vb_a1s = '0;
    vb_a2s = '0;
    v_a1s_nzero = 1'b0;
    v_a2s_nzero = 1'b0;
    v_ena = 1'b0;


    if ((|i_a1[62: 0]) == 1'b1) begin
        v_a1s_nzero = 1'b1;
    end
    if ((v_a1s_nzero && i_a1[63]) == 1'b1) begin
        vb_a1s = ((~i_a1) + 1);
    end else begin
        vb_a1s = i_a1;
    end

    if ((|i_a2[62: 0]) == 1'b1) begin
        v_a2s_nzero = 1'b1;
    end
    if ((v_a2s_nzero && i_a2[63]) == 1'b1) begin
        vb_a2s = ((~i_a2) + 1);
    end else begin
        vb_a2s = i_a2;
    end

    v_ena = (i_ena && (~r.busy));
    v.ena = {r.ena[2: 0], v_ena};

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.inv = 1'b0;
        v.zero = 1'b0;
        if (i_rv32 == 1'b1) begin
            vb_a1 = i_a1[31: 0];
            if ((i_unsigned == 1'b0) && (i_a1[31] == 1'b1)) begin
                vb_a1[63: 32] = '1;
            end
            vb_a2 = i_a2[31: 0];
            if ((i_unsigned == 1'b0) && (i_a2[31] == 1'b1)) begin
                vb_a2[63: 32] = '1;
            end
        end else if (i_high == 1'b1) begin
            if (i_hsu == 1'b1) begin
                if ((v_a1s_nzero == 1'b0) || ((|i_a2) == 1'b0)) begin
                    v.zero = 1'b1;
                end
                v.inv = i_a1[63];
                vb_a1 = vb_a1s;
                vb_a2 = i_a2;
            end else if (i_unsigned == 1'b1) begin
                vb_a1 = i_a1;
                vb_a2 = i_a2;
            end else begin
                v.zero = ((~v_a1s_nzero) || (~v_a2s_nzero));
                v.inv = (i_a1[63] ^ i_a2[63]);
                vb_a1 = vb_a1s;
                vb_a2 = vb_a2s;
            end
        end else begin
            vb_a1 = i_a1;
            vb_a2 = i_a2;
        end
        v.a1 = vb_a1;
        v.a2 = vb_a2;
        v.rv32 = i_rv32;
        v.unsign = i_unsigned;
        v.high = i_high;

        // Just for run-rime control (not for VHDL)
        v.a1_dbg = i_a1;
        v.a2_dbg = i_a2;
    end

    if (r.ena[0] == 1'b1) begin
        for (int i = 0; i < 32; i++) begin
            wb_mux_lvl0 = r.a2[(2 * i) +: 2];
            if (wb_mux_lvl0 == 2'd0) begin
                wb_lvl0[i] = '0;
            end else if (wb_mux_lvl0 == 2'd1) begin
                wb_lvl0[i] = {2'd0, r.a1};
            end else if (wb_mux_lvl0 == 2'd2) begin
                wb_lvl0[i] = {r.a1, 1'b0};
            end else begin
                wb_lvl0[i] = ({2'd0, r.a1}
                        + {1'b0, r.a1, 1'b0});
            end
        end
        for (int i = 0; i < 16; i++) begin
            v.lvl1[i] = ({1'b0, wb_lvl0[((2 * i) + 1)], 2'd0}
                    + {3'd0, wb_lvl0[(2 * i)]});
        end
    end

    if (r.ena[1] == 1'b1) begin
        for (int i = 0; i < 8; i++) begin
            wb_lvl2[i] = ({r.lvl1[((2 * i) + 1)], 4'd0}
                    + r.lvl1[(2 * i)]);
        end
        for (int i = 0; i < 4; i++) begin
            v.lvl3[i] = ({wb_lvl2[((2 * i) + 1)], 8'd0}
                    + wb_lvl2[(2 * i)]);
        end
    end

    if (r.ena[2] == 1'b1) begin
        v.busy = 1'b0;
        for (int i = 0; i < 2; i++) begin
            wb_lvl4[i] = ({r.lvl3[((2 * i) + 1)], 16'd0}
                    + r.lvl3[(2 * i)]);
        end
        wb_lvl5 = ({wb_lvl4[1], 32'd0}
                + wb_lvl4[0]);
        if (r.rv32 == 1'b1) begin
            wb_res32[31: 0] = wb_lvl5[31: 0];
            if ((r.unsign == 1'b1) || (wb_lvl5[31] == 1'b0)) begin
                wb_res32[127: 32] = '0;
            end else begin
                wb_res32[127: 32] = '1;
            end
            v.result = wb_res32;
        end else if (r.high == 1'b1) begin
            if (r.zero == 1'b1) begin
                v.result = '0;
            end else if (r.inv == 1'b1) begin
                v.result = (~wb_lvl5);
            end else begin
                v.result = wb_lvl5;
            end
        end else begin
            v.result = wb_lvl5;
        end
    end

    wb_res = r.result[63: 0];
    if (r.high == 1'b1) begin
        wb_res = r.result[127: 64];
    end

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v.busy = 1'b0;
        v.ena = '0;
        v.a1 = '0;
        v.a2 = '0;
        v.unsign = 1'b0;
        v.high = 1'b0;
        v.rv32 = 1'b0;
        v.zero = 1'b0;
        v.inv = 1'b0;
        v.result = '0;
        v.a1_dbg = '0;
        v.a2_dbg = '0;
        v.reference_mul = 64'd0;
        for (int i = 0; i < 16; i++) begin
            v.lvl1[i] = '0;
        end
        for (int i = 0; i < 4; i++) begin
            v.lvl3[i] = '0;
        end
    end

    o_res = wb_res;
    o_valid = r.ena[3];

    rin.busy = v.busy;
    rin.ena = v.ena;
    rin.a1 = v.a1;
    rin.a2 = v.a2;
    rin.unsign = v.unsign;
    rin.high = v.high;
    rin.rv32 = v.rv32;
    rin.zero = v.zero;
    rin.inv = v.inv;
    rin.result = v.result;
    rin.a1_dbg = v.a1_dbg;
    rin.a2_dbg = v.a2_dbg;
    rin.reference_mul = v.reference_mul;
    for (int i = 0; i < 16; i++) begin
        rin.lvl1[i] = v.lvl1[i];
    end
    for (int i = 0; i < 4; i++) begin
        rin.lvl3[i] = v.lvl3[i];
    end
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r.busy <= 1'b0;
                r.ena <= '0;
                r.a1 <= '0;
                r.a2 <= '0;
                r.unsign <= 1'b0;
                r.high <= 1'b0;
                r.rv32 <= 1'b0;
                r.zero <= 1'b0;
                r.inv <= 1'b0;
                r.result <= '0;
                r.a1_dbg <= '0;
                r.a2_dbg <= '0;
                r.reference_mul <= 64'd0;
                for (int i = 0; i < 16; i++) begin
                    r.lvl1[i] <= '0;
                end
                for (int i = 0; i < 4; i++) begin
                    r.lvl3[i] <= '0;
                end
            end else begin
                r.busy <= rin.busy;
                r.ena <= rin.ena;
                r.a1 <= rin.a1;
                r.a2 <= rin.a2;
                r.unsign <= rin.unsign;
                r.high <= rin.high;
                r.rv32 <= rin.rv32;
                r.zero <= rin.zero;
                r.inv <= rin.inv;
                r.result <= rin.result;
                r.a1_dbg <= rin.a1_dbg;
                r.a2_dbg <= rin.a2_dbg;
                r.reference_mul <= rin.reference_mul;
                for (int i = 0; i < 16; i++) begin
                    r.lvl1[i] <= rin.lvl1[i];
                end
                for (int i = 0; i < 4; i++) begin
                    r.lvl3[i] <= rin.lvl3[i];
                end
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r.busy <= rin.busy;
            r.ena <= rin.ena;
            r.a1 <= rin.a1;
            r.a2 <= rin.a2;
            r.unsign <= rin.unsign;
            r.high <= rin.high;
            r.rv32 <= rin.rv32;
            r.zero <= rin.zero;
            r.inv <= rin.inv;
            r.result <= rin.result;
            r.a1_dbg <= rin.a1_dbg;
            r.a2_dbg <= rin.a2_dbg;
            r.reference_mul <= rin.reference_mul;
            for (int i = 0; i < 16; i++) begin
                r.lvl1[i] <= rin.lvl1[i];
            end
            for (int i = 0; i < 4; i++) begin
                r.lvl3[i] <= rin.lvl3[i];
            end
        end

    end: async_r_dis
endgenerate

endmodule: IntMul
