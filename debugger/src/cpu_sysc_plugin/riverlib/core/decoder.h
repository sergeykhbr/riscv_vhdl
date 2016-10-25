/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Decoder stage.
 */

#ifndef __DEBUGGER_RIVERLIB_DECODER_H__
#define __DEBUGGER_RIVERLIB_DECODER_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrDecoder) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_f_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_f_pc;
    sc_in<sc_bv<32>> i_instr;
    sc_out<bool> o_f_ready;

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;


    void comb();
    void registers();

    SC_HAS_PROCESS(InstrDecoder);

    InstrDecoder(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DECODER_H__
