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
 */

#include "api_utils.h"
#include "arm-isa.h"
#include "cpu_arm7_func.h"

namespace debugger {

static const uint64_t PREFETCH_OFFSET[InstrModes_Total] = {
    8,
    4
};

/** Data processing default behaviour can be re-implemeted: */
int ArmDataProcessingInstruction::exec_checked(Reg64Type *payload) {
    DataProcessingType u;
    u.value = payload->buf32[0];
    uint32_t A = static_cast<uint32_t>(R[u.reg_bits.rn]);
    uint32_t M;
    uint32_t Res;
    if (u.imm_bits.I) {
        M = imm12(u.imm_bits);
    } else {
        M = static_cast<uint32_t>(R[u.reg_bits.rm]);
        M = shift12(u.reg_bits, M, R[u.reg_bits.shift >> 1]);
    }
    if (do_operation(A, M, &Res)) {
        R[u.reg_bits.rd] = Res;
    }
    if (is_flags_changed(u)) {
        set_flags(A, M, Res);
    }
    return 4;
}

bool ArmDataProcessingInstruction::is_flags_changed(DataProcessingType u) {
    return u.reg_bits.S && u.reg_bits.rd != Reg_pc;
}

void ArmDataProcessingInstruction::set_flags(uint32_t A, uint32_t M,
                                             uint32_t Res) {
    icpu_->setC(0);
    icpu_->setZ(Res == 0);
    icpu_->setN(Res >> 31);
}

/** @brief Subtruct specific instruction set

 Specific flags computation with subtraction
 */
class ArmSubInstruction : public ArmDataProcessingInstruction {
 public:
    ArmSubInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmDataProcessingInstruction(icpu, name, bits) {}
 protected:
    virtual bool is_inverted() = 0;
    virtual bool with_carry() = 0;
    virtual EOperationResult op_result() = 0;

    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        if (!is_inverted()) {
            *pRes = A - M;
        } else {
            *pRes = M - A;
        }
        if (with_carry()) {
            *pRes += (icpu_->getC() - 1);
        }
        return op_result();
    }

    virtual void set_flags(uint32_t A, uint32_t M, uint32_t Res) {
        if (is_inverted()) {
            uint32_t t1 = A;
            A = M;
            M = t1;
        }
        uint32_t C = !(((~A & M) | (M & Res) | (Res & ~A)) >> 31);
        uint32_t V = ((A & ~M & ~Res) | (~A & M & Res)) >> 31;
        icpu_->setC(C);
        icpu_->setV(V);
        icpu_->setZ(Res == 0);
        icpu_->setN(Res >> 31);
    }
};

/** @brief addictive specific instruction set

 Specific flags computation with addition
 */
class ArmAddInstruction : public ArmDataProcessingInstruction {
 public:
    ArmAddInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmDataProcessingInstruction(icpu, name, bits) {}
 protected:
    virtual EOperationResult op_result() = 0;
    virtual bool with_carry() = 0;

    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A + M;
        if (with_carry()) {
            *pRes += icpu_->getC();
        }
        return op_result();
    }

    virtual void set_flags(uint32_t A, uint32_t M, uint32_t Res) {
        uint32_t C = ((A & M) | (M & ~Res) | (A & ~Res)) >> 31;
        uint32_t V = ((A & M & ~Res) | (~A & ~M & Res)) >> 31;
        icpu_->setC(C);
        icpu_->setV(V);
        icpu_->setZ(Res == 0);
        icpu_->setN(Res >> 31);
    }
};

/** Single memory transfer: LDR, STR
*/
class SingleDataTransferInstruction : public ArmInstruction {
 public:
    SingleDataTransferInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        SingleDataTransferType u;
        u.value = payload->buf32[0];
        uint32_t opsz[2] = {4, 1};
        uint64_t off = R[u.imm_bits.rn];

        if (u.imm_bits.rn == Reg_pc) {
            off += PREFETCH_OFFSET[icpu_->getInstrMode()];
            if (u.imm_bits.rd == Reg_pc && !u.imm_bits.I 
                && (u.imm_bits.imm & 0x1)) {
                // DSB, ISB
                return 4;
            }
        }

        if (u.imm_bits.P) {
            off += get_increment(u);
            if (u.imm_bits.W) {
                R[u.imm_bits.rn] = off;
            }
        }

        trans_.addr = off;
        trans_.wstrb = 0;
        trans_.xsize = opsz[u.imm_bits.B];
        if (u.imm_bits.L) {
            trans_.action = MemAction_Read;
            trans_.rpayload.b64[0] = 0;
        } else {
            trans_.action = MemAction_Write;
            trans_.wstrb = (1 << trans_.xsize) - 1;
            trans_.wpayload.b64[0] = R[u.imm_bits.rd];
        }
        icpu_->dma_memop(&trans_);

