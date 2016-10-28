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

SC_MODULE(RegIntBank) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<5>> i_radr1;
    sc_out<sc_uint<RISCV_ARCH>> o_rdata1;

    sc_in<sc_uint<5>> i_radr2;
    sc_out<sc_uint<RISCV_ARCH>> o_rdata2;

    sc_in<sc_uint<5>> i_waddr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;

    sc_out<sc_uint<RISCV_ARCH>> o_ra;   // Return address

    void comb();
    void registers();

    SC_HAS_PROCESS(RegIntBank);

    RegIntBank(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_uint<32> mem[Reg_Total];
    } v, r;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_REGIBANK_H__
