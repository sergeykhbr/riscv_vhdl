/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RISC-V extension-C (Comporessed Instructions).
 */

#include "api_utils.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

// Regsiter
union ISA_CR_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t rdrs1  : 5;  // [11:7]
        uint16_t funct4 : 4;  // [15:12]
    } bits;
    uint16_t value;
};

// Immediate
union ISA_CI_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t imm    : 5;  // [6:2]
        uint16_t rdrs   : 5;  // [11:7]
        uint16_t imm6   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    struct sp_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t imm5    : 1; // [2]
        uint16_t imm8_7  : 2; // [4:3]
        uint16_t imm6  : 1;   // [5]
        uint16_t imm4  : 1;   // [6]
        uint16_t sp    : 5;   // [11:7]
        uint16_t imm9   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } spbits;
    uint16_t value;
};

// Wide immediate
union ISA_CIW_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rd     : 3;  // [4:2]
        uint16_t imm3   : 1;  // [5]
        uint16_t imm2   : 1;  // [6]
        uint16_t imm9_6 : 4;  // [10:7]
        uint16_t imm5_4 : 2;  // [12:11]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Load
union ISA_CL_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rd     : 3;  // [4:2]
        uint16_t imm6   : 1;  // [5]
        uint16_t imm27  : 1;  // [6]
        uint16_t rs1    : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Store
union ISA_CS_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 3;  // [4:2]
        uint16_t imm6   : 1;  // [5]
        uint16_t imm27  : 1;  // [6]
        uint16_t rs1    : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Stack relative Store
union ISA_CSS_type {
    struct w_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t imm7_6 : 2;  // [8:7]
        uint16_t imm5_2 : 4;  // [12:9]
        uint16_t funct3 : 3;  // [15:13]
    } wbits;
    struct d_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t imm8_6 : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } dbits;
    uint16_t value;
};

/** 
 * @brief Add register to register
 *
 * C.ADD adds the values in registers rd and rs2 and writes the result to
 * register rd. C.ADD expands into add rd, rd, rs2.
 */
class C_ADD : public RiscvInstruction16 {
public:
    C_ADD(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADD", "????????????????1001??????????10") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        u.value = payload->buf16[0];
        if (u.bits.rdrs1) {
            R[u.bits.rdrs1] += R[u.bits.rs2];
        }
        return 2;
    }
};

/** 
 * @brief Add immediate
 *
 * C.ADDI adds the non-zero sign-extended 6-bit immediate to the value in
 * register rd then writes the result to rd. C.ADDI expands into
 * addi rd, rd, nzimm[5:0].
 */
class C_ADDI : public RiscvInstruction16 {
public:
    C_ADDI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADDI", "????????????????000???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        R[u.bits.rdrs] = R[u.bits.rdrs] + imm;
        return 2;
    }
};

/** 
 * @brief Stack-relative Add immediate
 *
 * C.ADDI16SP shares the opcode with C.LUI, but has a destination field of x2.
 * C.ADDI16SP adds the non-zero sign-extended 6-bit immediate to the value in
 * the stack pointer (sp=x2), where the immediate is scaled to represent
 * multiples of 16 in the range (-512,496). C.ADDI16SP is used to adjust the
 * stack pointer in procedure prologues and epilogues. It expands into
 * addi x2, x2, nzimm[9:4].
 */
class C_ADDI16SP : public RiscvInstruction16 {
public:
    C_ADDI16SP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADDI16SP", "????????????????011?00010?????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = (u.spbits.imm8_7 << 3) | (u.spbits.imm6 << 2)
                    | (u.spbits.imm5 << 1) | u.spbits.imm4;
        if (u.spbits.imm9) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 4;
        R[Reg_sp] = R[Reg_sp] + imm;
        return 2;
    }
};

/** 
 * @brief Stack-relative Add wide immediate
 *
 * C.ADDI4SPN is a CIW-format RV32C/RV64C-only instruction that adds a
 * zero-extended non-zero immediate, scaled by 4, to the stack pointer, x2,
 * and writes the result to rd0. This instruction is used to generate
 * pointers to stack-allocated variables, and expands to
 * addi rd0, x2, zimm[9:2].
 */
class C_ADDI4SPN : public RiscvInstruction16 {
public:
    C_ADDI4SPN(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADDI4SPN", "????????????????000???????????00") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CIW_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = (u.bits.imm9_6 << 4) | (u.bits.imm5_4 << 2)
                    | (u.bits.imm3 << 1) | u.bits.imm2;
        imm <<= 2;
        if (u.bits.rd == 0) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
        } else {
            R[u.bits.rd] = R[Reg_sp] + imm;
        }
        return 2;
    }
};

