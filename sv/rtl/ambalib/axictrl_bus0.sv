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

module axictrl_bus0 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    output types_pnp_pkg::dev_config_type o_cfg,            // Slave config descriptor
    input types_bus0_pkg::bus0_xmst_out_vector i_xmsto,     // AXI4 masters output vector
    output types_bus0_pkg::bus0_xmst_in_vector o_xmsti,     // AXI4 masters input vector
    input types_bus0_pkg::bus0_xslv_out_vector i_xslvo,     // AXI4 slaves output vectors
    output types_bus0_pkg::bus0_xslv_in_vector o_xslvi,     // AXI4 slaves input vectors
    output types_bus0_pkg::bus0_mapinfo_vector o_mapinfo    // AXI devices memory mapping information
);

import types_pnp_pkg::*;
import types_amba_pkg::*;
import types_bus0_pkg::*;
import axictrl_bus0_pkg::*;

mapinfo_type wb_def_mapinfo;
axi4_slave_in_type wb_def_xslvi;
axi4_slave_out_type wb_def_xslvo;
logic w_def_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_def_req_addr;
logic [7:0] wb_def_req_size;
logic w_def_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_def_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_def_req_wstrb;
logic w_def_req_last;
logic w_def_req_ready;
logic w_def_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_def_resp_rdata;
logic w_def_resp_err;
axictrl_bus0_registers r, rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_AXI_INTERCONNECT)
) xdef0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(wb_def_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(wb_def_xslvi),
    .o_xslvo(wb_def_xslvo),
    .o_req_valid(w_def_req_valid),
    .o_req_addr(wb_def_req_addr),
    .o_req_size(wb_def_req_size),
    .o_req_write(w_def_req_write),
    .o_req_wdata(wb_def_req_wdata),
    .o_req_wstrb(wb_def_req_wstrb),
    .o_req_last(w_def_req_last),
    .i_req_ready(w_def_req_ready),
    .i_resp_valid(w_def_resp_valid),
    .i_resp_rdata(wb_def_resp_rdata),
    .i_resp_err(w_def_resp_err)
);

