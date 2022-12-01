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
#pragma once

#include <systemc.h>
#include "river_cfg.h"
#include "../ambalib/types_amba.h"
#include "types_river.h"
#include "dmi/dmidebug.h"
#include "dmi/ic_dport.h"
#include "ic_axi4_to_l1.h"
#include "river_amba.h"
#include "dummycpu.h"
#include "l2cache/l2_top.h"
#include "l2cache/l2dummy.h"
#include "l2cache/l2serdes.h"

namespace debugger {

SC_MODULE(Workgroup) {
 public:
    sc_in<bool> i_cores_nrst;                               // System reset without DMI inteface
    sc_in<bool> i_dmi_nrst;                                 // Debug interface reset
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_trst;
    sc_in<bool> i_tck;
    sc_in<bool> i_tms;
    sc_in<bool> i_tdi;
    sc_out<bool> o_tdo;
    sc_in<sc_uint<CFG_CPU_MAX>> i_msip;
    sc_in<sc_uint<CFG_CPU_MAX>> i_mtip;
    sc_in<sc_uint<CFG_CPU_MAX>> i_meip;
    sc_in<sc_uint<CFG_CPU_MAX>> i_seip;
    sc_in<sc_uint<64>> i_mtimer;                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // coherent port:
    sc_in<axi4_master_out_type> i_acpo;
    sc_out<axi4_master_in_type> o_acpi;
    // System bus port
    sc_out<dev_config_type> o_xmst_cfg;                     // Workgroup master interface descriptor
    sc_in<axi4_master_in_type> i_msti;
    sc_out<axi4_master_out_type> o_msto;
    // APB debug access:
    sc_in<mapinfo_type> i_dmi_mapinfo;                      // DMI APB itnerface mapping information
    sc_out<dev_config_type> o_dmi_cfg;                      // DMI device descriptor
    sc_in<apb_in_type> i_dmi_apbi;
    sc_out<apb_out_type> o_dmi_apbo;
    sc_out<bool> o_dmreset;                                 // reset everything except DMI debug interface

    void comb();

    SC_HAS_PROCESS(Workgroup);

    Workgroup(sc_module_name name,
              bool async_reset,
              uint32_t cpu_num,
              uint32_t ilog2_nways,
              uint32_t ilog2_lines_per_way,
              uint32_t dlog2_nways,
              uint32_t dlog2_lines_per_way,
              uint32_t l2cache_ena,
              uint32_t l2log2_nways,
              uint32_t l2log2_lines_per_way);
    virtual ~Workgroup();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t cpu_num_;
    uint32_t ilog2_nways_;
    uint32_t ilog2_lines_per_way_;
    uint32_t dlog2_nways_;
    uint32_t dlog2_lines_per_way_;
    uint32_t l2cache_ena_;
    uint32_t l2log2_nways_;
    uint32_t l2log2_lines_per_way_;
    bool coherence_ena;

    static const uint32_t ACP_SLOT_IDX = CFG_CPU_MAX;

    axi4_l1_out_vector coreo;
    axi4_l1_in_vector corei;
    sc_signal<axi4_l2_in_type> l2i;
    sc_signal<axi4_l2_out_type> l2o;
    dport_in_vector wb_dport_i;
    dport_out_vector wb_dport_o;
    hart_irq_vector vec_irq;
    hart_signal_vector vec_halted;
    hart_signal_vector vec_available;
    hart_signal_vector vec_flush_l2;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_halted;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_available;
    sc_signal<sc_uint<CFG_LOG2_CPU_MAX>> wb_dmi_hartsel;
    sc_signal<bool> w_dmi_haltreq;
    sc_signal<bool> w_dmi_resumereq;
    sc_signal<bool> w_dmi_resethaltreq;
    sc_signal<bool> w_dmi_hartreset;
    sc_signal<bool> w_dmi_dport_req_valid;
    sc_signal<sc_uint<DPortReq_Total>> wb_dmi_dport_req_type;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dmi_dport_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dmi_dport_wdata;
    sc_signal<sc_uint<3>> wb_dmi_dport_size;
    sc_signal<bool> w_ic_dport_req_ready;
    sc_signal<bool> w_dmi_dport_resp_ready;
    sc_signal<bool> w_ic_dport_resp_valid;
    sc_signal<bool> w_ic_dport_resp_error;
    sc_signal<sc_uint<RISCV_ARCH>> wb_ic_dport_rdata;
    sc_signal<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> wb_progbuf;
    sc_signal<bool> w_flush_l2;
    dev_config_type wb_xmst_cfg;

    dmidebug *dmi0;
    ic_dport *dport_ic0;
    ic_axi4_to_l1 *acp_bridge;
    RiverAmba *cpux[CFG_CPU_MAX];
    DummyCpu *dumx[CFG_CPU_MAX];
    L2Top *l2cache;
    L2Dummy *l2dummy;
    L2SerDes *l2serdes0;

};

}  // namespace debugger

