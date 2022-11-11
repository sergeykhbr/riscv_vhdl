// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(DecoderRvc) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_flush_pipeline;                           // reset pipeline and cache
    sc_in<bool> i_progbuf_ena;                              // executing from progbuf
    sc_in<sc_uint<RISCV_ARCH>> i_f_pc;                      // Fetched pc
    sc_in<sc_uint<32>> i_f_instr;                           // Fetched instruction value
    sc_in<bool> i_instr_load_fault;                         // fault instruction's address
    sc_in<bool> i_instr_page_fault_x;                       // IMMU page fault signal
    sc_out<sc_uint<6>> o_radr1;                             // register bank address 1 (rs1)
    sc_out<sc_uint<6>> o_radr2;                             // register bank address 2 (rs2)
    sc_out<sc_uint<6>> o_waddr;                             // register bank output (rd)
    sc_out<sc_uint<12>> o_csr_addr;                         // CSR bank output
    sc_out<sc_uint<RISCV_ARCH>> o_imm;                      // immediate constant decoded from instruction
    sc_out<sc_uint<RISCV_ARCH>> o_pc;                       // Current instruction pointer value
    sc_out<sc_uint<32>> o_instr;                            // Current instruction value
    sc_out<bool> o_memop_store;                             // Store to memory operation
    sc_out<bool> o_memop_load;                              // Load from memoru operation
    sc_out<bool> o_memop_sign_ext;                          // Load memory value with sign extending
    sc_out<sc_uint<2>> o_memop_size;                        // Memory transaction size
    sc_out<bool> o_rv32;                                    // 32-bits instruction
    sc_out<bool> o_compressed;                              // C-type instruction
    sc_out<bool> o_amo;                                     // A-type instruction
    sc_out<bool> o_f64;                                     // 64-bits FPU (D-extension)
    sc_out<bool> o_unsigned_op;                             // Unsigned operands
    sc_out<sc_uint<ISA_Total>> o_isa_type;                  // Instruction format accordingly with ISA
    sc_out<sc_biguint<Instr_Total>> o_instr_vec;            // One bit per decoded instruction bus
    sc_out<bool> o_exception;                               // Exception detected
    sc_out<bool> o_instr_load_fault;                        // fault instruction's address
    sc_out<bool> o_instr_page_fault_x;                      // IMMU page fault signal
    sc_out<bool> o_progbuf_ena;                             // Debug execution from progbuf

    void comb();
    void registers();

    SC_HAS_PROCESS(DecoderRvc);

    DecoderRvc(sc_module_name name,
               bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t OPCODE_C_ADDI4SPN = 0x00;
    static const uint8_t OPCODE_C_NOP_ADDI = 0x01;
    static const uint8_t OPCODE_C_SLLI = 0x02;
    static const uint8_t OPCODE_C_JAL_ADDIW = 0x05;
    static const uint8_t OPCODE_C_LW = 0x08;
    static const uint8_t OPCODE_C_LI = 0x09;
    static const uint8_t OPCODE_C_LWSP = 0x0A;
    static const uint8_t OPCODE_C_LD = 0x0C;
    static const uint8_t OPCODE_C_ADDI16SP_LUI = 0xD;
    static const uint8_t OPCODE_C_LDSP = 0x0E;
    static const uint8_t OPCODE_C_MATH = 0x11;
    static const uint8_t OPCODE_C_JR_MV_EBREAK_JALR_ADD = 0x12;
    static const uint8_t OPCODE_C_J = 0x15;
    static const uint8_t OPCODE_C_SW = 0x18;
    static const uint8_t OPCODE_C_BEQZ = 0x19;
    static const uint8_t OPCODE_C_SWSP = 0x1A;
    static const uint8_t OPCODE_C_SD = 0x1C;
    static const uint8_t OPCODE_C_BNEZ = 0x1D;
    static const uint8_t OPCODE_C_SDSP = 0x1E;

    struct DecoderRvc_registers {
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<ISA_Total>> isa_type;
        sc_signal<sc_biguint<Instr_Total>> instr_vec;
        sc_signal<sc_uint<16>> instr;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_page_fault_x;
        sc_signal<bool> instr_unimplemented;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> imm;
        sc_signal<bool> progbuf_ena;
    } v, r;

    void DecoderRvc_r_reset(DecoderRvc_registers &iv) {
        iv.pc = ~0ull;
        iv.isa_type = 0;
        iv.instr_vec = 0ull;
        iv.instr = ~0ul;
        iv.memop_store = 0;
        iv.memop_load = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = MEMOP_1B;
        iv.rv32 = 0;
        iv.instr_load_fault = 0;
        iv.instr_page_fault_x = 0;
        iv.instr_unimplemented = 0;
        iv.radr1 = 0;
        iv.radr2 = 0;
        iv.waddr = 0;
        iv.imm = 0ull;
        iv.progbuf_ena = 0;
    }

};

}  // namespace debugger

