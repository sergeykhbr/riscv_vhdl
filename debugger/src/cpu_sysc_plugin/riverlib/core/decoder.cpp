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

    bool w_error = false;
    sc_uint<5> wb_opcode1 = i_f_instr.read().range(6, 2);
    sc_uint<3> wb_opcode2 = i_f_instr.read().range(14, 12);

    if (i_f_instr.read().range(1, 0) != 0x3) {
        w_error = true;
    }

    bool w_user_level = 0;
    bool w_priv_level = 0;
    bool w_sign_ext = 0;
    sc_bv<Instr_Total> wb_instr_vec = 0;
    sc_bv<ISA_Total> wb_isa_type = 0;

    switch (wb_opcode1) {
    // User level:
    case OPCODE_ADD:
        w_user_level = 1;
        wb_isa_type[ISA_R_type] = 1;
        switch (wb_opcode2) {
        case 0:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_ADD] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_MUL] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SUB] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x1:
            wb_instr_vec[Instr_SLL] = 1;
            break;
        case 0x2:
            wb_instr_vec[Instr_SLT] = 1;
            break;
        case 0x3:
            wb_instr_vec[Instr_SLTU] = 1;
            break;
        case 0x4:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_XOR] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_DIV] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x5:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_SRL] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_DIVU] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SRA] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x6:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_OR] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_REM] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x7:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_AND] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_REMU] = 1;
            } else {
                w_error = true;
            }
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_ADDI:
        w_user_level = 1;
        wb_isa_type[ISA_I_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_ADDI] = 1;
            break;
        case 0x1:
            wb_instr_vec[Instr_SLLI] = 1;
            break;
        case 0x2:
            wb_instr_vec[Instr_SLTI] = 1;
            break;
        case 0x3:
            wb_instr_vec[Instr_SLTIU] = 1;
            break;
        case 0x4:
            wb_instr_vec[Instr_XORI] = 1;
            break;
        case 0x5:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_SRLI] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SRAI] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x6:
            wb_instr_vec[Instr_ORI] = 1;
            break;
        case 7:
            wb_instr_vec[Instr_ANDI] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_ADDIW:
        w_user_level = 1;
        wb_isa_type[ISA_I_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_ADDIW] = 1;
            break;
        case 0x1:
            wb_instr_vec[Instr_SLLIW] = 1;
            break;
        case 0x5:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_SRLIW] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SRAIW] = 1;
            } else {
                w_error = true;
            }
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_ADDW:
        w_user_level = 1;
        wb_isa_type[ISA_R_type] = 1;
        switch (wb_opcode2) {
        case 0:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_ADDW] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_MULW] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SUBW] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x1:
            wb_instr_vec[Instr_SLLW] = 1;
            break;
        case 0x4:
            if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_DIVW] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x5:
            if (i_f_instr.read().range(31, 25) == 0x00) {
                wb_instr_vec[Instr_SRLW] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_DIVUW] = 1;
            } else if (i_f_instr.read().range(31, 25) == 0x20) {
                wb_instr_vec[Instr_SRAW] = 1;
            } else {
                w_error = true;
            }
            break;
        case 0x6:
            if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_REMW] = 1;
            } else {
                w_error = true;
            }
        case 0x7:
            if (i_f_instr.read().range(31, 25) == 0x01) {
                wb_instr_vec[Instr_REMUW] = 1;
            } else {
                w_error = true;
            }
        default:
            w_error = true;
        }
        break;
    case OPCODE_AUIPC:
        w_user_level = 1;
        wb_isa_type[ISA_U_type] = 1;
        wb_instr_vec[Instr_AUIPC] = 1;
        break;
    case OPCODE_BEQ:
        w_user_level = 1;
        wb_isa_type[ISA_SB_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_BEQ] = 1;
            break;
        case 1:
            wb_instr_vec[Instr_BNE] = 1;
            break;
        case 4:
            wb_instr_vec[Instr_BLT] = 1;
            break;
        case 5:
            wb_instr_vec[Instr_BGE] = 1;
            break;
        case 6:
            wb_instr_vec[Instr_BLTU] = 1;
            break;
        case 7:
            wb_instr_vec[Instr_BGEU] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_JAL:
        w_user_level = 1;
        wb_isa_type[ISA_UJ_type] = 1;
        wb_instr_vec[Instr_JAL] = 1;
        break;
    case OPCODE_JALR:
        w_user_level = 1;
        wb_isa_type[ISA_I_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_JALR] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_LB:
        w_user_level = 1;
        wb_isa_type[ISA_I_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_LB] = 1;
            break;
        case 1:
            wb_instr_vec[Instr_LH] = 1;
            break;
        case 2:
            wb_instr_vec[Instr_LW] = 1;
            break;
        case 3:
            wb_instr_vec[Instr_LD] = 1;
            break;
        case 4:
            wb_instr_vec[Instr_LBU] = 1;
            break;
        case 5:
            wb_instr_vec[Instr_LHU] = 1;
            break;
        case 6:
            wb_instr_vec[Instr_LWU] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_LUI:
        w_user_level = 1;
        wb_isa_type[ISA_U_type] = 1;
        wb_instr_vec[Instr_LUI] = 1;
        break;
    case OPCODE_SB:
        w_user_level = 1;
        wb_isa_type[ISA_S_type] = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_SB] = 1;
            break;
        case 1:
            wb_instr_vec[Instr_SH] = 1;
            break;
        case 2:
            wb_instr_vec[Instr_SW] = 1;
            break;
        case 3:
            wb_instr_vec[Instr_SD] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_CSRR:
        w_priv_level = 1;
        wb_isa_type[ISA_I_type] = 1;
        switch (wb_opcode2) {
        case 0:
            if (i_f_instr.read() == 0x00200073) {
                wb_instr_vec[Instr_URET] = 1;
            } else if (i_f_instr.read() == 0x10200073) {
                wb_instr_vec[Instr_SRET] = 1;
            } else if (i_f_instr.read() == 0x20200073) {
                wb_instr_vec[Instr_HRET] = 1;
            } else if (i_f_instr.read() == 0x30200073) {
                wb_instr_vec[Instr_MRET] = 1;
            } else {
                w_error = true;
            }
            break;
        case 1:
            wb_instr_vec[Instr_CSRRW] = 1;
            break;
        case 2:
            wb_instr_vec[Instr_CSRRS] = 1;
            break;
        case 3:
            wb_instr_vec[Instr_CSRRC] = 1;
            break;
        case 5:
            wb_instr_vec[Instr_CSRRWI] = 1;
            break;
        case 6:
            wb_instr_vec[Instr_CSRRSI] = 1;
            break;
        case 7:
            wb_instr_vec[Instr_CSRRCI] = 1;
            break;
        default:
            w_error = true;
        }
        break;
    case OPCODE_FENCE:
        w_user_level = 1;
        switch (wb_opcode2) {
        case 0:
            wb_instr_vec[Instr_FENCE] = 1;
            break;
        case 1:
            wb_instr_vec[Instr_FENCE_I] = 1;
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
        v.instr = i_f_instr;

        v.sign_ext = w_sign_ext;
        v.user_level = w_user_level;
        v.priv_level = w_priv_level;
        v.isa_type = wb_isa_type;
        v.instr_vec = wb_instr_vec;
        
        v.instr_unimplemented = w_error;
    } else if (!i_any_hold.read()) {
        v.valid = 0;
    }

    if (!i_nrst.read()) {
        v.valid = false;
        v.pc = 0;
        v.isa_type = 0;
        v.instr_vec = 0;

        v.user_level = 0;
        v.priv_level = 0;
        v.instr_unimplemented = false;
    }

    o_valid = r.valid.read() && !i_any_hold.read();
    o_pc = r.pc;
    o_instr = r.instr;
    o_sign_ext = r.sign_ext;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_user_level = r.user_level;
    o_priv_level = r.priv_level;
    o_exception = r.instr_unimplemented;
}

void InstrDecoder::registers() {
    r = v;
}

}  // namespace debugger

