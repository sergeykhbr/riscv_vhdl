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

module RiverAmba #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned hartid = 0,
    parameter bit fpu_ena = 1'b1,
    parameter bit coherence_ena = 1'b0,
    parameter bit tracer_ena = 1'b1,
    parameter int unsigned ilog2_nways = 2,                 // I$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned ilog2_lines_per_way = 7,         // I$ Cache length: 7=16KB; 8=32KB; ..
    parameter int unsigned dlog2_nways = 2,                 // D$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned dlog2_lines_per_way = 7          // D$ Cache length: 7=16KB; 8=32KB; ..
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    input types_river_pkg::axi4_l1_in_type i_msti,
    output types_river_pkg::axi4_l1_out_type o_msto,
    input types_river_pkg::dport_in_type i_dport,
    output types_river_pkg::dport_out_type o_dport,
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    output logic o_flush_l2,                                // Flush L2 after D$ has been finished
    output logic o_halted,                                  // CPU halted via debug interface
    output logic o_available,                               // CPU was instantitated of stubbed
    input logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] i_progbuf// progam buffer
);

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;
import river_amba_pkg::*;

logic req_mem_ready_i;
logic req_mem_path_o;
logic req_mem_valid_o;
logic [REQ_MEM_TYPE_BITS-1:0] req_mem_type_o;
logic [2:0] req_mem_size_o;
logic [CFG_CPU_ADDR_BITS-1:0] req_mem_addr_o;
logic [L1CACHE_BYTES_PER_LINE-1:0] req_mem_strob_o;
logic [L1CACHE_LINE_BITS-1:0] req_mem_data_o;
logic [L1CACHE_LINE_BITS-1:0] resp_mem_data_i;
logic resp_mem_valid_i;
logic resp_mem_load_fault_i;
logic resp_mem_store_fault_i;
// D$ Snoop interface
logic req_snoop_valid_i;
logic [SNOOP_REQ_TYPE_BITS-1:0] req_snoop_type_i;
logic req_snoop_ready_o;
logic [CFG_CPU_ADDR_BITS-1:0] req_snoop_addr_i;
logic resp_snoop_ready_i;
logic resp_snoop_valid_o;
logic [L1CACHE_LINE_BITS-1:0] resp_snoop_data_o;
logic [DTAG_FL_TOTAL-1:0] resp_snoop_flags_o;
logic w_dporti_haltreq;
logic w_dporti_resumereq;
logic w_dporti_resethaltreq;
logic w_dporti_hartreset;
logic w_dporti_req_valid;
logic [DPortReq_Total-1:0] wb_dporti_dtype;
logic [RISCV_ARCH-1:0] wb_dporti_addr;
logic [RISCV_ARCH-1:0] wb_dporti_wdata;
logic [2:0] wb_dporti_size;
logic w_dporti_resp_ready;
logic w_dporto_req_ready;
logic w_dporto_resp_valid;
logic w_dporto_resp_error;
logic [RISCV_ARCH-1:0] wb_dporto_rdata;
RiverAmba_registers r, rin;

function logic [3:0] reqtype2arsnoop(input logic [REQ_MEM_TYPE_BITS-1:0] reqtype);
logic [3:0] ret;
begin
    ret = '0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 1'b0) begin
        ret = ARSNOOP_READ_NO_SNOOP;
    end else begin
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 1'b0) begin
            ret = ARSNOOP_READ_SHARED;
        end else begin
            ret = ARSNOOP_READ_MAKE_UNIQUE;
        end
    end
    return ret;
end
endfunction: reqtype2arsnoop

function logic [3:0] reqtype2awsnoop(input logic [REQ_MEM_TYPE_BITS-1:0] reqtype);
logic [3:0] ret;
begin
    ret = '0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 1'b0) begin
        ret = AWSNOOP_WRITE_NO_SNOOP;
    end else begin
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 1'b0) begin
            ret = AWSNOOP_WRITE_BACK;
        end else begin
            ret = AWSNOOP_WRITE_LINE_UNIQUE;
        end
    end
    return ret;
end
endfunction: reqtype2awsnoop

