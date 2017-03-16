/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack trace buffer on hardware level.
 */

#include "stacktrbuf.h"

namespace debugger {

StackTraceBuffer::StackTraceBuffer(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_raddr;
    sensitive << i_we;
    sensitive << i_waddr;
    sensitive << i_wdata;
    sensitive << raddr;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void StackTraceBuffer::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
}

void StackTraceBuffer::comb() {
    o_rdata = stackbuf[raddr.read()];
}

void StackTraceBuffer::registers() {
    if (i_we.read()) {
        stackbuf[i_waddr.read()] = i_wdata;
    }
    raddr = i_raddr.read();
}

}  // namespace debugger

