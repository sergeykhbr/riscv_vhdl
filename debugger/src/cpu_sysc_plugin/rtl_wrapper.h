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

#ifndef __DEBUGGER_RTL_WRAPPER_H__
#define __DEBUGGER_RTL_WRAPPER_H__

#include "async_tqueue.h"
#include "api_core.h"
#include "coreservices/imemop.h"
#include "coreservices/ireset.h"
#include "coreservices/icpugen.h"
#include "coreservices/iclock.h"
#include "coreservices/icpuriscv.h"
#include "ambalib/types_amba.h"
#include "riverlib/river_cfg.h"
#include <systemc.h>

namespace debugger {

class RtlWrapper : public sc_module,
                   public IResetListener,
                   public ICpuGeneric,
                   public ICpuRiscV {
 public:
    sc_clock o_clk;
    sc_out<bool> o_nrst;
    // Timer:
    sc_in<sc_uint<RISCV_ARCH>> i_time;
    sc_out<axi4_master_in_type> o_msti;
    sc_in<axi4_master_out_type> i_msto;
    /** Interrupt line from external interrupts controller. */
    sc_out<bool> o_interrupt;
    // Debug interface
    sc_out<bool> o_dport_valid;                          // Debug access from DSU is valid
    sc_out<bool> o_dport_write;                          // Write value
    sc_out<sc_uint<2>> o_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_out<sc_uint<12>> o_dport_addr;                    // Register index
    sc_out<sc_uint<RISCV_ARCH>> o_dport_wdata;           // Write value
    sc_in<bool> i_dport_ready;                           // Response is ready
    sc_in<sc_uint<RISCV_ARCH>> i_dport_rdata;            // Response value
    sc_in<bool> i_halted;

    enum EState {
        State_Idle,
        State_Read,
        State_Write,
        State_Reset,
    };

    struct RegistersType {
        // AXI4 Request 
        sc_signal<sc_uint<CFG_BUS_ADDR_WIDTH>> req_addr;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<sc_uint<2>> req_burst;
        // AXI4 B-Channel
        sc_signal<bool> b_valid;
        sc_signal<sc_uint<2>> b_resp;
        //
        sc_signal<sc_bv<5>> nrst;
        sc_signal<bool> interrupt;
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> halted;
        sc_signal<bool> r_error;
        sc_signal<bool> w_error;
    } r, v;

    sc_event bus_event_;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_wdata;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_wstrb;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_data;
    sc_signal<bool> w_r_error;
    sc_signal<bool> w_w_error;

    sc_signal<bool> w_dport_valid;
    sc_signal<bool> w_dport_write;
    sc_signal<sc_uint<2>> wb_dport_region;
    sc_signal<sc_uint<12>> wb_dport_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_wdata;

    sc_signal<bool> w_interrupt;

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
    void setBus(IMemoryOperation *v) { ibus_ = v; }
    /** Default time resolution 1 picosecond. */
    void setClockHz(double hz);
   
    /** ICpuGeneric interface */
    virtual void raiseSignal(int idx);
    virtual void lowerSignal(int idx);
    virtual void nb_transport_debug_port(DebugPortTransactionType *trans,
                                        IDbgNbResponse *cb);

    /** ICpuRiscV interface */
    virtual uint64_t readCSR(int idx) { return 0;}
    void virtual writeCSR(int idx, uint64_t val) {}

    /** IClock */
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
    IMemoryOperation *ibus_;
    IFace *iparent_;    // pointer on parent module object (used for logging)
    int clockCycles_;   // default in [ps]
    ClockAsyncTQueueType step_queue_;

    sc_uint<32> t_trans_idx_up;
    sc_uint<32> t_trans_idx_down;

    struct DebugPortType {
        event_def valid;
        DebugPortTransactionType *trans;
        IDbgNbResponse *cb;
        unsigned trans_idx_up;
        unsigned trans_idx_down;
        unsigned idx_missmatch;
    } dport_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RTL_WRAPPER_H__
