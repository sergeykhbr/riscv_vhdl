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
 * @brief      RISC-V extension-C (Comporessed Instructions).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload)
                && u.bits.rdrs1 && u.bits.rs2;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        if (u.bits.rdrs1) {
            res = R[u.bits.rdrs1] + R[u.bits.rs2]; 
            icpu_->setReg(u.bits.rdrs1, res);
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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        icpu_->setReg(u.bits.rdrs, R[u.bits.rdrs] + imm);
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
        icpu_->setReg(ICpuRiscV::Reg_sp, R[ICpuRiscV::Reg_sp] + imm);
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
        icpu_->setReg(8 + u.bits.rd, R[ICpuRiscV::Reg_sp] + imm);
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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        uint64_t res;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        res = (R[u.bits.rdrs] + imm) & 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rdrs, res);
        return 2;
    }
};

/**
 * @brief C_ADDW
 *
 * C.ADDW is an RV64C/RV128C-only instruction that adds the values in registers
 * rd' and rs2', then sign-extends the lower 32 bits of the sum before writing
 * the result to register rd'. C.ADDW expands into addw rd', rd', rs2'.
 */
class C_ADDW : public RiscvInstruction16 {
public:
    C_ADDW(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ADDW", "????????????????100111???01???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        res = R[8 + u.bits.rs1] + R[8 + u.bits.rs2];
        res &= 0xFFFFFFFFLL;
        
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(8 + u.bits.rs1, res);
        return 2;
    }
};


/**
 * @brief C_AND
 *
 * C.AND computes the bitwise AND of the values in registers rd' and rs2', then
 * writes the result to register rd'. C.AND expands into and rd', rd', rs2'.
 */
class C_AND : public RiscvInstruction16 {
public:
    C_AND(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_AND", "????????????????100011???11???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        res = R[8 + u.bits.rs1] & R[8 + u.bits.rs2];
        icpu_->setReg(8 + u.bits.rs1, res);
        return 2;
    }
};

/** 
 * @brief AND with sign-extended immediate
 *
 * C.ANDI is a CB-format instruction that computes the bitwise AND of the
 * value in register rd' and the sign-extended 6-bit immediate, then writes
 * the result to rd'. C.ANDI expands to andi rd', rd', imm[5:0].
 */
class C_ANDI : public RiscvInstruction16 {
public:
    C_ANDI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_ANDI", "????????????????100?10????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        uint64_t res;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.shbits.shamt;
        if (u.shbits.shamt5) {
            imm |= EXT_SIGN_6;
        }
        res = R[8 + u.bits.rs1] & imm;
        icpu_->setReg(8 + u.bits.rs1, res);
        return 2;
    }
};

/**
 * @brief Branch if registers zero
 *
 * C.BEQZ performs conditional control transfers. The offset is sign-extended
 * and added to the pc to form the branch target address. It can therefore
 * target a 256B range. C.BEQZ takes the branch if the value in register rs1'
 *  is zero. It expands to beq rs1', x0, offset[8:1].
 */