always_comb
begin: comb_proc
    axictrl_bus0_registers v;
    axi4_master_in_type vmsti[0: (CFG_BUS0_XMST_TOTAL + 1)-1];
    axi4_master_out_type vmsto[0: (CFG_BUS0_XMST_TOTAL + 1)-1];
    axi4_slave_in_type vslvi[0: (CFG_BUS0_XSLV_TOTAL + 1)-1];
    axi4_slave_out_type vslvo[0: (CFG_BUS0_XSLV_TOTAL + 1)-1];
    mapinfo_type vb_def_mapinfo;
    int i_ar_midx;
    int i_aw_midx;
    int i_ar_sidx;
    int i_aw_sidx;
    int i_r_midx;
    int i_r_sidx;
    int i_w_midx;
    int i_w_sidx;
    int i_b_midx;
    int i_b_sidx;
    logic v_aw_fire;
    logic v_ar_fire;
    logic v_w_fire;
    logic v_w_busy;
    logic v_r_fire;
    logic v_r_busy;
    logic v_b_fire;
    logic v_b_busy;

    for (int i = 0; i < (CFG_BUS0_XMST_TOTAL + 1); i++) begin
        vmsti[i] = axi4_master_in_none;
    end
    for (int i = 0; i < (CFG_BUS0_XMST_TOTAL + 1); i++) begin
        vmsto[i] = axi4_master_out_none;
    end
    for (int i = 0; i < (CFG_BUS0_XSLV_TOTAL + 1); i++) begin
        vslvi[i] = axi4_slave_in_none;
    end
    for (int i = 0; i < (CFG_BUS0_XSLV_TOTAL + 1); i++) begin
        vslvo[i] = axi4_slave_out_none;
    end
    vb_def_mapinfo = mapinfo_none;
    i_ar_midx = 0;
    i_aw_midx = 0;
    i_ar_sidx = 0;
    i_aw_sidx = 0;
    i_r_midx = 0;
    i_r_sidx = 0;
    i_w_midx = 0;
    i_w_sidx = 0;
    i_b_midx = 0;
    i_b_sidx = 0;
    v_aw_fire = 1'b0;
    v_ar_fire = 1'b0;
    v_w_fire = 1'b0;
    v_w_busy = 1'b0;
    v_r_fire = 1'b0;
    v_r_busy = 1'b0;
    v_b_fire = 1'b0;
    v_b_busy = 1'b0;

    v = r;

    vb_def_mapinfo.addr_start = 0;
    vb_def_mapinfo.addr_end = 0;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        vmsto[i] = i_xmsto[i];                              // Cannot read vector item from port in systemc
        vmsti[i] = axi4_master_in_none;
    end
    // Unmapped default slots:
    vmsto[CFG_BUS0_XMST_TOTAL] = axi4_master_out_none;
    vmsti[CFG_BUS0_XMST_TOTAL] = axi4_master_in_none;

    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        vslvo[i] = i_xslvo[i];                              // Cannot read vector item from port in systemc
        vslvi[i] = axi4_slave_in_none;
    end
    // Unmapped default slots:
    vslvo[CFG_BUS0_XSLV_TOTAL] = wb_def_xslvo;
    vslvi[CFG_BUS0_XSLV_TOTAL] = axi4_slave_in_none;

    w_def_req_ready = 1'b1;
    w_def_resp_valid = 1'b1;
    wb_def_resp_rdata = '1;
    w_def_resp_err = 1'b1;
    i_ar_midx = CFG_BUS0_XMST_TOTAL;
    i_aw_midx = CFG_BUS0_XMST_TOTAL;
    i_ar_sidx = CFG_BUS0_XSLV_TOTAL;
    i_aw_sidx = CFG_BUS0_XSLV_TOTAL;
    i_r_midx = int'(r.r_midx);
    i_r_sidx = int'(r.r_sidx);
    i_w_midx = int'(r.w_midx);
    i_w_sidx = int'(r.w_sidx);
    i_b_midx = int'(r.b_midx);
    i_b_sidx = int'(r.b_sidx);

    // Select Master bus:
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        if (vmsto[i].ar_valid == 1'b1) begin
            i_ar_midx = i;
        end
        if (vmsto[i].aw_valid == 1'b1) begin
            i_aw_midx = i;
        end
    end

    // Select Slave interface:
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        if ((CFG_BUS0_MAP[i].addr_start[(CFG_SYSBUS_ADDR_BITS - 1): 12] <= vmsto[i_ar_midx].ar_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12])
                && (vmsto[i_ar_midx].ar_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12] < CFG_BUS0_MAP[i].addr_end[(CFG_SYSBUS_ADDR_BITS - 1): 12])) begin
            i_ar_sidx = i;
        end
        if ((CFG_BUS0_MAP[i].addr_start[(CFG_SYSBUS_ADDR_BITS - 1): 12] <= vmsto[i_aw_midx].aw_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12])
                && (vmsto[i_aw_midx].aw_bits.addr[(CFG_SYSBUS_ADDR_BITS - 1): 12] < CFG_BUS0_MAP[i].addr_end[(CFG_SYSBUS_ADDR_BITS - 1): 12])) begin
            i_aw_sidx = i;
        end
    end

    // Read Channel:
    v_ar_fire = (vmsto[i_ar_midx].ar_valid & vslvo[i_ar_sidx].ar_ready);
    v_r_fire = (vmsto[i_r_midx].r_ready & vslvo[i_r_sidx].r_valid & vslvo[i_r_sidx].r_last);
    // Write channel:
    v_aw_fire = (vmsto[i_aw_midx].aw_valid & vslvo[i_aw_sidx].aw_ready);
    v_w_fire = (vmsto[i_w_midx].w_valid & vmsto[i_w_midx].w_last & vslvo[i_w_sidx].w_ready);
    // Write confirm channel
    v_b_fire = (vmsto[i_b_midx].b_ready & vslvo[i_b_sidx].b_valid);

    if ((r.r_sidx != CFG_BUS0_XSLV_TOTAL) && (v_r_fire == 1'b0)) begin
        v_r_busy = 1'b1;
    end

    if (((r.w_sidx != CFG_BUS0_XSLV_TOTAL) && (v_w_fire == 1'b0))
            || ((r.b_sidx != CFG_BUS0_XSLV_TOTAL) && (v_b_fire == 1'b0))) begin
        v_w_busy = 1'b1;
    end

    if ((r.b_sidx != CFG_BUS0_XSLV_TOTAL) && (v_b_fire == 1'b0)) begin
        v_b_busy = 1'b1;
    end

    if ((v_ar_fire == 1'b1) && (v_r_busy == 1'b0)) begin
        v.r_sidx = i_ar_sidx;
        v.r_midx = i_ar_midx;
    end else if (v_r_fire == 1'b1) begin
        v.r_sidx = CFG_BUS0_XSLV_TOTAL;
        v.r_midx = CFG_BUS0_XMST_TOTAL;
    end

    if ((v_aw_fire == 1'b1) && (v_w_busy == 1'b0)) begin
        v.w_sidx = i_aw_sidx;
        v.w_midx = i_aw_midx;
    end else if ((v_w_fire == 1'b1) && (v_b_busy == 1'b0)) begin
        v.w_sidx = CFG_BUS0_XSLV_TOTAL;
        v.w_midx = CFG_BUS0_XMST_TOTAL;
    end

    if ((v_w_fire == 1'b1) && (v_b_busy == 1'b0)) begin
        v.b_sidx = r.w_sidx;
        v.b_midx = r.w_midx;
    end else if (v_b_fire == 1'b1) begin
        v.b_sidx = CFG_BUS0_XSLV_TOTAL;
        v.b_midx = CFG_BUS0_XMST_TOTAL;
    end

    vmsti[i_ar_midx].ar_ready = (vslvo[i_ar_sidx].ar_ready & (~v_r_busy));
    vslvi[i_ar_sidx].ar_valid = (vmsto[i_ar_midx].ar_valid & (~v_r_busy));
    vslvi[i_ar_sidx].ar_bits = vmsto[i_ar_midx].ar_bits;
    vslvi[i_ar_sidx].ar_id = vmsto[i_ar_midx].ar_id;
    vslvi[i_ar_sidx].ar_user = vmsto[i_ar_midx].ar_user;

    vmsti[i_r_midx].r_valid = vslvo[i_r_sidx].r_valid;
    vmsti[i_r_midx].r_resp = vslvo[i_r_sidx].r_resp;
    vmsti[i_r_midx].r_data = vslvo[i_r_sidx].r_data;
    vmsti[i_r_midx].r_last = vslvo[i_r_sidx].r_last;
    vmsti[i_r_midx].r_id = vslvo[i_r_sidx].r_id;
    vmsti[i_r_midx].r_user = vslvo[i_r_sidx].r_user;
    vslvi[i_r_sidx].r_ready = vmsto[i_r_midx].r_ready;

    vmsti[i_aw_midx].aw_ready = (vslvo[i_aw_sidx].aw_ready & (~v_w_busy));
    vslvi[i_aw_sidx].aw_valid = (vmsto[i_aw_midx].aw_valid & (~v_w_busy));
    vslvi[i_aw_sidx].aw_bits = vmsto[i_aw_midx].aw_bits;
    vslvi[i_aw_sidx].aw_id = vmsto[i_aw_midx].aw_id;
    vslvi[i_aw_sidx].aw_user = vmsto[i_aw_midx].aw_user;

    vmsti[i_w_midx].w_ready = (vslvo[i_w_sidx].w_ready & (~v_b_busy));
    vslvi[i_w_sidx].w_valid = (vmsto[i_w_midx].w_valid & (~v_b_busy));
    vslvi[i_w_sidx].w_data = vmsto[i_w_midx].w_data;
    vslvi[i_w_sidx].w_last = vmsto[i_w_midx].w_last;
    vslvi[i_w_sidx].w_strb = vmsto[i_w_midx].w_strb;
    vslvi[i_w_sidx].w_user = vmsto[i_w_midx].w_user;

    vmsti[i_b_midx].b_valid = vslvo[i_b_sidx].b_valid;
    vmsti[i_b_midx].b_resp = vslvo[i_b_sidx].b_resp;
    vmsti[i_b_midx].b_id = vslvo[i_b_sidx].b_id;
    vmsti[i_b_midx].b_user = vslvo[i_b_sidx].b_user;
    vslvi[i_b_sidx].b_ready = vmsto[i_b_midx].b_ready;

    if (~async_reset && i_nrst == 1'b0) begin
        v = axictrl_bus0_r_reset;
    end

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) begin
        o_xmsti[i] = vmsti[i];
    end
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) begin
        o_xslvi[i] = vslvi[i];
        o_mapinfo[i] = CFG_BUS0_MAP[i];
    end
    wb_def_xslvi = vslvi[CFG_BUS0_XSLV_TOTAL];
    wb_def_mapinfo = vb_def_mapinfo;

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= axictrl_bus0_r_reset;
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

endmodule: axictrl_bus0
