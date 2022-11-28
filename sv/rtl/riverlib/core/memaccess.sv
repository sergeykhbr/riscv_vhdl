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

module MemAccess #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_pc,     // Execution stage instruction pointer
    input logic [31:0] i_e_instr,                           // Execution stage instruction value
    input logic i_flushd_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_flushd_addr,
    output logic o_flushd,
    input logic i_mmu_ena,                                  // MMU enabled
    input logic i_mmu_sv39,                                 // MMU sv39 mode is enabled
    input logic i_mmu_sv48,                                 // MMU sv48 mode is enabled
    output logic o_mmu_ena,                                 // Delayed MMU enabled
    output logic o_mmu_sv39,                                // Delayed MMU sv39 mode is enabled
    output logic o_mmu_sv48,                                // Delayed MMU sv48 mode is enabled
    input logic [5:0] i_reg_waddr,                          // Register address to be written (0=no writing)
    input logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] i_reg_wtag,// Register tag for writeback operation
    input logic i_memop_valid,                              // Memory request is valid
    input logic i_memop_debug,                              // Memory debug request
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_memop_wdata,// Register value to be written
    input logic i_memop_sign_ext,                           // Load data with sign extending (if less than 8 Bytes)
    input logic [river_cfg_pkg::MemopType_Total-1:0] i_memop_type,// [0] 1=store;0=Load data from memory and write to i_res_addr
    input logic [1:0] i_memop_size,                         // Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_memop_addr,// Memory access address
    output logic o_memop_ready,                             // Ready to accept memop request
    output logic o_wb_wena,                                 // Write enable signal
    output logic [5:0] o_wb_waddr,                          // Output register address (0 = x0 = no write)
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_wb_wdata,// Register value
    output logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] o_wb_wtag,
    input logic i_wb_ready,
    // Memory interface:
    input logic i_mem_req_ready,                            // Data cache is ready to accept read/write request
    output logic o_mem_valid,                               // Memory request is valid
    output logic [river_cfg_pkg::MemopType_Total-1:0] o_mem_type,// Memory operation type
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_mem_addr,// Data path requested address
    output logic [63:0] o_mem_wdata,                        // Data path requested data (write transaction)
    output logic [7:0] o_mem_wstrb,                         // 8-bytes aligned strobs
    output logic [1:0] o_mem_size,                          // 1,2,4 or 8-bytes operation for uncached access
    input logic i_mem_data_valid,                           // Data path memory response is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_data_addr,// Data path memory response address
    input logic [63:0] i_mem_data,                          // Data path memory response value
    output logic o_mem_resp_ready,                          // Pipeline is ready to accept memory operation response
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,      // executed memory/flush request only
    output logic o_valid,                                   // memory/flush operation completed
    output logic o_idle,                                    // All memory operation completed
    output logic o_debug_valid                              // Debug request processed, response is valid
);

import river_cfg_pkg::*;
import memaccess_pkg::*;

logic queue_we;
logic queue_re;
logic [QUEUE_WIDTH-1:0] queue_data_i;
logic [QUEUE_WIDTH-1:0] queue_data_o;
logic queue_nempty;
logic queue_full;
MemAccess_registers r, rin;

Queue #(
    .async_reset(async_reset),
    .abits(CFG_MEMACCESS_QUEUE_DEPTH),
    .dbits(QUEUE_WIDTH)
) queue0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_re(queue_re),
    .i_we(queue_we),
    .i_wdata(queue_data_i),
    .o_rdata(queue_data_o),
    .o_full(queue_full),
    .o_nempty(queue_nempty)
);