class C_BEQZ : public RiscvInstruction16 {
public:
    C_BEQZ(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_BEQZ", "????????????????110???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        u.value = payload->buf32[0];
    
        if (R[8 + u.bits.rs1] == 0) {
            uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
                    | (u.bits.off4_3 << 2) | u.bits.off2_1;
            imm <<= 1;
            if (u.bits.off8) {
                imm |= EXT_SIGN_9;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 2;
    }
};


/**
 * @brief Branch if registers not zero
 *
 * C.BNEZ is defined analogously, but it takes the branch if rs1' contains a 
 * nonzero value. It expands to bne rs1', x0, offset[8:1].
 */
class C_BNEZ : public RiscvInstruction16 {
public:
    C_BNEZ(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_BNEZ", "????????????????111???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        u.value = payload->buf32[0];
    
        if (R[8 + u.bits.rs1]) {
            uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
                    | (u.bits.off4_3 << 2) | u.bits.off2_1;
            imm <<= 1;
            if (u.bits.off8) {
                imm |= EXT_SIGN_9;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 2;
    }
};

/**
 * @brief C.EBREAK (breakpoint instruction)
 *
 * The C.EBREAK instruction is used by debuggers to cause control to be
 * transferred back to a debug-ging environment.
 */
class C_EBREAK : public RiscvInstruction16 {
public:
    C_EBREAK(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_EBREAK", "????????????????1001000000000010") {}

    virtual int exec(Reg64Type *payload) {
        icpu_->generateException(ICpuRiscV::EXCEPTION_Breakpoint, icpu_->getPC());
        icpu_->doNotCache(icpu_->getPC());
        return 2;
    }
};


/**
 * @brief Unconditional jump
 *
 * C.J performs an unconditional control transfer. The offset is sign-extended
 *  and added to the pc to form the jump target address. C.J can therefore
 * target a 2 KiB range. C.J expands to jal x0, offset[11:1].
 */
class C_J : public RiscvInstruction16 {
public:
    C_J(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_J", "????????????????101???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CJ_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.off10 << 9) | (u.bits.off9_8 << 7)
                     | (u.bits.off7 << 6) | (u.bits.off6 << 5)
                     | (u.bits.off5 << 4) | (u.bits.off4 << 3)
                     | u.bits.off3_1;
        off <<= 1;
        if (u.bits.off11) {
            off |= EXT_SIGN_11;
        }
        icpu_->setBranch(icpu_->getPC() + off);
        return 2;
    }
};

#ifdef RV32C_ONLY  // conflict with C_ADDIW
/**
 * @brief Unconditional jump with write to ra
 *
 * C.JAL is an RV32C-only instruction that performs the same operation as C.J,
 * but additionally writes the address of the instruction following the jump
 * (pc+2) to the link register, x1. C.JAL expands to jal x1, offset[11:1].
 */
class C_JAL : public RiscvInstruction16 {
public:
    C_JAL(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_JAL", "????????????????001???????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CJ_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.off10 << 9) | (u.bits.off9_8 << 7)
                     | (u.bits.off7 << 6) | (u.bits.off6 << 5)
                     | (u.bits.off5 << 4) | (u.bits.off4 << 3)
                     | u.bits.off3_1;
        off <<= 1;
        if (u.bits.off11) {
            off |= EXT_SIGN_11;
        }
        R[ICpuRiscV::Reg_ra] = icpu_->getPC() + 2;
        icpu_->setBranch(icpu_->getPC() + off);
        icpu_->pushStackTrace();
        return 2;
    }
};
#endif

/**
 * @brief Unconditional jump with write to ra
 *
 * C.JALR (jump and link register) performs the same operation as C.JR, but
 * additionally writes the address of the instruction following the jump (pc+2)
 * to the link register, x1. C.JALR expands to jalr x1, rs1, 0.
 */
class C_JALR : public RiscvInstruction16 {
public:
    C_JALR(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_JALR", "????????????????1001?????0000010") {}

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        u.value = payload->buf16[0];
        icpu_->setReg(ICpuRiscV::Reg_ra, icpu_->getPC() + 2);
        icpu_->setBranch(R[u.bits.rdrs1]);
        icpu_->pushStackTrace();
        return 2;
    }
};

/**
 * @brief Unconditional jump
 *
 * C.JR (jump register) performs an unconditional control transfer to the
 * address in register rs1. C.JR expands to jalr x0, rs1, 0.
 */
class C_JR : public RiscvInstruction16 {
public:
    C_JR(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_JR", "????????????????1000?????0000010") {}

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        u.value = payload->buf16[0];
        icpu_->setBranch(R[u.bits.rdrs1]);
        if (u.bits.rdrs1 == ICpuRiscV::Reg_ra) {
            icpu_->popStackTrace();
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
        uint64_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3)
                     | u.bits.imm5_3;
        off <<= 3;
        trans.action = MemAction_Read;
        trans.addr = R[8 + u.bits.rs1] + off;
        trans.xsize = 8;
        if (trans.addr & 0x7) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            }
        }
        icpu_->setReg(8 + u.bits.rd, trans.rpayload.b64[0]);
        return 2;
    }
};

/**
 * @brief LOAD stack-relative dword.
 *
 * C.LDSP is an RV64C/RV128C-only instruction that loads a 64-bit value from
 * memory into register rd. It computes its eective address by adding the
 * zero-extended oset, scaled by 8, to the stack pointer, x2.
 * It expands to ld rd, offset[8:3](x2).
 */ 
class C_LDSP : public RiscvInstruction16 {
public:
    C_LDSP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_LDSP", "????????????????011???????????10") {}

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CI_type u;
        u.value = payload->buf16[0];
        uint64_t off = (u.ldspbits.off8_6 << 3) | (u.ldspbits.off5 << 2)
                     | u.ldspbits.off4_3;
        off <<= 3;
        trans.action = MemAction_Read;
        trans.addr = R[ICpuRiscV::Reg_sp] + off;
        trans.xsize = 8;
        if (trans.addr & 0x7) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            }
        }
        icpu_->setReg(u.ldspbits.rd, trans.rpayload.b64[0]);
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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        icpu_->setReg(u.bits.rdrs, imm);
        return 2;
    }
};

