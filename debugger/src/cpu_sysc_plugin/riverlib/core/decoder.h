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

const uint8_t OPCODE_LB     = 0x00; // 00100: LB, LH, LW, LD, LBU, LHU, LWU
const uint8_t OPCODE_FENCE  = 0x03; // 00011: FENCE, FENCE_I
const uint8_t OPCODE_ADDI   = 0x04; // 00100: ADDI, ANDI, ORI, SLLI, SLTI, SLTIU, SRAI, SRLI, XORI
const uint8_t OPCODE_AUIPC  = 0x05; // 00101: AUIPC
const uint8_t OPCODE_ADDIW  = 0x06; // 00110: ADDIW, SLLIW, SRAIW, SRLIW
const uint8_t OPCODE_SB     = 0x08; // 01000: SB, SH, SW, SD
const uint8_t OPCODE_ADD    = 0x0C; // 01100: ADD, AND, OR, SLT, SLTU, SLL, SRA, SRL, SUB, XOR
const uint8_t OPCODE_LUI    = 0x0D; // 01101: LUI
const uint8_t OPCODE_ADDW   = 0x0E; // 01110: ADDW, SLLW, SRAW, SRLW, SUBW
const uint8_t OPCODE_BEQ    = 0x18; // 11000: BEQ, BNE, BLT, BGE, BLTU, BGEU
const uint8_t OPCODE_JALR   = 0x19; // 11001: JALR
const uint8_t OPCODE_JAL    = 0x1B; // 11011: JAL
const uint8_t OPCODE_CSRR   = 0x1C; // 11100: CSRRC, CSRRCI, CSRRS, CSRRSI, CSRRW, CSRRWI, URET, SRET, HRET, MRET

SC_MODULE(InstrDecoder) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_f_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_f_pc;
    sc_in<sc_uint<32>> i_f_instr;

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<32>> o_instr;
    sc_out<bool> o_sign_ext;
    sc_out<sc_bv<ISA_Total>> o_isa_type;
    sc_out<sc_bv<Instr_Total>> o_instr_vec;
    sc_out<bool> o_user_level;
    sc_out<bool> o_priv_level;
    sc_out<bool> o_exception;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrDecoder);

    InstrDecoder(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_bv<ISA_Total> isa_type;
        sc_bv<Instr_Total> instr_vec;
        sc_signal<bool> user_level;
        sc_signal<bool> priv_level;
        sc_signal<bool> sign_ext;
        sc_signal<sc_uint<32>> instr;

        sc_signal<bool> instr_unimplemented;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DECODER_H__
