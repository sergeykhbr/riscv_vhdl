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
#include "generic/dmi/cmd_dmi_cpu.h"

namespace debugger {

CpuRiscV_RTL::CpuRiscV_RTL(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    corei("corei", CFG_SLOT_L1_TOTAL),
    coreo("coreo", CFG_SLOT_L1_TOTAL),
    wb_irq_pending("wb_irq_pending", IRQ_TOTAL),
    wb_dporti("wb_dporti", CFG_CPU_MAX),
    wb_dporto("wb_dporto", CFG_CPU_MAX) {
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
    coherenceEnable_ = false;;
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
    dmislv_->setBaseAddress(dmibar_.to_uint64());
    dmislv_->setLength(4096);
    dmi_->generateVCD(i_vcd_, o_vcd_);
    if (l2cache_) {
        l2cache_->generateVCD(i_vcd_, o_vcd_);
    }
    if (l1serdes_) {
        l1serdes_->generateVCD(i_vcd_, o_vcd_);
    }
    core_->generateVCD(i_vcd_, o_vcd_);

    pcmd_br_ = new CmdBrRiscv(dmibar_.to_uint64(), 0);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));

    pcmd_cpu_ = new CmdDmiCpuRiscV(static_cast<IService *>(this));
    pcmd_cpu_->enableDMA(ibus_, dmibar_.to_uint64());
    icmdexec_->registerCommand(pcmd_cpu_);

    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void CpuRiscV_RTL::predeleteService() {
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    icmdexec_->unregisterCommand(pcmd_cpu_);
    delete pcmd_br_;
    delete pcmd_cpu_;
}

void CpuRiscV_RTL::createSystemC() {
    sc_set_default_time_unit(1, SC_NS);

    wb_available = 0x1; // only 1 core available

    /** Create all objects, then initilize SystemC context: */
    wrapper_ = new RtlWrapper(static_cast<IService *>(this), "wrapper");
    registerInterface(static_cast<ICpuRiscV *>(wrapper_));
    registerInterface(static_cast<IResetListener *>(wrapper_));
    w_clk = wrapper_->o_clk;
    wrapper_->o_sys_nrst(w_sys_nrst);
    wrapper_->o_dmi_nrst(w_dmi_nrst);
    wrapper_->o_msti(msti);
    wrapper_->i_msto(msto);
    wrapper_->o_mtimer(wb_mtimer);
    wrapper_->o_irq_pending(wb_irq_pending);
    wrapper_->i_dporti(wb_dporti[0]);
    wrapper_->i_ndmreset(w_ndmreset);
    wrapper_->i_halted0(w_halted0);
    wrapper_->o_halted(wb_halted);

    tapbb_ = new TapBitBang("tapbb");
    registerPortInterface("tap", static_cast<IJtagTap *>(tapbb_));
    tapbb_->i_clk(wrapper_->o_clk);
    tapbb_->o_trst(w_trst);
    tapbb_->o_tck(w_tck);
    tapbb_->o_tms(w_tms);
    tapbb_->o_tdo(w_tdi);
    tapbb_->i_tdi(w_tdo);

    dmislv_ = new BusSlave("dmislv");
    registerPortInterface("dmi", static_cast<IMemoryOperation *>(dmislv_));
    dmislv_->i_clk(wrapper_->o_clk),
    dmislv_->i_nrst(w_dmi_nrst),
    dmislv_->o_req_valid(w_bus_req_valid);
    dmislv_->i_req_ready(w_bus_req_ready);
    dmislv_->o_req_addr(wb_bus_req_addr);
    dmislv_->o_req_write(w_bus_req_write);
    dmislv_->o_req_wdata(wb_bus_req_wdata);
    dmislv_->i_slv_resp_valid(w_bus_resp_valid);
    dmislv_->o_slv_resp_ready(w_bus_resp_ready);
    dmislv_->i_slv_resp_rdata(wb_bus_resp_rdata);


    dmi_ = new DmiDebug(static_cast<IService *>(this),
                        "dmidbg",
                        asyncReset_.to_bool());
    dmi_->i_clk(wrapper_->o_clk);
    dmi_->i_nrst(w_dmi_nrst);
    dmi_->i_trst(w_trst);
    dmi_->i_tck(w_tck);
    dmi_->i_tms(w_tms);
    dmi_->i_tdi(w_tdi);
    dmi_->o_tdo(w_tdo);
    dmi_->i_bus_req_valid(w_bus_req_valid);
    dmi_->o_bus_req_ready(w_bus_req_ready);
    dmi_->i_bus_req_addr(wb_bus_req_addr);
    dmi_->i_bus_req_write(w_bus_req_write);
    dmi_->i_bus_req_wdata(wb_bus_req_wdata);
    dmi_->o_bus_resp_valid(w_bus_resp_valid);
    dmi_->i_bus_resp_ready(w_bus_resp_ready);
    dmi_->o_bus_resp_rdata(wb_bus_resp_rdata);
    dmi_->o_ndmreset(w_ndmreset);               // reset whole system
    dmi_->i_halted(wb_halted);
    dmi_->i_available(wb_available);
    dmi_->o_hartsel(wb_hartsel);
    dmi_->i_dporto(wb_dporto[0]);
    dmi_->o_dporti(wb_dporti[0]);
    dmi_->o_progbuf(wb_progbuf);


    if (l2CacheEnable_.to_bool()) {
        l2cache_ = new L2Top("l2top", asyncReset_.to_bool());
        l2cache_->i_clk(wrapper_->o_clk);
        l2cache_->i_nrst(w_sys_nrst);
        l2cache_->o_l1i(corei);
        l2cache_->i_l1o(coreo);
        l2cache_->i_l2i(l2i);
        l2cache_->o_l2o(l2o);
        l2cache_->i_flush_valid(w_flush_l2);

        coherenceEnable_ = cpuNum_.to_int() > 1;
        l1serdes_ = 0;
    } else {
        l1serdes_ = new L1SerDes("l1serdes0", asyncReset_.to_bool());
        l1serdes_->i_clk(wrapper_->o_clk);
        l1serdes_->i_nrst(w_sys_nrst);
        l1serdes_->o_corei(corei[0]);
        l1serdes_->i_coreo(coreo[0]);
        l1serdes_->i_msti(msti);
        l1serdes_->o_msto(msto);

        l2cache_ = 0;
    }

    core_ = new RiverAmba("core0", hartid_.to_uint32(),
                               asyncReset_.to_bool(),
                               CFG_HW_FPU_ENABLE,
                               coherenceEnable_,
                               CFG_TRACER_ENABLE);
    core_->i_clk(wrapper_->o_clk);
    core_->i_nrst(w_sys_nrst);
    core_->i_mtimer(wb_mtimer);
    core_->i_msti(corei[0]);
    core_->o_msto(coreo[0]);
    core_->o_xcfg(xcfg);
    core_->i_dport(wb_dporti[0]);
    core_->o_dport(wb_dporto[0]);
    core_->i_msip(wb_irq_pending[IRQ_MSIP]);
    core_->i_mtip(wb_irq_pending[IRQ_MTIP]);
    core_->i_meip(wb_irq_pending[IRQ_MEIP]);
    core_->i_seip(wb_irq_pending[IRQ_SEIP]);
    core_->o_flush_l2(w_flush_l2);
    core_->o_halted(w_halted0);
    core_->o_available(w_available0);
    core_->i_progbuf(wb_progbuf);

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
    delete dmi_;
    delete core_;
    if (l1serdes_) {
        delete l1serdes_;
    }
    if (l2cache_) {
        delete l2cache_;
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

