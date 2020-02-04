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
 *
 * @details    Use the following targets attributes to generate trace files:
 *             GenerateRef - Generate memory and registers write accesses
 *                           trace files to compare them with functional model
 *             InVcdFile   - Stimulus VCD file
 *             OutVcdFile  - Reference VCD file with any number of signals
 *
 * @note       When GenerateRef is true Core uses step counter instead 
 *             of clock counter to generate callbacks.
 */

#ifndef __DEBUGGER_CPU_RISCV_RTL_H__
#define __DEBUGGER_CPU_RISCV_RTL_H__

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/icmdexec.h"
#include "coreservices/itap.h"
#include "cmds/cmd_br_riscv.h"
#include "cmds/cmd_reg_riscv.h"
#include "cmds/cmd_regs_riscv.h"
#include "cmds/cmd_csr.h"
#include "rtl_wrapper.h"
#include "riverlib/river_amba.h"
#include <systemc.h>

namespace debugger {

class CpuRiscV_RTL : public IService, 
                 public IThread,
                 public IClock,
                 public IHap {
 public:
    CpuRiscV_RTL(const char *name);
    virtual ~CpuRiscV_RTL();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** IClock */
    virtual uint64_t getStepCounter() {
        return wb_time.read() + 2;
    }

    virtual uint64_t getExecCounter() override {
        return wb_exec_cnt.read();
    };


    virtual void registerStepCallback(IClockListener *cb, uint64_t t) {
        wrapper_->registerStepCallback(cb, t);
    }

    bool moveStepCallback(IClockListener *cb, uint64_t t) {
        registerStepCallback(cb, t);
        return false;
    }

    virtual double getFreqHz() {
        return static_cast<double>(freqHz_.to_uint64());
    }

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    virtual void stop();

 protected:
    /** IThread interface */
    virtual void busyLoop();

 private:
    void createSystemC();
    void deleteSystemC();

 private:
    AttributeType hartid_;
    AttributeType asyncReset_;
    AttributeType fpuEnable_;
    AttributeType tracerEnable_;
    AttributeType bus_;
    AttributeType cmdexec_;
    AttributeType tap_;
    AttributeType freqHz_;
    AttributeType InVcdFile_;
    AttributeType OutVcdFile_;
    event_def config_done_;

    ICmdExecutor *icmdexec_;
    ITap *itap_;
    IMemoryOperation *ibus_;

    sc_signal<bool> w_clk;
    sc_signal<bool> w_nrst;
    // Timer:
    sc_signal<sc_uint<64>> wb_time;
    sc_signal<sc_uint<64>> wb_exec_cnt;

    // AXI4 input structure:
    sc_signal<bool> msti_aw_ready_i;
    sc_signal<bool> msti_w_ready_i;
    sc_signal<bool> msti_b_valid_i;
    sc_signal<sc_uint<2>> msti_b_resp_i;
    sc_signal<sc_uint<CFG_ID_BITS>> msti_b_id_i;
    sc_signal<bool> msti_b_user_i;
    sc_signal<bool> msti_ar_ready_i;
    sc_signal<bool> msti_r_valid_i;
    sc_signal<sc_uint<4>> msti_r_resp_i;
    sc_signal<sc_biguint<DCACHE_LINE_BITS>> msti_r_data_i;
    sc_signal<bool> msti_r_last_i;
    sc_signal<sc_uint<CFG_ID_BITS>> msti_r_id_i;
    sc_signal<bool> msti_r_user_i;
    // AXI4 output structure:
    sc_signal<bool> msto_aw_valid_o;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> msto_aw_bits_addr_o;
    sc_signal<sc_uint<8>> msto_aw_bits_len_o;
    sc_signal<sc_uint<3>> msto_aw_bits_size_o;
    sc_signal<sc_uint<2>> msto_aw_bits_burst_o;
    sc_signal<bool> msto_aw_bits_lock_o;
    sc_signal<sc_uint<4>> msto_aw_bits_cache_o;
    sc_signal<sc_uint<3>> msto_aw_bits_prot_o;
    sc_signal<sc_uint<4>> msto_aw_bits_qos_o;
    sc_signal<sc_uint<4>> msto_aw_bits_region_o;
    sc_signal<sc_uint<CFG_ID_BITS>> msto_aw_id_o;
    sc_signal<bool> msto_aw_user_o;
    sc_signal<bool> msto_w_valid_o;
    sc_signal<sc_biguint<DCACHE_LINE_BITS>> msto_w_data_o;
    sc_signal<bool> msto_w_last_o;
    sc_signal<sc_uint<BUS_DATA_BYTES>> msto_w_strb_o;
    sc_signal<bool> msto_w_user_o;
    sc_signal<bool> msto_b_ready_o;
    sc_signal<bool> msto_ar_valid_o;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> msto_ar_bits_addr_o;
    sc_signal<sc_uint<8>> msto_ar_bits_len_o;
    sc_signal<sc_uint<3>> msto_ar_bits_size_o;
    sc_signal<sc_uint<2>> msto_ar_bits_burst_o;
    sc_signal<bool> msto_ar_bits_lock_o;
    sc_signal<sc_uint<4>> msto_ar_bits_cache_o;
    sc_signal<sc_uint<3>> msto_ar_bits_prot_o;
    sc_signal<sc_uint<4>> msto_ar_bits_qos_o;
    sc_signal<sc_uint<4>> msto_ar_bits_region_o;
    sc_signal<sc_uint<CFG_ID_BITS>> msto_ar_id_o;
    sc_signal<bool> msto_ar_user_o;
    sc_signal<bool> msto_r_ready_o;
    // ACE signals
    sc_signal<bool> msti_ac_valid_i;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> msti_ac_addr_i;
    sc_signal<sc_uint<4>> msti_ac_snoop_i;
    sc_signal<sc_uint<3>> msti_ac_prot_i;
    sc_signal<bool> msti_cr_ready_i;
    sc_signal<bool> msti_cd_ready_i;
    sc_signal<sc_uint<2>> msto_ar_domain_o;
    sc_signal<sc_uint<4>> msto_ar_snoop_o;
    sc_signal<sc_uint<2>> msto_ar_bar_o;
    sc_signal<sc_uint<2>> msto_aw_domain_o;
    sc_signal<sc_uint<4>> msto_aw_snoop_o;
    sc_signal<sc_uint<2>> msto_aw_bar_o;
    sc_signal<bool> msto_ac_ready_o;
    sc_signal<bool> msto_cr_valid_o;
    sc_signal<sc_uint<5>> msto_cr_resp_o;
    sc_signal<bool> msto_cd_valid_o;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> msto_cd_data_o;
    sc_signal<bool> msto_cd_last_o;
    sc_signal<bool> msto_rack_o;
    sc_signal<bool> msto_wack_o;
    /** Interrupt line from external interrupts controller. */
    sc_signal<bool> w_interrupt;
    // Debug interface
    sc_signal<bool> w_dport_valid;
    sc_signal<bool> w_dport_write;
    sc_signal<sc_uint<2>> wb_dport_region;
    sc_signal<sc_uint<12>> wb_dport_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_wdata;
    sc_signal<bool> w_dport_ready;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_rdata;
    sc_signal<bool> w_halted;

    sc_trace_file *i_vcd_;      // stimulus pattern
    sc_trace_file *o_vcd_;      // reference pattern for comparision
    RiverAmba *core_;
    RtlWrapper *wrapper_;

    CmdBrRiscv *pcmd_br_;
    CmdRegRiscv *pcmd_reg_;
    CmdRegsRiscv *pcmd_regs_;
    CmdCsr *pcmd_csr_;
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_RTL_H__