/**
 * @brief LOAD instructions with sign extending.
 *
 * C.LW loads a 32-bit value from memory into register rd0. It computes an
 * effective address by adding the zero-extended offset, scaled by 4, to the
 * base address in register rs10. It expands to lw rd0, offset[6:2](rs10).
 */ 
class C_LW : public RiscvInstruction16 {
public:
    C_LW(CpuRiver_Functional *icpu) :
        RiscvInstruction16(icpu, "C_LW", "????????????????010???????????00") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CL_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        uint64_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
        off <<= 2;
        trans.action = MemAction_Read;
        trans.addr = R[8 + u.bits.rs1] + off;
        trans.xsize = 4;
        trans.rpayload.b64[0] = 0;
        if (trans.addr & 0x3) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            }
        }
        res = trans.rpayload.b32[0];
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(8 + u.bits.rd, res);
        return 2;
    }
};

/**
 * @brief LOAD stack-relative word.
 *
 * C.LWSP loads a 32-bit value from memory into register rd. It computes
 * an eective address by adding the zero-extended oset, scaled by 4, to the
 * stack pointer, x2. It expands to lw rd, offset[7:2](x2).
 */ 
class C_LWSP : public RiscvInstruction16 {
public:
    C_LWSP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_LWSP", "????????????????010???????????10") {}

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_CI_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        uint64_t off = (u.lwspbits.off7_6 << 4) | (u.lwspbits.off5 << 3)
                     | u.lwspbits.off4_2;
        off <<= 2;
        trans.action = MemAction_Read;
        trans.addr = R[ICpuRiscV::Reg_sp] + off;
        trans.xsize = 4;
        if (trans.addr & 0x3) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            }
        }
        res = trans.rpayload.b32[0];
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.lwspbits.rd, res);
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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1
            && u.bits.rdrs1 != ICpuRiscV::Reg_sp;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CI_type u;
        u.value = payload->buf16[0];
    
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 12;
        icpu_->setReg(u.bits.rdrs, imm);
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

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload)
            && u.bits.rs2 && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CR_type u;
        u.value = payload->buf16[0];
        icpu_->setReg(u.bits.rdrs1, R[u.bits.rs2]);
        return 2;
    }
};

/** 
 * @brief empty cycle
 */
