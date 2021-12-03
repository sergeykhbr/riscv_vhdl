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

#include "dec_rvc.h"

namespace debugger {

DecoderRvc::DecoderRvc(sc_module_name name_, bool async_reset
    ) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush_pipeline("i_flush_pipeline"),
    i_progbuf_ena("i_progbuf_ena"),
    i_f_valid("i_f_valid"),
    i_f_pc("i_f_pc"),
    i_f_instr("i_f_instr"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_executable("i_instr_executable"),
    o_radr1("o_radr1"),
    o_radr2("o_radr2"),
    o_waddr("o_waddr"),
    o_csr_addr("o_csr_addr"),
    o_imm("o_imm"),
    o_pc("o_pc"),
    o_instr("o_instr"),
    o_memop_store("o_memop_store"),
    o_memop_load("o_memop_load"),
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_size("o_memop_size"),
    o_rv32("o_rv32"),
    o_compressed("o_compressed"),
    o_amo("o_amo"),
    o_f64("o_f64"),
    o_unsigned_op("o_unsigned_op"),
    o_isa_type("o_isa_type"),
    o_instr_vec("o_instr_vec"),
    o_exception("o_exception"),
    o_instr_load_fault("o_instr_load_fault"),
    o_instr_executable("o_instr_executable"),
    o_progbuf_ena("o_progbuf_ena") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_f_valid;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_executable;
    sensitive << i_flush_pipeline;
    sensitive << i_progbuf_ena;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.memop_load;
    sensitive << r.memop_store;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.rv32;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;
    sensitive << r.instr_unimplemented;
    sensitive << r.radr1;
    sensitive << r.radr2;
    sensitive << r.waddr;
    sensitive << r.imm;
    sensitive << r.progbuf_ena;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void DecoderRvc::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_f_valid, i_f_valid.name());
        sc_trace(o_vcd, i_f_pc, i_f_pc.name());
        sc_trace(o_vcd, i_f_instr, i_f_instr.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_isa_type, o_isa_type.name());
        sc_trace(o_vcd, o_instr_vec, o_instr_vec.name());
        sc_trace(o_vcd, o_exception, o_exception.name());
        sc_trace(o_vcd, o_compressed, o_compressed.name());
        sc_trace(o_vcd, o_amo, o_amo.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_radr1, o_radr1.name());
        sc_trace(o_vcd, o_radr2, o_radr2.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_imm, o_imm.name());

        std::string pn(name());
        sc_trace(o_vcd, r.pc, pn + ".r_pc");
    }
}