/** 
 * @brief Add immediate with sign extending
 *
 * C.ADDIW is an RV64C/RV128C-only instruction that performs the same
 * computation but produces a 32-bit result, then sign-extends result
 * to 64 bits. C.ADDIW expands into addiw rd, rd, imm[5:0]. The immediate
 * can be zero for C.ADDIW, where this corresponds to sext.w rd.
 */
class C_ADDIW : public RiscvInstruction16 {
public:
    C_ADDIW(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADDIW", "????????????????001???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        R[u.bits.rdrs] = (R[u.bits.rdrs] + imm) & 0xFFFFFFFFLL;
        if (R[u.bits.rdrs] & (1LL << 31)) {
            R[u.bits.rdrs] |= EXT_SIGN_32;
        }
        return 2;
    }
};

/**
 * @brief LOAD instructions with sign extending.
 *
 * C.LD is an RV64C/RV128C-only instruction that loads a 64-bit value from
 * memory into register rd0. It computes an eective address by adding the
 * zero-extended oset, scaled by 8, to the base address in register rs10.
 * It expands to ld rd0, offset[7:3](rs10).
 */ 
class C_LD : public RiscvInstruction16 {
public:
    C_LD(CpuRiver_Functional *icpu) :
        RiscvInstruction16(icpu, "C_LD", "????????????????011???????????00") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CL_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
        off <<= 3;
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 8;
        if (trans.addr & 0x7) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        R[u.bits.rd] = trans.rpayload.b64[0];
        return 2;
    }
};

/**
 * @brief LOAD instructions with sign extending.
 *
 * C.LWloads a 32-bit value from memory into register rd0. It computes an
 * effective address by adding the zero-extended oset, scaled by 4, to the
 * base address in register rs10. It expands to lw rd0, offset[6:2](rs10).
 */ 
class C_LW : public RiscvInstruction16 {
public:
    C_LW(CpuRiver_Functional *icpu) :
        RiscvInstruction16(icpu, "C_LW", "????????????????010???????????00") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CL_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
        off <<= 2;
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 4;
        if (trans.addr & 0x3) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        R[u.bits.rd] = trans.rpayload.b64[0];
        if (R[u.bits.rd] & (1LL << 31)) {
            R[u.bits.rd] |= EXT_SIGN_32;
        }
        return 2;
    }
};

/** 
 * @brief Constant generation
 *
 * C.LI loads the sign-extended 6-bit immediate, imm, into register rd. C.LI is
 * only valid when rd /= x0. C.LI expands into addi rd, x0, imm[5:0].
 */
class C_LI : public RiscvInstruction16 {
public:
    C_LI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_LI", "????????????????010???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        if (u.bits.rdrs == 0) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
        } else {
            R[u.bits.rdrs] = imm;
        }
        return 2;
    }
};

/** 
 * @brief Constant generation
 *
 * C.LUI loads the non-zero 6-bit immediate eld into bits 17-12 of the
 * destination register, clears the bottom 12 bits, and sign-extends
 * bit 17 into all higher bits of the destination. C.LUI is only
 * valid when rd /= {x0; x2}, and when the immediate is not equal to zero.
 * C.LUI expands into lui rd, nzuimm[17:12].
 */
class C_LUI : public RiscvInstruction16 {
public:
    C_LUI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_LUI", "????????????????011???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 12;
        if (u.bits.rdrs == 0 || u.bits.rdrs == Reg_sp || imm == 0) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
        } else {
            R[u.bits.rdrs] = imm;
        }
        return 2;
    }
};

/** 
 * @brief Move register to register
 *
 * C.MV copies the value in register rs2 into register rd. C.MV expands
 * into add rd, x0, rs2.
 */
class C_MV : public RiscvInstruction16 {
public:
    C_MV(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_MV", "????????????????1000??????????10") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        u.value = payload->buf16[0];
        if (u.bits.rdrs1) {
            R[u.bits.rdrs1] = R[u.bits.rs2];
        }
        return 2;
    }
};

/** 
 * @brief Store 64-bits data
 *
 * C.SD is an RV64C/RV128C-only instruction that stores a 64-bit value in
 * register rs20 to memory. It computes an eective address by adding the
 * zero-extended oset, scaled by 8, to the base address in register rs10. It
 * expands to sd rs20, offset[7:3](rs10).
 */
class C_SD : public RiscvInstruction16 {
public:
    C_SD(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SD", "????????????????111???????????00") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CS_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
        off <<= 3;
        trans.action = MemAction_Write;
        trans.xsize = 8;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2];
        if (trans.addr & 0x7) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        return 2;
    }
};

