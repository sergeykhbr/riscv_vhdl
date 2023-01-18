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

#include "workgroup.h"
#include "api_core.h"

namespace debugger {

Workgroup::Workgroup(sc_module_name name,
                     bool async_reset,
                     uint32_t cpu_num,
                     uint32_t ilog2_nways,
                     uint32_t ilog2_lines_per_way,
                     uint32_t dlog2_nways,
                     uint32_t dlog2_lines_per_way,
                     uint32_t l2cache_ena,
                     uint32_t l2log2_nways,
                     uint32_t l2log2_lines_per_way)
    : sc_module(name),
    i_cores_nrst("i_cores_nrst"),
    i_dmi_nrst("i_dmi_nrst"),
    i_clk("i_clk"),
    i_trst("i_trst"),
    i_tck("i_tck"),
    i_tms("i_tms"),
    i_tdi("i_tdi"),
    o_tdo("o_tdo"),
    i_msip("i_msip"),
    i_mtip("i_mtip"),
    i_meip("i_meip"),
    i_seip("i_seip"),
    i_mtimer("i_mtimer"),
    i_acpo("i_acpo"),
    o_acpi("o_acpi"),
    o_xmst_cfg("o_xmst_cfg"),
    i_msti("i_msti"),
    o_msto("o_msto"),
    i_dmi_mapinfo("i_dmi_mapinfo"),
    o_dmi_cfg("o_dmi_cfg"),
    i_dmi_apbi("i_dmi_apbi"),
    o_dmi_apbo("o_dmi_apbo"),
    o_dmreset("o_dmreset"),
    coreo("coreo", CFG_SLOT_L1_TOTAL),
    corei("corei", CFG_SLOT_L1_TOTAL),
    wb_dport_i("wb_dport_i", CFG_CPU_MAX),
    wb_dport_o("wb_dport_o", CFG_CPU_MAX),
    vec_irq("vec_irq", CFG_CPU_MAX),
    vec_halted("vec_halted", CFG_CPU_MAX),
    vec_available("vec_available", CFG_CPU_MAX),
    vec_flush_l2("vec_flush_l2", CFG_CPU_MAX) {

    async_reset_ = async_reset;
    cpu_num_ = cpu_num;
    ilog2_nways_ = ilog2_nways;
    ilog2_lines_per_way_ = ilog2_lines_per_way;
    dlog2_nways_ = dlog2_nways;
    dlog2_lines_per_way_ = dlog2_lines_per_way;
    l2cache_ena_ = l2cache_ena;
    l2log2_nways_ = l2log2_nways;
    l2log2_lines_per_way_ = l2log2_lines_per_way;
    coherence_ena = ((cpu_num * l2cache_ena) > 1 ? 1: 0);
    dmi0 = 0;
    dport_ic0 = 0;
    acp_bridge = 0;
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        cpux[i] = 0;
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        dumx[i] = 0;
    }
    l2cache = 0;
    l2dummy = 0;
    l2serdes0 = 0;

