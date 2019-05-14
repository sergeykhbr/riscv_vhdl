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
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(CSR_fcsr);
            fcsr.bits.DZ = 1;
            icpu_->writeCSR(CSR_fcsr, fcsr.value);
            dest.val = 0;
        }
        RF[u.bits.rd] = dest.val;
        return 4;
    }
};

/**
 * @brief The FEQ.D quiet comparision
 */
class FEQ_D : public RiscvInstruction {
 public:
    FEQ_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FEQ_D", "1010001??????????010?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        uint64_t eq = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(CSR_fcsr, fcsr.value);
        } else {
            eq = src1.val == src2.val ? 1ull: 0;
        }
        R[u.bits.rd] = eq;
        return 4;
    }
};

/** @brief The FLD loads a double-precision floating-point value from memory
 *         into floating-point register rd.
 */
class FLD : public RiscvInstruction {
public:
    FLD(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "FLD", "?????????????????011?????0000111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 8;
        if (trans.addr & 0x7) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        RF[u.bits.rd] = trans.rpayload.b64[0];
        return 4;
    }
};

/**
 * @brief The FLE.D quiet comparision less or equal
 */
class FLE_D : public RiscvInstruction {
 public:
    FLE_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FLE_D", "1010001??????????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        uint64_t le = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(CSR_fcsr, fcsr.value);
        } else {
            le = src1.val <= src2.val ? 1ull: 0;
        }
        R[u.bits.rd] = le;
        return 4;
    }
};

/**
 * @brief The FLT.D quiet comparision less than
 */
class FLT_D : public RiscvInstruction {
 public:
    FLT_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FLT_D", "1010001??????????001?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        uint64_t le = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(CSR_fcsr, fcsr.value);
        } else {
            le = src1.val < src2.val ? 1ull: 0;
        }
        R[u.bits.rd] = le;
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
 * @brief The FMOV.D.X move fp value into integer register
 */
class FMOV_D_X : public RiscvInstruction {
 public:
    FMOV_D_X(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FMOV_D_X", "111100100000?????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        R[u.bits.rd] = src1.val;
        return 4;
    }
};

/**
 * @brief The FMOV.X.D move value from integer register into fp register
 */
class FMOV_X_D : public RiscvInstruction {
 public:
    FMOV_X_D(CpuRiver_Functional *icpu) : RiscvInstruction(icpu,
        "FMOV_X_D", "111000100000?????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        RF[u.bits.rd] = src1.val;
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

/** @brief The FSD stores a double-precision value from the floating-point registers
           to memory.
 */
class FSD : public RiscvInstruction {
public:
    FSD(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "FSD", "?????????????????011?????0100111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_S_type u;
        u.value = payload->buf32[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Write;
        trans.xsize = 8;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = RF[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2];
        if (trans.addr & 0x7) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
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

void CpuRiver_Functional::addIsaExtensionD() {
    addSupportedInstruction(new FADD_D(this));
    addSupportedInstruction(new FCVT_D_L(this));
    addSupportedInstruction(new FCVT_D_LU(this));
    addSupportedInstruction(new FCVT_L_D(this));
    addSupportedInstruction(new FCVT_LU_D(this));
    addSupportedInstruction(new FDIV_D(this));
    addSupportedInstruction(new FEQ_D(this));
    addSupportedInstruction(new FLD(this));
    addSupportedInstruction(new FLE_D(this));
    addSupportedInstruction(new FLT_D(this));
    addSupportedInstruction(new FMAX_D(this));
    addSupportedInstruction(new FMIN_D(this));
    addSupportedInstruction(new FMOV_D_X(this));
    addSupportedInstruction(new FMOV_X_D(this));
    addSupportedInstruction(new FMUL_D(this));
    addSupportedInstruction(new FSD(this));
    addSupportedInstruction(new FSUB_D(this));

    uint64_t isa = 0x8000000000000000LL;
    isa |= (1LL << ('D' - 'A'));
    portCSR_.write(CSR_misa, isa);
}

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
    addInstr("FLW",                "?????????????????010?????0000111", NULL, out);
    addInstr("FSW",                "?????????????????010?????0100111", NULL, out);
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
    addInstr("FLD",                "?????????????????011?????0000111", NULL, out);
    addInstr("FSD",                "?????????????????011?????0100111", NULL, out);
    addInstr("FMADD_D",            "?????01??????????????????1000011", NULL, out);
    addInstr("FMSUB_D",            "?????01??????????????????1000111", NULL, out);
    addInstr("FNMSUB_D",           "?????01??????????????????1001011", NULL, out);
    addInstr("FNMADD_D",           "?????01??????????????????1001111", NULL, out);

    // pseudo-instruction of access to CSR
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
