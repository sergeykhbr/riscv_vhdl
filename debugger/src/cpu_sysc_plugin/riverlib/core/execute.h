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
#include "int_div.h"
#include "int_mul.h"

namespace debugger {

SC_MODULE(InstrExecute) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cache_hold;
    sc_in<bool> i_d_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_d_pc;
    sc_in<sc_uint<32>> i_d_instr;
    sc_in<bool> i_wb_done;                      // write back done (Used to clear hazardness)
    sc_in<bool> i_memop_store;                  // Store to memory operation
    sc_in<bool> i_memop_load;                   // Load from memoru operation
    sc_in<bool> i_memop_sign_ext;               // Load memory value with sign extending
    sc_in<sc_uint<2>> i_memop_size;             // Memory transaction size
    sc_in<bool> i_unsigned_op;                  // Unsigned operands
    sc_in<bool> i_rv32;                         // 32-bits instruction
    sc_in<sc_bv<ISA_Total>> i_isa_type;         // Type of the instruction's structure (ISA spec.)
    sc_in<sc_bv<Instr_Total>> i_ivec;           // One pulse per supported instruction.
    sc_in<bool> i_ie;                           // Interrupt enable bit
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_mtvec;     // Interrupt descriptor table
    sc_in<sc_uint<2>> i_mode;                   // Current processor mode
    sc_in<bool> i_unsup_exception;              // Unsupported instruction exception
    sc_in<bool> i_ext_irq;                      // External interrupt from PLIC (todo: timer & software interrupts)

    sc_out<sc_uint<5>> o_radr1;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;
    sc_out<sc_uint<5>> o_radr2;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;
    sc_out<sc_uint<5>> o_res_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_res_data;     // 
    sc_out<bool> o_pipeline_hold;               // Hold pipeline while 'writeback' not done or multi-clock instruction.
    sc_out<sc_uint<12>> o_csr_addr;             // CSR address. 0 if not a CSR instruction
    sc_out<bool> o_csr_wena;                    // Write new CSR value
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;     // CSR current value
    sc_out<sc_uint<RISCV_ARCH>> o_csr_wdata;    // CSR new value

    sc_out<bool> o_memop_sign_ext;              // Load data with sign extending
    sc_out<bool> o_memop_load;                  // Load data instruction
    sc_out<bool> o_memop_store;                 // Store data instruction
    sc_out<sc_uint<2>> o_memop_size;            // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_memop_addr;// Memory access address

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_npc;
    sc_out<sc_uint<32>> o_instr;


    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name_, sc_trace_file *vcd=0);
    virtual ~InstrExecute();

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> npc;
        sc_signal<sc_uint<32>> instr;
        sc_uint<5> res_addr;
        sc_uint<RISCV_ARCH> res_val;
        bool memop_load;
        bool memop_store;
        bool memop_sign_ext;
        sc_uint<2> memop_size;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> memop_addr;

        sc_signal<sc_uint<6>> multiclock_cnt;           // Instruction execution longer than 1 clock
        sc_signal<sc_uint<5>> hazard_addr[2];
        sc_signal<sc_uint<2>> hazard_depth;
    } v, r;
    sc_signal<bool> w_hazard_detected;
    sc_signal<bool> w_multiclock_instr;

    IntMul *mul0;
    sc_signal<bool> w_mul_ena;
    sc_signal<sc_uint<RISCV_ARCH>> wb_mul_res;

    IntDiv *div0;
    sc_signal<bool> w_div_ena;
    sc_signal<sc_uint<RISCV_ARCH>> wb_div_result;
    sc_signal<sc_uint<RISCV_ARCH>> wb_div_residual;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