    dmi0 = new dmidebug("dmi0", async_reset);
    dmi0->i_clk(i_clk);
    dmi0->i_nrst(i_dmi_nrst);
    dmi0->i_trst(i_trst);
    dmi0->i_tck(i_tck);
    dmi0->i_tms(i_tms);
    dmi0->i_tdi(i_tdi);
    dmi0->o_tdo(o_tdo);
    dmi0->i_mapinfo(i_dmi_mapinfo);
    dmi0->o_cfg(o_dmi_cfg);
    dmi0->i_apbi(i_dmi_apbi);
    dmi0->o_apbo(o_dmi_apbo);
    dmi0->o_ndmreset(o_dmreset);
    dmi0->i_halted(wb_halted);
    dmi0->i_available(wb_available);
    dmi0->o_hartsel(wb_dmi_hartsel);
    dmi0->o_haltreq(w_dmi_haltreq);
    dmi0->o_resumereq(w_dmi_resumereq);
    dmi0->o_resethaltreq(w_dmi_resethaltreq);
    dmi0->o_hartreset(w_dmi_hartreset);
    dmi0->o_dport_req_valid(w_dmi_dport_req_valid);
    dmi0->o_dport_req_type(wb_dmi_dport_req_type);
    dmi0->o_dport_addr(wb_dmi_dport_addr);
    dmi0->o_dport_wdata(wb_dmi_dport_wdata);
    dmi0->o_dport_size(wb_dmi_dport_size);
    dmi0->i_dport_req_ready(w_ic_dport_req_ready);
    dmi0->o_dport_resp_ready(w_dmi_dport_resp_ready);
    dmi0->i_dport_resp_valid(w_ic_dport_resp_valid);
    dmi0->i_dport_resp_error(w_ic_dport_resp_error);
    dmi0->i_dport_rdata(wb_ic_dport_rdata);
    dmi0->o_progbuf(wb_progbuf);


    dport_ic0 = new ic_dport("dport_ic0", async_reset);
    dport_ic0->i_clk(i_clk);
    dport_ic0->i_nrst(i_dmi_nrst);
    dport_ic0->i_hartsel(wb_dmi_hartsel);
    dport_ic0->i_haltreq(w_dmi_haltreq);
    dport_ic0->i_resumereq(w_dmi_resumereq);
    dport_ic0->i_resethaltreq(w_dmi_resethaltreq);
    dport_ic0->i_hartreset(w_dmi_hartreset);
    dport_ic0->i_dport_req_valid(w_dmi_dport_req_valid);
    dport_ic0->i_dport_req_type(wb_dmi_dport_req_type);
    dport_ic0->i_dport_addr(wb_dmi_dport_addr);
    dport_ic0->i_dport_wdata(wb_dmi_dport_wdata);
    dport_ic0->i_dport_size(wb_dmi_dport_size);
    dport_ic0->o_dport_req_ready(w_ic_dport_req_ready);
    dport_ic0->i_dport_resp_ready(w_dmi_dport_resp_ready);
    dport_ic0->o_dport_resp_valid(w_ic_dport_resp_valid);
    dport_ic0->o_dport_resp_error(w_ic_dport_resp_error);
    dport_ic0->o_dport_rdata(wb_ic_dport_rdata);
    dport_ic0->o_dporti(wb_dport_i);
    dport_ic0->i_dporto(wb_dport_o);


    acp_bridge = new ic_axi4_to_l1("acp_bridge", async_reset);
    acp_bridge->i_clk(i_clk);
    acp_bridge->i_nrst(i_cores_nrst);
    acp_bridge->i_xmsto(i_acpo);
    acp_bridge->o_xmsti(o_acpi);
    acp_bridge->i_l1i(corei[ACP_SLOT_IDX]);
    acp_bridge->o_l1o(coreo[ACP_SLOT_IDX]);