        if (!u.imm_bits.P) {
            off += get_increment(u);
            R[u.imm_bits.rn] = off;
            if (u.imm_bits.W) {
            /** In the case of post-indexed addressing, the write back bit is
                redundant and must be set to zero, since the old base value can be
                retained by setting the offset to zero. Therefore post-indexed
                data transfers always write back the modified base. The only use of
                the W bit in a post-indexed data transfer is in privileged mode
                code, where setting the W bit forces non-privileged mode for the
                transfer, allowing the operating system to generate a user address
                in a system where the memory management hardware makes suitable
                use of this hardware.
            */
                RISCV_error("Post-index LDR,STR with W=1", 0);
            }
        }
        if (u.imm_bits.L) {
            R[u.imm_bits.rd] = trans_.rpayload.b32[0];
            if (u.imm_bits.rd == Reg_pc) {
                icpu_->setBranch(R[u.imm_bits.rd]);
            }
        }
        return 4;
    }

 protected:
    virtual uint64_t get_increment(SingleDataTransferType u) {
        uint64_t incr;
        if (!u.imm_bits.I) {
            incr = u.imm_bits.imm;
        } else {
            /**
             @warning The register specified shift amounts
                      are not available in this instruction class.
             */
            DataProcessingType u2;
            u2.value = u.value;
            incr = shift12(u2.reg_bits,
                           static_cast<uint32_t>(R[u.reg_bits.rm]),
                           0);  // !! not available
        }
        if (!u.reg_bits.U) {
            incr = (~incr) + 1;
        }
        return incr;
    }
};

/** Block memory transfer: LDM, STM
*/
class BlockDataTransferInstruction : public ArmInstruction {
 public:
    BlockDataTransferInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        BlockDataTransferType u;
        uint64_t adrincr[2] = {static_cast<uint64_t>(-4), 4};
        u.value = payload->buf32[0];
        uint32_t R15 = (u.bits.reglist >> 15) & 0x1;
        int ridx;
        // @todo Mode switching depending R15 value!!!!

        trans_.addr = R[u.bits.rn];
        trans_.xsize = 4;
        trans_.wstrb = 0;
        for (int i = 0; i < 16; i++) {
            if (u.bits.L) {
                ridx = 15 - i;
            } else {
                ridx = i;
            }
            if ((u.bits.reglist & (0x1 << ridx)) == 0) {
                continue;
            }
            if (u.bits.L) {
                trans_.action = MemAction_Read;
                trans_.rpayload.b64[0] = 0;
            } else {
                trans_.action = MemAction_Write;
                trans_.wstrb = (1 << trans_.xsize) - 1;
                trans_.wpayload.b64[0] = R[ridx];
            }
            if (u.bits.P) {
                trans_.addr += adrincr[u.bits.U];
                if (u.bits.W) {
                    R[u.bits.rn] = trans_.addr;
                }
            }

            icpu_->dma_memop(&trans_);

            if (!u.bits.P) {
                trans_.addr += adrincr[u.bits.U];
                R[u.bits.rn] = trans_.addr;
            }

            if (u.bits.L) {
                R[ridx] = trans_.rpayload.b32[0];
                if (ridx == Reg_pc) {
                    icpu_->setBranch(R[ridx]);
                }
            }
        }

        if (!u.bits.P) {
            if (u.bits.W) {
                RISCV_error("Post-index LDM,STM with W=1", 0);
            }
        }
        if (u.bits.S) {
            // TODO: force user mode
        }
        return 4;
    }
};


/** Halfword and Signed Data Transfer: LDRH/STRH/LDRSB/LDRSH
*/
class HWordSignedDataTransferInstruction : public ArmInstruction {
 public:
    HWordSignedDataTransferInstruction(CpuCortex_Functional *icpu,
        const char *name, const char *bits)
        : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        HWordSignedDataTransferType u;
        u.value = payload->buf32[0];
        uint32_t opsz[2] = {1, 2};
        uint64_t off = R[u.bits.rn];
        if (u.bits.rn == Reg_pc) {
            off += PREFETCH_OFFSET[icpu_->getInstrMode()];
        }

        if (u.bits.P) {
            off += get_increment(u);
        }

        trans_.addr = off;
        trans_.xsize = opsz[u.bits.h];
        trans_.wstrb = 0;
        if (u.bits.L) {
            trans_.action = MemAction_Read;
            trans_.rpayload.b64[0] = 0;
        } else {
            trans_.action = MemAction_Write;
            trans_.wstrb = (1 << trans_.xsize) - 1;
            trans_.wpayload.b64[0] = R[u.bits.rd];
        }
        icpu_->dma_memop(&trans_);

