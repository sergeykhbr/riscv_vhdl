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

#pragma once

#include "async_tqueue.h"
#include "api_core.h"
#include "coreservices/imemop.h"
#include "coreservices/iirq.h"
#include "coreservices/ireset.h"
#include "coreservices/iclock.h"
#include "coreservices/icpuriscv.h"
#include "ambalib/types_amba.h"
#include "riverlib/river_cfg.h"
#include "riverlib/types_river.h"
#include <systemc.h>

namespace debugger {

class RtlWrapper : public sc_module,
                   public IResetListener,
                   public ICpuRiscV {
 public:
    sc_clock o_clk;
    sc_out<bool> o_sys_nrst;
    sc_out<bool> o_dmi_nrst;
    // Timer:
    sc_out<axi4_master_in_type> o_msti;
    sc_in<axi4_master_out_type> i_msto;
    // Interrupt lines:
    sc_out<sc_uint<64>> o_mtimer;
    sc_out<sc_uint<CFG_CPU_MAX>> o_msip;
    sc_out<sc_uint<CFG_CPU_MAX>> o_mtip;
    sc_out<sc_uint<CFG_CPU_MAX>> o_meip;
    sc_out<sc_uint<CFG_CPU_MAX>> o_seip;
    sc_in<bool> i_ndmreset;

    enum EState {
        State_Idle,
        State_Read,
        State_Write,
        State_Reset,
    };

    struct RegistersType {
        sc_signal<sc_uint<64>> clk_cnt;
        // AXI4 Request 
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<sc_uint<2>> req_burst;
        // AXI4 B-Channel
        sc_signal<bool> b_valid;
        sc_signal<sc_uint<2>> b_resp;
        //
        sc_signal<sc_bv<5>> nrst;
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> r_error;
        sc_signal<bool> w_error;
    } r, v;

    sc_event bus_req_event_;
    sc_event bus_resp_event_;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_wstrb;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_resp_data;
    sc_signal<bool> w_r_error;
    sc_signal<bool> w_w_error;

    sc_signal<bool> w_interrupt;

    int w_msip;    // machine software interrupt pending
    int w_mtip;    // machine timer interrupt pending
    int w_meip;    // machine external interrupt pending
    int w_seip;    // supervisor external interrupt pending

    Axi4TransactionType trans;
    ETransStatus resp;

    bool request_reset;
    bool async_interrupt;

    void clk_gen();
    void comb();
    void registers();
    void sys_bus_proc();
    void dbg_bus_proc();

    SC_HAS_PROCESS(RtlWrapper);

    RtlWrapper(IFace *parent, sc_module_name name);
    virtual ~RtlWrapper();

 public:
    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);
    void setCLINT(IIrqController *v) { iirqloc_ = v; }
    void setPLIC(IIrqController *v) { iirqext_ = v; }
    void setBus(IMemoryOperation *v) { ibus_ = v; }
    /** Default time resolution 1 picosecond. */
    void setClockHz(double hz);
   
    /** ICpuGeneric interface */
    virtual bool isHalt();

    /** ICpuRiscV interface */
    virtual uint64_t readCSR(uint32_t regno) { return 0; }
    virtual void writeCSR(uint32_t regno, uint64_t val) {}
    virtual uint64_t readGPR(uint32_t regno) { return 0; }
    virtual void writeGPR(uint32_t regno, uint64_t val) {}
    virtual uint64_t readNonStandardReg(uint32_t regno) { return 0; }
    virtual void writeNonStandardReg(uint32_t regno, uint64_t val) {}
    virtual void mmuAddrReserve(uint64_t addr) { }
    virtual bool mmuAddrRelease(uint64_t addr) { return true; }

    /** IClock */
    virtual uint64_t getClockCounter() { return r.clk_cnt.read(); }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);

    /** IResetListener */
    virtual void reset(IFace *isource) {
        request_reset = true;
    }


 private:
    IFace *getInterface(const char *name) { return iparent_; }
    uint64_t mask2offset(uint8_t mask);
    uint32_t mask2size(uint8_t mask);       // nask with removed offset

 private:
    IIrqController *iirqloc_;
    IIrqController *iirqext_;
    IMemoryOperation *ibus_;
    IFace *iparent_;    // pointer on parent module object (used for logging)
    int clockCycles_;   // default in [ps]
    ClockAsyncTQueueType step_queue_;

    sc_uint<32> t_trans_idx_up;
    sc_uint<32> t_trans_idx_down;
};

}  // namespace debugger