void DecoderRvc::comb() {
    bool w_error;
    sc_uint<16> wb_instr;
    sc_uint<5> wb_opcode1;
    sc_uint<3> wb_opcode2;
    sc_bv<Instr_Total> wb_dec;
    sc_bv<ISA_Total> wb_isa_type;
    sc_uint<6> vb_radr1;
    sc_uint<6> vb_radr2;
    sc_uint<6> vb_waddr;
    sc_uint<RISCV_ARCH> vb_imm;
    bool v_memop_store;
    bool v_memop_load;
    bool v_memop_sign_ext;
    sc_uint<2> vb_memop_size;
    bool v_rv32;

    v = r;
    w_error = false;
    wb_instr = i_f_instr.read();
    wb_dec = 0;
    wb_isa_type = 0;
    vb_radr1 = 0;
    vb_radr2 = 0;
    vb_waddr = 0;
    vb_imm = 0;
    v_memop_store = 0;
    v_memop_load = 0;
    v_memop_sign_ext = 0;
    vb_memop_size = 0;
    v_rv32 = 0;

    wb_opcode1 = (wb_instr(15, 13), wb_instr(1, 0));
    switch (wb_opcode1) {
    case OPCODE_C_ADDI4SPN:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_ADDI] = 1;
        vb_radr1 = 0x2;                             // rs1 = sp
        vb_waddr = 0x8 | wb_instr(4, 2);            // rd
        vb_imm(9,2) = (wb_instr(10, 7), wb_instr(12, 11), wb_instr[5], wb_instr[6]);
        break;
    case OPCODE_C_NOP_ADDI:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_ADDI] = 1;
        vb_radr1 = wb_instr(11, 7);                 // rs1
        vb_waddr = wb_instr(11, 7);                 // rd
        vb_imm(4, 0) = wb_instr(6, 2);
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 5) = ~0ull;
        }
        break;
    case OPCODE_C_SLLI:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_SLLI] = 1;
        vb_radr1 = wb_instr(11, 7);                 // rs1
        vb_waddr = wb_instr(11, 7);                 // rd
        vb_imm(5, 0) = (wb_instr[12], wb_instr(6, 2));
        break;
    case OPCODE_C_JAL_ADDIW:
        // JAL is the RV32C only instruction
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_ADDIW] = 1;
        vb_radr1 = wb_instr(11, 7);                 // rs1
        vb_waddr = wb_instr(11, 7);                 // rd
        vb_imm(4, 0) = wb_instr(6, 2);
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 5) = ~0ull;
        }
        break;
    case OPCODE_C_LW:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_LW] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);                // rs1
        vb_waddr = 0x8 | wb_instr(4, 2);                // rd
        vb_imm(6, 2) = (wb_instr[5], wb_instr(12, 10), wb_instr[6]);
        break;
    case OPCODE_C_LI:  // ADDI rd = r0 + imm
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_ADDI] = 1;
        vb_waddr = wb_instr(11, 7);      // rd
        vb_imm(4, 0) = wb_instr(6, 2);
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 5) = ~0ull;
        }
        break;
    case OPCODE_C_LWSP:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_LW] = 1;
        vb_radr1 = 0x2;                             // rs1 = sp
        vb_waddr = wb_instr(11, 7);                 // rd
        vb_imm(7, 2) = (wb_instr(3, 2), wb_instr[12], wb_instr(6, 4));
        break;
    case OPCODE_C_LD:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_LD] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);//(0, wb_instr.range(19, 15));
        vb_waddr = 0x8 | wb_instr(4, 2);     // rd
        vb_imm(7, 3) = (wb_instr[6], wb_instr[5], wb_instr(12, 10));
        break;
    case OPCODE_C_ADDI16SP_LUI:
        if (wb_instr(11, 7) == 0x2) {
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ADDI] = 1;
            vb_radr1 = 0x2;                 // rs1 = sp
            vb_waddr = 0x2;                 // rd = sp
            vb_imm(8, 4) = (wb_instr(4, 3), wb_instr[5], wb_instr[2], wb_instr[6]);
            if (wb_instr[12]) {
                vb_imm(RISCV_ARCH-1, 9) = ~0ull;
            }
        } else {
            wb_isa_type[ISA_U_type] = 1;
            wb_dec[Instr_LUI] = 1;
            vb_waddr = wb_instr(11, 7);  // rd
            vb_imm(16, 12) = wb_instr(6, 2);
            if (wb_instr[12]) {
                vb_imm(RISCV_ARCH-1, 17) = ~0ull;
            }
        }
        break;
    case OPCODE_C_LDSP:
        wb_isa_type[ISA_I_type] = 1;
        wb_dec[Instr_LD] = 1;
        vb_radr1 = 0x2;                         // rs1 = sp
        vb_waddr = wb_instr(11, 7);             // rd
        vb_imm(8, 3) = (wb_instr(4, 2), wb_instr[12], wb_instr(6, 5));
        break;
    case OPCODE_C_MATH:
        if (wb_instr(11, 10) == 0) {
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_SRLI] = 1;
            vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
            vb_waddr = 0x8 | wb_instr(9, 7);    // rd
            vb_imm(5, 0) = (wb_instr[12], wb_instr(6, 2));
        } else if (wb_instr(11, 10) == 1) {
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_SRAI] = 1;
            vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
            vb_waddr = 0x8 | wb_instr(9, 7);    // rd
            vb_imm(5, 0) = (wb_instr[12], wb_instr(6, 2));
        } else if (wb_instr(11, 10) == 2) {
            wb_isa_type[ISA_I_type] = 1;
            wb_dec[Instr_ANDI] = 1;
            vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
            vb_waddr = 0x8 | wb_instr(9, 7);    // rd
            vb_imm(4, 0) = wb_instr(6, 2);
            if (wb_instr[12]) {
                vb_imm(RISCV_ARCH-1, 5) = ~0ull;
            }
        } else if (wb_instr[12] == 0) {
            wb_isa_type[ISA_R_type] = 1;
            vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
            vb_radr2 = 0x8 | wb_instr(4, 2);    // rs2
            vb_waddr = 0x8 | wb_instr(9, 7);    // rd
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
            vb_radr1 = 0x8 | wb_instr(9, 7);        // rs1
            vb_radr2 = 0x8 | wb_instr(4, 2);        // rs2
            vb_waddr = 0x8 | wb_instr(9, 7);        // rd
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
                vb_radr1 = wb_instr(11, 7);     // rs1
            } else {
                wb_dec[Instr_ADDI] = 1;
                vb_radr1 = wb_instr(6, 2);      // rs1
                vb_waddr = wb_instr(11, 7);     // rd
            }
        } else {
            if (wb_instr(11, 7) == 0 && wb_instr(6, 2) == 0) {
                wb_dec[Instr_EBREAK] = 1;
            } else if (wb_instr(6, 2) == 0) {
                wb_dec[Instr_JALR] = 1;
                vb_radr1 = wb_instr(11, 7);              // rs1
                vb_waddr = 0x1;
            } else {
                wb_dec[Instr_ADD] = 1;
                wb_isa_type[ISA_R_type] = 1;
                vb_radr1 = wb_instr(11, 7);     // rs1
                vb_radr2 = wb_instr(6, 2);      // rs2
                vb_waddr = wb_instr(11, 7);     // rd
            }
        }
        break;
    case OPCODE_C_J:   // JAL with rd = 0
        wb_isa_type[ISA_UJ_type] = 1;
        wb_dec[Instr_JAL] = 1;
        vb_imm(10, 1) = (wb_instr[8], wb_instr(10, 9), wb_instr[6], wb_instr[7],
                        wb_instr[2], wb_instr[11], wb_instr(5, 3));
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 11) = ~0ull;
        }
        break;
    case OPCODE_C_SW:
        wb_isa_type[ISA_S_type] = 1;
        wb_dec[Instr_SW] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
        vb_radr2 = 0x8 | wb_instr(4, 2);    // rs2
        vb_imm(6, 2) = (wb_instr[5] , wb_instr[12], wb_instr(11, 10), wb_instr[6]);
        break;
    case OPCODE_C_BEQZ:
        wb_isa_type[ISA_SB_type] = 1;
        wb_dec[Instr_BEQ] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
        vb_imm(7, 1) = (wb_instr(6, 5), wb_instr[2], wb_instr(11, 10), wb_instr(4, 3));
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 8) = ~0ull;
        }
        break;
    case OPCODE_C_SWSP:
        wb_isa_type[ISA_S_type] = 1;
        wb_dec[Instr_SW] = 1;
        vb_radr1 = 0x2;             // rs1 = sp
        vb_radr2 = wb_instr(6, 2);   // rs2
        vb_imm(7, 2) = (wb_instr(8, 7), wb_instr[12], wb_instr(11, 9));
        break;
    case OPCODE_C_SD:
        wb_isa_type[ISA_S_type] = 1;
        wb_dec[Instr_SD] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
        vb_radr2 = 0x8 | wb_instr(4, 2);    // rs2
        vb_imm(7, 3) = (wb_instr(6, 5), wb_instr[12], wb_instr(11, 10));
        break;
    case OPCODE_C_BNEZ:
        wb_isa_type[ISA_SB_type] = 1;
        wb_dec[Instr_BNE] = 1;
        vb_radr1 = 0x8 | wb_instr(9, 7);    // rs1
        vb_imm(7, 1) = (wb_instr(6, 5), wb_instr[2], wb_instr(11, 10), wb_instr(4, 3));
        if (wb_instr[12]) {
            vb_imm(RISCV_ARCH-1, 8) = ~0ull;
        }
        break;
    case OPCODE_C_SDSP:
        wb_isa_type[ISA_S_type] = 1;
        wb_dec[Instr_SD] = 1;
        vb_radr1 = 0x2;             // rs1 = sp
        vb_radr2 = wb_instr(6, 2);  // rs2
        vb_imm(8, 3) = (wb_instr(9, 7), wb_instr[12], wb_instr(11, 10));
        break;
    default:
        w_error = true;
    }


    v_memop_store = (wb_dec[Instr_SD] | wb_dec[Instr_SW]).to_bool();
    v_memop_load = (wb_dec[Instr_LD] | wb_dec[Instr_LW]).to_bool();
    v_memop_sign_ext = (wb_dec[Instr_LD] | wb_dec[Instr_LW]).to_bool();
    if (wb_dec[Instr_LD] || wb_dec[Instr_SD]) {
        vb_memop_size = MEMOP_8B;
    } else if (wb_dec[Instr_LW] || wb_dec[Instr_SW]) {
        vb_memop_size = MEMOP_4B;
    } else {
        vb_memop_size = MEMOP_8B;
    }
    v_rv32 = (wb_dec[Instr_ADDW] | wb_dec[Instr_ADDIW] 
            | wb_dec[Instr_SUBW]).to_bool();

       

    v.pc = i_f_pc;
    v.isa_type = wb_isa_type;
    v.instr_vec = wb_dec;
    v.instr = i_f_instr;
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.rv32 = v_rv32;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_executable = i_instr_executable;
    v.instr_unimplemented = w_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena.read();


    if ((!async_reset_ && !i_nrst.read()) || i_flush_pipeline.read() == 1) {
        R_RESET(v);
    }

    o_pc = r.pc;
    o_instr = (0, r.instr.read());
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_size = r.memop_size;
    o_unsigned_op = 0;
    o_rv32 = r.rv32;
    o_f64 = 0;
    o_compressed = 1;
    o_amo = 0;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_executable = r.instr_executable;

    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = 0;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;
}

void DecoderRvc::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

