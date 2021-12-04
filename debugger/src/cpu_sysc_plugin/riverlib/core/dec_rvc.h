/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

// Compressed instruction set
const uint8_t OPCODE_C_ADDI4SPN = 0x00;
const uint8_t OPCODE_C_NOP_ADDI = 0x01;
const uint8_t OPCODE_C_SLLI     = 0x02;
const uint8_t OPCODE_C_JAL_ADDIW = 0x05;
const uint8_t OPCODE_C_LW       = 0x08;
const uint8_t OPCODE_C_LI       = 0x09;
const uint8_t OPCODE_C_LWSP     = 0x0A;
const uint8_t OPCODE_C_LD       = 0x0C;
const uint8_t OPCODE_C_ADDI16SP_LUI = 0xD;
const uint8_t OPCODE_C_LDSP     = 0x0E;
const uint8_t OPCODE_C_MATH     = 0x11;
const uint8_t OPCODE_C_JR_MV_EBREAK_JALR_ADD = 0x12;
const uint8_t OPCODE_C_J        = 0x15;
const uint8_t OPCODE_C_SW       = 0x18;
const uint8_t OPCODE_C_BEQZ     = 0x19;
const uint8_t OPCODE_C_SWSP     = 0x1A;
const uint8_t OPCODE_C_SD       = 0x1C;
const uint8_t OPCODE_C_BNEZ     = 0x1D;
const uint8_t OPCODE_C_SDSP     = 0x1E;

SC_MODULE(DecoderRvc) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                         // Reset active low
    sc_in<bool> i_flush_pipeline;               // reset pipeline and cache
    sc_in<bool> i_progbuf_ena;                  // executing from progbuf
    sc_in<bool> i_f_valid;                      // Fetch input valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_f_pc;   // Fetched pc
    sc_in<sc_uint<32>> i_f_instr;               // Fetched instruction value
    sc_in<bool> i_instr_load_fault;             // fault instruction's address
    sc_in<bool> i_instr_executable;             // MPU flag

    sc_out<sc_uint<6>> o_radr1;                 // register bank address 1 (rs1)
    sc_out<sc_uint<6>> o_radr2;                 // register bank address 2 (rs2)
    sc_out<sc_uint<6>> o_waddr;                 // register bank output (rd)
    sc_out<sc_uint<12>> o_csr_addr;             // CSR bank output
    sc_out<sc_uint<RISCV_ARCH>> o_imm;          // immediate constant decoded from instruction
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_pc;       // Current instruction pointer value
    sc_out<sc_uint<32>> o_instr;                // Current instruction value zero extended to 32 bits
    sc_out<bool> o_memop_store;                 // Store to memory operation
    sc_out<bool> o_memop_load;                  // Load from memoru operation
    sc_out<bool> o_memop_sign_ext;              // Load memory value with sign extending
    sc_out<sc_uint<2>> o_memop_size;            // Memory transaction size
    sc_out<bool> o_rv32;                        // 32-bits instruction
    sc_out<bool> o_compressed;                  // C-type instruction
    sc_out<bool> o_amo;                         // A-type instruction
    sc_out<bool> o_f64;                         // 64-bits FPU (D-extension)
    sc_out<bool> o_unsigned_op;                 // Unsigned operands
    sc_out<sc_bv<ISA_Total>> o_isa_type;        // Instruction format accordingly with ISA
    sc_out<sc_bv<Instr_Total>> o_instr_vec;     // One bit per decoded instruction bus
    sc_out<bool> o_exception;
    sc_out<bool> o_instr_load_fault;            // fault instruction's address
    sc_out<bool> o_instr_executable;            // MPU flag
    sc_out<bool> o_progbuf_ena;

    void comb();
    void registers();

    SC_HAS_PROCESS(DecoderRvc);

    DecoderRvc(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_bv<ISA_Total> isa_type;
        sc_bv<Instr_Total> instr_vec;
        sc_signal<sc_uint<16>> instr;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;

        sc_signal<bool> instr_unimplemented;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> imm;
        sc_signal<bool> progbuf_ena;
    };

    RegistersType v;
    RegistersType r;

    void R_RESET(RegistersType &iv) {
        iv.pc = ~0ull;
        iv.instr = ~0ull;
        iv.isa_type = 0;
        iv.instr_vec = 0;
        iv.memop_store = 0;
        iv.memop_load = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = MEMOP_8B;
        iv.rv32 = 0;
        iv.instr_load_fault = 0;
        iv.instr_executable = 0;
        iv.instr_unimplemented = 0;
        iv.radr1 = 0;
        iv.radr2 = 0;
        iv.waddr = 0;
        iv.imm = 0;
        iv.progbuf_ena = 0;
    }

    bool async_reset_;
};

}  // namespace debugger