    // generate
    for (int i = 0; i < cpu_num_; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "cpux%d", i);
        cpux[i] = new RiverAmba(tstr, async_reset,
                                 i,
                                 CFG_HW_FPU_ENABLE,
                                 coherence_ena,
                                 CFG_TRACER_ENABLE,
                                 ilog2_nways,
                                 ilog2_lines_per_way,
                                 dlog2_nways,
                                 dlog2_lines_per_way);
        cpux[i]->i_clk(i_clk);
        cpux[i]->i_nrst(i_cores_nrst);
        cpux[i]->i_mtimer(i_mtimer);
        cpux[i]->i_msti(corei[i]);
        cpux[i]->o_msto(coreo[i]);
        cpux[i]->i_dport(wb_dport_i[i]);
        cpux[i]->o_dport(wb_dport_o[i]);
        cpux[i]->i_irq_pending(vec_irq[i]);
        cpux[i]->o_flush_l2(vec_flush_l2[i]);
        cpux[i]->o_halted(vec_halted[i]);
        cpux[i]->o_available(vec_available[i]);
        cpux[i]->i_progbuf(wb_progbuf);
    }
    for (int i = cpu_num_; i < CFG_CPU_MAX; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "dumx%d", i);
        dumx[i] = new DummyCpu(tstr);
        dumx[i]->o_msto(coreo[i]);
        dumx[i]->o_dport(wb_dport_o[i]);
        dumx[i]->o_flush_l2(vec_flush_l2[i]);
        dumx[i]->o_halted(vec_halted[i]);
        dumx[i]->o_available(vec_available[i]);
    }

    // endgenerate

    if (l2cache_ena_ == 1) {
        l2cache = new L2Top("l2cache", async_reset,
                             l2log2_nways,
                             l2log2_lines_per_way);
        l2cache->i_clk(i_clk);
        l2cache->i_nrst(i_cores_nrst);
        l2cache->i_l1o(coreo);
        l2cache->o_l1i(corei);
        l2cache->i_l2i(l2i);
        l2cache->o_l2o(l2o);
        l2cache->i_flush_valid(w_flush_l2);
    } else {
        l2dummy = new L2Dummy("l2dummy", async_reset);
        l2dummy->i_clk(i_clk);
        l2dummy->i_nrst(i_cores_nrst);
        l2dummy->i_l1o(coreo);
        l2dummy->o_l1i(corei);
        l2dummy->i_l2i(l2i);
        l2dummy->o_l2o(l2o);
        l2dummy->i_flush_valid(w_flush_l2);

    }

    l2serdes0 = new L2SerDes("l2serdes0", async_reset);
    l2serdes0->i_clk(i_clk);
    l2serdes0->i_nrst(i_cores_nrst);
    l2serdes0->o_l2i(l2i);
    l2serdes0->i_l2o(l2o);
    l2serdes0->i_msti(i_msti);
    l2serdes0->o_msto(o_msto);



    SC_METHOD(comb);
    sensitive << i_cores_nrst;
    sensitive << i_dmi_nrst;
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << i_msip;
    sensitive << i_mtip;
    sensitive << i_meip;
    sensitive << i_seip;
    sensitive << i_mtimer;
    sensitive << i_acpo;
    sensitive << i_msti;
    sensitive << i_dmi_mapinfo;
    sensitive << i_dmi_apbi;
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        sensitive << coreo[i];
    }
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        sensitive << corei[i];
    }
    sensitive << l2i;
    sensitive << l2o;
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << wb_dport_i[i];
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << wb_dport_o[i];
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << vec_irq[i];
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << vec_halted[i];
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << vec_available[i];
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << vec_flush_l2[i];
    }
    sensitive << wb_halted;
    sensitive << wb_available;
    sensitive << wb_dmi_hartsel;
    sensitive << w_dmi_haltreq;
    sensitive << w_dmi_resumereq;
    sensitive << w_dmi_resethaltreq;
    sensitive << w_dmi_hartreset;
    sensitive << w_dmi_dport_req_valid;
    sensitive << wb_dmi_dport_req_type;
    sensitive << wb_dmi_dport_addr;
    sensitive << wb_dmi_dport_wdata;
    sensitive << wb_dmi_dport_size;
    sensitive << w_ic_dport_req_ready;
    sensitive << w_dmi_dport_resp_ready;
    sensitive << w_ic_dport_resp_valid;
    sensitive << w_ic_dport_resp_error;
    sensitive << wb_ic_dport_rdata;
    sensitive << wb_progbuf;
    sensitive << w_flush_l2;
}

Workgroup::~Workgroup() {
    if (dmi0) {
        delete dmi0;
    }
    if (dport_ic0) {
        delete dport_ic0;
    }
    if (acp_bridge) {
        delete acp_bridge;
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        if (cpux[i]) {
            delete cpux[i];
        }
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        if (dumx[i]) {
            delete dumx[i];
        }
    }
    if (l2cache) {
        delete l2cache;
    }
    if (l2dummy) {
        delete l2dummy;
    }
    if (l2serdes0) {
        delete l2serdes0;
    }
}