always_comb
begin: comb_proc
    MemAccess_registers v;
    logic [RISCV_ARCH-1:0] vb_req_addr;
    logic [63:0] vb_memop_wdata;
    logic [7:0] vb_memop_wstrb;
    logic v_mem_valid;
    logic v_mem_debug;
    logic [MemopType_Total-1:0] vb_mem_type;
    logic v_mem_sign_ext;
    logic [1:0] vb_mem_sz;
    logic [RISCV_ARCH-1:0] vb_mem_addr;
    logic [63:0] vb_mem_rdata;
    logic v_queue_re;
    logic v_flushd;
    logic v_mmu_ena;
    logic v_mmu_sv39;
    logic v_mmu_sv48;
    logic [CFG_REG_TAG_WIDTH-1:0] vb_res_wtag;
    logic [63:0] vb_mem_wdata;
    logic [7:0] vb_mem_wstrb;
    logic [63:0] vb_mem_resp_shifted;
    logic [63:0] vb_mem_data_unsigned;
    logic [63:0] vb_mem_data_signed;
    logic [RISCV_ARCH-1:0] vb_res_data;
    logic [5:0] vb_res_addr;
    logic [RISCV_ARCH-1:0] vb_e_pc;
    logic [31:0] vb_e_instr;
    logic v_memop_ready;
    logic v_o_wena;
    logic [5:0] vb_o_waddr;
    logic [RISCV_ARCH-1:0] vb_o_wdata;
    logic [CFG_REG_TAG_WIDTH-1:0] vb_o_wtag;
    logic v_valid;
    logic v_idle;
    logic t_memop_debug;

    vb_req_addr = 0;
    vb_memop_wdata = 0;
    vb_memop_wstrb = 0;
    v_mem_valid = 0;
    v_mem_debug = 0;
    vb_mem_type = 0;
    v_mem_sign_ext = 0;
    vb_mem_sz = 0;
    vb_mem_addr = 0;
    vb_mem_rdata = 0;
    v_queue_re = 0;
    v_flushd = 0;
    v_mmu_ena = 0;
    v_mmu_sv39 = 0;
    v_mmu_sv48 = 0;
    vb_res_wtag = 0;
    vb_mem_wdata = 0;
    vb_mem_wstrb = 0;
    vb_mem_resp_shifted = 0;
    vb_mem_data_unsigned = 0;
    vb_mem_data_signed = 0;
    vb_res_data = 0;
    vb_res_addr = 0;
    vb_e_pc = 0;
    vb_e_instr = 0;
    v_memop_ready = 0;
    v_o_wena = 0;
    vb_o_waddr = 0;
    vb_o_wdata = 0;
    vb_o_wtag = 0;
    v_valid = 0;
    v_idle = 0;
    t_memop_debug = 0;

    v = r;

    v.valid = 1'b0;                                         // valid on next clock

    if (i_flushd_valid == 1'b1) begin
        vb_req_addr = i_flushd_addr;
    end else begin
        vb_req_addr = i_memop_addr;
    end

    case (i_memop_size)
    2'h0: begin
        vb_memop_wdata = {i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0],
                i_memop_wdata[7: 0]};
        if (i_memop_addr[2: 0] == 3'h0) begin
            vb_memop_wstrb = 8'h01;
        end else if (i_memop_addr[2: 0] == 3'h1) begin
            vb_memop_wstrb = 8'h02;
        end else if (i_memop_addr[2: 0] == 3'h2) begin
            vb_memop_wstrb = 8'h04;
        end else if (i_memop_addr[2: 0] == 3'h3) begin
            vb_memop_wstrb = 8'h08;
        end else if (i_memop_addr[2: 0] == 3'h4) begin
            vb_memop_wstrb = 8'h10;
        end else if (i_memop_addr[2: 0] == 3'h5) begin
            vb_memop_wstrb = 8'h20;
        end else if (i_memop_addr[2: 0] == 3'h6) begin
            vb_memop_wstrb = 8'h40;
        end else if (i_memop_addr[2: 0] == 3'h7) begin
            vb_memop_wstrb = 8'h80;
        end
    end
    2'h1: begin
        vb_memop_wdata = {i_memop_wdata[15: 0],
                i_memop_wdata[15: 0],
                i_memop_wdata[15: 0],
                i_memop_wdata[15: 0]};
        if (i_memop_addr[2: 1] == 2'h0) begin
            vb_memop_wstrb = 8'h03;
        end else if (i_memop_addr[2: 1] == 2'h1) begin
            vb_memop_wstrb = 8'h0c;
        end else if (i_memop_addr[2: 1] == 2'h2) begin
            vb_memop_wstrb = 8'h30;
        end else begin
            vb_memop_wstrb = 8'hc0;
        end
    end
    2'h2: begin
        vb_memop_wdata = {i_memop_wdata[31: 0],
                i_memop_wdata[31: 0]};
        if (i_memop_addr[2] == 1'b1) begin
            vb_memop_wstrb = 8'hf0;
        end else begin
            vb_memop_wstrb = 8'h0f;
        end
    end
    2'h3: begin
        vb_memop_wdata = i_memop_wdata;
        vb_memop_wstrb = 8'hff;
    end
    default: begin
    end
    endcase

    // Form Queue inputs:
    t_memop_debug = i_memop_debug;                          // looks like bug in systemc, cannot handle bool properly
    queue_data_i = {t_memop_debug,
            i_flushd_valid,
            i_mmu_ena,
            i_mmu_sv39,
            i_mmu_sv48,
            i_reg_wtag,
            vb_memop_wdata,
            vb_memop_wstrb,
            i_memop_wdata,
            i_reg_waddr,
            i_e_instr,
            i_e_pc,
            i_memop_size,
            i_memop_sign_ext,
            i_memop_type,
            vb_req_addr};
    queue_we = ((i_memop_valid | i_flushd_valid) & (~queue_full));

    // Split Queue outputs:
    v_mem_debug = queue_data_o[316];
    v_flushd = queue_data_o[315];
    v_mmu_ena = queue_data_o[314];
    v_mmu_sv39 = queue_data_o[313];
    v_mmu_sv48 = queue_data_o[312];
    vb_res_wtag = queue_data_o[311: 309];
    vb_mem_wdata = queue_data_o[308: 245];
    vb_mem_wstrb = queue_data_o[244: 237];
    vb_res_data = queue_data_o[236: 173];
    vb_res_addr = queue_data_o[172: 167];
    vb_e_instr = queue_data_o[166: 135];
    vb_e_pc = queue_data_o[134: 71];
    vb_mem_sz = queue_data_o[70: 69];
    v_mem_sign_ext = queue_data_o[68];
    vb_mem_type = queue_data_o[67: 64];
    vb_mem_addr = queue_data_o[63: 0];

    case (r.memop_addr[2: 0])
    3'h1: begin
        vb_mem_resp_shifted[55: 0] = i_mem_data[63: 8];
    end
    3'h2: begin
        vb_mem_resp_shifted[47: 0] = i_mem_data[63: 16];
    end
    3'h3: begin
        vb_mem_resp_shifted[39: 0] = i_mem_data[63: 24];
    end
    3'h4: begin
        vb_mem_resp_shifted[31: 0] = i_mem_data[63: 32];
    end
    3'h5: begin
        vb_mem_resp_shifted[23: 0] = i_mem_data[63: 40];
    end
    3'h6: begin
        vb_mem_resp_shifted[15: 0] = i_mem_data[63: 48];
    end
    3'h7: begin
        vb_mem_resp_shifted[7: 0] = i_mem_data[63: 56];
    end
    default: begin
        vb_mem_resp_shifted = i_mem_data;
    end
    endcase
    case (r.memop_size)
    MEMOP_1B: begin
        vb_mem_data_unsigned[7: 0] = vb_mem_resp_shifted[7: 0];
        vb_mem_data_signed[7: 0] = vb_mem_resp_shifted[7: 0];
        if (vb_mem_resp_shifted[7] == 1'b1) begin
            vb_mem_data_signed[63: 8] = '1;
        end
    end
    MEMOP_2B: begin
        vb_mem_data_unsigned[15: 0] = vb_mem_resp_shifted[15: 0];
        vb_mem_data_signed[15: 0] = vb_mem_resp_shifted[15: 0];
        if (vb_mem_resp_shifted[15] == 1'b1) begin
            vb_mem_data_signed[63: 16] = '1;
        end
    end
    MEMOP_4B: begin
        vb_mem_data_unsigned[31: 0] = vb_mem_resp_shifted[31: 0];
        vb_mem_data_signed[31: 0] = vb_mem_resp_shifted[31: 0];
        if (vb_mem_resp_shifted[31] == 1'b1) begin
            vb_mem_data_signed[63: 32] = '1;
        end
    end
    default: begin
        vb_mem_data_unsigned = vb_mem_resp_shifted;
        vb_mem_data_signed = vb_mem_resp_shifted;
    end
    endcase

    if ((r.memop_type[MemopType_Store] == 1'b0) || (r.memop_type[MemopType_Release] == 1'b1)) begin
        if (r.memop_sign_ext == 1'b1) begin
            vb_mem_rdata = vb_mem_data_signed;
        end else begin
            vb_mem_rdata = vb_mem_data_unsigned;
        end
    end else begin
        vb_mem_rdata = r.memop_res_data;
    end
    case (r.state)
    State_Idle: begin
        v_queue_re = 1'b1;
        if (queue_nempty == 1'b1) begin
            v.pc = vb_e_pc;
            v_mem_valid = (~v_flushd);
            v.mmu_ena = v_mmu_ena;
            v.mmu_sv39 = v_mmu_sv39;
            v.mmu_sv48 = v_mmu_sv48;
            v.memop_res_pc = vb_e_pc;
            v.memop_res_instr = vb_e_instr;
            v.memop_res_addr = vb_res_addr;
            v.memop_res_wtag = vb_res_wtag;
            v.memop_res_data = vb_res_data;
            if (((|vb_res_addr) == 1'b1)
                    && (((~vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1'b1)) begin
                v.memop_res_wena = 1'b1;
            end else begin
                v.memop_res_wena = 1'b0;
            end
            v.memop_addr = vb_mem_addr;
            v.memop_wdata = vb_mem_wdata;
            v.memop_wstrb = vb_mem_wstrb;
            v.memop_type = vb_mem_type;
            v.memop_debug = v_mem_debug;
            v.memop_sign_ext = v_mem_sign_ext;
            v.memop_size = vb_mem_sz;
            if (v_flushd == 1'b1) begin
                // do nothing
                v.valid = 1'b1;
            end else if (i_mem_req_ready == 1'b1) begin
                v.state = State_WaitResponse;
            end else begin
                v.state = State_WaitReqAccept;
            end
        end
    end
    State_WaitReqAccept: begin
        v_mem_valid = 1'b1;
        v_mmu_ena = r.mmu_ena;
        v_mmu_sv39 = r.mmu_sv39;
        v_mmu_sv48 = r.mmu_sv48;
        vb_mem_type = r.memop_type;
        vb_mem_sz = r.memop_size;
        vb_mem_addr = r.memop_addr;
        vb_mem_wdata = r.memop_wdata;
        vb_mem_wstrb = r.memop_wstrb;
        vb_res_data = r.memop_res_data;
        if (i_mem_req_ready == 1'b1) begin
            v.state = State_WaitResponse;
        end
    end
    State_WaitResponse: begin
        if (i_mem_data_valid == 1'b0) begin
            // Do nothing
        end else begin
            v_o_wena = r.memop_res_wena;
            vb_o_waddr = r.memop_res_addr;
            vb_o_wdata = vb_mem_rdata;
            vb_o_wtag = r.memop_res_wtag;

            v_queue_re = 1'b1;
            if ((r.memop_res_wena == 1'b1) && (r.memop_debug == 1'b0) && (i_wb_ready == 1'b0)) begin
                // Inject only one clock hold-on and wait a couple of clocks while writeback finished
                v_queue_re = 1'b0;
                v.state = State_Hold;
                v.hold_rdata = vb_mem_rdata;
            end else if (queue_nempty == 1'b1) begin
                v_valid = 1'b1;
                v.pc = vb_e_pc;
                v_mem_valid = (~v_flushd);
                v.mmu_ena = v_mmu_ena;
                v.mmu_sv39 = v_mmu_sv39;
                v.mmu_sv48 = v_mmu_sv48;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                if (((|vb_res_addr) == 1'b1)
                        && (((~vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1'b1)) begin
                    v.memop_res_wena = 1'b1;
                end else begin
                    v.memop_res_wena = 1'b0;
                end
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_type = vb_mem_type;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;
                v.memop_debug = v_mem_debug;

                if (v_flushd == 1'b1) begin
                    v.state = State_Idle;
                    v.valid = 1'b1;
                end else if (i_mem_req_ready == 1'b1) begin
                    v.state = State_WaitResponse;
                end else begin
                    v.state = State_WaitReqAccept;
                end
            end else begin
                v.state = State_Idle;
                v_valid = 1'b1;
            end
        end
    end
    State_Hold: begin
        v_o_wena = r.memop_res_wena;
        vb_o_waddr = r.memop_res_addr;
        vb_o_wdata = r.hold_rdata;
        vb_o_wtag = r.memop_res_wtag;
        if (i_wb_ready == 1'b1) begin
            v_valid = 1'b1;
            v_queue_re = 1'b1;
            if (queue_nempty == 1'b1) begin
                v.pc = vb_e_pc;
                v_mem_valid = (~v_flushd);
                v.mmu_ena = v_mmu_ena;
                v.mmu_sv39 = v_mmu_sv39;
                v.mmu_sv48 = v_mmu_sv48;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                if (((|vb_res_addr) == 1'b1)
                        && (((~vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1'b1)) begin
                    v.memop_res_wena = 1'b1;
                end else begin
                    v.memop_res_wena = 1'b0;
                end
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_type = vb_mem_type;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;
                v.memop_debug = v_mem_debug;

                if (v_flushd == 1'b1) begin
                    v.state = State_Idle;
                    v.valid = 1'b1;
                end else if (i_mem_req_ready == 1'b1) begin
                    v.state = State_WaitResponse;
                end else begin
                    v.state = State_WaitReqAccept;
                end
            end else begin
                v.state = State_Idle;
            end
        end
    end
    default: begin
    end
    endcase

    v_memop_ready = 1'b1;
    if (queue_full == 1'b1) begin
        v_memop_ready = 1'b0;
    end

    if ((queue_nempty == 1'b0) && (r.state == State_Idle)) begin
        v_idle = 1'b1;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = MemAccess_r_reset;
    end

    queue_re = v_queue_re;
    o_flushd = (queue_nempty && v_flushd && v_queue_re);
    o_mmu_ena = v_mmu_ena;
    o_mmu_sv39 = v_mmu_sv39;
    o_mmu_sv48 = v_mmu_sv48;
    o_mem_resp_ready = 1'b1;
    o_mem_valid = v_mem_valid;
    o_mem_type = vb_mem_type;
    o_mem_addr = vb_mem_addr;
    o_mem_wdata = vb_mem_wdata;
    o_mem_wstrb = vb_mem_wstrb;
    o_mem_size = vb_mem_sz;
    o_memop_ready = v_memop_ready;
    o_wb_wena = (v_o_wena && (~r.memop_debug));
    o_wb_waddr = vb_o_waddr;
    o_wb_wdata = vb_o_wdata;
    o_wb_wtag = vb_o_wtag;
    o_pc = r.pc;
    o_valid = ((r.valid || v_valid) && (~r.memop_debug));
    o_idle = v_idle;
    o_debug_valid = ((r.valid || v_valid) && r.memop_debug);

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= MemAccess_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: MemAccess
