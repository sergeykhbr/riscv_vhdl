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
 * @brief      Base ISA implementation (extension I, user level).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

/** 
 * @brief Addition. Overflows are ignored
 */
class ADD : public RiscvInstruction {
public:
    ADD(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ADD", "0000000??????????000?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] + R[u.bits.rs2]);
        return 4;
    }
};

/** 
 * @brief Add immediate
 *
 * ADDI adds the sign-extended 12-bit immediate to register rs1. 
 * Arithmetic overflow is ignored and the result is simply the low 32-bits of
 * the result. ADDI rd, rs1, 0 is used to implement the MV rd, rs1 assembler 
 * pseudo-instruction.
 */
class ADDI : public RiscvInstruction {
public:
    ADDI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ADDI", "?????????????????000?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] + imm);
        return 4;
    }
};

/** 
 * @brief Add immediate with sign extending (RV64I)
 *
 * ADDIW is an RV64I-only instruction that adds the sign-extended 12-bit 
 * immediate to register rs1 and produces the proper sign-extension of 
 * a 32-bit result in rd. Overflows are ignored and the result is the low 
 * 32 bits of the result sign-extended to 64 bits. Note, ADDIW rd, rs1, 0 
 * writes the sign-extension of the lower 32 bits of register rs1 into 
 * register rd (assembler pseudo-op SEXT.W).
 */
class ADDIW : public RiscvInstruction {
public:
    ADDIW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ADDIW", "?????????????????000?????0011011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        uint64_t res = (R[u.bits.rs1] + imm) & 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/** 
 * @brief Add registers with sign extending (RV64I)
 *
 * ADDW is RV64I-only instructions that are defined analogously to 
 * ADD but operate on 32-bit values and produce signed 32-bit results.
 * Overflows are ignored, and the low 32-bits of the result is sign-extended 
 * to 64-bits and written to the destination register.
 */
class ADDW : public RiscvInstruction {
public:
    ADDW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ADDW", "0000000??????????000?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
    
        uint64_t res = (R[u.bits.rs1] + R[u.bits.rs2]) & 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/** 
 * @brief AND bitwise logical operation.
 */
class AND : public RiscvInstruction {
public:
    AND(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "AND", "0000000??????????111?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] & R[u.bits.rs2]);
        return 4;
    }
};

/** 
 * @brief ANDI logical operation with immediate.
 *
 * ANDI, ORI, XORI are logical operations that perform bitwise AND, OR, 
 * and XOR on register rs1 and the sign-extended 12-bit immediate and place 
 * the result in rd.
 */
class ANDI : public RiscvInstruction {
public:
    ANDI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ANDI", "?????????????????111?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }

        icpu_->setReg(u.bits.rd, R[u.bits.rs1] & imm);
        return 4;
    }
};

/**
 * @brief AUIPC (add upper immediate to pc)
 *
 * AUIPC is used to build pc-relative addresses and uses the U-type
 * format. AUIPC forms a 32-bit offset from the 20-bit U-immediate, 
 * filling in the lowest 12 bits with zeros, adds this offset to the pc, 
 * then places the result in register rd.
 */
class AUIPC : public RiscvInstruction {
public:
    AUIPC(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "AUIPC", "?????????????????????????0010111") {}

    virtual int exec(Reg64Type *payload) {
        ISA_U_type u;
        u.value = payload->buf32[0];

        uint64_t off = u.bits.imm31_12 << 12;
        if (off & (1LL << 31)) {
            off |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, icpu_->getPC() + off);
        return 4;
    }
};

/**
 * @brief The BEQ (Branch if registers are equal)
 */
