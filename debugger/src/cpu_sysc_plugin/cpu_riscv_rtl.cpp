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
#include "generic/dmi/cmd_dmi_runcontrol.h"

namespace debugger {

CpuRiscV_RTL::CpuRiscV_RTL(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("HartID", &hartid_);
    registerAttribute("AsyncReset", &asyncReset_);
    registerAttribute("FpuEnable", &fpuEnable_);
    registerAttribute("TracerEnable", &tracerEnable_);
    registerAttribute("L2CacheEnable", &l2CacheEnable_);
    registerAttribute("CoherenceEnable", &coherenceEnable_);
    registerAttribute("Bus", &bus_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("Tap", &tap_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("InVcdFile", &InVcdFile_);
    registerAttribute("OutVcdFile", &OutVcdFile_);

    bus_.make_string("");
    freqHz_.make_uint64(1);
    fpuEnable_.make_boolean(true);
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

    itap_ = static_cast<ITap *>(
       RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    if (!itap_) {
        RISCV_error("ITap interface '%s' not found", tap_.to_string());
        return;
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
    wrapper_->setClockHz(freqHz_.to_int());
    wrapper_->generateVCD(i_vcd_, o_vcd_);
    dmi_->generateVCD(i_vcd_, o_vcd_);
    if (l2cache_) {
        l2cache_->generateVCD(i_vcd_, o_vcd_);
    }
    if (l1serdes_) {
        l1serdes_->generateVCD(i_vcd_, o_vcd_);
    }
    core_->generateVCD(i_vcd_, o_vcd_);

    pcmd_br_ = new CmdBrRiscv(DMI_BASE_ADDRESS, itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));

    pcmd_csr_ = new CmdCsr(DMI_BASE_ADDRESS, itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_csr_));

    pcmd_reg_ = new CmdRegRiscv(DMI_BASE_ADDRESS, itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_reg_));

    pcmd_regs_ = new CmdRegsRiscv(DMI_BASE_ADDRESS, itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_regs_));

    pcmd_runctrl_ = new CmdDmiRunControl(static_cast<IService *>(this),
                                         DMI_BASE_ADDRESS,  // todo: change on attribute
                                         itap_);
    icmdexec_->registerCommand(pcmd_runctrl_);

    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }
}

void CpuRiscV_RTL::predeleteService() {
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_csr_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_reg_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_regs_));
    icmdexec_->unregisterCommand(pcmd_runctrl_);
    delete pcmd_br_;
    delete pcmd_csr_;
    delete pcmd_reg_;
    delete pcmd_regs_;
    delete pcmd_runctrl_;
}

