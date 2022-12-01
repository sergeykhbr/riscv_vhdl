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

module Workgroup #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned cpu_num = 1,
    parameter int unsigned ilog2_nways = 2,                 // I$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned ilog2_lines_per_way = 7,         // I$ Cache length: 7=16KB; 8=32KB; ..
    parameter int unsigned dlog2_nways = 2,                 // D$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned dlog2_lines_per_way = 7,         // D$ Cache length: 7=16KB; 8=32KB; ..
    parameter int unsigned l2cache_ena = 1,
    parameter int unsigned l2log2_nways = 4,                // L2$ Cache associativity. Default bits width = 4, means 16 ways
    parameter int unsigned l2log2_lines_per_way = 9         // L2$ Cache length: 9=64KB;
)
(
    input logic i_cores_nrst,                               // System reset without DMI inteface
    input logic i_dmi_nrst,                                 // Debug interface reset
    input logic i_clk,                                      // CPU clock
    input logic i_trst,
    input logic i_tck,
    input logic i_tms,
    input logic i_tdi,
    output logic o_tdo,
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_msip,
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_mtip,
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_meip,
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_seip,
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // coherent port:
    input types_amba_pkg::axi4_master_out_type i_acpo,
    output types_amba_pkg::axi4_master_in_type o_acpi,
    // System bus port
    output types_amba_pkg::dev_config_type o_xmst_cfg,      // Workgroup master interface descriptor
    input types_amba_pkg::axi4_master_in_type i_msti,
    output types_amba_pkg::axi4_master_out_type o_msto,
    // APB debug access:
    input types_amba_pkg::mapinfo_type i_dmi_mapinfo,       // DMI APB itnerface mapping information
    output types_amba_pkg::dev_config_type o_dmi_cfg,       // DMI device descriptor
    input types_amba_pkg::apb_in_type i_dmi_apbi,
    output types_amba_pkg::apb_out_type o_dmi_apbo,
    output logic o_dmreset                                  // reset everything except DMI debug interface
);

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;
import workgroup_pkg::*;

localparam bit coherence_ena = ((cpu_num * l2cache_ena) > 1 ? 1: 0);

axi4_l1_out_vector coreo;
axi4_l1_in_vector corei;
axi4_l2_in_type l2i;
axi4_l2_out_type l2o;
dport_in_vector wb_dport_i;
dport_out_vector wb_dport_o;
hart_irq_vector vec_irq;
hart_signal_vector vec_halted;
hart_signal_vector vec_available;
hart_signal_vector vec_flush_l2;
logic [CFG_CPU_MAX-1:0] wb_halted;
logic [CFG_CPU_MAX-1:0] wb_available;
logic [CFG_LOG2_CPU_MAX-1:0] wb_dmi_hartsel;
logic w_dmi_haltreq;
logic w_dmi_resumereq;
logic w_dmi_resethaltreq;
logic w_dmi_hartreset;
logic w_dmi_dport_req_valid;
logic [DPortReq_Total-1:0] wb_dmi_dport_req_type;
logic [RISCV_ARCH-1:0] wb_dmi_dport_addr;
logic [RISCV_ARCH-1:0] wb_dmi_dport_wdata;
logic [2:0] wb_dmi_dport_size;
logic w_ic_dport_req_ready;
logic w_dmi_dport_resp_ready;
logic w_ic_dport_resp_valid;
logic w_ic_dport_resp_error;
logic [RISCV_ARCH-1:0] wb_ic_dport_rdata;
logic [(32 * CFG_PROGBUF_REG_TOTAL)-1:0] wb_progbuf;
logic w_flush_l2;
dev_config_type wb_xmst_cfg;

dmidebug #(
    .async_reset(async_reset)
) dmi0 (
    .i_clk(i_clk),
    .i_nrst(i_dmi_nrst),
    .i_trst(i_trst),
    .i_tck(i_tck),
    .i_tms(i_tms),
    .i_tdi(i_tdi),
    .o_tdo(o_tdo),
    .i_mapinfo(i_dmi_mapinfo),
    .o_cfg(o_dmi_cfg),
    .i_apbi(i_dmi_apbi),
    .o_apbo(o_dmi_apbo),
    .o_ndmreset(o_dmreset),
    .i_halted(wb_halted),
    .i_available(wb_available),
    .o_hartsel(wb_dmi_hartsel),
    .o_haltreq(w_dmi_haltreq),
    .o_resumereq(w_dmi_resumereq),
    .o_resethaltreq(w_dmi_resethaltreq),
    .o_hartreset(w_dmi_hartreset),
    .o_dport_req_valid(w_dmi_dport_req_valid),
    .o_dport_req_type(wb_dmi_dport_req_type),
    .o_dport_addr(wb_dmi_dport_addr),
    .o_dport_wdata(wb_dmi_dport_wdata),
    .o_dport_size(wb_dmi_dport_size),
    .i_dport_req_ready(w_ic_dport_req_ready),
    .o_dport_resp_ready(w_dmi_dport_resp_ready),
    .i_dport_resp_valid(w_ic_dport_resp_valid),
    .i_dport_resp_error(w_ic_dport_resp_error),
    .i_dport_rdata(wb_ic_dport_rdata),
    .o_progbuf(wb_progbuf)
);