class BEQ : public RiscvInstruction {
public:
    BEQ(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BEQ", "?????????????????000?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (R[u.bits.rs1] == R[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};

/**
 * @brief The BGE (Branch if greater than or equal using signed comparision)
 *
 * All branch instructions use the SB-type instruction format. The 12-bit 
 * B-immediate encodes signed offsets in multiples of 2, and is added to the 
 * current pc to give the target address. The conditional branch range 
 * is ±4 KiB.
 */
class BGE : public RiscvInstruction {
public:
    BGE(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BGE", "?????????????????101?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (static_cast<int64_t>(R[u.bits.rs1]) >= 
            static_cast<int64_t>(R[u.bits.rs2])) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};


/**
 * @brief The BGEU (Branch if greater than or equal using unsigned comparision)
 */
class BGEU : public RiscvInstruction {
public:
    BGEU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BGEU", "?????????????????111?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (R[u.bits.rs1] >= R[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};

/**
 * @brief The BLT (Branch if less than using signed comparision)
 */
class BLT : public RiscvInstruction {
public:
    BLT(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BLT", "?????????????????100?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (static_cast<int64_t>(R[u.bits.rs1]) < 
            static_cast<int64_t>(R[u.bits.rs2])) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};

/**
 * @brief The BLTU (Branch if less than using unsigned comparision)
 */
class BLTU : public RiscvInstruction {
public:
    BLTU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BLTU", "?????????????????110?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (R[u.bits.rs1] < R[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};

/**
 * @brief The BNE (Branch if registers are unequal)
 */
class BNE : public RiscvInstruction {
public:
    BNE(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "BNE", "?????????????????001?????1100011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_SB_type u;
        u.value = payload->buf32[0];
    
        if (R[u.bits.rs1] != R[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            icpu_->setBranch(icpu_->getPC() + imm);
        }
        return 4;
    }
};

/**
 * @brief JAL (Jump and link).
 *
 * Unconditional jump. The offset is sign-extended and added to the pc to form
 * the jump target address. Jumps can therefore target a ±1 MiB range. JAL 
 * stores the address of the instruction following the jump (pc+4) into 
 * register rd. The standard software calling convention uses x1 as the return 
 * address register.
 *
 * J (pseudo-op) 0 Plain unconditional jumps are encoded as a JAL with rd=x0.
 */
class JAL : public RiscvInstruction {
public:
    JAL(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "JAL", "?????????????????????????1101111") {}

    virtual int exec(Reg64Type *payload) {
        ISA_UJ_type u;
        u.value = payload->buf32[0];
        uint64_t off = 0;
        if (u.bits.imm20) {
            off = 0xfffffffffff00000LL;
        }
        off |= (u.bits.imm19_12 << 12);
        off |= (u.bits.imm11 << 11);
        off |= (u.bits.imm10_1 << 1);
        if (u.bits.rd != 0) {
            icpu_->setReg(u.bits.rd, icpu_->getPC() + 4);
        }
        icpu_->setBranch(icpu_->getPC() + off);
        if (u.bits.rd == Reg_ra) {
            icpu_->pushStackTrace();
        }
        return 4;
    }
};

/**
 * @brief JALR (Jump and link register).
 *
 * The target address is obtained by adding the 12-bit signed I-immediate to 
 * the register rs1, then setting the least-significant bit of the result to 
 * zero. The address of the instruction following the jump (pc+4) is written 
 * to register rd. Register x0 can be used as the destination if the result 
 * is not required.
 */
class JALR : public RiscvInstruction {
public:
    JALR(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "JALR", "?????????????????000?????1100111") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (u.bits.imm & 0x800) {
            off |= 0xfffffffffffff000LL;
        }
        off += R[u.bits.rs1];
        off &= ~0x1LL;
        if (u.bits.rd != 0) {
            icpu_->setReg(u.bits.rd, icpu_->getPC() + 4);
        }
        icpu_->setBranch(off);

        // Stack trace buffer:
        if (u.bits.rd == Reg_ra) {
            icpu_->pushStackTrace();
        } else if (u.bits.imm == 0 && u.bits.rs1 == Reg_ra) {
            icpu_->popStackTrace();
        }
        return 4;
    }
};

/**
 * @brief LOAD instructions (LD, LW, LH, LB) with sign extending.
 *
 * Loads copy a value from memory to register rd.
 * The effective byte address is obtained by adding register rs1 to the 
 * sign-extended 12-bit offset.
 *   The LW instruction loads a 32-bit value from memory into rd. LH loads 
 * a 16-bit value from memory, then sign-extends to 32-bits before storing 
 * in rd.
 */ 
class LD : public RiscvInstruction {
public:
    LD(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LD", "?????????????????011?????0000011") {}

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
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionLoadData(&trans);
            }
        }
        icpu_->setReg(u.bits.rd, trans.rpayload.b64[0]);
        return 4;
    }
};

/**
 * Load 32-bits with sign extending.
 */
class LW : public RiscvInstruction {
public:
    LW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LW", "?????????????????010?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 4;
        if (trans.addr & 0x3) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionLoadData(&trans);
            }
        }
        uint64_t res = trans.rpayload.b64[0];
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * Load 32-bits with zero extending.
 */
class LWU : public RiscvInstruction {
public:
    LWU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LWU", "?????????????????110?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 4;
        if (trans.addr & 0x3) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionLoadData(&trans);
            }
        }
        icpu_->setReg(u.bits.rd, trans.rpayload.b64[0]);
        return 4;
    }
};

/**
 * Load 16-bits with sign extending.
 */
class LH : public RiscvInstruction {
public:
    LH(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LH", "?????????????????001?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 2;
        if (trans.addr & 0x1) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionLoadData(&trans);
            }
        }
        uint64_t res = trans.rpayload.b16[0];
        if (res & (1LL << 15)) {
            res |= EXT_SIGN_16;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * Load 16-bits with zero extending.
 */
class LHU : public RiscvInstruction {
public:
    LHU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LHU", "?????????????????101?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 2;
        if (trans.addr & 0x1) {
            trans.rpayload.b64[0] = 0;
            icpu_->raiseSignal(EXCEPTION_LoadMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionLoadData(&trans);
            }
        }
        icpu_->setReg(u.bits.rd, trans.rpayload.b16[0]);
        return 4;
    }
};

/**
 * Load 8-bits with sign extending.
 */
class LB : public RiscvInstruction {
public:
    LB(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LB", "?????????????????000?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 1;
        if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
            icpu_->exceptionLoadData(&trans);
        }
        uint64_t res = trans.rpayload.b8[0];
        if (res & (1LL << 7)) {
            res |= EXT_SIGN_8;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * Load 8-bits with zero extending.
 */
class LBU : public RiscvInstruction {
public:
    LBU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LBU", "?????????????????100?????0000011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.rpayload.b64[0] = 0;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 1;
        if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
            icpu_->exceptionLoadData(&trans);
        }
        icpu_->setReg(u.bits.rd, trans.rpayload.b8[0]);
        return 4;
    }
};

/**
 * @brief LUI (load upper immediate).
 *
 * It is used to build 32-bit constants and uses the U-type format. LUI places
 * the U-immediate value in the top 20 bits of the destination register rd,
 * filling in the lowest 12 bits with zeros.
 */
class LUI : public RiscvInstruction {
public:
    LUI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LUI", "?????????????????????????0110111") {}

    virtual int exec(Reg64Type *payload) {
        ISA_U_type u;
        u.value = payload->buf32[0];
        uint64_t tmp = u.bits.imm31_12 << 12;
        if (tmp & 0x80000000) {
            tmp |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, tmp);
        return 4;
    }
};

/** 
 * @brief OR bitwise operation
 */
class OR : public RiscvInstruction {
public:
    OR(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "OR", "0000000??????????110?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] | R[u.bits.rs2]);
        return 4;
    }
};

/** 
 * @brief OR on register rs1 and the sign-extended 12-bit immediate.
 */
class ORI : public RiscvInstruction {
public:
    ORI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "ORI", "?????????????????110?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] | imm);
        return 4;
    }
};

/**
 * @brief SLLI is a logical left shift (zeros are shifted into the lower bits)
 */
class SLLI : public RiscvInstruction {
public:
    SLLI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLLI", "000000???????????001?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint32_t shamt = u.bits.imm & 0x3f;
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] << shamt);
        return 4;
    }
};