/** 
 * @brief Stack-relative Store 64-bits data
 *
 * C.SDSP is an RV64C/RV128C-only instruction that stores a 64-bit value
 * in register rs2 to memory. It computes an eective address by adding the
 * zero-extended oset, scaled by 8, to the stack pointer, x2. It expands
 * to sd rs2, offset[8:3](x2).
 */
class C_SDSP : public RiscvInstruction16 {
public:
    C_SDSP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SDSP", "????????????????111???????????10") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CSS_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.dbits.imm8_6 << 3) | u.dbits.imm5_3;
        off <<= 3;
        trans.action = MemAction_Write;
        trans.xsize = 8;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[Reg_sp] + off;
        trans.wpayload.b64[0] = R[u.dbits.rs2];
        if (trans.addr & 0x7) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        return 2;
    }
};

/** 
 * @brief Store word
 *
 * C.SW stores a 32-bit value in register rs20 to memory. It computes an
 * effective address by adding the zero-extended oset, scaled by 4, to the
 * base address in register rs10. It expands to sw rs20, offset[6:2](rs10).
 */
class C_SW : public RiscvInstruction16 {
public:
    C_SW(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SW", "????????????????110???????????00") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CS_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
        off <<= 2;
        trans.action = MemAction_Write;
        trans.xsize = 4;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2];
        if (trans.addr & 0x3) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        return 2;
    }
};

/** 
 * @brief Stack-relative Store word
 *
 * C.SWSP stores a 32-bit value in register rs2 to memory. It computes an
 * effective address by adding the zero-extended oset, scaled by 4, to the
 * stack pointer, x2. It expands to sw rs2, offset[7:2](x2).
 */
class C_SWSP : public RiscvInstruction16 {
public:
    C_SWSP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SWSP", "????????????????110???????????10") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CSS_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.wbits.imm7_6 << 4) | u.wbits.imm5_2;
        off <<= 2;
        trans.action = MemAction_Write;
        trans.xsize = 4;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[Reg_sp] + off;
        trans.wpayload.b64[0] = R[u.wbits.rs2];
        if (trans.addr & 0x3) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            icpu_->dma_memop(&trans);
        }
        return 2;
    }
};

void CpuRiver_Functional::addIsaExtensionC() {
    addSupportedInstruction(new C_ADD(this));
    addSupportedInstruction(new C_ADDI(this));
    addSupportedInstruction(new C_ADDI16SP(this));
    addSupportedInstruction(new C_ADDI4SPN(this));
    addSupportedInstruction(new C_ADDIW(this));
    addSupportedInstruction(new C_LD(this));
    addSupportedInstruction(new C_LI(this));
    addSupportedInstruction(new C_LUI(this));
    addSupportedInstruction(new C_LW(this));
    addSupportedInstruction(new C_MV(this));
    addSupportedInstruction(new C_SD(this));
    addSupportedInstruction(new C_SDSP(this));
    addSupportedInstruction(new C_SW(this));
    addSupportedInstruction(new C_SWSP(this));
    /*
  def C_JAL              = BitPat("b????????????????001???????????01")
  def C_SRLI             = BitPat("b????????????????100?00????????01")
  def C_SRAI             = BitPat("b????????????????100?01????????01")
  def C_ANDI             = BitPat("b????????????????100?10????????01")
  def C_SUB              = BitPat("b????????????????100011???00???01")
  def C_XOR              = BitPat("b????????????????100011???01???01")
  def C_OR               = BitPat("b????????????????100011???10???01")
  def C_AND              = BitPat("b????????????????100011???11???01")
  def C_SUBW             = BitPat("b????????????????100111???00???01")
  def C_ADDW             = BitPat("b????????????????100111???01???01")
  def C_J                = BitPat("b????????????????101???????????01")
  def C_BEQZ             = BitPat("b????????????????110???????????01")
  def C_BNEZ             = BitPat("b????????????????111???????????01")
  def C_SLLI             = BitPat("b????????????????000???????????10")
  def C_FLDSP            = BitPat("b????????????????001???????????10")
  def C_LWSP             = BitPat("b????????????????010???????????10")
  def C_FLWSP            = BitPat("b????????????????011???????????10")
  def C_FSDSP            = BitPat("b????????????????101???????????10")
  def C_SWSP             = BitPat("b????????????????110???????????10")
  def C_FSWSP            = BitPat("b????????????????111???????????10")    */
    uint64_t isa = portCSR_.read(CSR_misa).val;
    portCSR_.write(CSR_misa, isa | (1LL << ('C' - 'C')));
}

}  // namespace debugger