RiverTop #(
    .async_reset(async_reset),
    .hartid(hartid),
    .fpu_ena(fpu_ena),
    .coherence_ena(coherence_ena),
    .tracer_ena(tracer_ena),
    .ilog2_nways(ilog2_nways),
    .ilog2_lines_per_way(ilog2_lines_per_way),
    .dlog2_nways(dlog2_nways),
    .dlog2_lines_per_way(dlog2_lines_per_way)
) river0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mtimer(i_mtimer),
    .i_req_mem_ready(req_mem_ready_i),
    .o_req_mem_path(req_mem_path_o),
    .o_req_mem_valid(req_mem_valid_o),
    .o_req_mem_type(req_mem_type_o),
    .o_req_mem_size(req_mem_size_o),
    .o_req_mem_addr(req_mem_addr_o),
    .o_req_mem_strob(req_mem_strob_o),
    .o_req_mem_data(req_mem_data_o),
    .i_resp_mem_valid(resp_mem_valid_i),
    .i_resp_mem_path(r.req_path),
    .i_resp_mem_data(resp_mem_data_i),
    .i_resp_mem_load_fault(resp_mem_load_fault_i),
    .i_resp_mem_store_fault(resp_mem_store_fault_i),
    .i_req_snoop_valid(req_snoop_valid_i),
    .i_req_snoop_type(req_snoop_type_i),
    .o_req_snoop_ready(req_snoop_ready_o),
    .i_req_snoop_addr(req_snoop_addr_i),
    .i_resp_snoop_ready(resp_snoop_ready_i),
    .o_resp_snoop_valid(resp_snoop_valid_o),
    .o_resp_snoop_data(resp_snoop_data_o),
    .o_resp_snoop_flags(resp_snoop_flags_o),
    .o_flush_l2(o_flush_l2),
    .i_irq_pending(i_irq_pending),
    .i_haltreq(w_dporti_haltreq),
    .i_resumereq(w_dporti_resumereq),
    .i_dport_req_valid(w_dporti_req_valid),
    .i_dport_type(wb_dporti_dtype),
    .i_dport_addr(wb_dporti_addr),
    .i_dport_wdata(wb_dporti_wdata),
    .i_dport_size(wb_dporti_size),
    .o_dport_req_ready(w_dporto_req_ready),
    .i_dport_resp_ready(w_dporti_resp_ready),
    .o_dport_resp_valid(w_dporto_resp_valid),
    .o_dport_resp_error(w_dporto_resp_error),
    .o_dport_rdata(wb_dporto_rdata),
    .i_progbuf(i_progbuf),
    .o_halted(o_halted)
);


