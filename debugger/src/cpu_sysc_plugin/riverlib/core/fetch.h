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
    sc_in<bool> i_pipeline_hold;
    sc_in<bool> i_mem_req_ready;
    sc_out<bool> o_mem_addr_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mem_addr;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_mem_data_addr;
    sc_in<sc_uint<32>> i_mem_data;
    sc_out<bool> o_mem_resp_ready;

    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_npc;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_predict_npc;
    sc_out<bool> o_predict_miss;

    sc_out<bool> o_mem_req_fire;                    // used by branch predictor to form new npc value
    sc_out<bool> o_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<32>> o_instr;
    sc_out<bool> o_hold;                                // Hold due no response from icache yet
    sc_in<bool> i_br_fetch_valid;                       // Fetch injection address/instr are valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_br_address_fetch;  // Fetch injection address to skip ebreak instruciton only once
    sc_in<sc_uint<32>> i_br_instr_fetch;                // Real instruction value that was replaced by ebreak
    sc_out<sc_biguint<DBG_FETCH_TRACE_SIZE*64>> o_instr_buf;               // todo: remove it

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrFetch);

    InstrFetch(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<bool> wait_resp;
        sc_signal<sc_uint<5>> pipeline_init;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc_z1;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> raddr_not_resp_yet;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> br_address;
        sc_signal<sc_uint<32>> br_instr;
        sc_signal<sc_biguint<DBG_FETCH_TRACE_SIZE*64>> instr_buf;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_FETCH_H__
