/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Branch predictor.
 */

#ifndef __DEBUGGER_RIVERLIB_BR_PREDIC_H__
#define __DEBUGGER_RIVERLIB_BR_PREDIC_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(BranchPredictor) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_hold;
    sc_in<bool> i_f_mem_request;
    sc_in<bool> i_f_predic_miss;
    sc_in<bool> i_f_instr_valid;
    sc_in<sc_uint<32>> i_f_instr;
    sc_in<sc_uint<32>> i_e_npc;
    sc_in<sc_uint<RISCV_ARCH>> i_ra;   // Return address

    sc_out<sc_uint<32>> o_npc_predict;

    void comb();
    void registers();

    SC_HAS_PROCESS(BranchPredictor);

    BranchPredictor(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> npc;
    } v, r;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_npc;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_BR_PREDIC_H__
