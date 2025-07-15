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

module plic #(
    parameter int ctxmax = 8,
    parameter int irqmax = 128,
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_pnp_pkg::dev_config_type o_cfg,            // Device descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI Slave to Bridge interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // AXI Bridge to Slave interface
    input logic [irqmax-1:0] i_irq_request,                 // [0] must be tight to GND
    output logic [ctxmax-1:0] o_ip
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
typedef struct {
    logic [3:0] priority_th;
    logic [1023:0] ie;                                      // interrupt enable per context
    logic [(4 * 1024)-1:0] ip_prio;                         // interrupt pending priority per context
    logic [15:0] prio_mask;                                 // pending interrupts priorites
    logic [3:0] sel_prio;                                   // the most available priority
    logic [9:0] irq_idx;                                    // currently selected most prio irq
    logic [9:0] irq_prio;                                   // currently selected prio level
} plic_context_type;

typedef struct {
    logic [(4 * 1024)-1:0] src_priority;
    logic [1023:0] pending;
    logic [ctxmax-1:0] ip;
    plic_context_type ctx[0: ctxmax - 1];
    logic [63:0] rdata;
    logic resp_valid;
} plic_registers;

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;
logic w_req_ready;
logic w_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_resp_rdata;
logic wb_resp_err;
plic_registers r;
plic_registers rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_PLIC)
) xslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i_xslvi),
    .o_xslvo(o_xslvo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(w_req_ready),
    .i_resp_valid(w_resp_valid),
    .i_resp_rdata(wb_resp_rdata),
    .i_resp_err(wb_resp_err)
);

