/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
 *
 * @brief      RISC-V extension-F (Floating-point Instructions).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

/**
 * @brief The FADD.D double precision adder
 */
class FADD_D : public RiscvInstruction {
 public:
    FADD_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FADD_D", "0000000??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        dest.f64 = src1.f64 + src2.f64;
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FCVT.D.L covert double to int64_t
 */
class FCVT_D_L: public RiscvInstruction {
 public:
    FCVT_D_L(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FCVT_D_L", "110100100010?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        dest.ival = static_cast<int64_t>(src1.f64);
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FCVT.D.LU covert double to uint64_t
 */
class FCVT_D_LU: public RiscvInstruction {
 public:
    FCVT_D_LU(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FCVT_D_LU", "110100100011?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        dest.val = static_cast<uint64_t>(src1.f64);
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FCVT.L.D covert int64_t to double
 */
class FCVT_L_D: public RiscvInstruction {
 public:
    FCVT_L_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FCVT_L_D", "110000100010?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        dest.f64 = static_cast<double>(src1.ival);
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FCVT.LU.D covert uint64_t to double
 */
class FCVT_LU_D: public RiscvInstruction {
 public:
    FCVT_LU_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FCVT_LU_D", "110000100011?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        dest.f64 = static_cast<double>(src1.val);
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FDIV.D double precision division
 */
class FDIV_D : public RiscvInstruction {
 public:
    FDIV_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FDIV_D", "0001101??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        if (RF[u.bits.rs2]) {
            dest.f64 = src1.f64 / src2.f64;
        } else {
            dest.val = 0;
        }
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FMAX.D select maximum
 */
class FMAX_D : public RiscvInstruction {
 public:
    FMAX_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FMAX_D", "0010101??????????001?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        dest.f64 = src1.f64 > src2.f64 ? src1.f64: src2.f64;
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FMAX.D select minimum
 */
class FMIN_D : public RiscvInstruction {
 public:
    FMIN_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FMIN_D", "0010101??????????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        dest.f64 = src1.f64 < src2.f64 ? src1.f64: src2.f64;
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FMUL.D double precision multiplication
 */
class FMUL_D : public RiscvInstruction {
 public:
    FMUL_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FMUL_D", "0001001??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        dest.f64 = src1.f64 * src2.f64;
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FSUB.D double precision subtractor
 */
class FSUB_D : public RiscvInstruction {
 public:
    FSUB_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FSUB_D", "0000100??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        dest.f64 = src1.f64 - src2.f64;
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};


void CpuRiver_Functional::addIsaExtensionF() {
    // TODO
    /*
    addInstr("FADD_S",             "0000000??????????????????1010011", NULL, out);
    addInstr("FSUB_S",             "0000100??????????????????1010011", NULL, out);
    addInstr("FMUL_S",             "0001000??????????????????1010011", NULL, out);
    addInstr("FDIV_S",             "0001100??????????????????1010011", NULL, out);
    addInstr("FSGNJ_S",            "0010000??????????000?????1010011", NULL, out);
    addInstr("FSGNJN_S",           "0010000??????????001?????1010011", NULL, out);
    addInstr("FSGNJX_S",           "0010000??????????010?????1010011", NULL, out);
    addInstr("FMIN_S",             "0010100??????????000?????1010011", NULL, out);
    addInstr("FMAX_S",             "0010100??????????001?????1010011", NULL, out);
    addInstr("FSQRT_S",            "010110000000?????????????1010011", NULL, out);
    addInstr("FLE_S",              "1010000??????????000?????1010011", NULL, out);
    addInstr("FLT_S",              "1010000??????????001?????1010011", NULL, out);
    addInstr("FEQ_S",              "1010000??????????010?????1010011", NULL, out);
    addInstr("FCVT_W_S",           "110000000000?????????????1010011", NULL, out);
    addInstr("FCVT_WU_S",          "110000000001?????????????1010011", NULL, out);
    addInstr("FCVT_L_S",           "110000000010?????????????1010011", NULL, out);
    addInstr("FCVT_LU_S",          "110000000011?????????????1010011", NULL, out);
    addInstr("FMV_X_S",            "111000000000?????000?????1010011", NULL, out);
    addInstr("FCLASS_S",           "111000000000?????001?????1010011", NULL, out);
    addInstr("FCVT_S_W",           "110100000000?????????????1010011", NULL, out);
    addInstr("FCVT_S_WU",          "110100000001?????????????1010011", NULL, out);
    addInstr("FCVT_S_L",           "110100000010?????????????1010011", NULL, out);
    addInstr("FCVT_S_LU",          "110100000011?????????????1010011", NULL, out);
    addInstr("FMV_S_X",            "111100000000?????000?????1010011", NULL, out);
    addInstr("FMADD_S",            "?????00??????????????????1000011", NULL, out);
    addInstr("FMSUB_S",            "?????00??????????????????1000111", NULL, out);
    addInstr("FNMSUB_S",           "?????00??????????????????1001011", NULL, out);
    addInstr("FNMADD_S",           "?????00??????????????????1001111", NULL, out);

    addInstr("FCVT_S_D",           "010000000001?????????????1010011", NULL, out);
    addInstr("FCVT_D_S",           "010000100000?????????????1010011", NULL, out);

    addInstr("FADD_D",             "0000001??????????????????1010011", NULL, out);
    addInstr("FSUB_D",             "0000101??????????????????1010011", NULL, out);
    addInstr("FMUL_D",             "0001001??????????????????1010011", NULL, out);
    addInstr("FDIV_D",             "0001101??????????????????1010011", NULL, out);
    addInstr("FSGNJ_D",            "0010001??????????000?????1010011", NULL, out);
    addInstr("FSGNJN_D",           "0010001??????????001?????1010011", NULL, out);
    addInstr("FSGNJX_D",           "0010001??????????010?????1010011", NULL, out);
    addInstr("FMIN_D",             "0010101??????????000?????1010011", NULL, out);
    addInstr("FMAX_D",             "0010101??????????001?????1010011", NULL, out);
    addInstr("FSQRT_D",            "010110100000?????????????1010011", NULL, out);
    addInstr("FLE_D",              "1010001??????????000?????1010011", NULL, out);
    addInstr("FLT_D",              "1010001??????????001?????1010011", NULL, out);
    addInstr("FEQ_D",              "1010001??????????010?????1010011", NULL, out);
    addInstr("FCVT_W_D",           "110000100000?????????????1010011", NULL, out);
    addInstr("FCVT_WU_D",          "110000100001?????????????1010011", NULL, out);
    addInstr("FCVT_L_D",           "110000100010?????????????1010011", NULL, out);
    addInstr("FCVT_LU_D",          "110000100011?????????????1010011", NULL, out);
    addInstr("FMV_X_D",            "111000100000?????000?????1010011", NULL, out);
    addInstr("FCLASS_D",           "111000100000?????001?????1010011", NULL, out);
    addInstr("FCVT_D_W",           "110100100000?????????????1010011", NULL, out);
    addInstr("FCVT_D_WU",          "110100100001?????????????1010011", NULL, out);
    addInstr("FCVT_D_L",           "110100100010?????????????1010011", NULL, out);
    addInstr("FCVT_D_LU",          "110100100011?????????????1010011", NULL, out);
    addInstr("FMV_D_X",            "111100100000?????000?????1010011", NULL, out);
    addInstr("FLW",                "?????????????????010?????0000111", NULL, out);
    addInstr("FLD",                "?????????????????011?????0000111", NULL, out);
    addInstr("FSW",                "?????????????????010?????0100111", NULL, out);
    addInstr("FSD",                "?????????????????011?????0100111", NULL, out);
    addInstr("FMADD_D",            "?????01??????????????????1000011", NULL, out);
    addInstr("FMSUB_D",            "?????01??????????????????1000111", NULL, out);
    addInstr("FNMSUB_D",           "?????01??????????????????1001011", NULL, out);
    addInstr("FNMADD_D",           "?????01??????????????????1001111", NULL, out);
    def FRFLAGS            = BitPat("b00000000000100000010?????1110011")
    def FSFLAGS            = BitPat("b000000000001?????001?????1110011")
    def FSFLAGSI           = BitPat("b000000000001?????101?????1110011")
    def FRRM               = BitPat("b00000000001000000010?????1110011")
    def FSRM               = BitPat("b000000000010?????001?????1110011")
    def FSRMI              = BitPat("b000000000010?????101?????1110011")
    def FSCSR              = BitPat("b000000000011?????001?????1110011")
    def FRCSR              = BitPat("b00000000001100000010?????1110011")
    */
    uint64_t isa = portCSR_.read(CSR_misa).val;
    isa |= (1LL << ('F' - 'A'));
    portCSR_.write(CSR_misa, isa);
}

}  // namespace debugger
