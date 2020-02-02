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
    // Memory interface:
    // AXI4 input structure:
    sc_out<bool> o_msti_aw_ready;
    sc_out<bool> o_msti_w_ready;
    sc_out<bool> o_msti_b_valid;
    sc_out<sc_uint<2>> o_msti_b_resp;
    sc_out<sc_uint<CFG_ID_BITS>> o_msti_b_id;
    sc_out<bool> o_msti_b_user;
    sc_out<bool> o_msti_ar_ready;
    sc_out<bool> o_msti_r_valid;
    sc_out<sc_uint<2>> o_msti_r_resp;                    // 0=OKAY;1=EXOKAY;2=SLVERR;3=DECER
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_msti_r_data;
    sc_out<bool> o_msti_r_last;
    sc_out<sc_uint<CFG_ID_BITS>> o_msti_r_id;
    sc_out<bool> o_msti_r_user;
    // AXI4 output structure:
    sc_in<bool> i_msto_aw_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_msto_aw_bits_addr;
    sc_in<sc_uint<8>> i_msto_aw_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_msto_aw_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_msto_aw_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_msto_aw_bits_lock;
    sc_in<sc_uint<4>> i_msto_aw_bits_cache;
    sc_in<sc_uint<3>> i_msto_aw_bits_prot;
    sc_in<sc_uint<4>> i_msto_aw_bits_qos;
    sc_in<sc_uint<4>> i_msto_aw_bits_region;
    sc_in<sc_uint<CFG_ID_BITS>> i_msto_aw_id;
    sc_in<bool> i_msto_aw_user;
    sc_in<bool> i_msto_w_valid;
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_msto_w_data;
    sc_in<bool> i_msto_w_last;
    sc_in<sc_uint<BUS_DATA_BYTES>> i_msto_w_strb;
    sc_in<bool> i_msto_w_user;
    sc_in<bool> i_msto_b_ready;
    sc_in<bool> i_msto_ar_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_msto_ar_bits_addr;
    sc_in<sc_uint<8>> i_msto_ar_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_msto_ar_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_msto_ar_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_msto_ar_bits_lock;
    sc_in<sc_uint<4>> i_msto_ar_bits_cache;
    sc_in<sc_uint<3>> i_msto_ar_bits_prot;
    sc_in<sc_uint<4>> i_msto_ar_bits_qos;
    sc_in<sc_uint<4>> i_msto_ar_bits_region;
    sc_in<sc_uint<CFG_ID_BITS>> i_msto_ar_id;
    sc_in<bool> i_msto_ar_user;
    sc_in<bool> i_msto_r_ready;
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
        State_Busy,
        State_Reset,
    };

    struct RegistersType {
        // AXI4 Request 
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<sc_uint<2>> req_burst;
        sc_signal<bool> req_write;
        // AXI4 B-Channel
        sc_signal<bool> b_valid;
        sc_signal<sc_uint<2>> b_resp;
        //
        sc_signal<sc_bv<5>> nrst;
        sc_signal<bool> interrupt;
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> halted;
        sc_signal<bool> r_error;
        sc_signal<bool> w_error;
    } r, v;

    sc_event bus_event_;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_resp_data;
    //sc_signal<bool> w_resp_store_fault;
    //sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_resp_store_fault_addr;
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
