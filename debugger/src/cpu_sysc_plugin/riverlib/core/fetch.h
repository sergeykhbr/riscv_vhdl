/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Fetch Instruction stage.
 */

#ifndef __DEBUGGER_RIVERLIB_FETCH_H__
#define __DEBUGGER_RIVERLIB_FETCH_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrFetch) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cache_hold;
    sc_in<bool> i_pipeline_hold;
    sc_out<bool> o_mem_addr_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mem_addr;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_mem_data_addr;
    sc_in<sc_uint<32>> i_mem_data;

    sc_in<bool> i_e_npc_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_npc;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_predict_npc;
    sc_out<bool> o_predict_miss;

    sc_out<bool> o_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<32>> o_instr;


    void comb();
    void registers();

    SC_HAS_PROCESS(InstrFetch);

    InstrFetch(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> f_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc_z0;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc_z1;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> raddr_not_resp_yet;
        sc_signal<sc_uint<32>> instr;
        //bool predict_miss;

        sc_signal<bool> wait_resp;
        sc_signal<bool> is_postponed;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> postponed_pc;
        sc_signal<sc_uint<32>> postponed_instr;
    } v, r;

    sc_signal<bool> w_mem_addr_valid;
    sc_signal<bool> w_wrong_address;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_addr_req;
    bool w_any_hold;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_FETCH_H__