class C_NOP : public RiscvInstruction16 {
public:
    C_NOP(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_NOP", "????????????????0000000000000001") {}

    virtual bool parse(uint32_t *payload) {
        return RiscvInstruction::parse(payload);
    }

    virtual int exec(Reg64Type *payload) {
        return 2;
    }
};


/**
 * @brief C_OR
 *
 * C.OR computes the bitwise OR of the values in registers rd' and rs2', then
 * writes the result to register rd'. C.OR expands into or rd', rd', rs2'.
 */
class C_OR : public RiscvInstruction16 {
public:
    C_OR(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_OR", "????????????????100011???10???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        res = R[8 + u.bits.rs1] | R[8 + u.bits.rs2];
        icpu_->setReg(8 + u.bits.rs1, res);
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
        trans.addr = R[8 + u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[8 + u.bits.rs2];
        if (trans.addr & 0x7) {
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            }
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
        trans.addr = R[ICpuRiscV::Reg_sp] + off;
        trans.wpayload.b64[0] = R[u.dbits.rs2];
        if (trans.addr & 0x7) {
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            }
        }
        return 2;
    }
};

/**
 * @brief Logical shift left
 *
 * C.SLLI is a CI-format instruction that performs a logical left shift
 * of the value in register rd then writes the result to rd. The shift amount
 * is encoded in the shamt eld, where shamt[5] must be zero for RV32C.
 * For RV32C and RV64C, the shift amount must be non-zero. For RV128C, a shift
 * amount of zero is used to encode a shift of 64. C.SLLI expands into 
 * slli rd, rd, shamt[5:0], except for RV128C with shamt=0, which expands
 * to slli rd, rd, 64.
 */
class C_SLLI : public RiscvInstruction16 {
public:
    C_SLLI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SLLI", "????????????????000???????????10") {}

    virtual bool parse(uint32_t *payload) {
        ISA_CR_type u;
        u.value = static_cast<uint16_t>(payload[0]);
        return RiscvInstruction::parse(payload) && u.bits.rdrs1;
    }

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        u.value = payload->buf16[0];
        uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
        uint32_t idx = (u.shbits.funct2 << 3) | u.shbits.rd;
        icpu_->setReg(idx, R[idx] << shamt);
        return 2;
    }
};

/** 
 * @brief Arith shift right
 *
 * C.SRAI is defined analogously to C.SRLI, but instead performs an arithmetic
 * right shift. C.SRAI expands to srai rd', rd', shamt[5:0].
 */
class C_SRAI : public RiscvInstruction16 {
public:
    C_SRAI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SRAI", "????????????????100?01????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
        res = static_cast<int64_t>(R[8 + u.shbits.rd]) >> shamt;
        icpu_->setReg(8 + u.shbits.rd, res);
        return 2;
    }
};

/**
 * @brief Logical shift right
 *
 * C.SRLI is a CB-format instruction that performs a logical right shift of
 * the value in register rd' then writes the result to rd'. The shift amount
 * is encoded in the shamt field, where shamt[5] must be zero for RV32C.
 * For RV32C and RV64C, the shift amount must be non-zero. For RV128C, a shift
 * amount of zero is used to encode a shift of 64. Furthermore, the shift
 * amount is sign-extended for RV128C, and so the legal shift amounts are
 * 1-31, 64, and 96-127. C.SRLI expands into srli rd0, rd0, shamt[5:0],
 * except for RV128C with shamt=0, which expands to srli rd0, rd0, 64.
 */
class C_SRLI : public RiscvInstruction16 {
public:
    C_SRLI(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SRLI", "????????????????100?00????????01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CB_type u;
        u.value = payload->buf16[0];
        uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
        icpu_->setReg(8 + u.shbits.rd, R[8 + u.shbits.rd] >> shamt);
        return 2;
    }
};

/**
 * @brief C_SUB
 *
 * C.SUB subtracts the value in register rs2' from the value in register rd',
 * then writes the result to register rd'. C.SUB expands into
 * sub rd', rd', rs2'.
 */
class C_SUB : public RiscvInstruction16 {
public:
    C_SUB(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SUB", "????????????????100011???00???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        u.value = payload->buf16[0];
        icpu_->setReg(8 + u.bits.rs1, R[8 + u.bits.rs1] - R[8 + u.bits.rs2]);
        return 2;
    }
};

