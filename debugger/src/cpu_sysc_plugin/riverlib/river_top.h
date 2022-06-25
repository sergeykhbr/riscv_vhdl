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

#ifndef __DEBUGGER_RIVER_TOP_H__
#define __DEBUGGER_RIVER_TOP_H__

#include <systemc.h>
#include "river_cfg.h"
#include "core/proc.h"
#include "cache/cache_top.h"

namespace debugger {

SC_MODULE(RiverTop) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    // Memory interface:
    sc_in<bool> i_req_mem_ready;                        // System Bus is ready to accept memory operation request
    sc_out<bool> o_req_mem_path;                        // 0=ctrl; 1=data path
    sc_out<bool> o_req_mem_valid;                       // AXI memory request is valid
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;   // AXI memory request type
    sc_out<sc_uint<3>> o_req_mem_size;                  // request size: 0=1 B;...; 7=128 B
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;     // AXI memory request address
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_mem_strob;  // Writing strob. 1 bit per Byte (uncached only)
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_mem_data;     // Writing data
    sc_in<bool> i_resp_mem_valid;                       // AXI response is valid
    sc_in<bool> i_resp_mem_path;                        // 0=ctrl; 1=data path
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_mem_data;     // Read data
    sc_in<bool> i_resp_mem_load_fault;
    sc_in<bool> i_resp_mem_store_fault;
    // D$ Snoop interface
    sc_in<bool> i_req_snoop_valid;
    sc_in<sc_uint<SNOOP_REQ_TYPE_BITS>> i_req_snoop_type;
    sc_out<bool> o_req_snoop_ready;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<sc_uint<1>> i_msip;                           // machine software pening interrupt
    sc_in<sc_uint<1>> i_mtip;                           // machine timer pening interrupt
    sc_in<sc_uint<1>> i_meip;                           // machine external pening interrupt
    sc_in<sc_uint<1>> i_seip;                           // supervisor external pening interrupt
    // Debug interface
    sc_in<bool> i_haltreq;                              // DMI: halt request from debug unit
    sc_in<bool> i_resumereq;                            // DMI: resume request from debug unit
    sc_in<bool> i_dport_req_valid;                      // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_type;        // Debug access type
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_dport_addr;     // dport address
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_in<sc_uint<3>> i_dport_size;                     // reg/mem access size:0=1B;...,4=128B;
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                     // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                    // Response is valid
    sc_out<bool> o_dport_resp_error;                    // Something wrong during command execution
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    sc_in<sc_biguint<32*CFG_PROGBUF_REG_TOTAL>> i_progbuf;  // progam buffer
    sc_out<bool> o_halted;                              // CPU halted via debug interface

    RiverTop(sc_module_name name_,
             bool async_reset,
             uint32_t hartid,
             bool fpu_ena,
             bool coherence_ena,
             bool tracer_ena);
    virtual ~RiverTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);
 private:

    Processor *proc0;
    CacheTop *cache0;

    // Control path:
    sc_signal<bool> w_req_ctrl_ready;
    sc_signal<bool> w_req_ctrl_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_ctrl_addr;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<64>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_ctrl_executable;
    sc_signal<bool> w_resp_ctrl_ready;
    // Data path:
    sc_signal<bool> w_req_data_ready;
    sc_signal<bool> w_req_data_valid;
    sc_signal<sc_uint<MemopType_Total>> wb_req_data_type;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_data_addr;
    sc_signal<sc_uint<64>> wb_req_data_wdata;
    sc_signal<sc_uint<8>> wb_req_data_wstrb;
    sc_signal<sc_uint<2>> wb_req_data_size;
    sc_signal<bool> w_resp_data_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_data_addr;
    sc_signal<sc_uint<64>> wb_resp_data_data;
    sc_signal<bool> w_resp_data_load_fault;
    sc_signal<bool> w_resp_data_store_fault;
    sc_signal<bool> w_resp_data_er_mpu_load;
    sc_signal<bool> w_resp_data_er_mpu_store;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_data_fault_addr;
    sc_signal<bool> w_resp_data_ready;
    sc_signal<bool> w_mpu_region_we;
    sc_signal<sc_uint<CFG_MPU_TBL_WIDTH>> wb_mpu_region_idx;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_mpu_region_addr;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_mpu_region_mask;
    sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> wb_mpu_region_flags;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_flush_address;
    sc_signal<bool> w_flush_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_data_flush_address;
    sc_signal<bool> w_data_flush_valid;
    sc_signal<bool> w_data_flush_end;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_TOP_H__