        if (u.bits.W) {
            if (!u.bits.P) {
                off += get_increment(u);
            }
            R[u.bits.rn] = off;
        }
        if (u.bits.L) {
            if (u.bits.h) {
                R[u.bits.rd] = trans_.rpayload.b16[0];
                if (u.bits.s) {
                    R[u.bits.rd] |= EXT_SIGN_16;
                }
            } else {
                R[u.bits.rd] = trans_.rpayload.b8[0];
                if (u.bits.s) {
                    R[u.bits.rd] |= EXT_SIGN_8;
                }
            }
            if (u.bits.rd == Reg_pc) {
                icpu_->setBranch(R[u.bits.rd]);
            }
        }
        return 4;
    }

 protected:
    virtual uint64_t get_increment(HWordSignedDataTransferType u) {
        uint64_t incr;
        if (!u.bits.reg_imm) {
            incr = (u.bits.imm_h << 4) | u.bits.rm;
        } else {
            incr = R[u.bits.rm];
        }
        if (!u.bits.U) {
            incr = (~incr) + 1;
        }
        return incr;
    }
};

/** Multiply common */
class MultiplyInstruction : public ArmInstruction {
 public:
    MultiplyInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        MulType u;
        u.value = payload->buf32[0];
        uint64_t res = R[u.bits.rm] * R[u.bits.rs];
        if (u.bits.A) {
            res += R[u.bits.rn];
        }
        R[u.bits.rd] = static_cast<uint32_t>(res);
        if (u.bits.S) {
            icpu_->setC(0); // set meaningless value
            icpu_->setZ(R[u.bits.rd] == 0);
            icpu_->setN(static_cast<uint32_t>(R[u.bits.rd]) >> 31);
        }
        return 4;
    }
};

/** Multiply Long common. */
class MultiplyLongInstruction : public ArmInstruction {
 public:
    MultiplyLongInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        MulLongType u;
        u.value = payload->buf32[0];
        uint64_t res = 0;
        if (u.bits.A) {
            res = static_cast<uint64_t>(R[u.bits.rdhi]) << 32;
            res |= R[u.bits.rdlo];
        }
        if (u.bits.S) {
            int64_t a = static_cast<int32_t>(R[u.bits.rm]);
            int64_t b = static_cast<int32_t>(R[u.bits.rs]);
            res = static_cast<uint64_t>(static_cast<int64_t>(res) + a * b);
        } else {
            uint64_t a = R[u.bits.rm];
            uint64_t b = R[u.bits.rs];
            res = res + a * b;
        }
        R[u.bits.rdlo] = static_cast<uint32_t>(res);
        R[u.bits.rdhi] = static_cast<uint32_t>(res >> 32);
        if (u.bits.S) {
            icpu_->setC(0);     // set to meaningless value
            icpu_->setV(0);     // set to meaningless value
            icpu_->setZ(res == 0);
            icpu_->setN(static_cast<uint32_t>(res >> 63));
        }
        return 4;
    }
};


/**
 * @brief AND.
 */
static const char *AND_OPCODES[2] = {
    "????0000000?????????????????????",
    "????0010000?????????????????????"
};

class AND : public ArmDataProcessingInstruction {
 public:
    AND(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "AND", AND_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A & M;
        return OP_Write;
    }
};

/** 
 * @brief EOR.
 */
static const char *EOR_OPCODES[2] = {
    "????0000001?????????????????????",
    "????0010001?????????????????????"
};

class EOR : public ArmDataProcessingInstruction {
 public:
    EOR(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "EOR", EOR_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A ^ M;
        return OP_Write;
    }
};

/** 
 * @brief Subtruct.
 */
static const char *SUB_OPCODES[2] = {
    "????0000010?????????????????????",
    "????0010010?????????????????????"
};

