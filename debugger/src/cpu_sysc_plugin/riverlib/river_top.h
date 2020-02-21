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
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<bool> i_ext_irq;
    sc_out<sc_uint<64>> o_time;                         // Clock/Step counter depending attribute "GenerateRef"
    sc_out<sc_uint<64>> o_exec_cnt;
    // Debug interface
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    sc_out<bool> o_halted;                              // CPU halted via debug interface

    RiverTop(sc_module_name name_,
             uint32_t hartid,
             bool async_reset,
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
    sc_signal<sc_uint<32>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_ctrl_executable;
    sc_signal<bool> w_resp_ctrl_ready;
    // Data path:
    sc_signal<bool> w_req_data_ready;
    sc_signal<bool> w_req_data_valid;
    sc_signal<bool> w_req_data_write;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_data_addr;
    sc_signal<sc_uint<64>> wb_req_data_wdata;
    sc_signal<sc_uint<8>> wb_req_data_wstrb;
    sc_signal<bool> w_resp_data_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_data_addr;
    sc_signal<sc_uint<64>> wb_resp_data_data;
    sc_signal<bool> w_resp_data_load_fault;
    sc_signal<bool> w_resp_data_store_fault;
    sc_signal<bool> w_resp_data_er_mpu_load;
    sc_signal<bool> w_resp_data_er_mpu_store;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_data_store_fault_addr;
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
    sc_signal<sc_uint<4>> wb_istate;
    sc_signal<sc_uint<4>> wb_dstate;
    sc_signal<sc_uint<2>> wb_cstate;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_TOP_H__
