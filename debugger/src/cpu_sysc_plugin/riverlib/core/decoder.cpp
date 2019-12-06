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

#include "decoder.h"

namespace debugger {

InstrDecoder::InstrDecoder(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_any_hold("i_any_hold"),
    i_f_valid("i_f_valid"),
    i_f_pc("i_f_pc"),
    i_f_instr("i_f_instr"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_executable("i_instr_executable"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr"),
    o_memop_store("o_memop_store"),
    o_memop_load("o_memop_load"),
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_size("o_memop_size"),
    o_rv32("o_rv32"),
    o_compressed("o_compressed"),
    o_f64("o_f64"),
    o_unsigned_op("o_unsigned_op"),
    o_isa_type("o_isa_type"),
    o_instr_vec("o_instr_vec"),
    o_exception("o_exception"),
    o_instr_load_fault("o_instr_load_fault"),
    o_instr_executable("o_instr_executable") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_any_hold;
    sensitive << i_f_valid;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_executable;
    sensitive << r.valid;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.memop_load;
    sensitive << r.memop_store;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.unsigned_op;
    sensitive << r.rv32;
    sensitive << r.f64;
    sensitive << r.compressed;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;
    sensitive << r.instr_unimplemented;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void InstrDecoder::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_any_hold, i_any_hold.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_isa_type, o_isa_type.name());
        sc_trace(o_vcd, o_instr_vec, o_instr_vec.name());
        sc_trace(o_vcd, o_exception, o_exception.name());
        sc_trace(o_vcd, o_compressed, o_compressed.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
    }
}

void InstrDecoder::comb() {
    v = r;

    bool w_o_valid;
    bool w_error = false;
    bool w_compressed = false;
    sc_uint<32> wb_instr = i_f_instr.read();
    sc_uint<32> wb_instr_out;
    sc_uint<5> wb_opcode1;
    sc_uint<3> wb_opcode2;
    sc_bv<Instr_Total> wb_dec = 0;
    sc_bv<ISA_Total> wb_isa_type = 0;

    if (wb_instr(1, 0) != 0x3) {
        w_compressed = 1;
        wb_opcode1 = (wb_instr(15, 13), wb_instr(1, 0));
        wb_instr_out = 0x00000003;
        switch (wb_opcode1) {
        case OPCODE_C_ADDI4SPN:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ADDI] = 1;
            wb_instr_out(11, 7) = 0x8 | wb_instr(4, 2);     // rd
            wb_instr_out(19, 15) = 0x2;                     // rs1 = sp
            wb_instr_out(29, 22) =
                (wb_instr(10, 7), wb_instr(12, 11), wb_instr[5], wb_instr[6]);
            break;
        case OPCODE_C_NOP_ADDI:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ADDI] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);      // rd
            wb_instr_out(19, 15) = wb_instr(11, 7);     // rs1
            wb_instr_out(24, 20) = wb_instr(6, 2);      // imm
            if (wb_instr[12]) {
                wb_instr_out(31, 25) = ~0;
            }
            break;
        case OPCODE_C_SLLI:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_SLLI] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);      // rd
            wb_instr_out(19, 15) = wb_instr(11, 7);     // rs1
            wb_instr_out(25, 20) = (wb_instr[12], wb_instr(6, 2));  // shamt
            break;
        case OPCODE_C_JAL_ADDIW:
            // JAL is the RV32C only instruction
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ADDIW] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);      // rd
            wb_instr_out(19, 15) = wb_instr(11, 7);     // rs1
            wb_instr_out(24, 20) = wb_instr(6, 2);      // imm
            if (wb_instr[12]) {
                wb_instr_out(31, 25) = ~0;
            }
            break;
        case OPCODE_C_LW:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_LW] = 1;
            wb_instr_out(11, 7) = 0x8 | wb_instr(4, 2);     // rd
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(26, 22) =
                (wb_instr[5], wb_instr(12, 10), wb_instr[6]);
            break;
        case OPCODE_C_LI:  // ADDI rd = r0 + imm
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ADDI] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);      // rd
            wb_instr_out(24, 20) = wb_instr(6, 2);      // imm
            if (wb_instr[12]) {
                wb_instr_out(31, 25) = ~0;
            }
            break;
        case OPCODE_C_LWSP:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_LW] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);      // rd
            wb_instr_out(19, 15) = 0x2;                 // rs1 = sp
            wb_instr_out(27, 22) =
                (wb_instr(3, 2), wb_instr[12], wb_instr(6, 4));
            break;
        case OPCODE_C_LD:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_LD] = 1;
            wb_instr_out(11, 7) = 0x8 | wb_instr(4, 2);     // rd
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(27, 23) =
                (wb_instr[6], wb_instr[5], wb_instr(12, 10));
            break;
        case OPCODE_C_ADDI16SP_LUI:
            if (wb_instr(11, 7) == 0x2) {
                wb_isa_type[ISA_I_type] = 1;
                wb_dec[Instr_ADDI] = 1;
                wb_instr_out(11, 7) = 0x2;     // rd = sp
                wb_instr_out(19, 15) = 0x2;    // rs1 = sp
                wb_instr_out(28, 24) =
                    (wb_instr(4, 3), wb_instr[5], wb_instr[2], wb_instr[6]);
                if (wb_instr[12]) {
                    wb_instr_out(31, 29) = ~0;
                }
            } else {
                wb_isa_type[ISA_U_type] = 1;
                wb_dec[Instr_LUI] = 1;
                wb_instr_out(11, 7) = wb_instr(11, 7);  // rd
                wb_instr_out(16, 12) = wb_instr(6, 2);
                if (wb_instr[12]) {
                    wb_instr_out(31, 17) = ~0;
                }
            }
            break;
        case OPCODE_C_LDSP:
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_LD] = 1;
            wb_instr_out(11, 7) = wb_instr(11, 7);  // rd
            wb_instr_out(19, 15) = 0x2;             // rs1 = sp
            wb_instr_out(28, 23) =
                (wb_instr(4, 2), wb_instr[12], wb_instr(6, 5));
            break;
        case OPCODE_C_MATH:
            if (wb_instr(11, 10) == 0) {
                wb_isa_type[ISA_I_type] = 1;
                wb_dec[Instr_SRLI] = 1;
                wb_instr_out(11, 7) = 0x8 | wb_instr(9, 7);   // rd
                wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);  // rs1
                wb_instr_out(25, 20) = (wb_instr[12], wb_instr(6, 2));  // shamt
            } else if (wb_instr(11, 10) == 1) {
                wb_isa_type[ISA_I_type] = 1;
                wb_dec[Instr_SRAI] = 1;
                wb_instr_out(11, 7) = 0x8 | wb_instr(9, 7);   // rd
                wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);  // rs1
                wb_instr_out(25, 20) = (wb_instr[12], wb_instr(6, 2));  // shamt
            } else if (wb_instr(11, 10) == 2) {
                wb_isa_type[ISA_I_type] = 1;
                wb_dec[Instr_ANDI] = 1;
                wb_instr_out(11, 7) = 0x8 | wb_instr(9, 7);   // rd
                wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);  // rs1
                wb_instr_out(24, 20) = wb_instr(6, 2);        // imm
                if (wb_instr[12]) {
                    wb_instr_out(31, 25) = ~0;
                }
            } else if (wb_instr[12] == 0) {
                wb_isa_type[ISA_R_type] = 1;
                wb_instr_out(11, 7) = 0x8 | wb_instr(9, 7);   // rd
                wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);  // rs1
                wb_instr_out(24, 20) = 0x8 | wb_instr(4, 2);  // rs2
                switch (wb_instr(6, 5)) {
                case 0:
                    wb_dec[Instr_SUB] = 1;
                    break;
                case 1:
                    wb_dec[Instr_XOR] = 1;
                    break;
                case 2:
                    wb_dec[Instr_OR] = 1;
                    break;
                default:
                    wb_dec[Instr_AND] = 1;
                }
            } else {
                wb_isa_type[ISA_R_type] = 1;
                wb_instr_out(11, 7) = 0x8 | wb_instr(9, 7);   // rd
                wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);  // rs1
                wb_instr_out(24, 20) = 0x8 | wb_instr(4, 2);  // rs2
                switch (wb_instr(6, 5)) {
                case 0:
                    wb_dec[Instr_SUBW] = 1;
                    break;
                case 1:
                    wb_dec[Instr_ADDW] = 1;
                    break;
                default:
                    w_error = true;
                }
            }
            break;
        case OPCODE_C_JR_MV_EBREAK_JALR_ADD:
            wb_isa_type[ISA_I_type] = 1;
            if (wb_instr[12] == 0) {
                if (wb_instr(6, 2) == 0) {
                    wb_dec[Instr_JALR] = 1;
                    wb_instr_out(19, 15) = wb_instr(11, 7);  // rs1
                } else {
                    wb_dec[Instr_ADDI] = 1;
                    wb_instr_out(11, 7) = wb_instr(11, 7);   // rd
                    wb_instr_out(19, 15) = wb_instr(6, 2);   // rs1
                }
            } else {
                if (wb_instr(11, 7) == 0 && wb_instr(6, 2) == 0) {
                    wb_dec[Instr_EBREAK] = 1;
                } else if (wb_instr(6, 2) == 0) {
                    wb_dec[Instr_JALR] = 1;
                    wb_instr_out(11, 7) = 0x1;               // rd = ra
                    wb_instr_out(19, 15) = wb_instr(11, 7);  // rs1
                } else {
                    wb_dec[Instr_ADD] = 1;
                    wb_isa_type[ISA_R_type] = 1;
                    wb_instr_out(11, 7) = wb_instr(11, 7);   // rd
                    wb_instr_out(19, 15) = wb_instr(11, 7);  // rs1
                    wb_instr_out(24, 20) = wb_instr(6, 2);   // rs2
                }
            }
            break;
        case OPCODE_C_J:   // JAL with rd = 0
            wb_isa_type[ISA_UJ_type] = 1;
            wb_dec[Instr_JAL] = 1;
            wb_instr_out[20] = wb_instr[12];            // imm11
            wb_instr_out(23, 21) = wb_instr(5, 3);      // imm10_1(3:1)
            wb_instr_out[24] = wb_instr[11];            // imm10_1(4)
            wb_instr_out[25] = wb_instr[2];             // imm10_1(5)
            wb_instr_out[26] = wb_instr[7];             // imm10_1(6)
            wb_instr_out[27] = wb_instr[6];             // imm10_1(7)
            wb_instr_out(29, 28) = wb_instr(10, 9);     // imm10_1(9:8)
            wb_instr_out[30] = wb_instr[8];             // imm10_1(10)
            if (wb_instr[12]) {
                wb_instr_out(19, 12) = ~0;              // imm19_12
                wb_instr_out[31] = 1;                   // imm20
            }
            break;
        case OPCODE_C_SW:
            wb_isa_type[ISA_S_type] = 1;
            wb_dec[Instr_SW] = 1;
            wb_instr_out(24, 20) = 0x8 | wb_instr(4, 2);    // rs2
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(11, 9) = (wb_instr(11, 10), wb_instr[6]);
            wb_instr_out(26, 25) = (wb_instr[5] , wb_instr[12]);
            break;
        case OPCODE_C_BEQZ:
            wb_isa_type[ISA_SB_type] = 1;
            wb_dec[Instr_BEQ] = 1;
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(11, 8) = (wb_instr(11, 10), wb_instr(4, 3));
            wb_instr_out(27, 25) = (wb_instr(6, 5), wb_instr[2]);
            if (wb_instr[12]) {
                wb_instr_out(30, 28) = ~0;
                wb_instr_out[7] = 1;
                wb_instr_out[31] = 1;
            }
            break;
        case OPCODE_C_SWSP:
            wb_isa_type[ISA_S_type] = 1;
            wb_dec[Instr_SW] = 1;
            wb_instr_out(24, 20) = wb_instr(6, 2);  // rs2
            wb_instr_out(19, 15) = 0x2;             // rs1 = sp
            wb_instr_out(11, 9) = wb_instr(11, 9);
            wb_instr_out(27, 25) = (wb_instr(8, 7), wb_instr[12]);
            break;
        case OPCODE_C_SD:
            wb_isa_type[ISA_S_type] = 1;
            wb_dec[Instr_SD] = 1;
            wb_instr_out(24, 20) = 0x8 | wb_instr(4, 2);    // rs2
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(11, 10) = wb_instr(11, 10);
            wb_instr_out(27, 25) = (wb_instr(6, 5), wb_instr[12]);
            break;
        case OPCODE_C_BNEZ:
            wb_isa_type[ISA_SB_type] = 1;
            wb_dec[Instr_BNE] = 1;
            wb_instr_out(19, 15) = 0x8 | wb_instr(9, 7);    // rs1
            wb_instr_out(11, 8) = (wb_instr(11, 10), wb_instr(4, 3));
            wb_instr_out(27, 25) = (wb_instr(6, 5), wb_instr[2]);
            if (wb_instr[12]) {
                wb_instr_out(30, 28) = ~0;
                wb_instr_out[7] = 1;
                wb_instr_out[31] = 1;
            }
            break;
        case OPCODE_C_SDSP:
            wb_isa_type[ISA_S_type] = 1;
            wb_dec[Instr_SD] = 1;
            wb_instr_out(24, 20) = wb_instr(6, 2);  // rs2
            wb_instr_out(19, 15) = 0x2;             // rs1 = sp
            wb_instr_out(11, 10) = wb_instr(11, 10);
            wb_instr_out(28, 25) = (wb_instr(9, 7), wb_instr[12]);
            break;
        default:
            w_error = true;
        }
    } else {
        wb_opcode1 = wb_instr(6, 2);
        wb_opcode2 = wb_instr(14, 12);
        switch (wb_opcode1) {
        case OPCODE_ADD:
            wb_isa_type[ISA_R_type] = 1;
            switch (wb_opcode2) {
            case 0:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_ADD] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_MUL] = 1;
                } else if (wb_instr(31, 25) == 0x20) {
                    wb_dec[Instr_SUB] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x1:
                wb_dec[Instr_SLL] = 1;
                break;
            case 0x2:
                wb_dec[Instr_SLT] = 1;
                break;
            case 0x3:
                wb_dec[Instr_SLTU] = 1;
                break;
            case 0x4:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_XOR] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_DIV] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x5:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SRL] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_DIVU] = 1;
                } else if (wb_instr(31, 25) == 0x20) {
                    wb_dec[Instr_SRA] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x6:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_OR] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_REM] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x7:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_AND] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_REMU] = 1;
                } else {
                    w_error = true;
                }
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_ADDI:
            wb_isa_type[ISA_I_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_ADDI] = 1;
                break;
            case 0x1:
                wb_dec[Instr_SLLI] = 1;
                break;
            case 0x2:
                wb_dec[Instr_SLTI] = 1;
                break;
            case 0x3:
                wb_dec[Instr_SLTIU] = 1;
                break;
            case 0x4:
                wb_dec[Instr_XORI] = 1;
                break;
            case 0x5:
                if (wb_instr(31, 26) == 0x00) {
                    wb_dec[Instr_SRLI] = 1;
                } else if (wb_instr(31, 26) == 0x10) {
                    wb_dec[Instr_SRAI] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x6:
                wb_dec[Instr_ORI] = 1;
                break;
            case 7:
                wb_dec[Instr_ANDI] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_ADDIW:
            wb_isa_type[ISA_I_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_ADDIW] = 1;
                break;
            case 0x1:
                wb_dec[Instr_SLLIW] = 1;
                break;
            case 0x5:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SRLIW] = 1;
                } else if (wb_instr(31, 25) == 0x20) {
                    wb_dec[Instr_SRAIW] = 1;
                } else {
                    w_error = true;
                }
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_ADDW:
            wb_isa_type[ISA_R_type] = 1;
            switch (wb_opcode2) {
            case 0:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_ADDW] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_MULW] = 1;
                } else if (wb_instr(31, 25) == 0x20) {
                    wb_dec[Instr_SUBW] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x1:
                wb_dec[Instr_SLLW] = 1;
                break;
            case 0x4:
                if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_DIVW] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x5:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SRLW] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_DIVUW] = 1;
                } else if (wb_instr(31, 25) == 0x20) {
                    wb_dec[Instr_SRAW] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x6:
                if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_REMW] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x7:
                if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_REMUW] = 1;
                } else {
                    w_error = true;
                }
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_AUIPC:
            wb_isa_type[ISA_U_type] = 1;
            wb_dec[Instr_AUIPC] = 1;
            break;
        case OPCODE_BEQ:
            wb_isa_type[ISA_SB_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_BEQ] = 1;
                break;
            case 1:
                wb_dec[Instr_BNE] = 1;
                break;
            case 4:
                wb_dec[Instr_BLT] = 1;
                break;
            case 5:
                wb_dec[Instr_BGE] = 1;
                break;
            case 6:
                wb_dec[Instr_BLTU] = 1;
                break;
            case 7:
                wb_dec[Instr_BGEU] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_JAL:
            wb_isa_type[ISA_UJ_type] = 1;
            wb_dec[Instr_JAL] = 1;
            break;
        case OPCODE_JALR:
            wb_isa_type[ISA_I_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_JALR] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_LB:
            wb_isa_type[ISA_I_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_LB] = 1;
                break;
            case 1:
                wb_dec[Instr_LH] = 1;
                break;
            case 2:
                wb_dec[Instr_LW] = 1;
                break;
            case 3:
                wb_dec[Instr_LD] = 1;
                break;
            case 4:
                wb_dec[Instr_LBU] = 1;
                break;
            case 5:
                wb_dec[Instr_LHU] = 1;
                break;
            case 6:
                wb_dec[Instr_LWU] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_LUI:
            wb_isa_type[ISA_U_type] = 1;
            wb_dec[Instr_LUI] = 1;
            break;
        case OPCODE_SB:
            wb_isa_type[ISA_S_type] = 1;
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_SB] = 1;
                break;
            case 1:
                wb_dec[Instr_SH] = 1;
                break;
            case 2:
                wb_dec[Instr_SW] = 1;
                break;
            case 3:
                wb_dec[Instr_SD] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_CSRR:
            wb_isa_type[ISA_I_type] = 1;
            switch (wb_opcode2) {
            case 0:
                if (wb_instr == 0x00000073) {
                    wb_dec[Instr_ECALL] = 1;
                } else if (wb_instr == 0x00100073) {
                    wb_dec[Instr_EBREAK] = 1;
                } else if (wb_instr == 0x00200073) {
                    wb_dec[Instr_URET] = 1;
                } else if (wb_instr == 0x10200073) {
                    wb_dec[Instr_SRET] = 1;
                } else if (wb_instr == 0x20200073) {
                    wb_dec[Instr_HRET] = 1;
                } else if (wb_instr == 0x30200073) {
                    wb_dec[Instr_MRET] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 1:
                wb_dec[Instr_CSRRW] = 1;
                break;
            case 2:
                wb_dec[Instr_CSRRS] = 1;
                break;
            case 3:
                wb_dec[Instr_CSRRC] = 1;
                break;
            case 5:
                wb_dec[Instr_CSRRWI] = 1;
                break;
            case 6:
                wb_dec[Instr_CSRRSI] = 1;
                break;
            case 7:
                wb_dec[Instr_CSRRCI] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_FENCE:
            switch (wb_opcode2) {
            case 0:
                wb_dec[Instr_FENCE] = 1;
                break;
            case 1:
                wb_dec[Instr_FENCE_I] = 1;
                break;
            default:
                w_error = true;
            }
            break;
        default:
            if (CFG_HW_FPU_ENABLE) {
                switch (wb_opcode1) {
                case OPCODE_FPU_LD:
                    wb_isa_type[ISA_I_type] = 1;
                    if (wb_opcode2 == 3) {
                        wb_dec[Instr_FLD] = 1;
                    } else {
                        w_error = true;
                    }
                    break;
                case OPCODE_FPU_SD:
                    wb_isa_type[ISA_S_type] = 1;
                    if (wb_opcode2 == 3) {
                        wb_dec[Instr_FSD] = 1;
                    } else {
                        w_error = true;
                    }
                    break;
                case OPCODE_FPU_OP:
                    wb_isa_type[ISA_R_type] = 1;
                    switch (wb_instr(31, 25)) {
                    case 0x1:
                        wb_dec[Instr_FADD_D] = 1;
                        break;
                    case 0x5:
                        wb_dec[Instr_FSUB_D] = 1;
                        break;
                    case 0x9:
                        wb_dec[Instr_FMUL_D] = 1;
                        break;
                    case 0xD:
                        wb_dec[Instr_FDIV_D] = 1;
                        break;
                    case 0x15:
                        if (wb_opcode2 == 0) {
                            wb_dec[Instr_FMIN_D] = 1;
                        } else if (wb_opcode2 == 1) {
                            wb_dec[Instr_FMAX_D] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x51:
                        if (wb_opcode2 == 0) {
                            wb_dec[Instr_FLE_D] = 1;
                        } else if (wb_opcode2 == 1) {
                            wb_dec[Instr_FLT_D] = 1;
                        } else if (wb_opcode2 == 2) {
                            wb_dec[Instr_FEQ_D] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x61:
                        if (wb_instr(24, 20) == 0) {
                            wb_dec[Instr_FCVT_W_D] = 1;
                        } else if (wb_instr(24, 20) == 1) {
                            wb_dec[Instr_FCVT_WU_D] = 1;
                        } else if (wb_instr(24, 20) == 2) {
                            wb_dec[Instr_FCVT_L_D] = 1;
                        } else if (wb_instr(24, 20) == 3) {
                            wb_dec[Instr_FCVT_LU_D] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x69:
                        if (wb_instr(24, 20) == 0) {
                            wb_dec[Instr_FCVT_D_W] = 1;
                        } else if (wb_instr(24, 20) == 1) {
                            wb_dec[Instr_FCVT_D_WU] = 1;
                        } else if (wb_instr(24, 20) == 2) {
                            wb_dec[Instr_FCVT_D_L] = 1;
                        } else if (wb_instr(24, 20) == 3) {
                            wb_dec[Instr_FCVT_D_LU] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x71:
                        if (wb_instr(24, 20) == 0 && wb_opcode2 == 0) {
                            wb_dec[Instr_FMOV_X_D] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x79:
                        if (wb_instr(24, 20) == 0 && wb_opcode2 == 0) {
                            wb_dec[Instr_FMOV_D_X] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    default:
                        w_error = true;
                    }
                    break;
                default:
                    w_error = true;
                }
            } else {
                w_error = true;
            }
        }
        wb_instr_out = wb_instr;
    }  // compressed/!compressed

    if (i_f_valid.read()) {
        v.valid = 1;
        v.pc = i_f_pc;
        v.instr = wb_instr_out;
        v.compressed = w_compressed;
        v.instr_load_fault = i_instr_load_fault.read();
        v.instr_executable = i_instr_executable.read();

        v.isa_type = wb_isa_type;
        v.instr_vec = wb_dec;
        v.memop_store = (wb_dec[Instr_SD] | wb_dec[Instr_SW] 
                | wb_dec[Instr_SH] | wb_dec[Instr_SB]).to_bool()
                | wb_dec[Instr_FSD].to_bool();
        v.memop_load = (wb_dec[Instr_LD] | wb_dec[Instr_LW] 
                | wb_dec[Instr_LH] | wb_dec[Instr_LB]
                | wb_dec[Instr_LWU] | wb_dec[Instr_LHU] 
                | wb_dec[Instr_LBU]).to_bool()
                | wb_dec[Instr_FLD].to_bool();
        v.memop_sign_ext = (wb_dec[Instr_LD] | wb_dec[Instr_LW]
                | wb_dec[Instr_LH] | wb_dec[Instr_LB]).to_bool();
        if (wb_dec[Instr_LD] || wb_dec[Instr_SD] ||
            wb_dec[Instr_FLD] || wb_dec[Instr_FSD]) {
            v.memop_size = MEMOP_8B;
        } else if (wb_dec[Instr_LW] || wb_dec[Instr_LWU] || wb_dec[Instr_SW]) {
            v.memop_size = MEMOP_4B;
        } else if (wb_dec[Instr_LH] || wb_dec[Instr_LHU] || wb_dec[Instr_SH]) {
            v.memop_size = MEMOP_2B;
        } else {
            v.memop_size = MEMOP_1B;
        }
        v.unsigned_op = (wb_dec[Instr_DIVU] | wb_dec[Instr_REMU] |
                wb_dec[Instr_DIVUW] | wb_dec[Instr_REMUW]).to_bool() |
                wb_dec[Instr_FCVT_WU_D].to_bool() |
                wb_dec[Instr_FCVT_LU_D].to_bool();

        v.rv32 = (wb_dec[Instr_ADDW] | wb_dec[Instr_ADDIW] 
            | wb_dec[Instr_SLLW] | wb_dec[Instr_SLLIW] | wb_dec[Instr_SRAW]
            | wb_dec[Instr_SRAIW]
            | wb_dec[Instr_SRLW] | wb_dec[Instr_SRLIW] | wb_dec[Instr_SUBW] 
            | wb_dec[Instr_DIVW] | wb_dec[Instr_DIVUW] | wb_dec[Instr_MULW]
            | wb_dec[Instr_REMW] | wb_dec[Instr_REMUW]).to_bool();

        v.f64 = (wb_dec[Instr_FADD_D] | wb_dec[Instr_FSUB_D]
            | wb_dec[Instr_FMUL_D] | wb_dec[Instr_FDIV_D]
            | wb_dec[Instr_FMIN_D] | wb_dec[Instr_FMAX_D]
            | wb_dec[Instr_FLE_D] | wb_dec[Instr_FLT_D]
            | wb_dec[Instr_FEQ_D] | wb_dec[Instr_FCVT_W_D]
            | wb_dec[Instr_FCVT_WU_D] | wb_dec[Instr_FCVT_L_D]
            | wb_dec[Instr_FCVT_LU_D] | wb_dec[Instr_FMOV_X_D]
            | wb_dec[Instr_FCVT_D_W] | wb_dec[Instr_FCVT_D_WU]
            | wb_dec[Instr_FCVT_D_L] | wb_dec[Instr_FCVT_D_LU]
            | wb_dec[Instr_FMOV_D_X] | wb_dec[Instr_FLD]
            | wb_dec[Instr_FSD]).to_bool();
        
        v.instr_unimplemented = w_error;
    } else if (!i_any_hold.read()) {
        v.valid = 0;
    }
    w_o_valid = r.valid.read();

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_valid = w_o_valid;
    o_pc = r.pc;
    o_instr = r.instr;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_size = r.memop_size;
    o_unsigned_op = r.unsigned_op;
    o_rv32 = r.rv32;
    o_f64 = r.f64;
    o_compressed = r.compressed;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_executable = r.instr_executable;
}

void InstrDecoder::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