void Workgroup::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_cores_nrst, i_cores_nrst.name());
        sc_trace(o_vcd, i_dmi_nrst, i_dmi_nrst.name());
        sc_trace(o_vcd, i_clk, i_clk.name());
        sc_trace(o_vcd, i_trst, i_trst.name());
        sc_trace(o_vcd, i_tck, i_tck.name());
        sc_trace(o_vcd, i_tms, i_tms.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());
        sc_trace(o_vcd, i_msip, i_msip.name());
        sc_trace(o_vcd, i_mtip, i_mtip.name());
        sc_trace(o_vcd, i_meip, i_meip.name());
        sc_trace(o_vcd, i_seip, i_seip.name());
        sc_trace(o_vcd, i_mtimer, i_mtimer.name());
        sc_trace(o_vcd, i_acpo, i_acpo.name());
        sc_trace(o_vcd, o_acpi, o_acpi.name());
        sc_trace(o_vcd, o_xmst_cfg, o_xmst_cfg.name());
        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, i_dmi_mapinfo, i_dmi_mapinfo.name());
        sc_trace(o_vcd, o_dmi_cfg, o_dmi_cfg.name());
        sc_trace(o_vcd, i_dmi_apbi, i_dmi_apbi.name());
        sc_trace(o_vcd, o_dmi_apbo, o_dmi_apbo.name());
        sc_trace(o_vcd, o_dmreset, o_dmreset.name());
    }

    if (dmi0) {
        dmi0->generateVCD(i_vcd, o_vcd);
    }
    if (dport_ic0) {
        dport_ic0->generateVCD(i_vcd, o_vcd);
    }
    if (acp_bridge) {
        acp_bridge->generateVCD(i_vcd, o_vcd);
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        if (cpux[i]) {
            cpux[i]->generateVCD(i_vcd, o_vcd);
        }
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        if (dumx[i]) {
            dumx[i]->generateVCD(i_vcd, o_vcd);
        }
    }
    if (l2cache) {
        l2cache->generateVCD(i_vcd, o_vcd);
    }
    if (l2dummy) {
        l2dummy->generateVCD(i_vcd, o_vcd);
    }
    if (l2serdes0) {
        l2serdes0->generateVCD(i_vcd, o_vcd);
    }
}

void Workgroup::comb() {
    bool v_flush_l2;
    sc_uint<CFG_CPU_MAX> vb_halted;
    sc_uint<CFG_CPU_MAX> vb_available;
    sc_uint<IRQ_TOTAL> vb_irq[CFG_CPU_MAX];

    v_flush_l2 = 0;
    vb_halted = 0;
    vb_available = 0;
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        vb_irq[i] = 0;
    }

    wb_xmst_cfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    wb_xmst_cfg.descrtype = PNP_CFG_TYPE_MASTER;
    wb_xmst_cfg.vid = VENDOR_OPTIMITECH;
    wb_xmst_cfg.did = RISCV_RIVER_WORKGROUP;

    // Vector to signal conversion is neccessary to implement compatibility with SystemC:
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        v_flush_l2 = (v_flush_l2 || vec_flush_l2[i]);
        vb_halted[i] = vec_halted[i];
        vb_available[i] = vec_available[i];
        vb_irq[i][IRQ_MSIP] = i_msip.read()[i];
        vb_irq[i][IRQ_MTIP] = i_mtip.read()[i];
        vb_irq[i][IRQ_MEIP] = i_meip.read()[i];
        vb_irq[i][IRQ_SEIP] = i_seip.read()[i];
        vec_irq[i] = vb_irq[i];
    }
    w_flush_l2 = v_flush_l2;
    wb_halted = vb_halted;
    wb_available = vb_available;
    o_xmst_cfg = wb_xmst_cfg;
}

}  // namespace debugger

