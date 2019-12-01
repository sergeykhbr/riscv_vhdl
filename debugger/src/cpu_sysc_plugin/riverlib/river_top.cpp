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

#include "river_top.h"

namespace debugger {

RiverTop::RiverTop(sc_module_name name_, uint32_t hartid, bool async_reset,
    bool fpu_ena, bool tracer_ena) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    i_resp_mem_data_valid("i_resp_mem_data_valid"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    i_resp_mem_store_fault_addr("i_resp_mem_store_fault_addr"),
    i_ext_irq("i_ext_irq"),
    o_time("o_time"),
    o_exec_cnt("o_exec_cnt"),
    i_dport_valid("i_dport_valid"),
    i_dport_write("i_dport_write"),
    i_dport_region("i_dport_region"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_ready("o_dport_ready"),
    o_dport_rdata("o_dport_rdata"),
    o_halted("o_halted") {

    proc0 = new Processor("proc0", hartid, async_reset, tracer_ena);
    proc0->i_clk(i_clk);
    proc0->i_nrst(i_nrst);
    proc0->i_req_ctrl_ready(w_req_ctrl_ready);
    proc0->o_req_ctrl_valid(w_req_ctrl_valid);
    proc0->o_req_ctrl_addr(wb_req_ctrl_addr);
    proc0->i_resp_ctrl_valid(w_resp_ctrl_valid);
    proc0->i_resp_ctrl_addr(wb_resp_ctrl_addr);
    proc0->i_resp_ctrl_data(wb_resp_ctrl_data);
    proc0->i_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    proc0->o_resp_ctrl_ready(w_resp_ctrl_ready);
    proc0->i_req_data_ready(w_req_data_ready);
    proc0->o_req_data_valid(w_req_data_valid);
    proc0->o_req_data_write(w_req_data_write);
    proc0->o_req_data_addr(wb_req_data_addr);
    proc0->o_req_data_size(wb_req_data_size);
    proc0->o_req_data_data(wb_req_data_data);
    proc0->i_resp_data_valid(w_resp_data_valid);
    proc0->i_resp_data_addr(wb_resp_data_addr);
    proc0->i_resp_data_data(wb_resp_data_data);
    proc0->i_resp_data_load_fault(w_resp_data_load_fault);
    proc0->i_resp_data_store_fault(w_resp_data_store_fault);
    proc0->i_resp_data_store_fault_addr(wb_resp_data_store_fault_addr);
    proc0->o_resp_data_ready(w_resp_data_ready);
    proc0->i_ext_irq(i_ext_irq);
    proc0->o_time(o_time);
    proc0->o_exec_cnt(o_exec_cnt);
    proc0->i_dport_valid(i_dport_valid);
    proc0->i_dport_write(i_dport_write);
    proc0->i_dport_region(i_dport_region);
    proc0->i_dport_addr(i_dport_addr);
    proc0->i_dport_wdata(i_dport_wdata);
    proc0->o_dport_ready(o_dport_ready);
    proc0->o_dport_rdata(o_dport_rdata);
    proc0->o_halted(o_halted);
    proc0->o_flush_address(wb_flush_address);
    proc0->o_flush_valid(w_flush_valid);
    proc0->i_istate(wb_istate);
    proc0->i_dstate(wb_dstate);
    proc0->i_cstate(wb_cstate);

    cache0 = new CacheTop("cache0", async_reset);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_ctrl_valid(w_req_ctrl_valid);
    cache0->i_req_ctrl_addr(wb_req_ctrl_addr);
    cache0->o_req_ctrl_ready(w_req_ctrl_ready);
    cache0->o_resp_ctrl_valid(w_resp_ctrl_valid);
    cache0->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    cache0->o_resp_ctrl_data(wb_resp_ctrl_data);
    cache0->o_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    cache0->i_resp_ctrl_ready(w_resp_ctrl_ready);
    cache0->i_req_data_valid(w_req_data_valid);
    cache0->i_req_data_write(w_req_data_write);
    cache0->i_req_data_addr(wb_req_data_addr);
    cache0->i_req_data_size(wb_req_data_size);
    cache0->i_req_data_data(wb_req_data_data);
    cache0->o_req_data_ready(w_req_data_ready);
    cache0->o_resp_data_valid(w_resp_data_valid);
    cache0->o_resp_data_addr(wb_resp_data_addr);
    cache0->o_resp_data_data(wb_resp_data_data);
    cache0->o_resp_data_load_fault(w_resp_data_load_fault);
    cache0->o_resp_data_store_fault(w_resp_data_store_fault);
    cache0->o_resp_data_store_fault_addr(wb_resp_data_store_fault_addr);
    cache0->i_resp_data_ready(w_resp_data_ready);
    cache0->i_req_mem_ready(i_req_mem_ready);
    cache0->o_req_mem_valid(o_req_mem_valid);
    cache0->o_req_mem_write(o_req_mem_write);
    cache0->o_req_mem_addr(o_req_mem_addr);
    cache0->o_req_mem_strob(o_req_mem_strob);
    cache0->o_req_mem_data(o_req_mem_data);
    cache0->o_req_mem_len(o_req_mem_len);
    cache0->o_req_mem_burst(o_req_mem_burst);
    cache0->i_resp_mem_data_valid(i_resp_mem_data_valid);
    cache0->i_resp_mem_data(i_resp_mem_data);
    cache0->i_resp_mem_load_fault(i_resp_mem_load_fault);
    cache0->i_resp_mem_store_fault(i_resp_mem_store_fault);
    cache0->i_resp_mem_store_fault_addr(i_resp_mem_store_fault_addr);
    cache0->i_flush_address(wb_flush_address);
    cache0->i_flush_valid(w_flush_valid);
    cache0->o_istate(wb_istate);
    cache0->o_dstate(wb_dstate);
    cache0->o_cstate(wb_cstate);
};

RiverTop::~RiverTop() {
    delete cache0;
    delete proc0;
}

void RiverTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    /**
     * ModelSim commands for automatic comparision Stimulus vs SystemC reference:
     *
     * Convert VCD to WLF and back to VCD because ModelSim supports only 
     * 1-bit signals, such conversion allows to create compatible VCD-file.
     * 
     * 1. Prepare compatible VCD/wlf files:
     *      vcd2wlf E:/../win32build/Debug/i_river.vcd -o e:/i_river.wlf
     *      vcd2wlf E:/../win32build/Debug/o_river.vcd -o e:/o_river.wlf
     *      wlf2vcd e:/i_river.wlf -o e:/i_river.vcd
     *
     * 2. Add waves to simulation view and simulate 350 us:
     *      vsim -t 1ps -vcdstim E:/i_river.vcd riverlib.RiverTop
     *      vsim -view e:/o_river.wlf
     *      add wave o_river:/SystemC/ *
     *      add wave sim:/rivertop/ *
     *      run 350us
     *
     * 3. Start automatic comparision:
     *      compare start o_river sim
     *      compare add -wave sim:/RiverTop/o_req_mem_valid o_river:/SystemC/o_req_mem_valid
     *      compare add -wave sim:/RiverTop/o_req_mem_write o_river:/SystemC/o_req_mem_write
     *      compare add -wave sim:/RiverTop/o_req_mem_addr o_river:/SystemC/o_req_mem_addr
     *      compare add -wave sim:/RiverTop/o_req_mem_strob o_river:/SystemC/o_req_mem_strob
     *      compare add -wave sim:/RiverTop/o_req_mem_data o_river:/SystemC/o_req_mem_data
     *      compare add -wave sim:/RiverTop/o_step_cnt o_river:/SystemC/o_step_cnt
     *      compare run
     *
     */
    if (i_vcd) {
        sc_trace(i_vcd, i_clk, "i_clk");
        sc_trace(i_vcd, i_nrst, "i_nrst");
        sc_trace(i_vcd, i_req_mem_ready, "i_req_mem_ready");
        sc_trace(i_vcd, i_resp_mem_data_valid, "i_resp_mem_data_valid");
        sc_trace(i_vcd, i_resp_mem_data, "i_resp_mem_data");
        sc_trace(i_vcd, i_resp_mem_load_fault, "i_resp_mem_load_fault");
        sc_trace(i_vcd, i_resp_mem_store_fault, "i_resp_mem_store_fault");
        sc_trace(i_vcd, i_ext_irq, "i_ext_irq");
        sc_trace(i_vcd, i_dport_valid, "i_dport_valid");
        sc_trace(i_vcd, i_dport_write, "i_dport_write");
        sc_trace(i_vcd, i_dport_region, "i_dport_region");
        sc_trace(i_vcd, i_dport_addr, "i_dport_addr");
        sc_trace(i_vcd, i_dport_wdata, "i_dport_wdata");

    }
    if (o_vcd) {
        sc_trace(o_vcd, o_req_mem_valid, "o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_write, "o_req_mem_write");
        sc_trace(o_vcd, o_req_mem_addr, "o_req_mem_addr");
        sc_trace(o_vcd, o_req_mem_strob, "o_req_mem_strob");
        sc_trace(o_vcd, o_req_mem_data, "o_req_mem_data");
        sc_trace(o_vcd, o_time, "o_time");
        sc_trace(o_vcd, o_halted, "o_halted");
        sc_trace(o_vcd, o_dport_ready, "o_dport_ready");
        sc_trace(o_vcd, o_dport_rdata, "o_dport_rdata");
    }

    proc0->generateVCD(i_vcd, o_vcd);
    cache0->generateVCD(i_vcd, o_vcd);
}

}  // namespace debugger

