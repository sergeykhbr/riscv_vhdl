/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "api_core.h"
#include "cpu_riscv_rtl.h"

namespace debugger {

CpuRiscV_RTL::CpuRiscV_RTL(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    w_sd_cmd("w_sd_cmd") {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("HartID", &hartid_);
    registerAttribute("AsyncReset", &asyncReset_);
    registerAttribute("CpuNum", &cpuNum_);
    registerAttribute("L2CacheEnable", &l2CacheEnable_);
    registerAttribute("CLINT", &clint_);
    registerAttribute("PLIC", &plic_);
    registerAttribute("Bus", &bus_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("DmiBAR", &dmibar_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("InVcdFile", &InVcdFile_);
    registerAttribute("OutVcdFile", &OutVcdFile_);

    bus_.make_string("");
    freqHz_.make_uint64(1);
    InVcdFile_.make_string("");
    OutVcdFile_.make_string("");
    RISCV_event_create(&config_done_, "riscv_sysc_config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
}

CpuRiscV_RTL::~CpuRiscV_RTL() {
    deleteSystemC();
    RISCV_event_close(&config_done_);
}

void CpuRiscV_RTL::initService(const AttributeType *args) {
    IService::initService(args);

    createSystemC();
}

void CpuRiscV_RTL::postinitService() {
    ibus_ = static_cast<IMemoryOperation *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_MEMORY_OPERATION));

    if (!ibus_) {
        RISCV_error("Bus interface '%s' not found", 
                    bus_.to_string());
        return;
    }

    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
        return;
    }

    iirqext_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        plic_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirqext_) {
        RISCV_error("Interface IIrqController in %s not found",
                    plic_.to_string());
    }

    iirqloc_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        clint_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirqloc_) {
        RISCV_error("Interface IIrqController in %s not found",
                    clint_.to_string());
    }

    if (InVcdFile_.size()) {
        i_vcd_ = sc_create_vcd_trace_file(InVcdFile_.to_string());
        i_vcd_->set_time_unit(1, SC_PS);
    } else {
        i_vcd_ = 0;
    }

    if (OutVcdFile_.size()) {
        o_vcd_ = sc_create_vcd_trace_file(OutVcdFile_.to_string());
        o_vcd_->set_time_unit(1, SC_PS);
    } else {
        o_vcd_ = 0;
    }