/**
 * @brief The SLT (signed comparision)
 *
 * It places the value 1 in register rd if rs1 < rs2, 0 otherwise 
 */
class SLT : public RiscvInstruction {
public:
    SLT(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLT", "0000000??????????010?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        if (static_cast<int64_t>(R[u.bits.rs1]) <
                static_cast<int64_t>(R[u.bits.rs2])) {
            res = 1;
        } else {
            res = 0;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief The SLTI (set less than immediate)
 *
 * It places the value 1 in register rd if register rs1 is less than the
 * sign-extended immediate when both are treated as signed numbers, else 0 
 * is written to rd. 
 */
class SLTI : public RiscvInstruction {
public:
    SLTI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLTI", "?????????????????010?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        uint64_t res;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        if (static_cast<int64_t>(R[u.bits.rs1]) <
                static_cast<int64_t>(imm)) {
            res = 1;
        } else {
            res = 0;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief The SLTU (unsigned comparision)
 *
 * SLTU perform unsigned compares, writing 1 to rd if rs1 < rs2, 0 otherwise.
 * @note SLTU rd, x0, rs2 sets rd to 1 if rs2 is not equal to zero, otherwise 
 * sets rd to zero (assembler pseudo-op SNEZ rd, rs).
 */
class SLTU : public RiscvInstruction {
public:
    SLTU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLTU", "0000000??????????011?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        if (R[u.bits.rs1] < R[u.bits.rs2]) {
            res = 1;
        } else {
            res = 0;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief The SLTIU (set less than immediate comparing unsgined values)
 *
 * SLTIU is similar but compares the values as unsigned numbers (i.e., the 
 * immediate is first sign-extended to 32-bits then treated as an unsigned 
 * number). Note, SLTIU rd, rs1, 1 sets rd to 1 if rs1 equals zero, otherwise
 * sets rd to 0 (assembler pseudo-op SEQZ rd, rs).
 */
class SLTIU : public RiscvInstruction {
public:
    SLTIU(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLTIU", "?????????????????011?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        uint64_t res;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        if (R[u.bits.rs1] < imm) {
            res = 1;
        } else {
            res = 0;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief SLL logical shift left
 */
class SLL : public RiscvInstruction {
public:
    SLL(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLL", "0000000??????????001?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] << (R[u.bits.rs2] & 0x3F));
        return 4;
    }
};

/**
 * @brief SLLW is a left shifts by register defined value (RV64I).
 */
class SLLW : public RiscvInstruction {
public:
    SLLW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLLW", "0000000??????????001?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        res = R[u.bits.rs1] << (R[u.bits.rs2] & 0x1F);
        res &= 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief SLLIW is a shifts by a constant (RV64I).
 *
 * SLLIW, SRLIW, and SRAIW are RV64I-only instructions that operate on 32-bit 
 * values and produce signed 32-bit results.
 * @exception Illegal_Instruction if imm[5] not equal to 0.
 */
class SLLIW : public RiscvInstruction {
public:
    SLLIW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SLLIW", "0000000??????????001?????0011011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        uint32_t shamt = u.bits.imm & 0x1f;
        res = R[u.bits.rs1] << shamt;
        res &= 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        if ((u.bits.imm >> 5) & 0x1) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
        }
        return 4;
    }
};

/**
 * @brief SRA arithmetic shift right
 */
class SRA : public RiscvInstruction {
public:
    SRA(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRA", "0100000??????????101?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd,
                static_cast<int64_t>(R[u.bits.rs1]) >> (R[u.bits.rs2] & 0x3F));
        return 4;
    }
};

/**
 * @brief SRAW 32-bits arithmetic shift right (RV64I)
 */
class SRAW : public RiscvInstruction {
public:
    SRAW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRAW", "0100000??????????101?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        int32_t t1 = static_cast<int32_t>(R[u.bits.rs1]);
        icpu_->setReg(u.bits.rd,
                    static_cast<int64_t>(t1 >> (R[u.bits.rs2] & 0x1F)));
        return 4;
    }
};


/**
 * @brief SRAI is an arithmetic right shift.
 *
 * The original sign bit is copied into the vacanted upper bits.
 */
class SRAI : public RiscvInstruction {
public:
    SRAI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRAI", "010000???????????101?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint32_t shamt = u.bits.imm & 0x3f;
        icpu_->setReg(u.bits.rd, static_cast<int64_t>(R[u.bits.rs1]) >> shamt);
        return 4;
    }
};

/**
 * @brief SRAIW arithmetic right shift (RV64I)
 */
class SRAIW : public RiscvInstruction {
public:
    SRAIW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRAIW", "0100000??????????101?????0011011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        int32_t t1 = static_cast<int32_t>(R[u.bits.rs1]);
        uint32_t shamt = u.bits.imm & 0x1f;
        icpu_->setReg(u.bits.rd, static_cast<int64_t>(t1 >> shamt));
        if ((u.bits.imm >> 5) & 0x1) {
            icpu_->raiseSignal(EXCEPTION_InstrIllegal);
        }
        return 4;
    }
};


/**
 * @brief SRL logical shift right
 */
class SRL : public RiscvInstruction {
public:
    SRL(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRL", "0000000??????????101?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] >> (R[u.bits.rs2] & 0x3F));
        return 4;
    }
};

/**
 * @brief SRLI is a logical right shift (zeros are shifted into the upper bits)
 */
class SRLI : public RiscvInstruction {
public:
    SRLI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRLI", "000000???????????101?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint32_t shamt = u.bits.imm & 0x3f;
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] >> shamt);
        return 4;
    }
};