/**
 * @brief C_SUBW
 *
 * C.SUBW is an RV64C/RV128C-only instruction that subtracts the value in
 * register rs2' from the value in register rd', then sign-extends the lower
 * 32 bits of the difference before writing the result to register rd'.
 * C.SUBW expands into subw rd', rd', rs2'.
 */
class C_SUBW : public RiscvInstruction16 {
public:
    C_SUBW(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_SUBW", "????????????????100111???00???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        uint64_t res;
        u.value = payload->buf16[0];
        res = R[8 + u.bits.rs1] - R[8 + u.bits.rs2];
        res &= 0xFFFFFFFFLL;
        
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(8 + u.bits.rs1, res);
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
        trans.addr = R[8 + u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[8 + u.bits.rs2];
        if (trans.addr & 0x3) {
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            }
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
        trans.addr = R[ICpuRiscV::Reg_sp] + off;
        trans.wpayload.b64[0] = R[u.wbits.rs2];
        if (trans.addr & 0x3) {
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            }
        }
        return 2;
    }
};

/**
 * @brief C_XOR
 *
 * C.XOR computes the bitwise XOR of the values in registers rd' and rs2', then
 * writes the result to register rd'. C.XOR expands into xor rd', rd', rs2'.
 */
class C_XOR : public RiscvInstruction16 {
public:
    C_XOR(CpuRiver_Functional *icpu) : RiscvInstruction16(icpu,
        "C_XOR", "????????????????100011???01???01") {}

    virtual int exec(Reg64Type *payload) {
        ISA_CS_type u;
        u.value = payload->buf16[0];
        icpu_->setReg(8 + u.bits.rs1, R[8 + u.bits.rs1] ^ R[8 + u.bits.rs2]);
        return 2;
    }
};


void CpuRiver_Functional::addIsaExtensionC() {
    addSupportedInstruction(new C_ADD(this));
    addSupportedInstruction(new C_ADDI(this));
    addSupportedInstruction(new C_ADDI16SP(this));
    addSupportedInstruction(new C_ADDI4SPN(this));
    addSupportedInstruction(new C_ADDIW(this));
    addSupportedInstruction(new C_ADDW(this));
    addSupportedInstruction(new C_AND(this));
    addSupportedInstruction(new C_ANDI(this));
    addSupportedInstruction(new C_BEQZ(this));
    addSupportedInstruction(new C_BNEZ(this));
    addSupportedInstruction(new C_EBREAK(this));
    addSupportedInstruction(new C_J(this));
#ifdef RV32C_ONLY
    addSupportedInstruction(new C_JAL(this));
#endif
    addSupportedInstruction(new C_JALR(this));
    addSupportedInstruction(new C_JR(this));
    addSupportedInstruction(new C_LD(this));
    addSupportedInstruction(new C_LDSP(this));
    addSupportedInstruction(new C_LWSP(this));
    addSupportedInstruction(new C_LI(this));
    addSupportedInstruction(new C_LUI(this));
    addSupportedInstruction(new C_LW(this));
    addSupportedInstruction(new C_MV(this));
    addSupportedInstruction(new C_NOP(this));
    addSupportedInstruction(new C_OR(this));
    addSupportedInstruction(new C_SD(this));
    addSupportedInstruction(new C_SDSP(this));
    addSupportedInstruction(new C_SLLI(this));
    addSupportedInstruction(new C_SRAI(this));
    addSupportedInstruction(new C_SRLI(this));
    addSupportedInstruction(new C_SUB(this));
    addSupportedInstruction(new C_SUBW(this));
    addSupportedInstruction(new C_SW(this));
    addSupportedInstruction(new C_SWSP(this));
    addSupportedInstruction(new C_XOR(this));

    portCSR_.write(CSR_misa, portCSR_.read(CSR_misa).val | (1LL << ('C' - 'A')));
}

}  // namespace debugger