always_comb
begin: comb_proc
    RiverAmba_registers v;
    logic v_resp_mem_valid;
    logic v_mem_er_load_fault;
    logic v_mem_er_store_fault;
    logic v_next_ready;
    axi4_l1_out_type vmsto;
    dport_out_type vdporto;
    logic v_snoop_next_ready;
    logic req_snoop_valid;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_req_snoop_addr;
    logic [SNOOP_REQ_TYPE_BITS-1:0] vb_req_snoop_type;
    logic v_cr_valid;
    logic [4:0] vb_cr_resp;
    logic v_cd_valid;
    logic [L1CACHE_LINE_BITS-1:0] vb_cd_data;

    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;
    vmsto = axi4_l1_out_none;
    vdporto = dport_out_none;
    v_snoop_next_ready = 0;
    req_snoop_valid = 0;
    vb_req_snoop_addr = 0;
    vb_req_snoop_type = 0;
    v_cr_valid = 0;
    vb_cr_resp = 0;
    v_cd_valid = 0;
    vb_cd_data = 0;

    v = r;


    w_dporti_haltreq = i_dport.haltreq;                     // systemc compatibility
    w_dporti_resumereq = i_dport.resumereq;                 // systemc compatibility
    w_dporti_resethaltreq = i_dport.resethaltreq;           // systemc compatibility
    w_dporti_hartreset = i_dport.hartreset;                 // systemc compatibility
    w_dporti_req_valid = i_dport.req_valid;                 // systemc compatibility
    wb_dporti_dtype = i_dport.dtype;                        // systemc compatibility
    wb_dporti_addr = i_dport.addr;                          // systemc compatibility
    wb_dporti_wdata = i_dport.wdata;                        // systemc compatibility
    wb_dporti_size = i_dport.size;                          // systemc compatibility
    w_dporti_resp_ready = i_dport.resp_ready;               // systemc compatibility

    vdporto.req_ready = w_dporto_req_ready;                 // systemc compatibility
    vdporto.resp_valid = w_dporto_resp_valid;               // systemc compatibility
    vdporto.resp_error = w_dporto_resp_error;               // systemc compatibility
    vdporto.rdata = wb_dporto_rdata;                        // systemc compatibility

    vmsto = axi4_l1_out_none;
    vmsto.ar_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    vmsto.aw_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    case (r.state)
    state_idle: begin
        v_next_ready = 1'b1;
        if (req_mem_valid_o == 1'b1) begin
            v.req_path = req_mem_path_o;
            v.req_addr = req_mem_addr_o;
            v.req_size = req_mem_size_o;
            // [0] 0=Unpriv/1=Priv;
            // [1] 0=Secure/1=Non-secure;
            // [2] 0=Data/1=Instruction
            v.req_prot = {req_mem_path_o, {2{1'b0}}};
            if (req_mem_type_o[REQ_MEM_TYPE_WRITE] == 1'b0) begin
                v.state = state_ar;
                v.req_wdata = '0;
                v.req_wstrb = '0;
                if (req_mem_type_o[REQ_MEM_TYPE_CACHED] == 1'b1) begin
                    v.req_cached = ARCACHE_WRBACK_READ_ALLOCATE;
                end else begin
                    v.req_cached = ARCACHE_DEVICE_NON_BUFFERABLE;
                end
                if (coherence_ena == 1'b1) begin
                    v.req_ar_snoop = reqtype2arsnoop(req_mem_type_o);
                end
            end else begin
                v.state = state_aw;
                v.req_wdata = req_mem_data_o;
                v.req_wstrb = req_mem_strob_o;
                if (req_mem_type_o[REQ_MEM_TYPE_CACHED] == 1'b1) begin
                    v.req_cached = AWCACHE_WRBACK_WRITE_ALLOCATE;
                end else begin
                    v.req_cached = AWCACHE_DEVICE_NON_BUFFERABLE;
                end
                if (coherence_ena == 1'b1) begin
                    v.req_aw_snoop = reqtype2awsnoop(req_mem_type_o);
                end
            end
        end
    end
    state_ar: begin
        vmsto.ar_valid = 1'b1;
        vmsto.ar_bits.addr = r.req_addr;
        vmsto.ar_bits.cache = r.req_cached;
        vmsto.ar_bits.size = r.req_size;
        vmsto.ar_bits.prot = r.req_prot;
        vmsto.ar_snoop = r.req_ar_snoop;
        if (i_msti.ar_ready == 1'b1) begin
            v.state = state_r;
        end
    end
    state_r: begin
        vmsto.r_ready = 1'b1;
        v_mem_er_load_fault = i_msti.r_resp[1];
        v_resp_mem_valid = i_msti.r_valid;
        // r_valid and r_last always should be in the same time
        if ((i_msti.r_valid == 1'b1) && (i_msti.r_last == 1'b1)) begin
            v.state = state_idle;
        end
    end
    state_aw: begin
        vmsto.aw_valid = 1'b1;
        vmsto.aw_bits.addr = r.req_addr;
        vmsto.aw_bits.cache = r.req_cached;
        vmsto.aw_bits.size = r.req_size;
        vmsto.aw_bits.prot = r.req_prot;
        vmsto.aw_snoop = r.req_aw_snoop;
        // axi lite to simplify L2-cache
        vmsto.w_valid = 1'b1;
        vmsto.w_last = 1'b1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if (i_msti.aw_ready == 1'b1) begin
            if (i_msti.w_ready == 1'b1) begin
                v.state = state_b;
            end else begin
                v.state = state_w;
            end
        end
    end
    state_w: begin
        // Shoudln't get here because of Lite interface:
        vmsto.w_valid = 1'b1;
        vmsto.w_last = 1'b1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if (i_msti.w_ready == 1'b1) begin
            v.state = state_b;
        end
    end
    state_b: begin
        vmsto.b_ready = 1'b1;
        v_resp_mem_valid = i_msti.b_valid;
        v_mem_er_store_fault = i_msti.b_resp[1];
        if (i_msti.b_valid == 1'b1) begin
            v.state = state_idle;
        end
    end
    default: begin
    end
    endcase

    // Snoop processing:
    case (r.snoop_state)
    snoop_idle: begin
        v_snoop_next_ready = 1'b1;
    end
    snoop_ac_wait_accept: begin
        req_snoop_valid = 1'b1;
        vb_req_snoop_addr = r.ac_addr;
        vb_req_snoop_type = r.req_snoop_type;
        if (req_snoop_ready_o == 1'b1) begin
            if (r.cache_access == 1'b0) begin
                v.snoop_state = snoop_cr;
            end else begin
                v.snoop_state = snoop_cd;
            end
        end
    end
    snoop_cr: begin
        if (resp_snoop_valid_o == 1'b1) begin
            v_cr_valid = 1'b1;
            if ((resp_snoop_flags_o[TAG_FL_VALID] == 1'b1)
                    && ((resp_snoop_flags_o[DTAG_FL_SHARED] == 1'b0)
                            || (r.ac_snoop == AC_SNOOP_READ_UNIQUE))) begin
                // Need second request with cache access
                v.cache_access = 1'b1;
                // see table C3-21 "Snoop response bit allocation"
                vb_cr_resp[0] = 1'b1;                       // will be Data transfer
                vb_cr_resp[4] = 1'b1;                       // WasUnique
                if (r.ac_snoop == AC_SNOOP_READ_UNIQUE) begin
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READCLEAN] = 1'b1;
                end else begin
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READDATA] = 1'b1;
                end
                v.req_snoop_type = vb_req_snoop_type;
                v.snoop_state = snoop_ac_wait_accept;
                if (i_msti.cr_ready == 1'b1) begin
                    v.snoop_state = snoop_ac_wait_accept;
                end else begin
                    v.snoop_state = snoop_cr_wait_accept;
                end
            end else begin
                vb_cr_resp = '0;
                if (i_msti.cr_ready == 1'b1) begin
                    v.snoop_state = snoop_idle;
                end else begin
                    v.snoop_state = snoop_cr_wait_accept;
                end
            end
            v.cr_resp = vb_cr_resp;
        end
    end
    snoop_cr_wait_accept: begin
        v_cr_valid = 1'b1;
        vb_cr_resp = r.cr_resp;
        if (i_msti.cr_ready == 1'b1) begin
            if (r.cache_access == 1'b1) begin
                v.snoop_state = snoop_ac_wait_accept;
            end else begin
                v.snoop_state = snoop_idle;
            end
        end
    end
    snoop_cd: begin
        if (resp_snoop_valid_o == 1'b1) begin
            v_cd_valid = 1'b1;
            vb_cd_data = resp_snoop_data_o;
            v.resp_snoop_data = resp_snoop_data_o;
            if (i_msti.cd_ready == 1'b1) begin
                v.snoop_state = snoop_idle;
            end else begin
                v.snoop_state = snoop_cd_wait_accept;
            end
        end
    end
    snoop_cd_wait_accept: begin
        v_cd_valid = 1'b1;
        vb_cd_data = r.resp_snoop_data;
        if (i_msti.cd_ready == 1'b1) begin
            v.snoop_state = snoop_idle;
        end
    end
    default: begin
    end
    endcase

    if ((coherence_ena == 1'b1)
            && (v_snoop_next_ready == 1'b1)
            && (i_msti.ac_valid == 1'b1)) begin
        req_snoop_valid = 1'b1;
        v.cache_access = 1'b0;
        vb_req_snoop_type = '0;                             // First snoop operation always just to read flags
        v.req_snoop_type = '0;
        vb_req_snoop_addr = i_msti.ac_addr;
        v.ac_addr = i_msti.ac_addr;
        v.ac_snoop = i_msti.ac_snoop;
        if (req_snoop_ready_o == 1'b1) begin
            v.snoop_state = snoop_cr;
        end else begin
            v.snoop_state = snoop_ac_wait_accept;
        end
    end else begin
        v_snoop_next_ready = 1'b1;
        v_cr_valid = 1'b1;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = RiverAmba_r_reset;
    end

    vmsto.ac_ready = v_snoop_next_ready;
    vmsto.cr_valid = v_cr_valid;
    vmsto.cr_resp = vb_cr_resp;
    vmsto.cd_valid = v_cd_valid;
    vmsto.cd_data = vb_cd_data;
    vmsto.cd_last = v_cd_valid;
    vmsto.rack = 1'b0;
    vmsto.wack = 1'b0;

    req_mem_ready_i = v_next_ready;
    resp_mem_valid_i = v_resp_mem_valid;
    resp_mem_data_i = i_msti.r_data;
    resp_mem_load_fault_i = v_mem_er_load_fault;
    resp_mem_store_fault_i = v_mem_er_store_fault;
    // AXI Snoop IOs:
    req_snoop_valid_i = req_snoop_valid;
    req_snoop_type_i = vb_req_snoop_type;
    req_snoop_addr_i = vb_req_snoop_addr;
    resp_snoop_ready_i = 1'b1;

    o_msto = vmsto;
    o_dport = vdporto;                                      // systemc compatibility
    o_available = 1'h1;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= RiverAmba_r_reset;
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

endmodule: RiverAmba
