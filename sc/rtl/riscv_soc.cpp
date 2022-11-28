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
    bus0_mapinfo("bus0_mapinfo", CFG_BUS0_XSLV_TOTAL),
    aximi("aximi", CFG_BUS0_XMST_TOTAL),
    aximo("aximo", CFG_BUS0_XMST_TOTAL),
    axisi("axisi", CFG_BUS0_XSLV_TOTAL),
    axiso("axiso", CFG_BUS0_XSLV_TOTAL),
    bus1_mapinfo("bus1_mapinfo", CFG_BUS1_PSLV_TOTAL),
    apbmi("apbmi", CFG_BUS1_PMST_TOTAL),
    apbmo("apbmo", CFG_BUS1_PMST_TOTAL),
    apbsi("apbsi", CFG_BUS1_PSLV_TOTAL),
    apbso("apbso", CFG_BUS1_PSLV_TOTAL),
    dev_pnp("dev_pnp", SOC_PNP_TOTAL) {

    apbrdg0 = 0;
    uart1 = 0;
    group0 = 0;

    apbrdg0 = new axi2apb("apbrdg0", async_reset);
    apbrdg0->i_clk(i_clk);
    apbrdg0->i_nrst(w_sys_nrst);
    apbrdg0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->o_cfg(dev_pnp[SOC_PNP_PBRIDGE0]);
    apbrdg0->i_xslvi(axisi[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->o_xslvo(axiso[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->i_apbmi(apbmi[CFG_BUS1_PMST_PARENT]);
    apbrdg0->o_apbmo(apbmo[CFG_BUS1_PMST_PARENT]);


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
    group0->i_acpo(acpo);
    group0->o_acpi(acpi);
    group0->o_xmst_cfg(dev_pnp[SOC_PNP_GROUP0]);
    group0->i_msti(aximi[CFG_BUS0_XMST_GROUP0]);
    group0->o_msto(aximo[CFG_BUS0_XMST_GROUP0]);
    group0->i_dmi_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_DMI]);
    group0->o_dmi_cfg(dev_pnp[SOC_PNP_DMI]);
    group0->i_dmi_apbi(apbsi[CFG_BUS1_PSLV_DMI]);
    group0->o_dmi_apbo(apbso[CFG_BUS1_PSLV_DMI]);
    group0->o_dmreset(w_dmreset);


    uart1 = new apb_uart<CFG_SOC_UART1_LOG2_FIFOSZ>("uart1", async_reset);
    uart1->i_clk(i_clk);
    uart1->i_nrst(w_sys_nrst);
    uart1->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_UART1]);
    uart1->o_cfg(dev_pnp[SOC_PNP_UART1]);
    uart1->i_apbi(apbsi[CFG_BUS1_PSLV_UART1]);
    uart1->o_apbo(apbso[CFG_BUS1_PSLV_UART1]);
    uart1->i_rd(i_uart1_rd);
    uart1->o_td(o_uart1_td);
    uart1->o_irq(w_irq_uart1);



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
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << bus0_mapinfo[i];
    }
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
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << bus1_mapinfo[i];
    }
    for (int i = 0; i < CFG_BUS1_PMST_TOTAL; i++) {
        sensitive << apbmi[i];
    }
    for (int i = 0; i < CFG_BUS1_PMST_TOTAL; i++) {
        sensitive << apbmo[i];
    }
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << apbsi[i];
    }
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << apbso[i];
    }
    for (int i = 0; i < SOC_PNP_TOTAL; i++) {
        sensitive << dev_pnp[i];
    }
    sensitive << wb_clint_mtimer;
    sensitive << wb_clint_msip;
    sensitive << wb_clint_mtip;
    sensitive << wb_plic_meip;
    sensitive << wb_plic_seip;
    sensitive << w_irq_uart1;
}

riscv_soc::~riscv_soc() {
    if (apbrdg0) {
        delete apbrdg0;
    }
    if (uart1) {
        delete uart1;
    }
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

    if (apbrdg0) {
        apbrdg0->generateVCD(i_vcd, o_vcd);
    }
    if (uart1) {
        uart1->generateVCD(i_vcd, o_vcd);
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

    // TODO: APB interconnect
    apbsi[CFG_BUS1_PSLV_DMI] = apb_in_none;
    apbsi[CFG_BUS1_PSLV_UART1] = apbmo[CFG_BUS1_PMST_PARENT];
    apbmi[CFG_BUS1_PMST_PARENT] = apbso[CFG_BUS1_PSLV_UART1];
}

}  // namespace debugger