/**
 * @brief SRLIW logical right shift (RV64I)
 */
class SRLIW : public RiscvInstruction {
public:
    SRLIW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRLIW", "0000000??????????101?????0011011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint32_t shamt = u.bits.imm & 0x1f;
        uint32_t res = static_cast<uint32_t>(R[u.bits.rs1]);
        res >>= shamt;
        icpu_->setReg(u.bits.rd, static_cast<int64_t>(static_cast<int32_t>(res)));
        return 4;
    }
};

/**
 * @brief SRLW is a right shifts by register defined value (RV64I).
 */
class SRLW : public RiscvInstruction {
public:
    SRLW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SRLW", "0000000??????????101?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        res = static_cast<uint32_t>(R[u.bits.rs1]) >> (R[u.bits.rs2] & 0x1f);
        res &= 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/**
 * @brief STORE instructions (SD, SW, SH, SB)
 *
 * This instruction copies the value in register rs2 to memory.
 * The effective byte address is obtained by adding register rs1 to the 
 * sign-extended 12-bit offset.
 *   The SW, SH, and SB instructions store 32-bit, 16-bit, and 8-bit values 
 * from the low bits of register rs2 to memory.
 */ 
class SD : public RiscvInstruction {
public:
    SD(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SD", "?????????????????011?????0100011") {}

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
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2];
        if (trans.addr & 0x7) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionStoreData(&trans);
            }
        }
        return 4;
    }
};

