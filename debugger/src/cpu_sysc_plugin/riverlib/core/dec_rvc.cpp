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

#include "dec_rvc.h"
#include "api_core.h"

namespace debugger {

DecoderRvc::DecoderRvc(sc_module_name name,
                       bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush_pipeline("i_flush_pipeline"),
    i_progbuf_ena("i_progbuf_ena"),
    i_f_pc("i_f_pc"),
    i_f_instr("i_f_instr"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_page_fault_x("i_instr_page_fault_x"),
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
    o_instr_page_fault_x("o_instr_page_fault_x"),
    o_progbuf_ena("o_progbuf_ena") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_flush_pipeline;
    sensitive << i_progbuf_ena;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_page_fault_x;
    sensitive << r.pc;
    sensitive << r.isa_type;
    sensitive << r.instr_vec;
    sensitive << r.instr;
    sensitive << r.memop_store;
    sensitive << r.memop_load;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.rv32;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_page_fault_x;
    sensitive << r.instr_unimplemented;
    sensitive << r.radr1;
    sensitive << r.radr2;
    sensitive << r.waddr;
    sensitive << r.imm;
    sensitive << r.progbuf_ena;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void DecoderRvc::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_progbuf_ena, i_progbuf_ena.name());
        sc_trace(o_vcd, i_f_pc, i_f_pc.name());
        sc_trace(o_vcd, i_f_instr, i_f_instr.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_page_fault_x, i_instr_page_fault_x.name());
        sc_trace(o_vcd, o_radr1, o_radr1.name());
        sc_trace(o_vcd, o_radr2, o_radr2.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_imm, o_imm.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_memop_store, o_memop_store.name());
        sc_trace(o_vcd, o_memop_load, o_memop_load.name());
        sc_trace(o_vcd, o_memop_sign_ext, o_memop_sign_ext.name());
        sc_trace(o_vcd, o_memop_size, o_memop_size.name());
        sc_trace(o_vcd, o_rv32, o_rv32.name());
        sc_trace(o_vcd, o_compressed, o_compressed.name());
        sc_trace(o_vcd, o_amo, o_amo.name());
        sc_trace(o_vcd, o_f64, o_f64.name());
        sc_trace(o_vcd, o_unsigned_op, o_unsigned_op.name());
        sc_trace(o_vcd, o_isa_type, o_isa_type.name());
        sc_trace(o_vcd, o_instr_vec, o_instr_vec.name());
        sc_trace(o_vcd, o_exception, o_exception.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_instr_page_fault_x, o_instr_page_fault_x.name());
        sc_trace(o_vcd, o_progbuf_ena, o_progbuf_ena.name());
        sc_trace(o_vcd, r.pc, pn + ".r_pc");
        sc_trace(o_vcd, r.isa_type, pn + ".r_isa_type");
        sc_trace(o_vcd, r.instr_vec, pn + ".r_instr_vec");
        sc_trace(o_vcd, r.instr, pn + ".r_instr");
        sc_trace(o_vcd, r.memop_store, pn + ".r_memop_store");
        sc_trace(o_vcd, r.memop_load, pn + ".r_memop_load");
        sc_trace(o_vcd, r.memop_sign_ext, pn + ".r_memop_sign_ext");
        sc_trace(o_vcd, r.memop_size, pn + ".r_memop_size");
        sc_trace(o_vcd, r.rv32, pn + ".r_rv32");
        sc_trace(o_vcd, r.instr_load_fault, pn + ".r_instr_load_fault");
        sc_trace(o_vcd, r.instr_page_fault_x, pn + ".r_instr_page_fault_x");
        sc_trace(o_vcd, r.instr_unimplemented, pn + ".r_instr_unimplemented");
        sc_trace(o_vcd, r.radr1, pn + ".r_radr1");
        sc_trace(o_vcd, r.radr2, pn + ".r_radr2");
        sc_trace(o_vcd, r.waddr, pn + ".r_waddr");
        sc_trace(o_vcd, r.imm, pn + ".r_imm");
        sc_trace(o_vcd, r.progbuf_ena, pn + ".r_progbuf_ena");
    }

}