ic_dport #(
    .async_reset(async_reset)
) dport_ic0 (
    .i_clk(i_clk),
    .i_nrst(i_dmi_nrst),
    .i_hartsel(wb_dmi_hartsel),
    .i_haltreq(w_dmi_haltreq),
    .i_resumereq(w_dmi_resumereq),
    .i_resethaltreq(w_dmi_resethaltreq),
    .i_hartreset(w_dmi_hartreset),
    .i_dport_req_valid(w_dmi_dport_req_valid),
    .i_dport_req_type(wb_dmi_dport_req_type),
    .i_dport_addr(wb_dmi_dport_addr),
    .i_dport_wdata(wb_dmi_dport_wdata),
    .i_dport_size(wb_dmi_dport_size),
    .o_dport_req_ready(w_ic_dport_req_ready),
    .i_dport_resp_ready(w_dmi_dport_resp_ready),
    .o_dport_resp_valid(w_ic_dport_resp_valid),
    .o_dport_resp_error(w_ic_dport_resp_error),
    .o_dport_rdata(wb_ic_dport_rdata),
    .o_dporti(wb_dport_i),
    .i_dporto(wb_dport_o)
);


ic_axi4_to_l1 #(
    .async_reset(async_reset)
) acp_bridge (
    .i_clk(i_clk),
    .i_nrst(i_cores_nrst),
    .i_xmsto(i_acpo),
    .o_xmsti(o_acpi),
    .i_l1i(corei[ACP_SLOT_IDX]),
    .o_l1o(coreo[ACP_SLOT_IDX])
);


generate
    for (genvar i = 0; i < cpu_num; i++) begin: xslotcpu
        RiverAmba #(
            .async_reset(async_reset),
            .hartid(i),
            .fpu_ena(CFG_HW_FPU_ENABLE),
            .coherence_ena(coherence_ena),
            .tracer_ena(CFG_TRACER_ENABLE),
            .ilog2_nways(ilog2_nways),
            .ilog2_lines_per_way(ilog2_lines_per_way),
            .dlog2_nways(dlog2_nways),
            .dlog2_lines_per_way(dlog2_lines_per_way)
        ) cpux (
            .i_clk(i_clk),
            .i_nrst(i_cores_nrst),
            .i_mtimer(i_mtimer),
            .i_msti(corei[i]),
            .o_msto(coreo[i]),
            .i_dport(wb_dport_i[i]),
            .o_dport(wb_dport_o[i]),
            .i_irq_pending(vec_irq[i]),
            .o_flush_l2(vec_flush_l2[i]),
            .o_halted(vec_halted[i]),
            .o_available(vec_available[i]),
            .i_progbuf(wb_progbuf)
        );
    end: xslotcpu
    for (genvar i = cpu_num; i < CFG_CPU_MAX; i++) begin: xdummycpu
        DummyCpu dumx (
            .o_msto(coreo[i]),
            .o_dport(wb_dport_o[i]),
            .o_flush_l2(vec_flush_l2[i]),
            .o_halted(vec_halted[i]),
            .o_available(vec_available[i])
        );
    end: xdummycpu

endgenerate

if (l2cache_ena == 1) begin: l2_en
    L2Top #(
        .async_reset(async_reset),
        .waybits(l2log2_nways),
        .ibits(l2log2_lines_per_way)
    ) l2cache (
        .i_clk(i_clk),
        .i_nrst(i_cores_nrst),
        .i_l1o(coreo),
        .o_l1i(corei),
        .i_l2i(l2i),
        .o_l2o(l2o),
        .i_flush_valid(w_flush_l2)
    );
end: l2_en
else begin: l2_dis
    L2Dummy #(
        .async_reset(async_reset)
    ) l2dummy (
        .i_clk(i_clk),
        .i_nrst(i_cores_nrst),
        .i_l1o(coreo),
        .o_l1i(corei),
        .i_l2i(l2i),
        .o_l2o(l2o),
        .i_flush_valid(w_flush_l2)
    );

end: l2_dis

L2SerDes #(
    .async_reset(async_reset)
) l2serdes0 (
    .i_clk(i_clk),
    .i_nrst(i_cores_nrst),
    .o_l2i(l2i),
    .i_l2o(l2o),
    .i_msti(i_msti),
    .o_msto(o_msto)
);


always_comb
begin: comb_proc
    logic v_flush_l2;
    logic [CFG_CPU_MAX-1:0] vb_halted;
    logic [CFG_CPU_MAX-1:0] vb_available;
    logic [IRQ_TOTAL-1:0] vb_irq[0: CFG_CPU_MAX-1];

    v_flush_l2 = 0;
    vb_halted = 0;
    vb_available = 0;
    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        vb_irq[i] = 16'h0000;
    end

    wb_xmst_cfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    wb_xmst_cfg.descrtype = PNP_CFG_TYPE_MASTER;
    wb_xmst_cfg.vid = VENDOR_OPTIMITECH;
    wb_xmst_cfg.did = RISCV_RIVER_WORKGROUP;

    // Vector to signal conversion is neccessary to implement compatibility with SystemC:
    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        v_flush_l2 = (v_flush_l2 || vec_flush_l2[i]);
        vb_halted[i] = vec_halted[i];
        vb_available[i] = vec_available[i];
        vb_irq[i][IRQ_MSIP] = i_msip[i];
        vb_irq[i][IRQ_MTIP] = i_mtip[i];
        vb_irq[i][IRQ_MEIP] = i_meip[i];
        vb_irq[i][IRQ_SEIP] = i_seip[i];
        vec_irq[i] = vb_irq[i];
    end
    w_flush_l2 = v_flush_l2;
    wb_halted = vb_halted;
    wb_available = vb_available;
    o_xmst_cfg = wb_xmst_cfg;
end: comb_proc

endmodule: Workgroup