void CpuRiscV_RTL::createSystemC() {
    sc_set_default_time_unit(1, SC_NS);

    wb_available = 0x1; // only 1 core available

    /** Create all objects, then initilize SystemC context: */
    wrapper_ = new RtlWrapper(static_cast<IService *>(this), "wrapper");
    registerInterface(static_cast<ICpuGeneric *>(wrapper_));
    registerInterface(static_cast<ICpuRiscV *>(wrapper_));
    registerInterface(static_cast<IResetListener *>(wrapper_));
    w_clk = wrapper_->o_clk;
    wrapper_->o_nrst(w_nrst);
    wrapper_->o_msti(msti);
    wrapper_->i_msto(msto);
    wrapper_->o_interrupt(w_plic_irq);
    wrapper_->i_hartreset(w_hartreset);
    wrapper_->i_ndmreset(w_ndmreset);
    wrapper_->i_halted0(w_halted0);
    wrapper_->o_halted(wb_halted);

    dmi_ = new DmiDebug(static_cast<IService *>(this),
                        "dmidbg",
                        asyncReset_.to_bool());
    registerPortInterface("dmi", static_cast<IMemoryOperation *>(dmi_));
    registerPortInterface("tap", static_cast<IJtagTap *>(dmi_));
    dmi_->i_clk(wrapper_->o_clk);
    dmi_->i_nrst(w_nrst);
    dmi_->o_ndmreset(w_ndmreset);               // reset whole system
    dmi_->i_halted(wb_halted);
    dmi_->i_available(wb_available);
    dmi_->o_hartsel(wb_hartsel);
    dmi_->o_haltreq(w_haltreq);
    dmi_->o_resumereq(w_resumereq);
    dmi_->o_resethaltreq(w_resethaltreq);       // Halt after reset
    dmi_->o_hartreset(w_hartreset);             // reselet only selected core
    dmi_->o_dport_req_valid(w_dport_req_valid);
    dmi_->o_dport_req_type(wb_dport_type);
    dmi_->o_dport_addr(wb_dport_addr);
    dmi_->o_dport_wdata(wb_dport_wdata);
    dmi_->o_dport_size(wb_dport_size);
    dmi_->i_dport_req_ready(w_dport_req_ready);
    dmi_->o_dport_resp_ready(w_dport_resp_ready);
    dmi_->i_dport_resp_valid(w_dport_resp_valid);
    dmi_->i_dport_resp_error(w_dport_resp_error);
    dmi_->i_dport_rdata(wb_dport_rdata);
    dmi_->o_progbuf(wb_progbuf);


    if (l2CacheEnable_.to_bool()) {
        l2cache_ = new L2Top("l2top", asyncReset_.to_bool());
        l2cache_->i_clk(wrapper_->o_clk);
        l2cache_->i_nrst(w_nrst);
        l2cache_->o_l1i0(corei0);
        l2cache_->i_l1o0(coreo0);
        l2cache_->o_l1i1(corei1);
        l2cache_->i_l1o1(coreo1);
        l2cache_->o_l1i2(corei2);
        l2cache_->i_l1o2(coreo2);
        l2cache_->o_l1i3(corei3);
        l2cache_->i_l1o3(coreo3);
        l2cache_->i_acpo(acpo);
        l2cache_->o_acpi(acpi);
        l2cache_->i_msti(msti);
        l2cache_->o_msto(msto);

        l1serdes_ = 0;
    } else {
        l1serdes_ = new L1SerDes("l1serdes0", asyncReset_.to_bool());
        l1serdes_->i_clk(wrapper_->o_clk);
        l1serdes_->i_nrst(w_nrst);
        l1serdes_->o_corei(corei0);
        l1serdes_->i_coreo(coreo0);
        l1serdes_->i_msti(msti);
        l1serdes_->o_msto(msto);

        l2cache_ = 0;
    }

    core_ = new RiverAmba("core0", hartid_.to_uint32(),
                               asyncReset_.to_bool(),
                               fpuEnable_.to_bool(),
                               coherenceEnable_.to_bool(),
                               tracerEnable_.to_bool());
    core_->i_clk(wrapper_->o_clk);
    core_->i_nrst(w_nrst);
    core_->i_msti(corei0);
    core_->o_msto(coreo0);
    core_->i_tmr_irq(w_tmr_irq);
    core_->i_ext_irq(w_plic_irq);
    core_->i_haltreq(w_haltreq);
    core_->i_resumereq(w_resumereq);
    core_->i_dport_req_valid(w_dport_req_valid);
    core_->i_dport_type(wb_dport_type);
    core_->i_dport_addr(wb_dport_addr);
    core_->i_dport_wdata(wb_dport_wdata);
    core_->i_dport_size(wb_dport_size);
    core_->o_dport_req_ready(w_dport_req_ready);
    core_->i_dport_resp_ready(w_dport_resp_ready);
    core_->o_dport_resp_valid(w_dport_resp_valid);
    core_->o_dport_resp_error(w_dport_resp_error);
    core_->o_dport_rdata(wb_dport_rdata);
    core_->i_progbuf(wb_progbuf);
    core_->o_halted(w_halted0);

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

