/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Execution stage.
 */

#ifndef __DEBUGGER_RIVERLIB_EXECUTE_H__
#define __DEBUGGER_RIVERLIB_EXECUTE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrExecute) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_d_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_d_pc;
    sc_in<sc_uint<32>> i_d_instr;
    sc_in<bool> i_sign_ext;
    sc_in<sc_bv<ISA_Total>> i_isa_type;
    sc_in<sc_bv<Instr_Total>> i_ivec;
    sc_in<bool> i_user_level;
    sc_in<bool> i_priv_level;
    sc_in<bool> i_exception;

    sc_out<sc_uint<5>> o_radr1;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;
    sc_out<sc_uint<5>> o_radr2;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;
    sc_out<sc_uint<5>> o_res_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_res_data;

    sc_out<bool> o_memop_load;
    sc_out<bool> o_memop_store;
    sc_out<sc_uint<2>> o_memop_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_npc;
    sc_out<sc_uint<32>> o_instr;


    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> npc;
        sc_signal<sc_uint<32>> instr;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
