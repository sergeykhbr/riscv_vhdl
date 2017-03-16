/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack trace buffer on hardware level.
 */

#ifndef __DEBUGGER_RIVERLIB_STACKTRBUF_H__
#define __DEBUGGER_RIVERLIB_STACKTRBUF_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(StackTraceBuffer) {
    sc_in<bool> i_clk;
    sc_in<sc_uint<5>> i_raddr;                   // todo: log2(CFG_STACK_TRACE_BUF_SIZE)
    sc_out<sc_biguint<2*BUS_ADDR_WIDTH>> o_rdata;
    sc_in<bool> i_we;
    sc_in<sc_uint<5>> i_waddr;                   // todo: log2(CFG_STACK_TRACE_BUF_SIZE)
    sc_in<sc_biguint<2*BUS_ADDR_WIDTH>> i_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(StackTraceBuffer);

    StackTraceBuffer(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    sc_signal<sc_uint<5>> raddr;
    sc_signal<sc_biguint<2*BUS_ADDR_WIDTH>> stackbuf[CFG_STACK_TRACE_BUF_SIZE]; // [pc, npc]
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_STACKTRBUF_H__
