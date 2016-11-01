/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Multi-port CPU Integer Registers memory.
 */

#ifndef __DEBUGGER_RIVERLIB_REGIBANK_H__
#define __DEBUGGER_RIVERLIB_REGIBANK_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

enum ERegNames {
    Reg_Zero,
    Reg_ra,// = 1;       // [1] Return address
    Reg_sp,// = 2;       // [2] Stack pointer
    Reg_gp,// = 3;       // [3] Global pointer
    Reg_tp,// = 4;       // [4] Thread pointer
    Reg_t0,// = 5;       // [5] Temporaries 0 s3
    Reg_t1,// = 6;       // [6] Temporaries 1 s4
    Reg_t2,// = 7;       // [7] Temporaries 2 s5
    Reg_s0,// = 8;       // [8] s0/fp Saved register/frame pointer
    Reg_s1,// = 9;       // [9] Saved register 1
    Reg_a0,// = 10;       // [10] Function argumentes 0
    Reg_a1,// = 11;       // [11] Function argumentes 1
    Reg_a2,// = 12;       // [12] Function argumentes 2
    Reg_a3,// = 13;       // [13] Function argumentes 3
    Reg_a4,// = 14;       // [14] Function argumentes 4
    Reg_a5,// = 15;       // [15] Function argumentes 5
    Reg_a6,// = 16;       // [16] Function argumentes 6
    Reg_a7,// = 17;       // [17] Function argumentes 7
    Reg_s2,// = 18;       // [18] Saved register 2
    Reg_s3,// = 19;       // [19] Saved register 3
    Reg_s4,// = 20;       // [20] Saved register 4
    Reg_s5,// = 21;       // [21] Saved register 5
    Reg_s6,// = 22;       // [22] Saved register 6
    Reg_s7,// = 23;       // [23] Saved register 7
    Reg_s8,// = 24;       // [24] Saved register 8
    Reg_s9,// = 25;       // [25] Saved register 9
    Reg_s10,// = 26;      // [26] Saved register 10
    Reg_s11,// = 27;      // [27] Saved register 11
    Reg_t3,// = 28;       // [28] 
    Reg_t4,// = 29;       // [29] 
    Reg_t5,// = 30;       // [30] 
    Reg_t6,// = 31;      // [31] 
    Reg_Total
};

static const char *const IREGS_NAMES[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

SC_MODULE(RegIntBank) {
    sc_in<bool> i_clk;                      // Clock
    sc_in<bool> i_nrst;                     // Reset. Active LOW
    sc_in<sc_uint<5>> i_radr1;              // Port 1 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata1;   // Port 1 read value

    sc_in<sc_uint<5>> i_radr2;              // Port 2 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata2;   // Port 2 read value

    sc_in<sc_uint<5>> i_waddr;              // Writing value
    sc_in<bool> i_wena;                     // Writing is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;     // Writing value

    sc_out<sc_uint<RISCV_ARCH>> o_ra;       // Return address for branch predictor

    void comb();
    void registers();

    SC_HAS_PROCESS(RegIntBank);

    RegIntBank(sc_module_name name_, sc_trace_file *vcd=0);

private:
    friend struct Processor; // for debug purposes(remove it)s

    struct RegistersType {
        sc_signal<bool> update;             // To generate SystemC delta event only.
        sc_uint<RISCV_ARCH> mem[Reg_Total]; // Multi-ports memory
    } v, r;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_REGIBANK_H__
