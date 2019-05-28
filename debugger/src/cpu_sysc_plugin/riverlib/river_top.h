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
    sc_out<bool> o_req_mem_valid;                       // AXI memory request is valid
    sc_out<bool> o_req_mem_write;                       // AXI memory request is write type
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_mem_addr;     // AXI memory request address
    sc_out<sc_uint<BUS_DATA_BYTES>> o_req_mem_strob;    // Writing strob. 1 bit per Byte
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_req_mem_data;     // Writing data
    sc_in<bool> i_resp_mem_data_valid;                  // AXI response is valid
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_resp_mem_data;     // Read data
    sc_in<bool> i_resp_mem_load_fault;
    sc_in<bool> i_resp_mem_store_fault;
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<bool> i_ext_irq;
    sc_out<sc_uint<64>> o_time;                         // Clock/Step counter depending attribute "GenerateRef"
    // Debug interface
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value

    RiverTop(sc_module_name name_, uint32_t hartid);
    virtual ~RiverTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);
    void generateRef(bool v) { proc0->generateRef(v); }
private:

    Processor *proc0;
    CacheTop *cache0;

    // Control path:
    sc_signal<bool> w_req_ctrl_ready;
    sc_signal<bool> w_req_ctrl_valid;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_req_ctrl_addr;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<32>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_ready;
    // Data path:
    sc_signal<bool> w_req_data_ready;
    sc_signal<bool> w_req_data_valid;
    sc_signal<bool> w_req_data_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_req_data_addr;
    sc_signal<sc_uint<2>> wb_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_signal<sc_uint<RISCV_ARCH>> wb_req_data_data;
    sc_signal<bool> w_resp_data_valid;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_resp_data_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_resp_data_data;
    sc_signal<bool> w_resp_data_load_fault;
    sc_signal<bool> w_resp_data_store_fault;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_data_ready;
    sc_signal<sc_uint<2>> wb_istate;
    sc_signal<sc_uint<2>> wb_dstate;
    sc_signal<sc_uint<2>> wb_cstate;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVER_TOP_H__