class SUB : public ArmSubInstruction {
 public:
    SUB(CpuCortex_Functional *icpu, int opidx) :
        ArmSubInstruction(icpu, "SUB", SUB_OPCODES[opidx]) {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Subtruct right.
 */
static const char *RSB_OPCODES[2] = {
    "????0000011?????????????????????",
    "????0010011?????????????????????"
};

class RSB : public ArmSubInstruction {
 public:
    RSB(CpuCortex_Functional *icpu, int opidx) :
        ArmSubInstruction(icpu, "RSB", RSB_OPCODES[opidx]) {}
 protected:
    virtual bool is_inverted() { return true; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Addition.
 */
static const char *ADD_OPCODES[2] = {
    "????0000100?????????????????????",
    "????0010100?????????????????????"
};

class ADD : public ArmAddInstruction {
 public:
    ADD(CpuCortex_Functional *icpu, int opidx) :
        ArmAddInstruction(icpu, "ADD", ADD_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult op_result() { return OP_Write; }
    virtual bool with_carry() { return false; }
};

/** 
 * @brief Addition with carry bit.
 */
static const char *ADC_OPCODES[2] = {
    "????0000101?????????????????????",
    "????0010101?????????????????????"
};

class ADC : public ArmAddInstruction {
 public:
    ADC(CpuCortex_Functional *icpu, int opidx) :
        ArmAddInstruction(icpu, "ADC", ADC_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult op_result() { return OP_Write; }
    virtual bool with_carry() { return true; }
};

/** 
 * @brief Subtruct with carry bit: Op1 - Op2 + C - 1 !!!!!!.
 */
static const char *SBC_OPCODES[2] = {
    "????0000110?????????????????????",
    "????0010110?????????????????????"
};

class SBC : public ArmSubInstruction {
 public:
    SBC(CpuCortex_Functional *icpu, int opidx) :
        ArmSubInstruction(icpu, "SBC", SBC_OPCODES[opidx]) {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return true; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Subtruct right with carry bit: Op2 - Op1 + C - 1 !!!!.
 */
static const char *RSC_OPCODES[2] = {
    "????0000111?????????????????????",
    "????0010111?????????????????????"
};

class RSC : public ArmSubInstruction {
 public:
    RSC(CpuCortex_Functional *icpu, int opidx) :
        ArmSubInstruction(icpu, "RSC", RSC_OPCODES[opidx]) {}
 protected:
    virtual bool is_inverted() { return true; }
    virtual bool with_carry() { return true; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/**
 * @brief Set condition codes on Op1 AND Op2.
 * S-flag must be set otherwise it can be MOVW instruction
 */
 static const char *TST_OPCODES[2] = {
    "????00010001????????????????????",
    "????00110001????????????????????"
};

class TST : public ArmDataProcessingInstruction {
 public:
    TST(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "TST", TST_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A & M;
        return OP_Drop;
    }
};

/**
 * @brief Set condition codes on Op1 EOR Op2.
 */
static const char *TEQ_OPCODES[2] = {
    "????0001001?????????????????????",
    "????0011001?????????????????????"
};

class TEQ : public ArmDataProcessingInstruction {
 public:
    TEQ(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "TEQ", TEQ_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A ^ M;
        return OP_Drop;
    }
};

/**
 * @brief Set condition codes on Op1 - Op2.
 */
static const char *CMP_OPCODES[2] = {
    "????0001010?????????????????????",
    "????0011010?????????????????????"
};

class CMP : public ArmSubInstruction {
 public:
    CMP(CpuCortex_Functional *icpu, int opidx) :
        ArmSubInstruction(icpu, "CMP", CMP_OPCODES[opidx]) {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Drop; }
};

/**
 * @brief Set condition codes on Op1 + Op2.
 */
static const char *CMN_OPCODES[2] = {
    "????0001011?????????????????????",
    "????0011011?????????????????????"
};

class CMN : public ArmAddInstruction {
 public:
    CMN(CpuCortex_Functional *icpu, int opidx) :
        ArmAddInstruction(icpu, "CMN", CMN_OPCODES[opidx]) {}
 protected:
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Drop; }
};

/**
 * @brief OR
 */
static const char *ORR_OPCODES[2] = {
    "????0001100?????????????????????",
    "????0011100?????????????????????"
};

class ORR : public ArmDataProcessingInstruction {
 public:
    ORR(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "ORR", ORR_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A | M;
        return OP_Write;
    }
};

/**
 * @brief Move data: Rd = Op2
 */
static const char *MOV_OPCODES[2] = {
    "????0001101?????????????????????",
    "????0011101?????????????????????"
};

class MOV : public ArmDataProcessingInstruction {
 public:
    MOV(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "MOV", MOV_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = M;
        return OP_Write;
    }
};

/** Move Top writes an immediate value to the top halfword of the destination
    register. It does not affect the contents of the bottom halfword. */
class MOVT : public ArmInstruction {
 public:
    MOVT(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MOVT", "????00110100????????????????????") {}
 protected:
    virtual int exec_checked(Reg64Type *payload) {
        DataProcessingType u;
        u.value = payload->buf32[0];
        uint32_t imm16 = (u.mov_bits.imm4 << 12) | u.mov_bits.imm12;
        R[u.mov_bits.rd] &= ~0xFFFF0000;
        R[u.mov_bits.rd] |= (imm16 << 16);
        return 4;
    }
};

/** This new variant of MOV (immediate) writes a 16-bit immediate 
    value to the destination register */
class MOVW : public ArmInstruction {
 public:
    MOVW(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MOVW", "????00110000????????????????????") {}
 protected:
    virtual int exec_checked(Reg64Type *payload) {
        DataProcessingType u;
        u.value = payload->buf32[0];
        uint32_t imm16 = (u.mov_bits.imm4 << 12) | u.mov_bits.imm12;
        R[u.mov_bits.rd] &= ~0xFFFF;
        R[u.mov_bits.rd] |= imm16;
        return 4;
    }
};

/** Rd:=Rm*Rs. */
class MUL : public MultiplyInstruction {
 public:
    MUL(CpuCortex_Functional *icpu) : MultiplyInstruction(icpu,
        "MUL", "????0000000?????????????1001????") {}
};

/** Multiply and accumulate:  Rd:=Rm*Rs+Rn */
class MLA : public MultiplyInstruction {
 public:
    MLA(CpuCortex_Functional *icpu) : MultiplyInstruction(icpu,
        "MLA", "????0000001?????????????1001????") {}
};

/** Unsigned Multiply Long */
class UMULL : public MultiplyLongInstruction {
 public:
    UMULL(CpuCortex_Functional *icpu) : MultiplyLongInstruction(icpu,
        "UMULL", "????0000100?????????????1001????") {}
};

/** Unsigned Multiply ^ Accumulate Long */
class UMLAL : public MultiplyLongInstruction {
 public:
    UMLAL(CpuCortex_Functional *icpu) : MultiplyLongInstruction(icpu,
        "UMLAL", "????0000101?????????????1001????") {}
};

/** Signed Multiply Long */
class SMULL : public MultiplyLongInstruction {
 public:
    SMULL(CpuCortex_Functional *icpu) : MultiplyLongInstruction(icpu,
        "SMULL", "????0000110?????????????1001????") {}
};

/** Signed Multiply ^ Accumulate Long */
class SMLAL : public MultiplyLongInstruction {
 public:
    SMLAL(CpuCortex_Functional *icpu) : MultiplyLongInstruction(icpu,
        "SMLAL", "????0000111?????????????1001????") {}
};

/**
 * @brief Bit clear: AND NOT
 */
static const char *BIC_OPCODES[2] = {
    "????0001110?????????????????????",
    "????0011110?????????????????????"
};

class BIC : public ArmDataProcessingInstruction {
 public:
    BIC(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "BIC", BIC_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = A & ~M;
        return OP_Write;
    }
};

/**
 * @brief Move inverted data: Rd = NOT Op2
 */
static const char *MVN_OPCODES[2] = {
    "????0001111?????????????????????",
    "????0011111?????????????????????"
};

class MVN : public ArmDataProcessingInstruction {
 public:
    MVN(CpuCortex_Functional *icpu, int opidx) :
        ArmDataProcessingInstruction(icpu, "MVN", MVN_OPCODES[opidx]) {}
 protected:
    virtual EOperationResult do_operation(uint32_t A, uint32_t M,
                                          uint32_t *pRes) {
        *pRes = ~M;
        return OP_Write;
    }
};

/** Branch */
class B : public ArmInstruction {
public:
    B(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "B", "????1010????????????????????????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchType u;
        u.value = payload->buf32[0];
        uint32_t off = u.bits.offset;
        if ((u.value >> 23) & 0x1) {
            off |= 0xFF000000;
        }
        off = static_cast<uint32_t>(
            R[Reg_pc] + (off << 2) + PREFETCH_OFFSET[icpu_->getInstrMode()]);
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        return 4;
    }
};

/** Branch with Link */
class BL : public ArmInstruction {
public:
    BL(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "BL", "????1011????????????????????????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchType u;
        u.value = payload->buf32[0];
        R[Reg_lr] = R[Reg_pc] + 4;
        uint32_t off = u.bits.offset;
        if ((u.value >> 23) & 0x1) {
            off |= 0xFF000000;
        }
        off = static_cast<uint32_t>(
            R[Reg_pc] + (off << 2) + PREFETCH_OFFSET[icpu_->getInstrMode()]);
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        return 4;
    }
};

/** Branch and Exchange */
class BX : public ArmInstruction {
public:
    BX(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "BX", "????000100101111111111110001????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchType u;
        u.value = payload->buf32[0];
        uint32_t off = static_cast<uint32_t>(R[u.bits.offset & 0xf]);
        if (off & 0x1) {
            icpu_->setInstrMode(THUMB_mode);
        } else {
            icpu_->setInstrMode(ARM_mode);
        }
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        return 4;
    }
};

/** Load word pre-adding immediate offset to base register */
static const char *LDR_OPCODES[4] = {
    "????0100???1????????????????????", // immedaiate, post incr
    "????0101???1????????????????????", // immedaiate, pre incr
    "????0110???1????????????????????", // shift reg, post incr
    "????0111???1????????????????????", // shift reg, pre incr
};

class LDR : public SingleDataTransferInstruction {
public:
    LDR(CpuCortex_Functional *icpu, int opidx) :
        SingleDataTransferInstruction(icpu, "LDR", LDR_OPCODES[opidx]) {}
};

/** Load Signed Byte */
static const char *LDRSB_OPCODES[4] = {
    "????0000???1????????????1101????", // post incr
    "????0001???1????????????1101????", // pre incr
};

class LDRSB : public HWordSignedDataTransferInstruction {
public:
    LDRSB(CpuCortex_Functional *icpu, int opidx) :
        HWordSignedDataTransferInstruction(icpu, "LDRSB", LDRSB_OPCODES[opidx])
        {}
};

/** Load Unsigned Half-Word */
static const char *LDRH_OPCODES[4] = {
    "????0000???1????????????1011????", // post incr
    "????0001???1????????????1011????", // pre incr
};

class LDRH : public HWordSignedDataTransferInstruction {
public:
    LDRH(CpuCortex_Functional *icpu, int opidx) :
        HWordSignedDataTransferInstruction(icpu, "LDRH", LDRH_OPCODES[opidx])
        {}
};

/** Load Signed Half-word */
static const char *LDRSH_OPCODES[4] = {
    "????0000???1????????????1111????", // post incr
    "????0001???1????????????1111????", // pre incr
};

class LDRSH : public HWordSignedDataTransferInstruction {
public:
    LDRSH(CpuCortex_Functional *icpu, int opidx) :
        HWordSignedDataTransferInstruction(icpu, "LDRSH", LDRSB_OPCODES[opidx])
        {}
};

/** Load Block data */
static const char *LDM_OPCODES[4] = {
    "????1000???1????????????????????", // post incr
    "????1001???1????????????????????", // pre incr
};

class LDM : public BlockDataTransferInstruction {
 public:
    LDM(CpuCortex_Functional *icpu, int opidx) :
        BlockDataTransferInstruction(icpu, "LDM", LDM_OPCODES[opidx]) {}
};



/** Store word */
static const char *STR_OPCODES[4] = {
    "????0100?0?0????????????????????", // immedaiate, post incr
    "????0101?0?0????????????????????", // immedaiate, pre incr
    "????0110?0?0????????????????????", // shift reg, post incr
    "????0111?0?0????????????????????", // shift reg, pre incr
};

class STR : public SingleDataTransferInstruction {
public:
    STR(CpuCortex_Functional *icpu, int opidx) :
        SingleDataTransferInstruction(icpu, "STR", STR_OPCODES[opidx]) {}
};

/** Store Byte */
static const char *STRB_OPCODES[4] = {
    "????0100?1?0????????????????????", // immedaiate, post incr
    "????0101?1?0????????????????????", // immedaiate, pre incr
    "????0110?1?0????????????????????", // shift reg, post incr
    "????0111?1?0????????????????????", // shift reg, pre incr
};

class STRB : public SingleDataTransferInstruction {
public:
    STRB(CpuCortex_Functional *icpu, int opidx) :
        SingleDataTransferInstruction(icpu, "STRB", STRB_OPCODES[opidx]) {}
};

/** Store Half-Word */
static const char *STRH_OPCODES[4] = {
    "????0000???0????????????1?11????", // post incr
    "????0001???0????????????1?11????", // pre incr
};

class STRH : public HWordSignedDataTransferInstruction {
public:
    STRH(CpuCortex_Functional *icpu, int opidx) :
        HWordSignedDataTransferInstruction(icpu, "STRH", STRH_OPCODES[opidx])
        {}
};

/** Store Block data */
static const char *STM_OPCODES[4] = {
    "????1000???0????????????????????", // post incr
    "????1001???0????????????????????", // pre incr
};

class STM : public BlockDataTransferInstruction {
 public:
    STM(CpuCortex_Functional *icpu, int opidx) :
        BlockDataTransferInstruction(icpu, "STM", STM_OPCODES[opidx]) {}
};

/** Move from coprocessor to ARM7TDMI-S register (L=1) */
class MRC : public ArmInstruction {
 public:
    MRC(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRC", "????1110???1???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        CoprocessorTransferType u;
        u.value = payload->buf32[0];
        return 4;
    }
};

/** Move from ARM7TDMI-S register to coprocessor (L=0) */
class MCR : public ArmInstruction {
 public:
    MCR(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRC", "????1110???0???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        CoprocessorTransferType u;
        u.value = payload->buf32[0];
        return 4;
    }
};

/** 
 * @brief Transfer register to PSR flags.
 */
static const char *MSR_OPCODES[2] = {
    "????00010?10????1111????????????",
    "????00110?10????1111????????????"
};

class MSR : public ArmInstruction {
public:
    MSR(CpuCortex_Functional *icpu, int opidx) :
        ArmInstruction(icpu, "MSR", MSR_OPCODES[opidx]) {}

    virtual int exec_checked(Reg64Type *payload) {
        PsrTransferType u;
        u.value = payload->buf32[0];
        return 4;
    }
};

class MRS : public ArmInstruction {
public:
    MRS(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRS", "????00010?001111????000000000000") {}

    virtual int exec_checked(Reg64Type *payload) {
        PsrTransferType u;
        u.value = payload->buf32[0];
        return 4;
    }
};

class NOP : public ArmInstruction {
public:
    NOP(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "NOP", "????0011001000001111000000000000") {}

    virtual int exec_checked(Reg64Type *payload) {
        return 4;
    }
};

/** Bytes zero-extending
 UXTB extracts an 8-bit value from a register and zero extends it to 32 bits.
 You can specify a rotation by 0, 8, 16, or 24 bits before extracting 
 the 8-bit value.
 */
class UXTB : public ArmInstruction {
public:
    UXTB(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTB", "????011011101111????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        t1 &= 0xFF;
        R[u.bits.rd] = static_cast<uint32_t>(t1);
        return 4;
    }
};

/** */
class UXTAB : public ArmInstruction {
public:
    UXTAB(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTAB", "????01101110????????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        t1 &= 0xFF;
        R[u.bits.rd] = R[u.bits.rn] + static_cast<uint32_t>(t1);
        return 4;
    }
};

/** UXTB16 extracts two 8-bit values from a register and zero extends them
  to 16 bits each. You can specify a rotation by 0, 8, 16, or 24 bits before
  extracting the 8-bit values.
 */
class UXTB16 : public ArmInstruction {
public:
    UXTB16(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTB16", "????011011001111????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        t1 &= 0x00FF00FF;
        R[u.bits.rd] = static_cast<uint32_t>(t1);
        return 4;
    }
};

/** UXTAB16 extracts two 8-bit values from a register, zero extends them
 to 16 bits each, and adds the results to the two values from another
 register. You can specify a rotation by 0, 8, 16, or 24 bits before
 extracting the 8-bit values.
 */
class UXTAB16 : public ArmInstruction {
public:
    UXTAB16(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTAB16", "????01101100????????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        uint32_t a = (R[u.bits.rd] & 0xFFFF) + (t1 & 0xFF);
        uint32_t b = ((R[u.bits.rd] >> 16) & 0xFFFF) + ((t1 >> 16) & 0xFF);
        R[u.bits.rd] = (b << 16) | a;
        return 4;
    }
};


/** UXTH extracts a 16-bit value from a register and zero extends it
 to 32 bits. You can specify a rotation by 0, 8, 16, or 24 bits before
 extracting the 16-bit value.
 */
class UXTH : public ArmInstruction {
public:
    UXTH(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTH", "????011011111111????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        t1 &= 0xFFFF;
        R[u.bits.rd] = static_cast<uint32_t>(t1);
        return 4;
    }
};

/** UXTAH extracts a 16-bit value from a register, zero extends it to 32 bits,
 and adds the result to a value in another register. You can specify a
 rotation by 0, 8, 16, or 24 bits before extracting the 16-bit value.
 */
class UXTAH : public ArmInstruction {
public:
    UXTAH(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "UXTAH", "????01101111????????????0111????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SignExtendType u;
        u.value = payload->buf32[0];
        uint64_t t1 = (R[u.bits.rm] & 0xFFFFFFFF) >> (8*u.bits.rotate);
        t1 = (R[u.bits.rm] << (32 - 8*u.bits.rotate)) | t1;
        t1 &= 0xFFFF;
        R[u.bits.rd] = R[u.bits.rn] + static_cast<uint32_t>(t1);
        return 4;
    }
};


/**
 * @brief SWI (software breakpoint instruction)
 *
 */
class SWI : public ArmInstruction {
 public:
    SWI(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "SWI", "????1110???1???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        icpu_->raiseSoftwareIrq();
        return 4;
    }
};



void CpuCortex_Functional::addArm7tmdiIsa() {

    // Arm V6 instructions:
    //      CPS, SRS, RFE
    //      REV, REV16, REVSH
    //      SETEND
    //      LDREX, STREX
    //      SXTB, SXTH, UXTB, UXTH
    addSupportedInstruction(new UXTB(this));    // same opcode as STRB
    addSupportedInstruction(new UXTAB(this));
    addSupportedInstruction(new UXTB16(this));  // rn=1111b
    addSupportedInstruction(new UXTAB16(this));  // rn=????b
    addSupportedInstruction(new UXTH(this));    // rn=1111b
    addSupportedInstruction(new UXTAH(this));   // rn=????

    // use the same as opcodes TEQ, TST, CMN and CMP without S-flag
    // and must be registered first.
    addSupportedInstruction(new BX(this));  // use TST,MSR opcode
    addSupportedInstruction(new NOP(this)); // use MSR opcode
    addSupportedInstruction(new MOVT(this));
    addSupportedInstruction(new MOVW(this));
    addSupportedInstruction(new MUL(this));
    addSupportedInstruction(new MLA(this));
    addSupportedInstruction(new UMULL(this));
    addSupportedInstruction(new UMLAL(this));
    addSupportedInstruction(new SMULL(this));
    addSupportedInstruction(new SMLAL(this));
    addSupportedInstruction(new MRS(this));
    addSupportedInstruction(new MSR(this, 0));
    addSupportedInstruction(new MSR(this, 1));

    addSupportedInstruction(new AND(this, 0));
    addSupportedInstruction(new AND(this, 1));
    addSupportedInstruction(new EOR(this, 0));
    addSupportedInstruction(new EOR(this, 1));
    addSupportedInstruction(new SUB(this, 0));
    addSupportedInstruction(new SUB(this, 1));
    addSupportedInstruction(new RSB(this, 0));
    addSupportedInstruction(new RSB(this, 1));
    addSupportedInstruction(new ADD(this, 0));
    addSupportedInstruction(new ADD(this, 1));
    addSupportedInstruction(new ADC(this, 0));
    addSupportedInstruction(new ADC(this, 1));
    addSupportedInstruction(new SBC(this, 0));
    addSupportedInstruction(new SBC(this, 1));
    addSupportedInstruction(new RSC(this, 0));
    addSupportedInstruction(new RSC(this, 1));
    addSupportedInstruction(new TST(this, 0));
    addSupportedInstruction(new TST(this, 1));
    addSupportedInstruction(new TEQ(this, 0));
    addSupportedInstruction(new TEQ(this, 1));
    addSupportedInstruction(new CMP(this, 0));
    addSupportedInstruction(new CMP(this, 1));
    addSupportedInstruction(new CMN(this, 0));
    addSupportedInstruction(new CMN(this, 1));
    addSupportedInstruction(new ORR(this, 0));
    addSupportedInstruction(new ORR(this, 1));
    addSupportedInstruction(new MOV(this, 0));
    addSupportedInstruction(new MOV(this, 1));
    addSupportedInstruction(new BIC(this, 0));
    addSupportedInstruction(new BIC(this, 1));
    addSupportedInstruction(new MVN(this, 0));
    addSupportedInstruction(new MVN(this, 1));
    addSupportedInstruction(new B(this));
    addSupportedInstruction(new BL(this));
    addSupportedInstruction(new LDR(this, 0));
    addSupportedInstruction(new LDR(this, 1));
    addSupportedInstruction(new LDR(this, 2));
    addSupportedInstruction(new LDR(this, 3));
    addSupportedInstruction(new LDRSB(this, 0));
    addSupportedInstruction(new LDRSB(this, 1));
    addSupportedInstruction(new LDRH(this, 0));
    addSupportedInstruction(new LDRH(this, 1));
    addSupportedInstruction(new LDRSH(this, 0));
    addSupportedInstruction(new LDRSH(this, 1));
    addSupportedInstruction(new LDM(this, 0));
    addSupportedInstruction(new LDM(this, 1));
    addSupportedInstruction(new STR(this, 0));
    addSupportedInstruction(new STR(this, 1));
    addSupportedInstruction(new STR(this, 2));
    addSupportedInstruction(new STR(this, 3));
    addSupportedInstruction(new STRB(this, 0));
    addSupportedInstruction(new STRB(this, 1));
    addSupportedInstruction(new STRB(this, 2));
    addSupportedInstruction(new STRB(this, 3));
    addSupportedInstruction(new STRH(this, 0));
    addSupportedInstruction(new STRH(this, 1));
    addSupportedInstruction(new STM(this, 0));
    addSupportedInstruction(new STM(this, 1));
    addSupportedInstruction(new MCR(this));
    addSupportedInstruction(new MRC(this));
    addSupportedInstruction(new SWI(this));
}

}  // namespace debugger