    wrapper_->setBus(ibus_);
    wrapper_->setCLINT(iirqloc_);
    wrapper_->setPLIC(iirqext_);
    wrapper_->setClockHz(freqHz_.to_int());
    wrapper_->generateVCD(i_vcd_, o_vcd_);
    if (dmislv_) {
        dmislv_->setBaseAddress(dmibar_.to_uint64());
        dmislv_->setLength(4096);
    }
    if (group0_) {
        group0_->generateVCD(i_vcd_, o_vcd_);
    }
    if (asic0_) {
        asic0_->generateVCD(i_vcd_, o_vcd_);
    }
    if (uart0_) {
        uart0_->generateVCD(i_vcd_, o_vcd_);
    }
    if (sdcard0_) {
        sdcard0_->generateVCD(i_vcd_, o_vcd_);
    }

    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void CpuRiscV_RTL::predeleteService() {
}

void CpuRiscV_RTL::createSystemC() {
    sc_set_default_time_unit(1, SC_NS);


    /** Create all objects, then initilize SystemC context: */
    wrapper_ = new RtlWrapper(static_cast<IService *>(this), "wrapper");
    registerInterface(static_cast<ICpuRiscV *>(wrapper_));
    registerInterface(static_cast<IResetListener *>(wrapper_));
    w_clk = wrapper_->o_clk;
    wrapper_->o_rst(w_rst);
    wrapper_->o_sys_nrst(w_sys_nrst);
    wrapper_->o_dmi_nrst(w_dmi_nrst);
    wrapper_->o_msti(msti);
    wrapper_->i_msto(msto);
    wrapper_->o_mtimer(wb_mtimer);
    wrapper_->o_msip(wb_msip);
    wrapper_->o_mtip(wb_mtip);
    wrapper_->o_meip(wb_meip);
    wrapper_->o_seip(wb_seip);
    wrapper_->i_ndmreset(w_ndmreset);

    tapbb_ = new TapBitBang("tapbb");
    registerPortInterface("tap", static_cast<IJtagBitBang *>(tapbb_));
    tapbb_->i_clk(wrapper_->o_clk);
    tapbb_->o_trst(w_trst);
    tapbb_->o_tck(w_tck);
    tapbb_->o_tms(w_tms);
    tapbb_->o_tdo(w_tdi);
    tapbb_->i_tdi(w_tdo);

#if 0
    dmislv_ = new BusSlave("dmislv");
    registerPortInterface("dmi", static_cast<IMemoryOperation *>(dmislv_));
    dmislv_->i_clk(wrapper_->o_clk),
    dmislv_->i_nrst(w_dmi_nrst),
    dmislv_->o_apbi(wb_dmi_apbi);
    dmislv_->i_apbo(wb_dmi_apbo);

    group0_ = new Workgroup("group0",
                            asyncReset_.to_bool(),
                            cpuNum_.to_uint32(),
                            2, 7,
                            2, 7,
                            l2CacheEnable_.to_uint32(),
                            4, 9);
    group0_->i_cores_nrst(w_sys_nrst);
    group0_->i_dmi_nrst(w_dmi_nrst);
    group0_->i_clk(wrapper_->o_clk);
    group0_->i_trst(w_trst);
    group0_->i_tck(w_tck);
    group0_->i_tms(w_tms);
    group0_->i_tdi(w_tdi);
    group0_->o_tdo(w_tdo);
    group0_->i_msip(wb_msip);
    group0_->i_mtip(wb_mtip);
    group0_->i_meip(wb_meip);
    group0_->i_seip(wb_seip);
    group0_->i_mtimer(wb_mtimer);
    group0_->i_acpo(acpo);
    group0_->o_acpi(acpi);
    group0_->o_xmst_cfg(xcfg);
    group0_->i_msti(msti);
    group0_->o_msto(msto);
    group0_->i_dmi_mapinfo(wb_dmi_mapinfo);
    group0_->o_dmi_cfg(wb_dmi_cfg);
    group0_->i_dmi_apbi(wb_dmi_apbi);
    group0_->o_dmi_apbo(wb_dmi_apbo);
    group0_->o_dmreset(w_ndmreset);

    asic0_ = 0;
    uart0_ = 0;
    sdcard0_ = 0;
#else
    int SIM_UART_SPEED_UP_RATE = 3;
    int uart_scaler = 8;   // expected uart bit edge in a range 8..16 of scaler counter
    asic0_ = new asic_top("tt",
                          SIM_UART_SPEED_UP_RATE);

    asic0_->i_rst(w_rst);
    asic0_->i_sclk_p(wrapper_->o_clk);
    asic0_->i_sclk_n(wrapper_->o_clk);
    asic0_->io_gpio(wb_gpio);
    asic0_->i_jtag_trst(w_trst);
    asic0_->i_jtag_tck(w_tck);
    asic0_->i_jtag_tms(w_tms);
    asic0_->i_jtag_tdi(w_tdi);
    asic0_->o_jtag_tdo(w_tdo);
    asic0_->o_jtag_vref(w_jtag_vref);
    asic0_->i_uart1_rd(w_uart1_rd);
    asic0_->o_uart1_td(w_uart1_td);
    asic0_->o_sd_sclk(w_sd_sclk);
    asic0_->io_sd_cmd(w_sd_cmd);
    asic0_->io_sd_dat0(w_sd_dat0);
    asic0_->io_sd_dat1(w_sd_dat1);
    asic0_->io_sd_dat2(w_sd_dat2);
    asic0_->io_sd_cd_dat3(w_sd_dat3);
    asic0_->i_sd_detected(w_sd_detected);
    asic0_->i_sd_protect(w_sd_protect);

    uart0_ = new vip_uart_top("uart0",
                              asyncReset_.to_bool(),
                              0,
                              115200 * (1 << SIM_UART_SPEED_UP_RATE),
                              uart_scaler,
                              "uart");
    uart0_->i_nrst(w_sys_nrst);
    uart0_->i_rx(w_uart1_td);
    uart0_->o_tx(w_uart1_rd);

    sdcard0_ = new vip_sdcard_top("sdcard0",
                                  true);
    sdcard0_->i_nrst(w_sys_nrst);
    sdcard0_->i_sclk(w_sd_sclk);
    sdcard0_->io_cmd(w_sd_cmd);
    sdcard0_->io_dat0(w_sd_dat0);
    sdcard0_->io_dat1(w_sd_dat1);
    sdcard0_->io_dat2(w_sd_dat2);
    sdcard0_->io_cd_dat3(w_sd_dat3);

    dmislv_ = 0;
    group0_ = 0;
#endif

#ifdef DBG_ICACHE_LRU_TB
    ICacheLru_tb *tb = new ICacheLru_tb("tb");
#endif
#ifdef DBG_DCACHE_LRU_TB
    DCacheLru_tb *tb = new DCacheLru_tb("tb");
#endif
#ifdef DBG_IDIV_TB
    IntDiv_tb *tb = new IntDiv_tb("tb");
#endif

    //sc_start(0, SC_NS);
    sc_initialize();
}

void CpuRiscV_RTL::deleteSystemC() {
    delete wrapper_;
    delete tapbb_;
    if (dmislv_) {
        delete dmislv_;
    }
    if (group0_) {
        delete group0_;
    }
    if (asic0_) {
        delete asic0_;
    }
    if (uart0_) {
        delete uart0_;
    }
    if (sdcard0_) {
        delete sdcard0_;
    }
}

void CpuRiscV_RTL::hapTriggered(EHapType type,
                                uint64_t param,
                                const char *descr) {
    RISCV_event_set(&config_done_);
}

void CpuRiscV_RTL::stop() {
    sc_stop();
    IThread::stop();
}

void CpuRiscV_RTL::busyLoop() {
    RISCV_event_wait(&config_done_);

    sc_start();

    if (i_vcd_) {
        sc_close_vcd_trace_file(i_vcd_);
    }
    if (o_vcd_) {
        sc_close_vcd_trace_file(o_vcd_);
    }
}

}  // namespace debugger