void DecoderRvc::comb() {
    bool v_error;
    sc_uint<16> vb_instr;
    sc_uint<5> vb_opcode1;
    sc_uint<3> vb_opcode2;
    sc_biguint<Instr_Total> vb_dec;
    sc_uint<ISA_Total> vb_isa_type;
    sc_uint<6> vb_radr1;
    sc_uint<6> vb_radr2;
    sc_uint<6> vb_waddr;
    sc_uint<RISCV_ARCH> vb_imm;
    bool v_memop_store;
    bool v_memop_load;
    bool v_memop_sign_ext;
    sc_uint<2> vb_memop_size;
    bool v_rv32;

    v_error = 0;
    vb_instr = 0;
    vb_opcode1 = 0;
    vb_opcode2 = 0;
    vb_dec = 0;
    vb_isa_type = 0;
    vb_radr1 = 0;
    vb_radr2 = 0;
    vb_waddr = 0;
    vb_imm = 0;
    v_memop_store = 0;
    v_memop_load = 0;
    v_memop_sign_ext = 0;
    vb_memop_size = 0;
    v_rv32 = 0;

    v = r;

    vb_instr = i_f_instr;

    vb_opcode1 = (vb_instr(15, 13), vb_instr(1, 0));
    switch (vb_opcode1) {
    case OPCODE_C_ADDI4SPN:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_ADDI] = 1;
        vb_radr1 = 0x2;                                     // rs1 = sp
        vb_waddr = (0x1, vb_instr(4, 2));                   // rd
        vb_imm(9, 2) = (vb_instr(10, 7), vb_instr(12, 11), vb_instr[5], vb_instr[6]);
        break;
    case OPCODE_C_NOP_ADDI:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_ADDI] = 1;
        vb_radr1 = vb_instr(11, 7);                         // rs1
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(4, 0) = vb_instr(6, 2);
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 5) = ~0ull;
        }
        break;
    case OPCODE_C_SLLI:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_SLLI] = 1;
        vb_radr1 = (0, vb_instr(11, 7));                    // rs1
        vb_waddr = (0, vb_instr(11, 7));                    // rd
        vb_imm(5, 0) = (vb_instr[12], vb_instr(6, 2));
        break;
    case OPCODE_C_JAL_ADDIW:
        // JAL is the RV32C only instruction
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_ADDIW] = 1;
        vb_radr1 = (0, vb_instr(11, 7));                    // rs1
        vb_waddr = (0, vb_instr(11, 7));                    // rd
        vb_imm(4, 0) = vb_instr(6, 2);
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 5) = ~0ull;
        }
        break;
    case OPCODE_C_LW:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_LW] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));                   // rs1
        vb_waddr = (0x1, vb_instr(4, 2));                   // rd
        vb_imm(6, 2) = (vb_instr[5], vb_instr(12, 10), vb_instr[6]);
        break;
    case OPCODE_C_LI:                                       // ADDI rd = r0 + imm
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_ADDI] = 1;
        vb_waddr = (0, vb_instr(11, 7));                    // rd
        vb_imm(4, 0) = vb_instr(6, 2);
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 5) = ~0ull;
        }
        break;
    case OPCODE_C_LWSP:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_LW] = 1;
        vb_radr1 = 0x2;                                     // rs1 = sp
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(7, 2) = (vb_instr(3, 2), vb_instr[12], vb_instr(6, 4));
        break;
    case OPCODE_C_LD:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_LD] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));
        vb_waddr = (0x1, vb_instr(4, 2));                   // rd
        vb_imm(7, 3) = (vb_instr[6], vb_instr[5], vb_instr(12, 10));
        break;
    case OPCODE_C_ADDI16SP_LUI:
        if (vb_instr(11, 7) == 0x2) {
            vb_isa_type[ISA_I_type] = 1;
            vb_dec[Instr_ADDI] = 1;
            vb_radr1 = 0x2;                                 // rs1 = sp
            vb_waddr = 0x2;                                 // rd = sp
            vb_imm(8, 4) = (vb_instr(4, 3), vb_instr[5], vb_instr[2], vb_instr[6]);
            if (vb_instr[12] == 1) {
                vb_imm((RISCV_ARCH - 1), 9) = ~0ull;
            }
        } else {
            vb_isa_type[ISA_U_type] = 1;
            vb_dec[Instr_LUI] = 1;
            vb_waddr = (0, vb_instr(11, 7));                // rd
            vb_imm(16, 12) = vb_instr(6, 2);
            if (vb_instr[12] == 1) {
                vb_imm((RISCV_ARCH - 1), 17) = ~0ull;
            }
        }
        break;
    case OPCODE_C_LDSP:
        vb_isa_type[ISA_I_type] = 1;
        vb_dec[Instr_LD] = 1;
        vb_radr1 = 0x2;                                     // rs1 = sp
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(8, 3) = (vb_instr(4, 2), vb_instr[12], vb_instr(6, 5));
        break;
    case OPCODE_C_MATH:
        if (vb_instr(11, 10) == 0) {
            vb_isa_type[ISA_I_type] = 1;
            vb_dec[Instr_SRLI] = 1;
            vb_radr1 = (0x1, vb_instr(9, 7));               // rs1
            vb_waddr = (0x1, vb_instr(9, 7));               // rd
            vb_imm(5, 0) = (vb_instr[12], vb_instr(6, 2));
        } else if (vb_instr(11, 10) == 1) {
            vb_isa_type[ISA_I_type] = 1;
            vb_dec[Instr_SRAI] = 1;
            vb_radr1 = (0x1, vb_instr(9, 7));               // rs1
            vb_waddr = (0x1, vb_instr(9, 7));               // rd
            vb_imm(5, 0) = (vb_instr[12], vb_instr(6, 2));
        } else if (vb_instr(11, 10) == 2) {
            vb_isa_type[ISA_I_type] = 1;
            vb_dec[Instr_ANDI] = 1;
            vb_radr1 = (0x1, vb_instr(9, 7));               // rs1
            vb_waddr = (0x1, vb_instr(9, 7));               // rd
            vb_imm(4, 0) = vb_instr(6, 2);
            if (vb_instr[12] == 1) {
                vb_imm((RISCV_ARCH - 1), 5) = ~0ull;
            }
        } else if (vb_instr[12] == 0) {
            vb_isa_type[ISA_R_type] = 1;
            vb_radr1 = (0x1, vb_instr(9, 7));               // rs1
            vb_radr2 = (0x1, vb_instr(4, 2));               // rs2
            vb_waddr = (0x1, vb_instr(9, 7));               // rd
            switch (vb_instr(6, 5)) {
            case 0:
                vb_dec[Instr_SUB] = 1;
                break;
            case 1:
                vb_dec[Instr_XOR] = 1;
                break;
            case 2:
                vb_dec[Instr_OR] = 1;
                break;
            default:
                vb_dec[Instr_AND] = 1;
                break;
            }
        } else {
            vb_isa_type[ISA_R_type] = 1;
            vb_radr1 = (0x1, vb_instr(9, 7));               // rs1
            vb_radr2 = (0x1, vb_instr(4, 2));               // rs2
            vb_waddr = (0x1, vb_instr(9, 7));               // rd
            switch (vb_instr(6, 5)) {
            case 0:
                vb_dec[Instr_SUBW] = 1;
                break;
            case 1:
                vb_dec[Instr_ADDW] = 1;
                break;
            default:
                v_error = 1;
                break;
            }
        }
        break;
    case OPCODE_C_JR_MV_EBREAK_JALR_ADD:
        vb_isa_type[ISA_I_type] = 1;
        if (vb_instr[12] == 0) {
            if (vb_instr(6, 2).or_reduce() == 0) {
                vb_dec[Instr_JALR] = 1;
                vb_radr1 = (0, vb_instr(11, 7));            // rs1
            } else {
                vb_dec[Instr_ADDI] = 1;
                vb_radr1 = (0, vb_instr(6, 2));             // rs1
                vb_waddr = (0, vb_instr(11, 7));            // rd
            }
        } else {
            if ((vb_instr(11, 7).or_reduce() == 0) && (vb_instr(6, 2).or_reduce() == 0)) {
                vb_dec[Instr_EBREAK] = 1;
            } else if (vb_instr(6, 2).or_reduce() == 0) {
                vb_dec[Instr_JALR] = 1;
                vb_radr1 = (0, vb_instr(11, 7));            // rs1
                vb_waddr = 0x1;
            } else {
                vb_dec[Instr_ADD] = 1;
                vb_isa_type[ISA_R_type] = 1;
                vb_radr1 = (0, vb_instr(11, 7));            // rs1
                vb_radr2 = (0, vb_instr(6, 2));             // rs2
                vb_waddr = (0, vb_instr(11, 7));            // rd
            }
        }
        break;
    case OPCODE_C_J:                                        // JAL with rd = 0
        vb_isa_type[ISA_UJ_type] = 1;
        vb_dec[Instr_JAL] = 1;
        vb_imm(10, 1) = (vb_instr[8],
                vb_instr(10, 9),
                vb_instr[6],
                vb_instr[7],
                vb_instr[2],
                vb_instr[11],
                vb_instr(5, 3));
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 11) = ~0ull;
        }
        break;
    case OPCODE_C_SW:
        vb_isa_type[ISA_S_type] = 1;
        vb_dec[Instr_SW] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));                   // rs1
        vb_radr2 = (0x1, vb_instr(4, 2));                   // rs2
        vb_imm(6, 2) = (vb_instr[5], vb_instr[12], vb_instr(11, 10), vb_instr[6]);
        break;
    case OPCODE_C_BEQZ:
        vb_isa_type[ISA_SB_type] = 1;
        vb_dec[Instr_BEQ] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));                   // rs1
        vb_imm(7, 1) = (vb_instr(6, 5), vb_instr[2], vb_instr(11, 10), vb_instr(4, 3));
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 8) = ~0ull;
        }
        break;
    case OPCODE_C_SWSP:
        vb_isa_type[ISA_S_type] = 1;
        vb_dec[Instr_SW] = 1;
        vb_radr1 = 0x2;                                     // rs1 = sp
        vb_radr2 = (0, vb_instr(6, 2));                     // rs2
        vb_imm(7, 2) = (vb_instr(8, 7), vb_instr[12], vb_instr(11, 9));
        break;
    case OPCODE_C_SD:
        vb_isa_type[ISA_S_type] = 1;
        vb_dec[Instr_SD] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));                   // rs1
        vb_radr2 = (0x1, vb_instr(4, 2));                   // rs2
        vb_imm(7, 3) = (vb_instr(6, 5), vb_instr[12], vb_instr(11, 10));
        break;
    case OPCODE_C_BNEZ:
        vb_isa_type[ISA_SB_type] = 1;
        vb_dec[Instr_BNE] = 1;
        vb_radr1 = (0x1, vb_instr(9, 7));                   // rs1
        vb_imm(7, 1) = (vb_instr(6, 5), vb_instr[2], vb_instr(11, 10), vb_instr(4, 3));
        if (vb_instr[12] == 1) {
            vb_imm((RISCV_ARCH - 1), 8) = ~0ull;
        }
        break;
    case OPCODE_C_SDSP:
        vb_isa_type[ISA_S_type] = 1;
        vb_dec[Instr_SD] = 1;
        vb_radr1 = 0x2;                                     // rs1 = sp
        vb_radr2 = (0, vb_instr(6, 2));                     // rs2
        vb_imm(8, 3) = (vb_instr(9, 7), vb_instr[12], vb_instr(11, 10));
        break;
    default:
        v_error = 1;
        break;
    }

    v_memop_store = (vb_dec[Instr_SD] || vb_dec[Instr_SW]);
    v_memop_load = (vb_dec[Instr_LD] || vb_dec[Instr_LW]);
    v_memop_sign_ext = (vb_dec[Instr_LD] || vb_dec[Instr_LW]);
    if ((vb_dec[Instr_LD] || vb_dec[Instr_SD]) == 1) {
        vb_memop_size = MEMOP_8B;
    } else if ((vb_dec[Instr_LW] || vb_dec[Instr_SW]) == 1) {
        vb_memop_size = MEMOP_4B;
    } else {
        vb_memop_size = MEMOP_8B;
    }
    v_rv32 = (vb_dec[Instr_ADDW] || vb_dec[Instr_ADDIW] || vb_dec[Instr_SUBW]);

    v.pc = i_f_pc;
    v.isa_type = vb_isa_type;
    v.instr_vec = vb_dec;
    v.instr = i_f_instr.read()(15, 0);
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.rv32 = v_rv32;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_page_fault_x = i_instr_page_fault_x;
    v.instr_unimplemented = v_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena;

    if ((!async_reset_ && i_nrst.read() == 0) || (i_flush_pipeline.read() == 1)) {
        DecoderRvc_r_reset(v);
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
    o_instr_page_fault_x = r.instr_page_fault_x;
    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = 0;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;
}

void DecoderRvc::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        DecoderRvc_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

