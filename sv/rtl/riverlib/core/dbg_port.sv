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

module DbgPort #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // "RIVER" Debug interface
    input logic i_dport_req_valid,                          // Debug access from DSU is valid
    input logic [river_cfg_pkg::DPortReq_Total-1:0] i_dport_type,// Debug access type
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_addr,// Debug address (register or memory)
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_wdata,// Write value
    input logic [2:0] i_dport_size,                         // reg/mem access size:0=1B;...,4=128B;
    output logic o_dport_req_ready,
    input logic i_dport_resp_ready,                         // ready to accepd response
    output logic o_dport_resp_valid,                        // Response is valid
    output logic o_dport_resp_error,                        // Something wrong during command execution
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_rdata,// Response value
    // CSR bus master interface:
    output logic o_csr_req_valid,                           // Region 0: Access to CSR bank is enabled.
    input logic i_csr_req_ready,
    output logic [river_cfg_pkg::CsrReq_TotalBits-1:0] o_csr_req_type,// Region 0: CSR operation read/modify/write
    output logic [11:0] o_csr_req_addr,                     // Address of the sub-region register
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_csr_req_data,// Write data
    input logic i_csr_resp_valid,
    output logic o_csr_resp_ready,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_csr_resp_data,// Region 0: CSR read value
    input logic i_csr_resp_exception,                       // Exception on CSR access
    input logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] i_progbuf,// progam buffer
    output logic o_progbuf_ena,                             // Execution from the progbuffer is in progress
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_progbuf_pc,// prog buffer instruction counter
    output logic [63:0] o_progbuf_instr,                    // prog buffer instruction opcode
    input logic i_csr_progbuf_end,                          // End of execution from progbuf
    input logic i_csr_progbuf_error,                        // Exception is occured during progbuf execution
    output logic [5:0] o_ireg_addr,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_ireg_wdata,// Write data
    output logic o_ireg_ena,                                // Region 1: Access to integer register bank is enabled
    output logic o_ireg_write,                              // Region 1: Integer registers bank write pulse
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_ireg_rdata,// Region 1: Integer register read value
    output logic o_mem_req_valid,                           // Type 2: request is valid
    input logic i_mem_req_ready,                            // Type 2: memory request was accepted
    input logic i_mem_req_error,                            // Type 2: memory request is invalid and cannot be processed
    output logic o_mem_req_write,                           // Type 2: is write
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_mem_req_addr,// Type 2: Debug memory request
    output logic [1:0] o_mem_req_size,                      // Type 2: memory operation size: 0=1B; 1=2B; 2=4B; 3=8B
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_mem_req_wdata,// Type 2: memory write data
    input logic i_mem_resp_valid,                           // Type 2: response is valid
    input logic i_mem_resp_error,                           // Type 2: response error (MPU or unmapped access)
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_resp_rdata,// Type 2: Memory response from memaccess module
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_pc,     // Instruction pointer
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_npc,    // Next Instruction pointer
    input logic i_e_call,                                   // pseudo-instruction CALL
    input logic i_e_ret,                                    // pseudo-instruction RET
    input logic i_e_memop_valid,                            // Memory request from executor
    input logic i_m_valid                                   // Memory request processed
);

import river_cfg_pkg::*;
import dbg_port_pkg::*;

logic [CFG_LOG2_STACK_TRACE_ADDR-1:0] wb_stack_raddr;
logic [(2 * RISCV_ARCH)-1:0] wb_stack_rdata;
logic w_stack_we;
logic [CFG_LOG2_STACK_TRACE_ADDR-1:0] wb_stack_waddr;
logic [(2 * RISCV_ARCH)-1:0] wb_stack_wdata;
DbgPort_registers r, rin;

generate
    if (CFG_LOG2_STACK_TRACE_ADDR != 0) begin: tracebuf_en
        StackTraceBuffer trbuf0 (
            .i_clk(i_clk),
            .i_raddr(wb_stack_raddr),
            .o_rdata(wb_stack_rdata),
            .i_we(w_stack_we),
            .i_waddr(wb_stack_waddr),
            .i_wdata(wb_stack_wdata)
        );
    end: tracebuf_en

endgenerate

