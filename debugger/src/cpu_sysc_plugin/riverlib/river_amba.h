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

#ifndef __DEBUGGER_RIVER_AMBA_H__
#define __DEBUGGER_RIVER_AMBA_H__

#include "river_cfg.h"
#include "types_river.h"
#include "river_top.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(RiverAmba) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    sc_in<axi4_l1_in_type> i_msti;
    sc_out<axi4_l1_out_type> o_msto;
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<sc_uint<1>> i_msip;
    sc_in<sc_uint<1>> i_mtip;
    sc_in<sc_uint<1>> i_meip;
    sc_in<sc_uint<1>> i_seip;
    // Debug interface
    sc_in<bool> i_haltreq;                              // DMI: halt request from debug unit
    sc_in<bool> i_resumereq;                            // DMI: resume request from debug unit
    sc_in<bool> i_dport_req_valid;                      // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_type;        // Debug access type
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_dport_addr;     // Debug address (register or memory)
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_in<sc_uint<3>> i_dport_size;                     // reg/mem access size:0=1B;...,4=128B;
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                     // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                    // Response is valid
    sc_out<bool> o_dport_resp_error;                    // Something wrong during command execution
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value (data or error code)
    sc_in<sc_biguint<32*CFG_PROGBUF_REG_TOTAL>> i_progbuf;  // progam buffer
    sc_out<bool> o_halted;                              // CPU halted via debug interface

    void comb();
    void snoopcomb();
    void registers();

    SC_HAS_PROCESS(RiverAmba);

    RiverAmba(sc_module_name name_,
             uint32_t hartid,
             bool async_reset,
             bool fpu_ena,
             bool coherence_ena,
             bool tracer_ena);
    virtual ~RiverAmba();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    RiverTop *river0;

    sc_uint<4> reqtype2arsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype) {
        sc_uint<4> ret = 0;
        if (reqtype[REQ_MEM_TYPE_CACHED] == 0) {
            ret = ARSNOOP_READ_NO_SNOOP;
        } else {
            if (reqtype[REQ_MEM_TYPE_UNIQUE] == 0) {
                ret = ARSNOOP_READ_SHARED;
            } else {
                ret = ARSNOOP_READ_MAKE_UNIQUE;
            }
        }
        return ret;
    }

    sc_uint<3> reqtype2awsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype) {
        sc_uint<3> ret = 0;
        if (reqtype[REQ_MEM_TYPE_CACHED] == 0) {
            ret = AWSNOOP_WRITE_NO_SNOOP;
        } else {
            if (reqtype[REQ_MEM_TYPE_UNIQUE] == 0) {
                ret = AWSNOOP_WRITE_BACK;
            } else {
                ret = AWSNOOP_WRITE_LINE_UNIQUE;
            }
        }
        return ret;
    }

    sc_signal<bool> req_mem_ready_i;
    sc_signal<bool> req_mem_path_o;
    sc_signal<bool> req_mem_valid_o;
    sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type_o;
    sc_signal<sc_uint<3>> req_mem_size_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr_o;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_data_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_mem_data_i;
    sc_signal<bool> resp_mem_valid_i;
    sc_signal<bool> resp_mem_load_fault_i;
    sc_signal<bool> resp_mem_store_fault_i;
    // D$ Snoop interface
    sc_signal<bool> req_snoop_valid_i;
    sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type_i;
    sc_signal<bool> req_snoop_ready_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_snoop_addr_i;
    sc_signal<bool> resp_snoop_ready_i;
    sc_signal<bool> resp_snoop_valid_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data_o;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> resp_snoop_flags_o;

    // Signals from snoopcomb to comb process
    sc_signal<bool> w_ac_ready;
    sc_signal<bool> w_cr_valid;
    sc_signal<sc_uint<5>> wb_cr_resp;
    sc_signal<bool> w_cd_valid;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> wb_cd_data;
    sc_signal<bool> w_cd_last;
    sc_signal<bool> w_rack;
    sc_signal<bool> w_wack;


    enum state_type {
        state_idle,
        state_ar,
        state_r,
        state_aw,
        state_w,
        state_b
    };

    enum snooptate_type {
        snoop_idle,
        snoop_ac_wait_accept,
        snoop_cr,
        snoop_cr_wait_accept,
        snoop_cd,
        snoop_cd_wait_accept,
    };

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<bool> req_path;
        sc_signal<sc_uint<4>> req_cached;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<4>> req_ar_snoop;
        sc_signal<sc_uint<3>> req_aw_snoop;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = state_idle;
        iv.req_addr = 0;
        iv.req_path = 0;
        iv.req_cached = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_ar_snoop = 0;
        iv.req_aw_snoop = 0;
    }

    struct SnoopRegistersType {
        sc_signal<sc_uint<3>> snoop_state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> ac_addr;
        sc_signal<sc_uint<4>> ac_snoop;                  // Table C3-19
        sc_signal<sc_uint<5>> cr_resp;
        sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data;
        sc_signal<bool> cache_access;
    } sv, sr;

    void SR_RESET(SnoopRegistersType &iv) {
        iv.snoop_state = snoop_idle;
        iv.ac_addr = 0;
        iv.ac_snoop = 0;
        iv.cr_resp = 0;
        iv.req_snoop_type = 0;
        iv.resp_snoop_data = 0;
        iv.cache_access = 0;
    }

    bool async_reset_;
    bool coherence_ena_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_AMBA_H__
