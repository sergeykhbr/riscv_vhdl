/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU pipeline implementation.
 */

#ifndef __DEBUGGER_RIVERLIB_PROC_H__
#define __DEBUGGER_RIVERLIB_PROC_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(Processor) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_out<bool> o_req_ctrl_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_ctrl_addr;
    sc_in<bool> i_resp_ctrl_ready;
    sc_in<sc_uint<32>> i_resp_ctrl_data;
    // Data path:
    sc_out<bool> o_req_data_valid;
    sc_out<bool> o_req_data_write;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_data_addr;
    sc_out<sc_uint<2>> o_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<RISCV_ARCH>> o_req_data_data;
    sc_in<bool> i_resp_data_ready;
    sc_in<sc_uint<RISCV_ARCH>> i_resp_data_data;


    void proc0();
    void registers();

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct FetchType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<bool> pc_valid;
    };
    struct InstructionDecodeType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> instr_valid;
    };
    struct ExecuteType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
    };
    struct MemoryType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
    };
    struct WriteBackType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
    };

    struct RegistersType {
        FetchType f;
        InstructionDecodeType i;
        ExecuteType e;
        MemoryType m;
        WriteBackType w;
    } v, r, rin;
    sc_signal<sc_uint<3>> dbgCnt_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_PROC_H__