always_comb
begin: comb_proc
    DbgPort_registers v;
    logic [CFG_LOG2_STACK_TRACE_ADDR-1:0] vb_stack_raddr;
    logic v_stack_we;
    logic [CFG_LOG2_STACK_TRACE_ADDR-1:0] vb_stack_waddr;
    logic [(2 * RISCV_ARCH)-1:0] vb_stack_wdata;
    logic v_csr_req_valid;
    logic v_csr_resp_ready;
    logic [CsrReq_TotalBits-1:0] vb_csr_req_type;
    logic [11:0] vb_csr_req_addr;
    logic [RISCV_ARCH-1:0] vb_csr_req_data;
    logic [5:0] vb_o_ireg_addr;
    logic [RISCV_ARCH-1:0] vb_o_ireg_wdata;
    logic [11:0] vb_idx;
    logic v_o_ireg_ena;
    logic v_o_ireg_write;
    logic v_mem_req_valid;
    logic v_req_ready;
    logic v_resp_valid;
    logic [63:0] vrdata;
    logic [4:0] t_idx;

    vb_stack_raddr = 0;
    v_stack_we = 0;
    vb_stack_waddr = 0;
    vb_stack_wdata = 0;
    v_csr_req_valid = 0;
    v_csr_resp_ready = 0;
    vb_csr_req_type = 0;
    vb_csr_req_addr = 0;
    vb_csr_req_data = 0;
    vb_o_ireg_addr = 0;
    vb_o_ireg_wdata = 0;
    vb_idx = 0;
    v_o_ireg_ena = 0;
    v_o_ireg_write = 0;
    v_mem_req_valid = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vrdata = 0;
    t_idx = 0;

    v = r;

    vb_idx = i_dport_addr[11: 0];
    vrdata = r.dport_rdata;

    if (CFG_LOG2_STACK_TRACE_ADDR != 0) begin
        if ((i_e_call == 1'b1) && (r.stack_trace_cnt != (STACK_TRACE_BUF_SIZE - 1))) begin
            v_stack_we = 1'b1;
            vb_stack_waddr = r.stack_trace_cnt;
            vb_stack_wdata = {i_e_npc, i_e_pc};
            v.stack_trace_cnt = (r.stack_trace_cnt + 1);
        end else if ((i_e_ret == 1'b1) && ((|r.stack_trace_cnt) == 1'b1)) begin
            v.stack_trace_cnt = (r.stack_trace_cnt - 1);
        end
    end

    case (r.dstate)
    idle: begin
        v_req_ready = 1'b1;
        vrdata = '0;
        v.req_accepted = 1'b0;
        v.resp_error = 1'b0;
        v.progbuf_ena = 1'b0;
        if (i_dport_req_valid == 1'b1) begin
            if (i_dport_type[DPortReq_RegAccess] == 1'b1) begin
                v.dport_write = i_dport_type[DPortReq_Write];
                v.dport_addr = i_dport_addr;
                v.dport_wdata = i_dport_wdata;
                if (i_dport_addr[15: 12] == 4'h0) begin
                    v.dstate = csr_region;
                end else if (i_dport_addr[15: 12] == 4'h1) begin
                    v.dstate = reg_bank;
                end else if (i_dport_addr[15: 12] == 4'hc) begin
                    // non-standard extension
                    if (vb_idx == 12'h040) begin
                        v.dstate = reg_stktr_cnt;
                    end else if ((vb_idx >= 128) && (vb_idx < (128 + (2 * STACK_TRACE_BUF_SIZE)))) begin
                        v.dstate = reg_stktr_buf_adr;
                    end
                end else begin
                    v.dstate = wait_to_accept;
                end
            end else if (i_dport_type[DPortReq_Progexec] == 1'b1) begin
                v.dstate = exec_progbuf_start;
            end else if (i_dport_type[DPortReq_MemAccess] == 1'b1) begin
                v.dstate = abstract_mem_request;
                v.dport_write = i_dport_type[DPortReq_Write];
                v.dport_addr = i_dport_addr;
                v.dport_wdata = i_dport_wdata;
                v.dport_size = i_dport_size[1: 0];
            end else begin
                // Unsupported request
                v.dstate = wait_to_accept;
                v.resp_error = 1'b1;
            end
        end
    end
    csr_region: begin
        v_csr_req_valid = (~r.req_accepted);
        v_csr_resp_ready = r.req_accepted;
        if ((r.req_accepted == 1'b0) && (i_csr_req_ready == 1'b1)) begin
            v.req_accepted = 1'b1;
        end
        if (r.dport_write == 1'b1) begin
            vb_csr_req_type = CsrReq_WriteCmd;
        end else begin
            vb_csr_req_type = CsrReq_ReadCmd;
        end
        vb_csr_req_addr = r.dport_addr[11: 0];
        vb_csr_req_data = r.dport_wdata;
        if ((r.req_accepted && i_csr_resp_valid) == 1'b1) begin
            vrdata = i_csr_resp_data;
            v.dstate = wait_to_accept;
        end
    end
    reg_bank: begin
        v_o_ireg_ena = 1'b1;
        vb_o_ireg_addr = r.dport_addr[5: 0];
        vrdata = i_ireg_rdata;
        if (r.dport_write == 1'b1) begin
            v_o_ireg_write = 1'b1;
            vb_o_ireg_wdata = r.dport_wdata;
        end
        v.dstate = wait_to_accept;
    end
    reg_stktr_cnt: begin
        vrdata = '0;
        vrdata[(CFG_LOG2_STACK_TRACE_ADDR - 1): 0] = r.stack_trace_cnt;
        if (r.dport_write == 1'b1) begin
            v.stack_trace_cnt = r.dport_wdata[(CFG_LOG2_STACK_TRACE_ADDR - 1): 0];
        end
        v.dstate = wait_to_accept;
    end
    reg_stktr_buf_adr: begin
        vb_stack_raddr = r.dport_addr[CFG_LOG2_STACK_TRACE_ADDR: 1];
        v.dstate = reg_stktr_buf_dat;
    end
    reg_stktr_buf_dat: begin
        if (r.dport_addr[0] == 1'b0) begin
            vrdata = wb_stack_rdata[(RISCV_ARCH - 1): 0];
        end else begin
            vrdata = wb_stack_rdata[((2 * RISCV_ARCH) - 1): RISCV_ARCH];
        end
        v.dstate = wait_to_accept;
    end
    exec_progbuf_start: begin
        v.progbuf_ena = 1'b1;
        v.progbuf_pc = '0;
        v.progbuf_instr = i_progbuf[63: 0];
        v.dstate = exec_progbuf_next;
    end
    exec_progbuf_next: begin
        if (i_csr_progbuf_end == 1'b1) begin
            v.progbuf_ena = 1'b0;
            v.resp_error = i_csr_progbuf_error;
            v.dstate = wait_to_accept;
        end else if (i_e_memop_valid == 1'b1) begin
            v.dstate = exec_progbuf_waitmemop;
        end else begin
            t_idx = i_e_npc[5: 2];
            v.progbuf_pc = {58'h000000000000000, {i_e_npc[5: 2], 2'h0}};
            if (t_idx == 4'hf) begin
                v.progbuf_instr = {32'h00000000, i_progbuf[255: 224]};
            end else begin
                v.progbuf_instr = i_progbuf[(32 * t_idx) +: 64];
            end
        end
    end
    exec_progbuf_waitmemop: begin
        if (i_m_valid == 1'b1) begin
            v.dstate = exec_progbuf_next;
        end
    end
    abstract_mem_request: begin
        v_mem_req_valid = 1'b1;
        if (i_mem_req_ready == 1'b1) begin
            if (i_mem_req_error == 1'b1) begin
                v.dstate = wait_to_accept;
                v.resp_error = 1'b1;
                vrdata = '1;
            end else begin
                v.dstate = abstract_mem_response;
            end
        end
    end
    abstract_mem_response: begin
        vrdata = i_mem_resp_rdata;
        if (i_mem_resp_valid == 1'b1) begin
            v.dstate = wait_to_accept;
            v.resp_error = i_mem_resp_error;
        end
    end
    wait_to_accept: begin
        v_resp_valid = 1'b1;
        if (i_dport_resp_ready == 1'b1) begin
            v.dstate = idle;
        end
    end
    default: begin
    end
    endcase

    v.dport_rdata = vrdata;

    if (~async_reset && i_nrst == 1'b0) begin
        v = DbgPort_r_reset;
    end

    wb_stack_raddr = vb_stack_raddr;
    w_stack_we = v_stack_we;
    wb_stack_waddr = vb_stack_waddr;
    wb_stack_wdata = vb_stack_wdata;

    o_csr_req_valid = v_csr_req_valid;
    o_csr_req_type = vb_csr_req_type;
    o_csr_req_addr = vb_csr_req_addr;
    o_csr_req_data = vb_csr_req_data;
    o_csr_resp_ready = v_csr_resp_ready;
    o_ireg_addr = vb_o_ireg_addr;
    o_ireg_wdata = vb_o_ireg_wdata;
    o_ireg_ena = v_o_ireg_ena;
    o_ireg_write = v_o_ireg_write;
    o_mem_req_valid = v_mem_req_valid;
    o_mem_req_write = r.dport_write;
    o_mem_req_addr = r.dport_addr;
    o_mem_req_wdata = r.dport_wdata;
    o_mem_req_size = r.dport_size;
    o_progbuf_ena = r.progbuf_ena;
    o_progbuf_pc = r.progbuf_pc;
    o_progbuf_instr = r.progbuf_instr;
    o_dport_req_ready = v_req_ready;
    o_dport_resp_valid = v_resp_valid;
    o_dport_resp_error = r.resp_error;
    o_dport_rdata = r.dport_rdata;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DbgPort_r_reset;
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

endmodule: DbgPort
