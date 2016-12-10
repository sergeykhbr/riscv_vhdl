/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      "River" CPU Top level.
 */

#include "river_top.h"

namespace debugger {

RiverTop::RiverTop(sc_module_name name_, sc_trace_file *i_vcd,
                   sc_trace_file *o_vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << r.timer;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    proc0 = new Processor("proc0", o_vcd);
    proc0->i_clk(i_clk);
    proc0->i_nrst(i_nrst);
    proc0->i_req_ctrl_ready(w_req_ctrl_ready);
    proc0->o_req_ctrl_valid(w_req_ctrl_valid);
    proc0->o_req_ctrl_addr(wb_req_ctrl_addr);
    proc0->i_resp_ctrl_valid(w_resp_ctrl_valid);
    proc0->i_resp_ctrl_addr(wb_resp_ctrl_addr);
    proc0->i_resp_ctrl_data(wb_resp_ctrl_data);
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
    proc0->o_resp_data_ready(w_resp_data_ready);
    proc0->i_ext_irq(i_ext_irq);
    proc0->o_step_cnt(o_step_cnt);

    cache0 = new CacheTop("cache0", o_vcd);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_ctrl_valid(w_req_ctrl_valid);
    cache0->i_req_ctrl_addr(wb_req_ctrl_addr);
    cache0->o_req_ctrl_ready(w_req_ctrl_ready);
    cache0->o_resp_ctrl_valid(w_resp_ctrl_valid);
    cache0->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    cache0->o_resp_ctrl_data(wb_resp_ctrl_data);
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
    cache0->i_resp_data_ready(w_resp_data_ready);
    cache0->i_req_mem_ready(i_req_mem_ready);
    cache0->o_req_mem_valid(o_req_mem_valid);
    cache0->o_req_mem_write(o_req_mem_write);
    cache0->o_req_mem_addr(o_req_mem_addr);
    cache0->o_req_mem_strob(o_req_mem_strob);
    cache0->o_req_mem_data(o_req_mem_data);
    cache0->i_resp_mem_data_valid(i_resp_mem_data_valid);
    cache0->i_resp_mem_data(i_resp_mem_data);

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
        sc_trace(i_vcd, i_ext_irq, "i_ext_irq");
    }
    if (o_vcd) {
        sc_trace(o_vcd, o_req_mem_valid, "o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_write, "o_req_mem_write");
        sc_trace(o_vcd, o_req_mem_addr, "o_req_mem_addr");
        sc_trace(o_vcd, o_req_mem_strob, "o_req_mem_strob");
        sc_trace(o_vcd, o_req_mem_data, "o_req_mem_data");
        sc_trace(o_vcd, o_timer, "o_timer");
        sc_trace(o_vcd, o_step_cnt, "o_step_cnt");
    }
};

RiverTop::~RiverTop() {
    delete cache0;
    delete proc0;
}

void RiverTop::comb() {
    v.timer = r.timer.read() + 1;

    if (!i_nrst.read()) {
        v.timer = 0;
    }

    o_timer = r.timer;
}

void RiverTop::registers() {
    r = v;
}

}  // namespace debugger