always_comb
begin: comb_proc
    plic_registers v;
    logic [CFG_SYSBUS_DATA_BITS-1:0] vrdata;
    logic [9:0] vb_irq_idx[0: ctxmax-1];                    // Currently selected most prio irq
    logic [9:0] vb_irq_prio[0: ctxmax-1];                   // Currently selected prio level
    logic [3:0] vb_ctx_priority_th[0: ctxmax-1];
    logic [1023:0] vb_ctx_ie[0: ctxmax-1];
    logic [(4 * 1024)-1:0] vb_ctx_ip_prio[0: ctxmax-1];
    logic [15:0] vb_ctx_prio_mask[0: ctxmax-1];
    logic [3:0] vb_ctx_sel_prio[0: ctxmax-1];
    logic [9:0] vb_ctx_irq_idx[0: ctxmax-1];
    logic [9:0] vb_ctx_irq_prio[0: ctxmax-1];
    logic [(4 * 1024)-1:0] vb_src_priority;
    logic [1023:0] vb_pending;
    logic [ctxmax-1:0] vb_ip;
    int rctx_idx;

    v.src_priority = r.src_priority;
    v.pending = r.pending;
    v.ip = r.ip;
    for (int i = 0; i < ctxmax; i++) begin
        v.ctx[i].priority_th = r.ctx[i].priority_th;
        v.ctx[i].ie = r.ctx[i].ie;
        v.ctx[i].ip_prio = r.ctx[i].ip_prio;
        v.ctx[i].prio_mask = r.ctx[i].prio_mask;
        v.ctx[i].sel_prio = r.ctx[i].sel_prio;
        v.ctx[i].irq_idx = r.ctx[i].irq_idx;
        v.ctx[i].irq_prio = r.ctx[i].irq_prio;
    end
    v.rdata = r.rdata;
    v.resp_valid = r.resp_valid;
    vrdata = '0;
    for (int i = 0; i < ctxmax; i++) begin
        vb_irq_idx[i] = '0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_irq_prio[i] = '0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_priority_th[i] = 4'd0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_ie[i] = '0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_ip_prio[i] = '0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_prio_mask[i] = 16'd0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_sel_prio[i] = 4'd0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_irq_idx[i] = 10'd0;
    end
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_irq_prio[i] = 10'd0;
    end
    vb_src_priority = '0;
    vb_pending = '0;
    vb_ip = '0;
    rctx_idx = 0;

    v.resp_valid = w_req_valid;
    // Warning SystemC limitation workaround:
    //   Cannot directly write into bitfields of the signals v.* registers
    //   So, use the following vb_* logic variables for that and then copy them.
    vb_src_priority = r.src_priority;
    vb_pending = r.pending;
    for (int i = 0; i < ctxmax; i++) begin
        vb_ctx_priority_th[i] = r.ctx[i].priority_th;
        vb_ctx_ie[i] = r.ctx[i].ie;
        vb_ctx_irq_idx[i] = r.ctx[i].irq_idx;
        vb_ctx_irq_prio[i] = r.ctx[i].irq_prio;
    end

    for (int i = 1; i < irqmax; i++) begin
        if ((i_irq_request[i] == 1'b1) && (int'(r.src_priority[(4 * i) +: 4]) > 4'd0)) begin
            vb_pending[i] = 1'b1;
        end
    end

    for (int n = 0; n < ctxmax; n++) begin
        for (int i = 0; i < irqmax; i++) begin
            if ((r.pending[i] == 1'b1)
                    && (r.ctx[n].ie[i] == 1'b1)
                    && (int'(r.src_priority[(4 * i) +: 4]) > r.ctx[n].priority_th)) begin
                vb_ctx_ip_prio[n][(4 * i) +: 4] = r.src_priority[(4 * i) +: 4];
                vb_ctx_prio_mask[n][int'(r.src_priority[(4 * i) +: 4])] = 1'b1;
            end
        end
    end

    // Select max priority in each context
    for (int n = 0; n < ctxmax; n++) begin
        for (int i = 0; i < 16; i++) begin
            if (r.ctx[n].prio_mask[i] == 1'b1) begin
                vb_ctx_sel_prio[n] = i;
            end
        end
    end

    // Select max priority in each context
    for (int n = 0; n < ctxmax; n++) begin
        for (int i = 0; i < irqmax; i++) begin
            if ((|r.ctx[n].sel_prio)
                    && (r.ctx[n].ip_prio[(4 * i) +: 4] == r.ctx[n].sel_prio)) begin
                // Most prio irq and prio level
                vb_irq_idx[n] = i;
                vb_irq_prio[n] = r.ctx[n].sel_prio;
            end
        end
    end

    for (int n = 0; n < ctxmax; n++) begin
        vb_ctx_irq_idx[n] = vb_irq_idx[n];
        vb_ctx_irq_prio[n] = vb_irq_prio[n];
        vb_ip[n] = (|vb_irq_idx[n]);
    end

    // R/W registers access:
    rctx_idx = int'(wb_req_addr[20: 12]);
    if (wb_req_addr[21: 12] == 10'd0) begin                 // src_prioirty
        // 0x000000..0x001000: Irq 0 unused
        if ((|wb_req_addr[11: 3]) == 1'b1) begin
            vrdata[3: 0] = r.src_priority[(8 * wb_req_addr[11: 3]) +: 4];
            if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
                if ((|wb_req_wstrb[3: 0]) == 1'b1) begin
                    vb_src_priority[(8 * wb_req_addr[11: 3]) +: 4] = wb_req_wdata[3: 0];
                end
            end
        end

        vrdata[35: 32] = r.src_priority[((8 * wb_req_addr[11: 3]) + 32) +: 4];
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            if ((|wb_req_wstrb[7: 4]) == 1'b1) begin
                vb_src_priority[((8 * wb_req_addr[11: 3]) + 32) +: 4] = wb_req_wdata[35: 32];
            end
        end
    end else if (wb_req_addr[21: 12] == 10'd1) begin
        // 0x001000..0x001080
        vrdata = r.pending[(64 * wb_req_addr[6: 3]) +: 64];
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            if ((|wb_req_wstrb[3: 0]) == 1'b1) begin
                vb_pending[(64 * wb_req_addr[6: 3]) +: 32] = wb_req_wdata[31: 0];
            end
            if ((|wb_req_wstrb[7: 4]) == 1'b1) begin
                vb_pending[((64 * wb_req_addr[6: 3]) + 32) +: 32] = wb_req_wdata[63: 32];
            end
        end
    end else if ((wb_req_addr[21: 12] == 10'd2)
                && (wb_req_addr[11: 7] < ctxmax)) begin
        // First 32 context of 15867 support only
        // 0x002000,0x002080,...,0x200000
        vrdata = r.ctx[wb_req_addr[11: 7]].ie[(64 * wb_req_addr[6: 3]) +: 64];
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            if ((|wb_req_wstrb[3: 0]) == 1'b1) begin
                vb_ctx_ie[wb_req_addr[11: 7]][(64 * wb_req_addr[6: 3]) +: 32] = wb_req_wdata[31: 0];
            end
            if ((|wb_req_wstrb[7: 4]) == 1'b1) begin
                vb_ctx_ie[wb_req_addr[11: 7]][((64 * wb_req_addr[6: 3]) + 32) +: 32] = wb_req_wdata[63: 32];
            end
        end
    end else if ((wb_req_addr[21: 12] >= 10'h200) && (wb_req_addr[20: 12] < ctxmax)) begin
        // 0x200000,0x201000,...,0x4000000
        if (wb_req_addr[11: 3] == 9'd0) begin
            // masking (disabling) all interrupt with <= priority
            vrdata[3: 0] = r.ctx[rctx_idx].priority_th;
            vrdata[41: 32] = r.ctx[rctx_idx].irq_idx;
            // claim/ complete. Reading clears pending bit
            if (r.ip[rctx_idx] == 1'b1) begin
                vb_pending[r.ctx[rctx_idx].irq_idx] = 1'b0;
            end
            if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
                if ((|wb_req_wstrb[3: 0]) == 1'b1) begin
                    vb_ctx_priority_th[rctx_idx] = wb_req_wdata[3: 0];
                end
                if ((|wb_req_wstrb[7: 4]) == 1'b1) begin
                    // claim/ complete. Reading clears pedning bit
                    vb_ctx_irq_idx[rctx_idx] = '0;
                end
            end
        end else begin
            // reserved
        end
    end
    v.rdata = vrdata;

    v.src_priority = vb_src_priority;
    v.pending = vb_pending;
    v.ip = vb_ip;
    for (int n = 0; n < ctxmax; n++) begin
        v.ctx[n].priority_th = vb_ctx_priority_th[n];
        v.ctx[n].ie = vb_ctx_ie[n];
        v.ctx[n].ip_prio = vb_ctx_ip_prio[n];
        v.ctx[n].prio_mask = vb_ctx_prio_mask[n];
        v.ctx[n].sel_prio = vb_ctx_sel_prio[n];
        v.ctx[n].irq_idx = vb_ctx_irq_idx[n];
        v.ctx[n].irq_prio = vb_ctx_irq_prio[n];
    end

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v.src_priority = '0;
        v.pending = '0;
        v.ip = '0;
        for (int i = 0; i < ctxmax; i++) begin
            v.ctx[i].priority_th = 4'd0;
            v.ctx[i].ie = '0;
            v.ctx[i].ip_prio = '0;
            v.ctx[i].prio_mask = 16'd0;
            v.ctx[i].sel_prio = 4'd0;
            v.ctx[i].irq_idx = 10'd0;
            v.ctx[i].irq_prio = 10'd0;
        end
        v.rdata = '0;
        v.resp_valid = 1'b0;
    end

    w_req_ready = 1'b1;
    w_resp_valid = r.resp_valid;
    wb_resp_rdata = r.rdata;
    wb_resp_err = 1'b0;
    o_ip = r.ip;

    rin.src_priority = v.src_priority;
    rin.pending = v.pending;
    rin.ip = v.ip;
    for (int i = 0; i < ctxmax; i++) begin
        rin.ctx[i].priority_th = v.ctx[i].priority_th;
        rin.ctx[i].ie = v.ctx[i].ie;
        rin.ctx[i].ip_prio = v.ctx[i].ip_prio;
        rin.ctx[i].prio_mask = v.ctx[i].prio_mask;
        rin.ctx[i].sel_prio = v.ctx[i].sel_prio;
        rin.ctx[i].irq_idx = v.ctx[i].irq_idx;
        rin.ctx[i].irq_prio = v.ctx[i].irq_prio;
    end
    rin.rdata = v.rdata;
    rin.resp_valid = v.resp_valid;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r.src_priority <= '0;
                r.pending <= '0;
                r.ip <= '0;
                for (int i = 0; i < ctxmax; i++) begin
                    r.ctx[i].priority_th <= 4'd0;
                    r.ctx[i].ie <= '0;
                    r.ctx[i].ip_prio <= '0;
                    r.ctx[i].prio_mask <= 16'd0;
                    r.ctx[i].sel_prio <= 4'd0;
                    r.ctx[i].irq_idx <= 10'd0;
                    r.ctx[i].irq_prio <= 10'd0;
                end
                r.rdata <= '0;
                r.resp_valid <= 1'b0;
            end else begin
                r.src_priority <= rin.src_priority;
                r.pending <= rin.pending;
                r.ip <= rin.ip;
                for (int i = 0; i < ctxmax; i++) begin
                    r.ctx[i].priority_th <= rin.ctx[i].priority_th;
                    r.ctx[i].ie <= rin.ctx[i].ie;
                    r.ctx[i].ip_prio <= rin.ctx[i].ip_prio;
                    r.ctx[i].prio_mask <= rin.ctx[i].prio_mask;
                    r.ctx[i].sel_prio <= rin.ctx[i].sel_prio;
                    r.ctx[i].irq_idx <= rin.ctx[i].irq_idx;
                    r.ctx[i].irq_prio <= rin.ctx[i].irq_prio;
                end
                r.rdata <= rin.rdata;
                r.resp_valid <= rin.resp_valid;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r.src_priority <= rin.src_priority;
            r.pending <= rin.pending;
            r.ip <= rin.ip;
            for (int i = 0; i < ctxmax; i++) begin
                r.ctx[i].priority_th <= rin.ctx[i].priority_th;
                r.ctx[i].ie <= rin.ctx[i].ie;
                r.ctx[i].ip_prio <= rin.ctx[i].ip_prio;
                r.ctx[i].prio_mask <= rin.ctx[i].prio_mask;
                r.ctx[i].sel_prio <= rin.ctx[i].sel_prio;
                r.ctx[i].irq_idx <= rin.ctx[i].irq_idx;
                r.ctx[i].irq_prio <= rin.ctx[i].irq_prio;
            end
            r.rdata <= rin.rdata;
            r.resp_valid <= rin.resp_valid;
        end

    end: async_r_dis
endgenerate

endmodule: plic
