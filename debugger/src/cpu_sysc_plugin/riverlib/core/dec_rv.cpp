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

#include "dec_rv.h"
#include "api_core.h"

namespace debugger {

DecoderRv::DecoderRv(sc_module_name name,
                     bool async_reset,
                     bool fpu_ena)
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
    fpu_ena_ = fpu_ena;

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
    sensitive << r.unsigned_op;
    sensitive << r.rv32;
    sensitive << r.f64;
    sensitive << r.compressed;
    sensitive << r.amo;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_page_fault_x;
    sensitive << r.instr_unimplemented;
    sensitive << r.radr1;
    sensitive << r.radr2;
    sensitive << r.waddr;
    sensitive << r.csr_addr;
    sensitive << r.imm;
    sensitive << r.progbuf_ena;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void DecoderRv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
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
        sc_trace(o_vcd, r.unsigned_op, pn + ".r_unsigned_op");
        sc_trace(o_vcd, r.rv32, pn + ".r_rv32");
        sc_trace(o_vcd, r.f64, pn + ".r_f64");
        sc_trace(o_vcd, r.compressed, pn + ".r_compressed");
        sc_trace(o_vcd, r.amo, pn + ".r_amo");
        sc_trace(o_vcd, r.instr_load_fault, pn + ".r_instr_load_fault");
        sc_trace(o_vcd, r.instr_page_fault_x, pn + ".r_instr_page_fault_x");
        sc_trace(o_vcd, r.instr_unimplemented, pn + ".r_instr_unimplemented");
        sc_trace(o_vcd, r.radr1, pn + ".r_radr1");
        sc_trace(o_vcd, r.radr2, pn + ".r_radr2");
        sc_trace(o_vcd, r.waddr, pn + ".r_waddr");
        sc_trace(o_vcd, r.csr_addr, pn + ".r_csr_addr");
        sc_trace(o_vcd, r.imm, pn + ".r_imm");
        sc_trace(o_vcd, r.progbuf_ena, pn + ".r_progbuf_ena");
    }

}

