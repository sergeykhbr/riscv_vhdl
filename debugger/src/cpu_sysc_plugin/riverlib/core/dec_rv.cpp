/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "dec_rv.h"

namespace debugger {

DecoderRv::DecoderRv(sc_module_name name_, bool async_reset,
    bool fpu_ena) : sc_module(name_),
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
    fpu_ena_ = fpu_ena;

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
    sensitive << r.unsigned_op;
    sensitive << r.rv32;
    sensitive << r.f64;
    sensitive << r.compressed;
    sensitive << r.amo;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;
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
};

void DecoderRv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
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

void DecoderRv::comb() {
    bool w_error;
    bool w_compressed;
    sc_uint<32> wb_instr;
    sc_uint<32> wb_instr_out;
    sc_uint<5> wb_opcode1;
    sc_uint<3> wb_opcode2;
    sc_bv<Instr_Total> wb_dec;
    sc_bv<ISA_Total> wb_isa_type;
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

    v = r;

    w_error = false;
    w_compressed = false;
    wb_instr = i_f_instr.read();
    wb_dec = 0;
    wb_isa_type = 0;
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

    if (wb_instr(1, 0) != 0x3) {
        w_compressed = true;
    } else {
        wb_opcode1 = wb_instr(6, 2);
        wb_opcode2 = wb_instr(14, 12);
        switch (wb_opcode1) {
        case OPCODE_AMO:
            wb_isa_type[ISA_R_type] = 1;
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_radr2 = (0, wb_instr.range(24, 20));
            vb_waddr = wb_instr(11, 7);
            switch (wb_instr(31, 27)) {
            case 0x0:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOADD_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOADD_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x1:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOSWAP_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOSWAP_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x2:
                if (wb_opcode2 == 2 && !wb_instr(24, 20).or_reduce()) {
                    wb_dec[Instr_LR_W] = 1;
                } else if (wb_opcode2 == 3 && !wb_instr(24, 20).or_reduce()) {
                    wb_dec[Instr_LR_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x3:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_SC_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_SC_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x4:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOXOR_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOXOR_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x8:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOOR_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOOR_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0xC:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOAND_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOAND_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x10:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOMIN_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOMIN_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x14:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOMAX_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOMAX_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x18:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOMINU_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOMINU_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x1C:
                if (wb_opcode2 == 2) {
                    wb_dec[Instr_AMOMAXU_W] = 1;
                } else if (wb_opcode2 == 3) {
                    wb_dec[Instr_AMOMAXU_D] = 1;
                } else {
                    w_error = true;
                }
                break;
            default:
                w_error = true;
            }
            break;
        case OPCODE_ADD:
            wb_isa_type[ISA_R_type] = 1;
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_radr2 = (0, wb_instr.range(24, 20));
            vb_waddr = wb_instr(11, 7);             // rdc
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
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SLL] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_MULH] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x2:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SLT] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_MULHSU] = 1;
                } else {
                    w_error = true;
                }
                break;
            case 0x3:
                if (wb_instr(31, 25) == 0x00) {
                    wb_dec[Instr_SLTU] = 1;
                } else if (wb_instr(31, 25) == 0x01) {
                    wb_dec[Instr_MULHU] = 1;
                } else {
                    w_error = true;
                }
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
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm = wb_instr.range(31, 20);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm = wb_instr.range(31, 20);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_radr2 = (0, wb_instr.range(24, 20));
            vb_waddr = wb_instr(11, 7);             // rd
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
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm(31, 12) = wb_instr(31, 12);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 32) = ~0ull;
            }
            break;
        case OPCODE_BEQ:
            wb_isa_type[ISA_SB_type] = 1;
            vb_radr1 = wb_instr(19, 15);
            vb_radr2 = wb_instr(24, 20);
            vb_imm(11, 1) = (wb_instr[7], wb_instr(30, 25), wb_instr(11, 8));
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm(19, 1) = (wb_instr(19, 12), wb_instr[20], wb_instr(30, 21));
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 20) = ~0ull;
            }
            break;
        case OPCODE_JALR:
            wb_isa_type[ISA_I_type] = 1;
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm(11, 0) = wb_instr.range(31, 20);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm(11, 0) = wb_instr.range(31, 20);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_waddr = wb_instr(11, 7);             // rd
            vb_imm(31, 12) = wb_instr(31, 12);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 32) = ~0ull;
            }
            break;
        case OPCODE_SB:
            wb_isa_type[ISA_S_type] = 1;
            vb_radr1 = wb_instr(19, 15);
            vb_radr2 = wb_instr(24, 20);
            vb_imm(11, 0) = (wb_instr(31, 25), wb_instr(11, 7));
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
            vb_radr1 = (0, wb_instr.range(19, 15));
            vb_waddr = wb_instr(11, 7);             // rd
            vb_csr_addr = wb_instr(31, 20);
            vb_imm(11, 0) = wb_instr.range(31, 20);
            if (wb_instr[31] == 1) {
                vb_imm(RISCV_ARCH-1, 12) = ~0ull;
            }
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
                } else if (wb_instr == 0x10500073) {
                    wb_dec[Instr_WFI] = 1;
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
            if (fpu_ena_) {
                switch (wb_opcode1) {
                case OPCODE_FPU_LD:
                    wb_isa_type[ISA_I_type] = 1;
                    vb_radr1 = (0, wb_instr.range(19, 15));
                    vb_waddr = (1, wb_instr(11, 7));             // rd
                    vb_imm(11, 0) = wb_instr.range(31, 20);
                    if (wb_instr[31] == 1) {
                        vb_imm(RISCV_ARCH-1, 12) = ~0ull;
                    }
                    if (wb_opcode2 == 3) {
                        wb_dec[Instr_FLD] = 1;
                    } else {
                        w_error = true;
                    }
                    break;
                case OPCODE_FPU_SD:
                    wb_isa_type[ISA_S_type] = 1;
                    vb_radr1 = (0, wb_instr(19, 15));
                    vb_radr2 = (1, wb_instr(24, 20));
                    vb_imm(11, 0) = (wb_instr(31, 25), wb_instr(11, 7));
                    if (wb_instr[31] == 1) {
                        vb_imm(RISCV_ARCH-1, 12) = ~0ull;
                    }
                    if (wb_opcode2 == 3) {
                        wb_dec[Instr_FSD] = 1;
                    } else {
                        w_error = true;
                    }
                    break;
                case OPCODE_FPU_OP:
                    wb_isa_type[ISA_R_type] = 1;
                    vb_radr1 = (1, wb_instr.range(19, 15));
                    vb_radr2 = (1, wb_instr.range(24, 20));
                    vb_waddr = (1, wb_instr(11, 7));             // rd
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
                        vb_waddr[5] = 0;
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
                        vb_waddr[5] = 0;
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
                        vb_radr1[5] = 0;
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
                        vb_waddr[5] = 0;
                        if (wb_instr(24, 20) == 0 && wb_opcode2 == 0) {
                            wb_dec[Instr_FMOV_X_D] = 1;
                        } else {
                            w_error = true;
                        }
                        break;
                    case 0x79:
                        vb_radr1[5] = 0;
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


    v_amo = (wb_dec[Instr_AMOADD_W] | wb_dec[Instr_AMOXOR_W] | wb_dec[Instr_AMOOR_W]
            | wb_dec[Instr_AMOAND_W] | wb_dec[Instr_AMOMIN_W] | wb_dec[Instr_AMOMAX_W]
            | wb_dec[Instr_AMOMINU_W] | wb_dec[Instr_AMOMAXU_W] | wb_dec[Instr_AMOSWAP_W]
            | wb_dec[Instr_AMOADD_D] | wb_dec[Instr_AMOXOR_D] | wb_dec[Instr_AMOOR_D]
            | wb_dec[Instr_AMOAND_D] | wb_dec[Instr_AMOMIN_D] | wb_dec[Instr_AMOMAX_D]
            | wb_dec[Instr_AMOMINU_D] | wb_dec[Instr_AMOMAXU_D] | wb_dec[Instr_AMOSWAP_D]).to_bool();
    v_memop_store = (wb_dec[Instr_SD] | wb_dec[Instr_SW] 
            | wb_dec[Instr_SH] | wb_dec[Instr_SB]
            | wb_dec[Instr_FSD]
            | wb_dec[Instr_SC_W] | wb_dec[Instr_SC_D]).to_bool();
    v_memop_load = (wb_dec[Instr_LD] | wb_dec[Instr_LW] 
            | wb_dec[Instr_LH] | wb_dec[Instr_LB]
            | wb_dec[Instr_LWU] | wb_dec[Instr_LHU] 
            | wb_dec[Instr_LBU]
            | wb_dec[Instr_FLD]
            | wb_dec[Instr_AMOADD_W] | wb_dec[Instr_AMOXOR_W] | wb_dec[Instr_AMOOR_W]
            | wb_dec[Instr_AMOAND_W] | wb_dec[Instr_AMOMIN_W] | wb_dec[Instr_AMOMAX_W]
            | wb_dec[Instr_AMOMINU_W] | wb_dec[Instr_AMOMAXU_W] | wb_dec[Instr_AMOSWAP_W]
            | wb_dec[Instr_LR_W]
            | wb_dec[Instr_AMOADD_D] | wb_dec[Instr_AMOXOR_D] | wb_dec[Instr_AMOOR_D]
            | wb_dec[Instr_AMOAND_D] | wb_dec[Instr_AMOMIN_D] | wb_dec[Instr_AMOMAX_D]
            | wb_dec[Instr_AMOMINU_D] | wb_dec[Instr_AMOMAXU_D] | wb_dec[Instr_AMOSWAP_D]
            | wb_dec[Instr_LR_D]).to_bool();
    v_memop_sign_ext = (wb_dec[Instr_LD] | wb_dec[Instr_LW]
            | wb_dec[Instr_LH] | wb_dec[Instr_LB]
            | wb_dec[Instr_AMOADD_W] | wb_dec[Instr_AMOXOR_W] | wb_dec[Instr_AMOOR_W]
            | wb_dec[Instr_AMOAND_W] | wb_dec[Instr_AMOMIN_W] | wb_dec[Instr_AMOMAX_W]
            | wb_dec[Instr_AMOMINU_W] | wb_dec[Instr_AMOMAXU_W] | wb_dec[Instr_AMOSWAP_W]
            | wb_dec[Instr_LR_W]).to_bool();
    if (wb_dec[Instr_LD] || wb_dec[Instr_SD] ||
        wb_dec[Instr_FLD] || wb_dec[Instr_FSD] ||
        wb_dec[Instr_AMOADD_D] || wb_dec[Instr_AMOXOR_D] || wb_dec[Instr_AMOOR_D] ||
        wb_dec[Instr_AMOAND_D] || wb_dec[Instr_AMOMIN_D] || wb_dec[Instr_AMOMAX_D] ||
        wb_dec[Instr_AMOMINU_D] || wb_dec[Instr_AMOMAXU_D] || wb_dec[Instr_AMOSWAP_D] ||
        wb_dec[Instr_LR_D] || wb_dec[Instr_SC_D]) {
        vb_memop_size = MEMOP_8B;
    } else if (wb_dec[Instr_LW] || wb_dec[Instr_LWU] || wb_dec[Instr_SW] ||
        wb_dec[Instr_AMOADD_W] || wb_dec[Instr_AMOXOR_W] || wb_dec[Instr_AMOOR_W] ||
        wb_dec[Instr_AMOAND_W] || wb_dec[Instr_AMOMIN_W] || wb_dec[Instr_AMOMAX_W] ||
        wb_dec[Instr_AMOMINU_W] || wb_dec[Instr_AMOMAXU_W] || wb_dec[Instr_AMOSWAP_W] ||
        wb_dec[Instr_LR_W] || wb_dec[Instr_SC_W]) {
        vb_memop_size = MEMOP_4B;
    } else if (wb_dec[Instr_LH] || wb_dec[Instr_LHU] || wb_dec[Instr_SH]) {
        vb_memop_size = MEMOP_2B;
    } else {
        vb_memop_size = MEMOP_1B;
    }
    v_unsigned_op = (wb_dec[Instr_DIVU] | wb_dec[Instr_REMU] |
            wb_dec[Instr_DIVUW] | wb_dec[Instr_REMUW] |
            wb_dec[Instr_MULHU] |
            wb_dec[Instr_FCVT_WU_D] | wb_dec[Instr_FCVT_LU_D] |
            wb_dec[Instr_AMOMINU_W] | wb_dec[Instr_AMOMAXU_W] |
            wb_dec[Instr_AMOMINU_D] | wb_dec[Instr_AMOMAXU_D]).to_bool();

    v_rv32 = (wb_dec[Instr_ADDW] | wb_dec[Instr_ADDIW] 
        | wb_dec[Instr_SLLW] | wb_dec[Instr_SLLIW] | wb_dec[Instr_SRAW]
        | wb_dec[Instr_SRAIW]
        | wb_dec[Instr_SRLW] | wb_dec[Instr_SRLIW] | wb_dec[Instr_SUBW] 
        | wb_dec[Instr_DIVW] | wb_dec[Instr_DIVUW] | wb_dec[Instr_MULW]
        | wb_dec[Instr_REMW] | wb_dec[Instr_REMUW]
        | wb_dec[Instr_AMOADD_W]| wb_dec[Instr_AMOXOR_W] | wb_dec[Instr_AMOOR_W]
        | wb_dec[Instr_AMOAND_W] | wb_dec[Instr_AMOMIN_W]| wb_dec[Instr_AMOMAX_W]
        | wb_dec[Instr_AMOMINU_W] | wb_dec[Instr_AMOMAXU_W]| wb_dec[Instr_AMOSWAP_W]
        | wb_dec[Instr_LR_W] | wb_dec[Instr_SC_W]).to_bool();

    v_f64 = (wb_dec[Instr_FADD_D] | wb_dec[Instr_FSUB_D]
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
        

    v.pc = i_f_pc;
    v.isa_type = wb_isa_type;
    v.instr_vec = wb_dec;
    v.instr = i_f_instr;
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.unsigned_op = v_unsigned_op;
    v.rv32 = v_rv32;
    v.f64 = v_f64;
    v.compressed = w_compressed;
    v.amo = v_amo;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_executable = i_instr_executable;
    v.instr_unimplemented = w_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.csr_addr = vb_csr_addr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena.read();


    if ((!async_reset_ && !i_nrst.read()) || i_flush_pipeline.read() == 1) {
        R_RESET(v);
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
    o_instr_executable = r.instr_executable;

    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = r.csr_addr;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;
}

void DecoderRv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

