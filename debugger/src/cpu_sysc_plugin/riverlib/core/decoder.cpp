/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Decoder stage.
 */

#include "decoder.h"

namespace debugger {

InstrDecoder::InstrDecoder(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_any_hold;
    sensitive << i_f_valid;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << r.valid;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, o_valid, "/top/proc0/dec0/o_valid");
        sc_trace(vcd, o_pc, "/top/proc0/dec0/o_pc");
        sc_trace(vcd, o_instr, "/top/proc0/dec0/o_instr");
        sc_trace(vcd, o_isa_type, "/top/proc0/dec0/o_isa_type");
        sc_trace(vcd, o_instr_vec, "/top/proc0/dec0/o_instr_vec");
        sc_trace(vcd, o_exception, "/top/proc0/dec0/o_exception");
    }
};


void InstrDecoder::comb() {
    v = r;

    bool w_o_valid;
    bool w_error = false;
    sc_uint<32> wb_instr = i_f_instr.read();
    sc_uint<5> wb_opcode1 = wb_instr(6, 2);
    sc_uint<3> wb_opcode2 = wb_instr(14, 12);
    sc_bv<Instr_Total> wb_dec = 0;
    sc_bv<ISA_Total> wb_isa_type = 0;

    if (wb_instr(1, 0) != 0x3) {
        w_error = true;
    }

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
            } else if (wb_instr(31, 26) == 0x20) {
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
            if (wb_instr == 0x00200073) {
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
        w_error = true;
    }


    if (i_f_valid.read()) {
        v.valid = 1;
        v.pc = i_f_pc;
        v.instr = wb_instr;

        v.isa_type = wb_isa_type;
        v.instr_vec = wb_dec;
        v.memop_store = (wb_dec[Instr_SD] | wb_dec[Instr_SW] 
                | wb_dec[Instr_SH] | wb_dec[Instr_SB]).to_bool();
        v.memop_load = (wb_dec[Instr_LD] | wb_dec[Instr_LW] 
                | wb_dec[Instr_LH] | wb_dec[Instr_LB]
                | wb_dec[Instr_LWU] | wb_dec[Instr_LHU] 
                | wb_dec[Instr_LBU]).to_bool();
        v.memop_sign_ext = (wb_dec[Instr_LD] | wb_dec[Instr_LW]
                | wb_dec[Instr_LH] | wb_dec[Instr_LB]).to_bool();
        if (wb_dec[Instr_LD] || wb_dec[Instr_SD]) {
            v.memop_size = MEMOP_8B;
        } else if (wb_dec[Instr_LW] || wb_dec[Instr_LWU] || wb_dec[Instr_SW]) {
            v.memop_size = MEMOP_4B;
        } else if (wb_dec[Instr_LH] || wb_dec[Instr_LHU] || wb_dec[Instr_SH]) {
            v.memop_size = MEMOP_2B;
        } else {
            v.memop_size = MEMOP_1B;
        }
        v.unsigned_op = (wb_dec[Instr_DIVU] | wb_dec[Instr_REMU] |
                wb_dec[Instr_DIVUW] | wb_dec[Instr_REMUW]).to_bool();

        v.rv32 = (wb_dec[Instr_ADDW] | wb_dec[Instr_ADDIW] 
            | wb_dec[Instr_SLLW] | wb_dec[Instr_SLLIW] | wb_dec[Instr_SRAW]
            | wb_dec[Instr_SRAIW]
            | wb_dec[Instr_SRLW] | wb_dec[Instr_SRLIW] | wb_dec[Instr_SUBW] 
            | wb_dec[Instr_DIVW] | wb_dec[Instr_DIVUW] | wb_dec[Instr_MULW]
            | wb_dec[Instr_REMW] | wb_dec[Instr_REMUW]).to_bool();
        
        v.instr_unimplemented = w_error;
    } else if (!i_any_hold.read()) {
        v.valid = 0;
    }
    w_o_valid = r.valid.read() && !i_any_hold.read();

    if (!i_nrst.read()) {
        v.valid = false;
        v.pc = 0;
        v.instr = 0;
        v.isa_type = 0;
        v.instr_vec = 0;
        v.memop_store = 0;
        v.memop_load = 0;
        v.memop_sign_ext = 0;
        v.memop_size = MEMOP_1B;
        v.unsigned_op = 0;
        v.rv32 = 0;

        v.instr_unimplemented = !wb_dec.or_reduce();
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
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
}

void InstrDecoder::registers() {
    r = v;
}

}  // namespace debugger

