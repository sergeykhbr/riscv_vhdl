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

#include "riscv_soc.h"
#include "api_core.h"

namespace debugger {

riscv_soc::riscv_soc(sc_module_name name)
    : sc_module(name),
    i_rst("i_rst"),
    i_clk("i_clk"),
    i_gpio("i_gpio"),
    o_gpio("o_gpio"),
    o_gpio_dir("o_gpio_dir"),
    i_jtag_trst("i_jtag_trst"),
    i_jtag_tck("i_jtag_tck"),
    i_jtag_tms("i_jtag_tms"),
    i_jtag_tdi("i_jtag_tdi"),
    o_jtag_tdo("o_jtag_tdo"),
    o_jtag_vref("o_jtag_vref"),
    i_uart1_rd("i_uart1_rd"),
    o_uart1_td("o_uart1_td"),
    aximi("aximi", CFG_BUS0_XMST_TOTAL),
    aximo("aximo", CFG_BUS0_XMST_TOTAL),
    axisi("axisi", CFG_BUS0_XSLV_TOTAL),
    axiso("axiso", CFG_BUS0_XSLV_TOTAL),
    dev_pnp("dev_pnp", PNP_SLOTS_TOTAL) {

    group0 = 0;

    group0 = new Workgroup("group0", async_reset, CFG_CPU_NUM, CFG_L2CACHE_ENA);
    group0->i_cores_nrst(w_sys_nrst);
    group0->i_dmi_nrst(w_dbg_nrst);
    group0->i_clk(i_clk);
    group0->i_trst(i_jtag_trst);
    group0->i_tck(i_jtag_tck);
    group0->i_tms(i_jtag_tms);
    group0->i_tdi(i_jtag_tdi);
    group0->o_tdo(o_jtag_tdo);
    group0->i_msip(wb_clint_msip);
    group0->i_mtip(wb_clint_mtip);
    group0->i_meip(wb_plic_meip);
    group0->i_seip(wb_plic_seip);
    group0->i_mtimer(wb_clint_mtimer);
    group0->o_xcfg(dev_pnp[CFG_BUS0_XMST_CPU0]);
    group0->i_acpo(acpo);
    group0->o_acpi(acpi);
    group0->i_msti(aximi[CFG_BUS0_XMST_CPU0]);
    group0->o_msto(aximo[CFG_BUS0_XMST_CPU0]);
    group0->i_dmi_apbi(apb_dmi_i);
    group0->o_dmi_apbo(apb_dmi_o);
    group0->o_dmreset(w_dmreset);



    SC_METHOD(comb);
    sensitive << i_rst;
    sensitive << i_gpio;
    sensitive << i_jtag_trst;
    sensitive << i_jtag_tck;
    sensitive << i_jtag_tms;
    sensitive << i_jtag_tdi;
    sensitive << i_uart1_rd;
    sensitive << w_sys_nrst;
    sensitive << w_dbg_nrst;
    sensitive << w_dmreset;
    sensitive << acpo;
    sensitive << acpi;
    sensitive << apb_dmi_i;
    sensitive << apb_dmi_o;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << aximi[i];
    }
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << aximo[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << axisi[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << axiso[i];
    }
    for (int i = 0; i < PNP_SLOTS_TOTAL; i++) {
        sensitive << dev_pnp[i];
    }
    sensitive << wb_clint_mtimer;
    sensitive << wb_clint_msip;
    sensitive << wb_clint_mtip;
    sensitive << wb_plic_meip;
    sensitive << wb_plic_seip;
}

riscv_soc::~riscv_soc() {
    if (group0) {
        delete group0;
    }
}

void riscv_soc::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_rst, i_rst.name());
        sc_trace(o_vcd, i_clk, i_clk.name());
        sc_trace(o_vcd, i_gpio, i_gpio.name());
        sc_trace(o_vcd, o_gpio, o_gpio.name());
        sc_trace(o_vcd, o_gpio_dir, o_gpio_dir.name());
        sc_trace(o_vcd, i_jtag_trst, i_jtag_trst.name());
        sc_trace(o_vcd, i_jtag_tck, i_jtag_tck.name());
        sc_trace(o_vcd, i_jtag_tms, i_jtag_tms.name());
        sc_trace(o_vcd, i_jtag_tdi, i_jtag_tdi.name());
        sc_trace(o_vcd, o_jtag_tdo, o_jtag_tdo.name());
        sc_trace(o_vcd, o_jtag_vref, o_jtag_vref.name());
        sc_trace(o_vcd, i_uart1_rd, i_uart1_rd.name());
        sc_trace(o_vcd, o_uart1_td, o_uart1_td.name());
    }

    if (group0) {
        group0->generateVCD(i_vcd, o_vcd);
    }
}

void riscv_soc::comb() {
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

}

}  // namespace debugger