/**
 * @brief Store rs2[31:0] to memory.
 */
class SW : public RiscvInstruction {
public:
    SW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SW", "?????????????????010?????0100011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.wpayload.b64[0] = 0;
        ISA_S_type u;
        u.value = payload->buf32[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Write;
        trans.xsize = 4;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2];
        if (trans.addr & 0x3) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionStoreData(&trans);
            }
        }
        return 4;
    }
};

/**
 * @brief Store rs2[15:0] to memory.
 */
class SH : public RiscvInstruction {
public:
    SH(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SH", "?????????????????001?????0100011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.wpayload.b64[0] = 0;
        ISA_S_type u;
        u.value = payload->buf32[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Write;
        trans.xsize = 2;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2] & 0xFFFF;
        if (trans.addr & 0x1) {
            icpu_->raiseSignal(EXCEPTION_StoreMisalign);
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->exceptionStoreData(&trans);
            }
        }
        return 4;
    }
};

/**
 * @brief Store rs2[7:0] to memory.
 */
class SB : public RiscvInstruction {
public:
    SB(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SB", "?????????????????000?????0100011") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        trans.wpayload.b64[0] = 0;
        ISA_S_type u;
        u.value = payload->buf32[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Write;
        trans.xsize = 1;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = R[u.bits.rs2] & 0xFF;
        if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
            icpu_->exceptionStoreData(&trans);
        }
        return 4;
    }
};

/** 
 * @brief Subtruction. Overflows are ignored
 */
class SUB : public RiscvInstruction {
public:
    SUB(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SUB", "0100000??????????000?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] - R[u.bits.rs2]);
        return 4;
    }
};

/** 
 * @brief Substruct registers with sign extending (RV64I)
 *
 * SUBW is RV64I-only instructions that are defined analogously to 
 * SUB but operate on 32-bit values and produce signed 32-bit results.
 * Overflows are ignored, and the low 32-bits of the result is sign-extended 
 * to 64-bits and written to the destination register.
 */
