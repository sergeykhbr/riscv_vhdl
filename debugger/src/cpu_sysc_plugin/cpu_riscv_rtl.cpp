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
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("HartID", &hartid_);
    registerAttribute("AsyncReset", &asyncReset_);
    registerAttribute("FpuEnable", &fpuEnable_);
    registerAttribute("TracerEnable", &tracerEnable_);
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

    //createSystemC();
}

CpuRiscV_RTL::~CpuRiscV_RTL() {
    deleteSystemC();
    RISCV_event_close(&config_done_);
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

    createSystemC();

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
    if (serdes_) {
        serdes_->generateVCD(i_vcd_, o_vcd_);
    }
    core_->generateVCD(i_vcd_, o_vcd_);

    pcmd_br_ = new CmdBrRiscv(itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));

    pcmd_csr_ = new CmdCsr(itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_csr_));

    pcmd_reg_ = new CmdRegRiscv(itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_reg_));

    pcmd_regs_ = new CmdRegsRiscv(itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_regs_));

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
    delete pcmd_br_;
    delete pcmd_csr_;
    delete pcmd_reg_;
    delete pcmd_regs_;
}

void CpuRiscV_RTL::createSystemC() {
    sc_set_default_time_unit(1, SC_NS);

    /** Create all objects, then initilize SystemC context: */
    wrapper_ = new RtlWrapper(static_cast<IService *>(this), "wrapper");
    registerInterface(static_cast<ICpuGeneric *>(wrapper_));
    registerInterface(static_cast<ICpuRiscV *>(wrapper_));
    registerInterface(static_cast<IResetListener *>(wrapper_));
    w_clk = wrapper_->o_clk;
    wrapper_->o_nrst(w_nrst);
    wrapper_->i_time(wb_time);
    wrapper_->o_msti_aw_ready(msti_aw_ready_i);
    wrapper_->o_msti_w_ready(msti_w_ready_i);
    wrapper_->o_msti_b_valid(msti_b_valid_i);
    wrapper_->o_msti_b_resp(msti_b_resp_i);
    wrapper_->o_msti_b_id(msti_b_id_i);
    wrapper_->o_msti_b_user(msti_b_user_i);
    wrapper_->o_msti_ar_ready(msti_ar_ready_i);
    wrapper_->o_msti_r_valid(msti_r_valid_i);
    wrapper_->o_msti_r_resp(msti_r_resp_i);
    wrapper_->o_msti_r_data(msti_r_data_i);
    wrapper_->o_msti_r_last(msti_r_last_i);
    wrapper_->o_msti_r_id(msti_r_id_i);
    wrapper_->o_msti_r_user(msti_r_user_i);
    wrapper_->i_msto_aw_valid(msto_aw_valid_o);
    wrapper_->i_msto_aw_bits_addr(msto_aw_bits_addr_o);
    wrapper_->i_msto_aw_bits_len(msto_aw_bits_len_o);
    wrapper_->i_msto_aw_bits_size(msto_aw_bits_size_o);
    wrapper_->i_msto_aw_bits_burst(msto_aw_bits_burst_o);
    wrapper_->i_msto_aw_bits_lock(msto_aw_bits_lock_o);
    wrapper_->i_msto_aw_bits_cache(msto_aw_bits_cache_o);
    wrapper_->i_msto_aw_bits_prot(msto_aw_bits_prot_o);
    wrapper_->i_msto_aw_bits_qos(msto_aw_bits_qos_o);
    wrapper_->i_msto_aw_bits_region(msto_aw_bits_region_o);
    wrapper_->i_msto_aw_id(msto_aw_id_o);
    wrapper_->i_msto_aw_user(msto_aw_user_o);
    wrapper_->i_msto_w_valid(msto_w_valid_o);
    wrapper_->i_msto_w_data(msto_w_data_o);
    wrapper_->i_msto_w_last(msto_w_last_o);
    wrapper_->i_msto_w_strb(msto_w_strb_o);
    wrapper_->i_msto_w_user(msto_w_user_o);
    wrapper_->i_msto_b_ready(msto_b_ready_o);
    wrapper_->i_msto_ar_valid(msto_ar_valid_o);
    wrapper_->i_msto_ar_bits_addr(msto_ar_bits_addr_o);
    wrapper_->i_msto_ar_bits_len(msto_ar_bits_len_o);
    wrapper_->i_msto_ar_bits_size(msto_ar_bits_size_o);
    wrapper_->i_msto_ar_bits_burst(msto_ar_bits_burst_o);
    wrapper_->i_msto_ar_bits_lock(msto_ar_bits_lock_o);
    wrapper_->i_msto_ar_bits_cache(msto_ar_bits_cache_o);
    wrapper_->i_msto_ar_bits_prot(msto_ar_bits_prot_o);
    wrapper_->i_msto_ar_bits_qos(msto_ar_bits_qos_o);
    wrapper_->i_msto_ar_bits_region(msto_ar_bits_region_o);
    wrapper_->i_msto_ar_id(msto_ar_id_o);
    wrapper_->i_msto_ar_user(msto_ar_user_o);
    wrapper_->i_msto_r_ready(msto_r_ready_o);
    wrapper_->o_interrupt(w_interrupt);
    wrapper_->o_dport_valid(w_dport_valid);
    wrapper_->o_dport_write(w_dport_write);
    wrapper_->o_dport_region(wb_dport_region);
    wrapper_->o_dport_addr(wb_dport_addr);
    wrapper_->o_dport_wdata(wb_dport_wdata);
    wrapper_->i_dport_ready(w_dport_ready);
    wrapper_->i_dport_rdata(wb_dport_rdata);
    wrapper_->i_halted(w_halted);

#if 1
    serdes_ = new AxiSerDes("serdes0", asyncReset_.to_bool());
    serdes_->i_clk(wrapper_->o_clk);
    serdes_->i_nrst(w_nrst);
    serdes_->o_slvi_aw_ready(corei_aw_ready_i);
    serdes_->o_slvi_w_ready(corei_w_ready_i);
    serdes_->o_slvi_b_valid(corei_b_valid_i);
    serdes_->o_slvi_b_resp(corei_b_resp_i);
    serdes_->o_slvi_b_id(corei_b_id_i);
    serdes_->o_slvi_b_user(corei_b_user_i);
    serdes_->o_slvi_ar_ready(corei_ar_ready_i);
    serdes_->o_slvi_r_valid(corei_r_valid_i);
    serdes_->o_slvi_r_resp(corei_r_resp_i);
    serdes_->o_slvi_r_data(corei_r_data_i);
    serdes_->o_slvi_r_last(corei_r_last_i);
    serdes_->o_slvi_r_id(corei_r_id_i);
    serdes_->o_slvi_r_user(corei_r_user_i);
    serdes_->o_slvi_ac_valid(corei_ac_valid_i);
    serdes_->o_slvi_ac_addr(corei_ac_addr_i);
    serdes_->o_slvi_ac_snoop(corei_ac_snoop_i);
    serdes_->o_slvi_ac_prot(corei_ac_prot_i);
    serdes_->o_slvi_cr_ready(corei_cr_ready_i);
    serdes_->o_slvi_cd_ready(corei_cd_ready_i);
    serdes_->i_slvo_aw_valid(coreo_aw_valid_o);
    serdes_->i_slvo_aw_bits_addr(coreo_aw_bits_addr_o);
    serdes_->i_slvo_aw_bits_len(coreo_aw_bits_len_o);
    serdes_->i_slvo_aw_bits_size(coreo_aw_bits_size_o);
    serdes_->i_slvo_aw_bits_burst(coreo_aw_bits_burst_o);
    serdes_->i_slvo_aw_bits_lock(coreo_aw_bits_lock_o);
    serdes_->i_slvo_aw_bits_cache(coreo_aw_bits_cache_o);
    serdes_->i_slvo_aw_bits_prot(coreo_aw_bits_prot_o);
    serdes_->i_slvo_aw_bits_qos(coreo_aw_bits_qos_o);
    serdes_->i_slvo_aw_bits_region(coreo_aw_bits_region_o);
    serdes_->i_slvo_aw_id(coreo_aw_id_o);
    serdes_->i_slvo_aw_user(coreo_aw_user_o);
    serdes_->i_slvo_w_valid(coreo_w_valid_o);
    serdes_->i_slvo_w_data(coreo_w_data_o);
    serdes_->i_slvo_w_last(coreo_w_last_o);
    serdes_->i_slvo_w_strb(coreo_w_strb_o);
    serdes_->i_slvo_w_user(coreo_w_user_o);
    serdes_->i_slvo_b_ready(coreo_b_ready_o);
    serdes_->i_slvo_ar_valid(coreo_ar_valid_o);
    serdes_->i_slvo_ar_bits_addr(coreo_ar_bits_addr_o);
    serdes_->i_slvo_ar_bits_len(coreo_ar_bits_len_o);
    serdes_->i_slvo_ar_bits_size(coreo_ar_bits_size_o);
    serdes_->i_slvo_ar_bits_burst(coreo_ar_bits_burst_o);
    serdes_->i_slvo_ar_bits_lock(coreo_ar_bits_lock_o);
    serdes_->i_slvo_ar_bits_cache(coreo_ar_bits_cache_o);
    serdes_->i_slvo_ar_bits_prot(coreo_ar_bits_prot_o);
    serdes_->i_slvo_ar_bits_qos(coreo_ar_bits_qos_o);
    serdes_->i_slvo_ar_bits_region(coreo_ar_bits_region_o);
    serdes_->i_slvo_ar_id(coreo_ar_id_o);
    serdes_->i_slvo_ar_user(coreo_ar_user_o);
    serdes_->i_slvo_r_ready(coreo_r_ready_o);
    serdes_->i_slvo_ar_domain(coreo_ar_domain_o);
    serdes_->i_slvo_ar_snoop(coreo_ar_snoop_o);
    serdes_->i_slvo_ar_bar(coreo_ar_bar_o);
    serdes_->i_slvo_aw_domain(coreo_aw_domain_o);
    serdes_->i_slvo_aw_snoop(coreo_aw_snoop_o);
    serdes_->i_slvo_aw_bar(coreo_aw_bar_o);
    serdes_->i_slvo_ac_ready(coreo_ac_ready_o);
    serdes_->i_slvo_cr_valid(coreo_cr_valid_o);
    serdes_->i_slvo_cr_resp(coreo_cr_resp_o);
    serdes_->i_slvo_cd_valid(coreo_cd_valid_o);
    serdes_->i_slvo_cd_data(coreo_cd_data_o);
    serdes_->i_slvo_cd_last(coreo_cd_last_o);
    serdes_->i_slvo_rack(coreo_rack_o);
    serdes_->i_slvo_wack(coreo_wack_o);
    serdes_->i_msti_aw_ready(msti_aw_ready_i);
    serdes_->i_msti_w_ready(msti_w_ready_i);
    serdes_->i_msti_b_valid(msti_b_valid_i);
    serdes_->i_msti_b_resp(msti_b_resp_i);
    serdes_->i_msti_b_id(msti_b_id_i);
    serdes_->i_msti_b_user(msti_b_user_i);
    serdes_->i_msti_ar_ready(msti_ar_ready_i);
    serdes_->i_msti_r_valid(msti_r_valid_i);
    serdes_->i_msti_r_resp(msti_r_resp_i);
    serdes_->i_msti_r_data(msti_r_data_i);
    serdes_->i_msti_r_last(msti_r_last_i);
    serdes_->i_msti_r_id(msti_r_id_i);
    serdes_->i_msti_r_user(msti_r_user_i);
    serdes_->o_msto_aw_valid(msto_aw_valid_o);
    serdes_->o_msto_aw_bits_addr(msto_aw_bits_addr_o);
    serdes_->o_msto_aw_bits_len(msto_aw_bits_len_o);
    serdes_->o_msto_aw_bits_size(msto_aw_bits_size_o);
    serdes_->o_msto_aw_bits_burst(msto_aw_bits_burst_o);
    serdes_->o_msto_aw_bits_lock(msto_aw_bits_lock_o);
    serdes_->o_msto_aw_bits_cache(msto_aw_bits_cache_o);
    serdes_->o_msto_aw_bits_prot(msto_aw_bits_prot_o);
    serdes_->o_msto_aw_bits_qos(msto_aw_bits_qos_o);
    serdes_->o_msto_aw_bits_region(msto_aw_bits_region_o);
    serdes_->o_msto_aw_id(msto_aw_id_o);
    serdes_->o_msto_aw_user(msto_aw_user_o);
    serdes_->o_msto_w_valid(msto_w_valid_o);
    serdes_->o_msto_w_data(msto_w_data_o);
    serdes_->o_msto_w_last(msto_w_last_o);
    serdes_->o_msto_w_strb(msto_w_strb_o);
    serdes_->o_msto_w_user(msto_w_user_o);
    serdes_->o_msto_b_ready(msto_b_ready_o);
    serdes_->o_msto_ar_valid(msto_ar_valid_o);
    serdes_->o_msto_ar_bits_addr(msto_ar_bits_addr_o);
    serdes_->o_msto_ar_bits_len(msto_ar_bits_len_o);
    serdes_->o_msto_ar_bits_size(msto_ar_bits_size_o);
    serdes_->o_msto_ar_bits_burst(msto_ar_bits_burst_o);
    serdes_->o_msto_ar_bits_lock(msto_ar_bits_lock_o);
    serdes_->o_msto_ar_bits_cache(msto_ar_bits_cache_o);
    serdes_->o_msto_ar_bits_prot(msto_ar_bits_prot_o);
    serdes_->o_msto_ar_bits_qos(msto_ar_bits_qos_o);
    serdes_->o_msto_ar_bits_region(msto_ar_bits_region_o);
    serdes_->o_msto_ar_id(msto_ar_id_o);
    serdes_->o_msto_ar_user(msto_ar_user_o);
    serdes_->o_msto_r_ready(msto_r_ready_o);
#else
    serdes_ = 0;
#endif

    core_ = new RiverAmba("core0", hartid_.to_uint32(),
                               asyncReset_.to_bool(),
                               fpuEnable_.to_bool(),
                               tracerEnable_.to_bool());
    core_->i_clk(wrapper_->o_clk);
    core_->i_nrst(w_nrst);
    core_->i_msti_aw_ready(corei_aw_ready_i);
    core_->i_msti_w_ready(corei_w_ready_i);
    core_->i_msti_b_valid(corei_b_valid_i);
    core_->i_msti_b_resp(corei_b_resp_i);
    core_->i_msti_b_id(corei_b_id_i);
    core_->i_msti_b_user(corei_b_user_i);
    core_->i_msti_ar_ready(corei_ar_ready_i);
    core_->i_msti_r_valid(corei_r_valid_i);
    core_->i_msti_r_resp(corei_r_resp_i);
    core_->i_msti_r_data(corei_r_data_i);
    core_->i_msti_r_last(corei_r_last_i);
    core_->i_msti_r_id(corei_r_id_i);
    core_->i_msti_r_user(corei_r_user_i);
    core_->o_msto_aw_valid(coreo_aw_valid_o);
    core_->o_msto_aw_bits_addr(coreo_aw_bits_addr_o);
    core_->o_msto_aw_bits_len(coreo_aw_bits_len_o);
    core_->o_msto_aw_bits_size(coreo_aw_bits_size_o);
    core_->o_msto_aw_bits_burst(coreo_aw_bits_burst_o);
    core_->o_msto_aw_bits_lock(coreo_aw_bits_lock_o);
    core_->o_msto_aw_bits_cache(coreo_aw_bits_cache_o);
    core_->o_msto_aw_bits_prot(coreo_aw_bits_prot_o);
    core_->o_msto_aw_bits_qos(coreo_aw_bits_qos_o);
    core_->o_msto_aw_bits_region(coreo_aw_bits_region_o);
    core_->o_msto_aw_id(coreo_aw_id_o);
    core_->o_msto_aw_user(coreo_aw_user_o);
    core_->o_msto_w_valid(coreo_w_valid_o);
    core_->o_msto_w_data(coreo_w_data_o);
    core_->o_msto_w_last(coreo_w_last_o);
    core_->o_msto_w_strb(coreo_w_strb_o);
    core_->o_msto_w_user(coreo_w_user_o);
    core_->o_msto_b_ready(coreo_b_ready_o);
    core_->o_msto_ar_valid(coreo_ar_valid_o);
    core_->o_msto_ar_bits_addr(coreo_ar_bits_addr_o);
    core_->o_msto_ar_bits_len(coreo_ar_bits_len_o);
    core_->o_msto_ar_bits_size(coreo_ar_bits_size_o);
    core_->o_msto_ar_bits_burst(coreo_ar_bits_burst_o);
    core_->o_msto_ar_bits_lock(coreo_ar_bits_lock_o);
    core_->o_msto_ar_bits_cache(coreo_ar_bits_cache_o);
    core_->o_msto_ar_bits_prot(coreo_ar_bits_prot_o);
    core_->o_msto_ar_bits_qos(coreo_ar_bits_qos_o);
    core_->o_msto_ar_bits_region(coreo_ar_bits_region_o);
    core_->o_msto_ar_id(coreo_ar_id_o);
    core_->o_msto_ar_user(coreo_ar_user_o);
    core_->o_msto_r_ready(coreo_r_ready_o);
    core_->i_msti_ac_valid(corei_ac_valid_i);
    core_->i_msti_ac_addr(corei_ac_addr_i);
    core_->i_msti_ac_snoop(corei_ac_snoop_i);
    core_->i_msti_ac_prot(corei_ac_prot_i);
    core_->i_msti_cr_ready(corei_cr_ready_i);
    core_->i_msti_cd_ready(corei_cd_ready_i);
    core_->o_msto_ar_domain(coreo_ar_domain_o);
    core_->o_msto_ar_snoop(coreo_ar_snoop_o);
    core_->o_msto_ar_bar(coreo_ar_bar_o);
    core_->o_msto_aw_domain(coreo_aw_domain_o);
    core_->o_msto_aw_snoop(coreo_aw_snoop_o);
    core_->o_msto_aw_bar(coreo_aw_bar_o);
    core_->o_msto_ac_ready(coreo_ac_ready_o);
    core_->o_msto_cr_valid(coreo_cr_valid_o);
    core_->o_msto_cr_resp(coreo_cr_resp_o);
    core_->o_msto_cd_valid(coreo_cd_valid_o);
    core_->o_msto_cd_data(coreo_cd_data_o);
    core_->o_msto_cd_last(coreo_cd_last_o);
    core_->o_msto_rack(coreo_rack_o);
    core_->o_msto_wack(coreo_wack_o);
    core_->i_ext_irq(w_interrupt);
    core_->o_time(wb_time);
    core_->o_exec_cnt(wb_exec_cnt);
    core_->i_dport_valid(w_dport_valid);
    core_->i_dport_write(w_dport_write);
    core_->i_dport_region(wb_dport_region);
    core_->i_dport_addr(wb_dport_addr);
    core_->i_dport_wdata(wb_dport_wdata);
    core_->o_dport_ready(w_dport_ready);
    core_->o_dport_rdata(wb_dport_rdata);
    core_->o_halted(w_halted);

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
    delete core_;
    if (serdes_) {
        delete serdes_;
    }
}

void CpuRiscV_RTL::hapTriggered(IFace *isrc, EHapType type,
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