void DecoderRv::comb() {
    bool v_error;
    bool v_compressed;
    sc_uint<32> vb_instr;
    sc_uint<5> vb_opcode1;
    sc_uint<3> vb_opcode2;
    sc_biguint<Instr_Total> vb_dec;
    sc_uint<ISA_Total> vb_isa_type;
    sc_uint<6> vb_radr1;
    sc_uint<6> vb_radr2;
    sc_uint<6> vb_waddr;
    sc_uint<12> vb_csr_addr;
    sc_uint<RISCV_ARCH> vb_imm;
    bool v_memop_store;
    bool v_memop_load;
    bool v_memop_sign_ext;
    sc_uint<2> vb_memop_size;
    bool v_unsigned_op;
    bool v_rv32;
    bool v_f64;
    bool v_amo;

    v_error = 0;
    v_compressed = 0;
    vb_instr = 0;
    vb_opcode1 = 0;
    vb_opcode2 = 0;
    vb_dec = 0;
    vb_isa_type = 0;
    vb_radr1 = 0;
    vb_radr2 = 0;
    vb_waddr = 0;
    vb_csr_addr = 0;
    vb_imm = 0;
    v_memop_store = 0;
    v_memop_load = 0;
    v_memop_sign_ext = 0;
    vb_memop_size = 0;
    v_unsigned_op = 0;
    v_rv32 = 0;
    v_f64 = 0;
    v_amo = 0;

    v = r;

    vb_instr = i_f_instr;

    if (vb_instr(1, 0) != 0x3) {
        v_compressed = 1;
    }

    vb_opcode1 = vb_instr(6, 2);
    vb_opcode2 = vb_instr(14, 12);
    switch (vb_opcode1) {
    case OPCODE_AMO:
        vb_isa_type[ISA_R_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_radr2 = (0, vb_instr(24, 20));
        vb_waddr = vb_instr(11, 7);
        switch (vb_instr(31, 27)) {
        case 0x0:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOADD_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOADD_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x1:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOSWAP_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOSWAP_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x2:
            if ((vb_opcode2 == 2) && (vb_instr(24, 20).or_reduce() == 0)) {
                vb_dec[Instr_LR_W] = 1;
            } else if ((vb_opcode2 == 3) && (vb_instr(24, 20).or_reduce() == 0)) {
                vb_dec[Instr_LR_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x3:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_SC_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_SC_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x4:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOXOR_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOXOR_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x8:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOOR_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOOR_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0xC:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOAND_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOAND_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x10:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOMIN_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOMIN_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x14:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOMAX_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOMAX_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x18:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOMINU_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOMINU_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x1C:
            if (vb_opcode2 == 2) {
                vb_dec[Instr_AMOMAXU_W] = 1;
            } else if (vb_opcode2 == 3) {
                vb_dec[Instr_AMOMAXU_D] = 1;
            } else {
                v_error = 1;
            }
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_ADD:
        vb_isa_type[ISA_R_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_radr2 = (0, vb_instr(24, 20));
        vb_waddr = vb_instr(11, 7);                         // rdc
        switch (vb_opcode2) {
        case 0:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_ADD] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_MUL] = 1;
            } else if (vb_instr(31, 25) == 0x20) {
                vb_dec[Instr_SUB] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x1:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SLL] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_MULH] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x2:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SLT] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_MULHSU] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x3:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SLTU] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_MULHU] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x4:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_XOR] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_DIV] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x5:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SRL] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_DIVU] = 1;
            } else if (vb_instr(31, 25) == 0x20) {
                vb_dec[Instr_SRA] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x6:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_OR] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_REM] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x7:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_AND] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_REMU] = 1;
            } else {
                v_error = 1;
            }
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_ADDI:
        vb_isa_type[ISA_I_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm = vb_instr(31, 20);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_ADDI] = 1;
            break;
        case 0x1:
            vb_dec[Instr_SLLI] = 1;
            break;
        case 0x2:
            vb_dec[Instr_SLTI] = 1;
            break;
        case 0x3:
            vb_dec[Instr_SLTIU] = 1;
            break;
        case 0x4:
            vb_dec[Instr_XORI] = 1;
            break;
        case 0x5:
            if (vb_instr(31, 26) == 0x00) {
                vb_dec[Instr_SRLI] = 1;
            } else if (vb_instr(31, 26) == 0x10) {
                vb_dec[Instr_SRAI] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x6:
            vb_dec[Instr_ORI] = 1;
            break;
        case 7:
            vb_dec[Instr_ANDI] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_ADDIW:
        vb_isa_type[ISA_I_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm = vb_instr(31, 20);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_ADDIW] = 1;
            break;
        case 0x1:
            vb_dec[Instr_SLLIW] = 1;
            break;
        case 0x5:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SRLIW] = 1;
            } else if (vb_instr(31, 25) == 0x20) {
                vb_dec[Instr_SRAIW] = 1;
            } else {
                v_error = 1;
            }
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_ADDW:
        vb_isa_type[ISA_R_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_radr2 = (0, vb_instr(24, 20));
        vb_waddr = vb_instr(11, 7);                         // rd
        switch (vb_opcode2) {
        case 0:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_ADDW] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_MULW] = 1;
            } else if (vb_instr(31, 25) == 0x20) {
                vb_dec[Instr_SUBW] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x1:
            vb_dec[Instr_SLLW] = 1;
            break;
        case 0x4:
            if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_DIVW] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x5:
            if (vb_instr(31, 25) == 0x00) {
                vb_dec[Instr_SRLW] = 1;
            } else if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_DIVUW] = 1;
            } else if (vb_instr(31, 25) == 0x20) {
                vb_dec[Instr_SRAW] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x6:
            if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_REMW] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 0x7:
            if (vb_instr(31, 25) == 0x01) {
                vb_dec[Instr_REMUW] = 1;
            } else {
                v_error = 1;
            }
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_AUIPC:
        vb_isa_type[ISA_U_type] = 1;
        vb_dec[Instr_AUIPC] = 1;
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(31, 12) = vb_instr(31, 12);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 32) = ~0ull;
        }
        break;
    case OPCODE_BEQ:
        vb_isa_type[ISA_SB_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_radr2 = vb_instr(24, 20);
        vb_imm(11, 1) = (vb_instr[7], vb_instr(30, 25), vb_instr(11, 8));
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_BEQ] = 1;
            break;
        case 1:
            vb_dec[Instr_BNE] = 1;
            break;
        case 4:
            vb_dec[Instr_BLT] = 1;
            break;
        case 5:
            vb_dec[Instr_BGE] = 1;
            break;
        case 6:
            vb_dec[Instr_BLTU] = 1;
            break;
        case 7:
            vb_dec[Instr_BGEU] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_JAL:
        vb_isa_type[ISA_UJ_type] = 1;
        vb_dec[Instr_JAL] = 1;
        vb_waddr = (0, vb_instr(11, 7));                    // rd
        vb_imm(19, 1) = (vb_instr(19, 12), vb_instr[20], vb_instr(30, 21));
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 20) = ~0ull;
        }
        break;
    case OPCODE_JALR:
        vb_isa_type[ISA_I_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(11, 0) = vb_instr(31, 20);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_JALR] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_LB:
        vb_isa_type[ISA_I_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(11, 0) = vb_instr(31, 20);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_LB] = 1;
            break;
        case 1:
            vb_dec[Instr_LH] = 1;
            break;
        case 2:
            vb_dec[Instr_LW] = 1;
            break;
        case 3:
            vb_dec[Instr_LD] = 1;
            break;
        case 4:
            vb_dec[Instr_LBU] = 1;
            break;
        case 5:
            vb_dec[Instr_LHU] = 1;
            break;
        case 6:
            vb_dec[Instr_LWU] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_LUI:
        vb_isa_type[ISA_U_type] = 1;
        vb_dec[Instr_LUI] = 1;
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_imm(31, 12) = vb_instr(31, 12);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 32) = ~0ull;
        }
        break;
    case OPCODE_SB:
        vb_isa_type[ISA_S_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_radr2 = (0, vb_instr(24, 20));
        vb_imm(11, 0) = (vb_instr(31, 25), vb_instr(11, 7));
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_SB] = 1;
            break;
        case 1:
            vb_dec[Instr_SH] = 1;
            break;
        case 2:
            vb_dec[Instr_SW] = 1;
            break;
        case 3:
            vb_dec[Instr_SD] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_CSRR:
        vb_isa_type[ISA_I_type] = 1;
        vb_radr1 = (0, vb_instr(19, 15));
        vb_waddr = vb_instr(11, 7);                         // rd
        vb_csr_addr = vb_instr(31, 20);
        vb_imm(11, 0) = vb_instr(31, 20);
        if (vb_instr[31] == 1) {
            vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
        }
        switch (vb_opcode2) {
        case 0:
            if (vb_instr == 0x00000073) {
                vb_dec[Instr_ECALL] = 1;
            } else if (vb_instr == 0x00100073) {
                vb_dec[Instr_EBREAK] = 1;
            } else if (vb_instr == 0x00200073) {
                vb_dec[Instr_URET] = 1;
            } else if (vb_instr == 0x10200073) {
                vb_dec[Instr_SRET] = 1;
            } else if (vb_instr == 0x10500073) {
                vb_dec[Instr_WFI] = 1;
            } else if (vb_instr == 0x20200073) {
                vb_dec[Instr_HRET] = 1;
            } else if (vb_instr == 0x30200073) {
                vb_dec[Instr_MRET] = 1;
            } else if ((vb_instr(31, 25) == 0x09) && (vb_waddr.or_reduce() == 0)) {
                vb_dec[Instr_SFENCE_VMA] = 1;
            } else {
                v_error = 1;
            }
            break;
        case 1:
            vb_dec[Instr_CSRRW] = 1;
            break;
        case 2:
            vb_dec[Instr_CSRRS] = 1;
            break;
        case 3:
            vb_dec[Instr_CSRRC] = 1;
            break;
        case 5:
            vb_dec[Instr_CSRRWI] = 1;
            break;
        case 6:
            vb_dec[Instr_CSRRSI] = 1;
            break;
        case 7:
            vb_dec[Instr_CSRRCI] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    case OPCODE_FENCE:
        switch (vb_opcode2) {
        case 0:
            vb_dec[Instr_FENCE] = 1;
            break;
        case 1:
            vb_dec[Instr_FENCE_I] = 1;
            break;
        default:
            v_error = 1;
            break;
        }
        break;
    default:
        if (fpu_ena_) {
            switch (vb_opcode1) {
            case OPCODE_FPU_LD:
                vb_isa_type[ISA_I_type] = 1;
                vb_radr1 = (0, vb_instr(19, 15));
                vb_waddr = (1, vb_instr(11, 7));            // rd
                vb_imm(11, 0) = vb_instr(31, 20);
                if (vb_instr[31] == 1) {
                    vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
                }
                if (vb_opcode2 == 3) {
                    vb_dec[Instr_FLD] = 1;
                } else {
                    v_error = 1;
                }
                break;
            case OPCODE_FPU_SD:
                vb_isa_type[ISA_S_type] = 1;
                vb_radr1 = (0, vb_instr(19, 15));
                vb_radr2 = (1, vb_instr(24, 20));
                vb_imm(11, 0) = (vb_instr(31, 25), vb_instr(11, 7));
                if (vb_instr[31] == 1) {
                    vb_imm((RISCV_ARCH - 1), 12) = ~0ull;
                }
                if (vb_opcode2 == 3) {
                    vb_dec[Instr_FSD] = 1;
                } else {
                    v_error = 1;
                }
                break;
            case OPCODE_FPU_OP:
                vb_isa_type[ISA_R_type] = 1;
                vb_radr1 = (1, vb_instr(19, 15));
                vb_radr2 = (1, vb_instr(24, 20));
                vb_waddr = (1, vb_instr(11, 7));            // rd
                switch (vb_instr(31, 25)) {
                case 0x1:
                    vb_dec[Instr_FADD_D] = 1;
                    break;
                case 0x5:
                    vb_dec[Instr_FSUB_D] = 1;
                    break;
                case 0x9:
                    vb_dec[Instr_FMUL_D] = 1;
                    break;
                case 0xD:
                    vb_dec[Instr_FDIV_D] = 1;
                    break;
                case 0x15:
                    if (vb_opcode2 == 0) {
                        vb_dec[Instr_FMIN_D] = 1;
                    } else if (vb_opcode2 == 1) {
                        vb_dec[Instr_FMAX_D] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                case 0x51:
                    vb_waddr[5] = 0;
                    if (vb_opcode2 == 0) {
                        vb_dec[Instr_FLE_D] = 1;
                    } else if (vb_opcode2 == 1) {
                        vb_dec[Instr_FLT_D] = 1;
                    } else if (vb_opcode2 == 2) {
                        vb_dec[Instr_FEQ_D] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                case 0x61:
                    vb_waddr[5] = 0;
                    if (vb_instr(24, 20) == 0) {
                        vb_dec[Instr_FCVT_W_D] = 1;
                    } else if (vb_instr(24, 20) == 1) {
                        vb_dec[Instr_FCVT_WU_D] = 1;
                    } else if (vb_instr(24, 20) == 2) {
                        vb_dec[Instr_FCVT_L_D] = 1;
                    } else if (vb_instr(24, 20) == 3) {
                        vb_dec[Instr_FCVT_LU_D] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                case 0x69:
                    vb_radr1[5] = 0;
                    if (vb_instr(24, 20) == 0) {
                        vb_dec[Instr_FCVT_D_W] = 1;
                    } else if (vb_instr(24, 20) == 1) {
                        vb_dec[Instr_FCVT_D_WU] = 1;
                    } else if (vb_instr(24, 20) == 2) {
                        vb_dec[Instr_FCVT_D_L] = 1;
                    } else if (vb_instr(24, 20) == 3) {
                        vb_dec[Instr_FCVT_D_LU] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                case 0x71:
                    vb_waddr[5] = 0;
                    if ((vb_instr(24, 20).or_reduce() == 0) && (vb_opcode2.or_reduce() == 0)) {
                        vb_dec[Instr_FMOV_X_D] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                case 0x79:
                    vb_radr1[5] = 0;
                    if ((vb_instr(24, 20).or_reduce() == 0) && (vb_opcode2.or_reduce() == 0)) {
                        vb_dec[Instr_FMOV_D_X] = 1;
                    } else {
                        v_error = 1;
                    }
                    break;
                default:
                    v_error = 1;
                    break;
                }
                break;
            default:
                v_error = 1;
                break;
            }
        } else {
            // FPU disabled
            v_error = 1;
        }
        break;
    }

    v_amo = (vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]);

    v_memop_store = (vb_dec[Instr_SD]
            || vb_dec[Instr_SW]
            || vb_dec[Instr_SH]
            || vb_dec[Instr_SB]
            || vb_dec[Instr_FSD]
            || vb_dec[Instr_SC_W]
            || vb_dec[Instr_SC_D]);

    v_memop_load = (vb_dec[Instr_LD]
            || vb_dec[Instr_LW]
            || vb_dec[Instr_LH]
            || vb_dec[Instr_LB]
            || vb_dec[Instr_LWU]
            || vb_dec[Instr_LHU]
            || vb_dec[Instr_LBU]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]
            || vb_dec[Instr_LR_D]);

    v_memop_sign_ext = (vb_dec[Instr_LD]
            || vb_dec[Instr_LW]
            || vb_dec[Instr_LH]
            || vb_dec[Instr_LB]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]);

    v_f64 = (vb_dec[Instr_FADD_D]
            || vb_dec[Instr_FSUB_D]
            || vb_dec[Instr_FMUL_D]
            || vb_dec[Instr_FDIV_D]
            || vb_dec[Instr_FMIN_D]
            || vb_dec[Instr_FMAX_D]
            || vb_dec[Instr_FLE_D]
            || vb_dec[Instr_FLT_D]
            || vb_dec[Instr_FEQ_D]
            || vb_dec[Instr_FCVT_W_D]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_L_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_FMOV_X_D]
            || vb_dec[Instr_FCVT_D_W]
            || vb_dec[Instr_FCVT_D_WU]
            || vb_dec[Instr_FCVT_D_L]
            || vb_dec[Instr_FCVT_D_LU]
            || vb_dec[Instr_FMOV_D_X]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]);

    if ((vb_dec[Instr_LD]
            || vb_dec[Instr_SD]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]
            || vb_dec[Instr_LR_D]
            || vb_dec[Instr_SC_D]) == 1) {
        vb_memop_size = MEMOP_8B;
    } else if ((vb_dec[Instr_LW]
                || vb_dec[Instr_LWU]
                || vb_dec[Instr_SW]
                || vb_dec[Instr_AMOADD_W]
                || vb_dec[Instr_AMOXOR_W]
                || vb_dec[Instr_AMOOR_W]
                || vb_dec[Instr_AMOAND_W]
                || vb_dec[Instr_AMOMIN_W]
                || vb_dec[Instr_AMOMAX_W]
                || vb_dec[Instr_AMOMINU_W]
                || vb_dec[Instr_AMOMAXU_W]
                || vb_dec[Instr_AMOSWAP_W]
                || vb_dec[Instr_LR_W]
                || vb_dec[Instr_SC_W]) == 1) {
        vb_memop_size = MEMOP_4B;
    } else if ((vb_dec[Instr_LH] || vb_dec[Instr_LHU] || vb_dec[Instr_SH]) == 1) {
        vb_memop_size = MEMOP_2B;
    } else {
        vb_memop_size = MEMOP_1B;
    }

    v_unsigned_op = (vb_dec[Instr_DIVU]
            || vb_dec[Instr_REMU]
            || vb_dec[Instr_DIVUW]
            || vb_dec[Instr_REMUW]
            || vb_dec[Instr_MULHU]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]);

    v_rv32 = (vb_dec[Instr_ADDW]
            || vb_dec[Instr_ADDIW]
            || vb_dec[Instr_SLLW]
            || vb_dec[Instr_SLLIW]
            || vb_dec[Instr_SRAW]
            || vb_dec[Instr_SRAIW]
            || vb_dec[Instr_SRLW]
            || vb_dec[Instr_SRLIW]
            || vb_dec[Instr_SUBW]
            || vb_dec[Instr_DIVW]
            || vb_dec[Instr_DIVUW]
            || vb_dec[Instr_MULW]
            || vb_dec[Instr_REMW]
            || vb_dec[Instr_REMUW]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]
            || vb_dec[Instr_SC_W]);

    v_f64 = (vb_dec[Instr_FADD_D]
            || vb_dec[Instr_FSUB_D]
            || vb_dec[Instr_FMUL_D]
            || vb_dec[Instr_FDIV_D]
            || vb_dec[Instr_FMIN_D]
            || vb_dec[Instr_FMAX_D]
            || vb_dec[Instr_FLE_D]
            || vb_dec[Instr_FLT_D]
            || vb_dec[Instr_FEQ_D]
            || vb_dec[Instr_FCVT_W_D]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_L_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_FMOV_X_D]
            || vb_dec[Instr_FCVT_D_W]
            || vb_dec[Instr_FCVT_D_WU]
            || vb_dec[Instr_FCVT_D_L]
            || vb_dec[Instr_FCVT_D_LU]
            || vb_dec[Instr_FMOV_D_X]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]);

    v.pc = i_f_pc;
    v.isa_type = vb_isa_type;
    v.instr_vec = vb_dec;
    v.instr = i_f_instr;
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.unsigned_op = v_unsigned_op;
    v.rv32 = v_rv32;
    v.f64 = v_f64;
    v.compressed = v_compressed;
    v.amo = v_amo;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_page_fault_x = i_instr_page_fault_x;
    v.instr_unimplemented = v_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.csr_addr = vb_csr_addr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena;

    if ((!async_reset_ && i_nrst.read() == 0) || (i_flush_pipeline.read() == 1)) {
        DecoderRv_r_reset(v);
    }

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
    o_amo = r.amo;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_page_fault_x = r.instr_page_fault_x;
    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = r.csr_addr;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;
}

void DecoderRv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        DecoderRv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