class SUBW : public RiscvInstruction {
public:
    SUBW(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SUBW", "0100000??????????000?????0111011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        uint64_t res;
        u.value = payload->buf32[0];
        
        res = (R[u.bits.rs1] - R[u.bits.rs2]) & 0xFFFFFFFFLL;
        if (res & (1LL << 31)) {
            res |= EXT_SIGN_32;
        }
        icpu_->setReg(u.bits.rd, res);
        return 4;
    }
};

/** 
 * @brief XOR bitwise operation
 */
class XOR : public RiscvInstruction {
public:
    XOR(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "XOR", "0000000??????????100?????0110011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        u.value = payload->buf32[0];
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] ^ R[u.bits.rs2]);
        return 4;
    }
};

/** 
 * @brief XOR on register rs1 and the sign-extended 12-bit immediate.
 *
 * XORI rd, rs1, -1 performs a bitwise logical inversion of register rs1 
 * (assembler pseudo-instruction NOT rd, rs).
 */
class XORI : public RiscvInstruction {
public:
    XORI(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "XORI", "?????????????????100?????0010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_I_type u;
        u.value = payload->buf32[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        icpu_->setReg(u.bits.rd, R[u.bits.rs1] ^ imm);
        return 4;
    }
};

void CpuRiver_Functional::addIsaUserRV64I() {
    addSupportedInstruction(new ADD(this));
    addSupportedInstruction(new ADDI(this));
    addSupportedInstruction(new ADDIW(this));
    addSupportedInstruction(new ADDW(this));
    addSupportedInstruction(new AND(this));
    addSupportedInstruction(new ANDI(this));
    addSupportedInstruction(new AUIPC(this));
    addSupportedInstruction(new BEQ(this));
    addSupportedInstruction(new BGE(this));
    addSupportedInstruction(new BGEU(this));
    addSupportedInstruction(new BLT(this));
    addSupportedInstruction(new BLTU(this));
    addSupportedInstruction(new BNE(this));
    addSupportedInstruction(new JAL(this));
    addSupportedInstruction(new JALR(this));
    addSupportedInstruction(new LD(this));
    addSupportedInstruction(new LW(this));
    addSupportedInstruction(new LWU(this));
    addSupportedInstruction(new LH(this));
    addSupportedInstruction(new LHU(this));
    addSupportedInstruction(new LB(this));
    addSupportedInstruction(new LBU(this));
    addSupportedInstruction(new LUI(this));
    addSupportedInstruction(new OR(this));
    addSupportedInstruction(new ORI(this));
    addSupportedInstruction(new SLL(this));
    addSupportedInstruction(new SLLI(this));
    addSupportedInstruction(new SLLIW(this));
    addSupportedInstruction(new SLLW(this));
    addSupportedInstruction(new SLT(this));
    addSupportedInstruction(new SLTI(this));
    addSupportedInstruction(new SLTU(this));
    addSupportedInstruction(new SLTIU(this));
    addSupportedInstruction(new SRA(this));
    addSupportedInstruction(new SRAI(this));
    addSupportedInstruction(new SRAIW(this));
    addSupportedInstruction(new SRAW(this));
    addSupportedInstruction(new SRL(this));
    addSupportedInstruction(new SRLI(this));
    addSupportedInstruction(new SRLIW(this));
    addSupportedInstruction(new SRLW(this));
    addSupportedInstruction(new SUB(this));
    addSupportedInstruction(new SUBW(this));
    addSupportedInstruction(new SD(this));
    addSupportedInstruction(new SW(this));
    addSupportedInstruction(new SH(this));
    addSupportedInstruction(new SB(this));
    addSupportedInstruction(new XOR(this));
    addSupportedInstruction(new XORI(this));

  /*
  def SLLI_RV32          = BitPat("b0000000??????????001?????0010011")
  def SRLI_RV32          = BitPat("b0000000??????????101?????0010011")
  def SRAI_RV32          = BitPat("b0100000??????????101?????0010011")
  */
    /** Base[XLEN-1:XLEN-2]
     *      1 = 32
     *      2 = 64
     *      3 = 128
     */
    uint64_t isa = 0x8000000000000000LL;
    isa |= (1LL << ('I' - 'A'));
    portCSR_.write(CSR_misa, isa);
}

}  // namespace debugger
