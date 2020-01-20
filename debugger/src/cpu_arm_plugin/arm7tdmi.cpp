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

#include "api_core.h"
#include "arm-isa.h"
#include "cpu_arm7_func.h"

namespace debugger {

static const uint64_t PREFETCH_OFFSET[InstrModes_Total] = {
    8,
    4
};

// PC offset computation in different modes
static const uint64_t PC_SHIFT[InstrModes_Total] = {
    2,
    1
};

// Target Bit[0]: 0 = ARM mode; 1= Thumb mode
static const uint64_t LR_LSB[InstrModes_Total] = {
    0,
    1
};

// PC masking bits on branch
static const uint64_t PC_MASK[InstrModes_Total] = {
    0xFFFFFFFC,
    0xFFFFFFFE
};

// Default instruction length in bytes: 4 = ARM mode; 2= Thumb mode
// Some instructions (BL) in Thumb mode could be 4 bytes length
static const int INSTR_LEN[InstrModes_Total] = {
    4,
    2
};

/** Data processing default behaviour can be re-implemeted: */
int ArmDataProcessingInstruction::exec_checked(Reg64Type *payload) {
    DataProcessingType u;
    u.value = payload->buf32[0];
    uint32_t A = static_cast<uint32_t>(R[u.reg_bits.rn]);
    uint32_t M;
    uint32_t Res;

    if (u.reg_bits.rn == Reg_pc) {
        A += 8;
    }

    if (u.imm_bits.I) {
        M = imm12(u.imm_bits);
    } else {
        M = static_cast<uint32_t>(R[u.reg_bits.rm]);
        M = shift12(u.reg_bits, M, R[u.reg_bits.shift >> 1]);
    }
    if (do_operation(A, M, &Res)) {
        R[u.reg_bits.rd] = Res;
        if (u.reg_bits.rd == Reg_pc) {
            icpu_->setBranch(R[u.imm_bits.rd]);
        }
    }
    if (is_flags_changed(u)) {
        set_flags(A, M, Res);
    }
    return INSTR_LEN[icpu_->getInstrMode()];
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
                icpu_->popStackTrace();
            }
        }
        return INSTR_LEN[icpu_->getInstrMode()];
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
        int rcount = 0;
        // @todo Mode switching depending R15 value!!!!

        for (int i = 0; i < 16; i++) {
            if ((u.bits.reglist & (0x1 << i)) != 0) {
                rcount++;
            }
        }

        if (u.bits.U) {
            trans_.addr = R[u.bits.rn];
        } else {
            trans_.addr = R[u.bits.rn] - rcount * 4;
            if (!u.bits.P) {
                trans_.addr += 4;
            }
        }

        trans_.xsize = 4;
        trans_.wstrb = 0;

        for (int i = 0; i < 16; i++) {
            if ((u.bits.reglist & (0x1 << i)) == 0) {
                continue;
            }
            if (u.bits.L) {
                trans_.action = MemAction_Read;
                trans_.rpayload.b64[0] = 0;
            } else {
                trans_.action = MemAction_Write;
                trans_.wstrb = (1 << trans_.xsize) - 1;
                trans_.wpayload.b64[0] = R[i];
            }

            icpu_->dma_memop(&trans_);

            trans_.addr += 4;
            if (u.bits.W) {
                R[u.bits.rn] += adrincr[u.bits.U];
            }

            if (u.bits.L) {
                R[i] = trans_.rpayload.b32[0];
                if (i == Reg_pc) {
                    icpu_->setBranch(R[i]);
                    icpu_->popStackTrace();
                }
            }
        }
/*
        if (!u.bits.P) {
            if (u.bits.W) {
                RISCV_error("Post-index LDM,STM with W=1", 0);
            }
        }
*/
        if (u.bits.S) {
            // TODO: force user mode
        }
        return INSTR_LEN[icpu_->getInstrMode()];
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
        uint32_t opsz[8] = {0, 2, 8, 8, 0, 2, 1, 2};
        uint64_t off = R[u.bits.rn];
        if (u.bits.rn == Reg_pc) {
            off += PREFETCH_OFFSET[icpu_->getInstrMode()];
        }

        if (u.bits.P) {
            off += get_increment(u);
        }

        trans_.addr = off;
        trans_.xsize = opsz[(u.bits.L << 2) | (u.bits.s << 1) | (u.bits.h)];
        trans_.wstrb = 0;
        if (u.bits.L) {
            trans_.action = MemAction_Read;
            trans_.rpayload.b64[0] = 0;
        } else {
            if (!u.bits.s) {
                trans_.action = MemAction_Write;
                trans_.wstrb = (1 << trans_.xsize) - 1;
                trans_.wpayload.b64[0] = R[u.bits.rd];
            } else {
                if (u.bits.h) {
                    trans_.action = MemAction_Write;
                    trans_.wstrb = (1 << trans_.xsize) - 1;
                    trans_.wpayload.b64[0] = R[u.bits.rd] | (R[u.bits.rd + 1] << 32);
                } else {
                    trans_.action = MemAction_Read;
                    trans_.rpayload.b64[0] = 0;
                }
            }
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
                    if ((R[u.bits.rd] & 0x8000) !=0 ) {
                        R[u.bits.rd] |= EXT_SIGN_16;
                    }
                }
            } else {
                R[u.bits.rd] = trans_.rpayload.b8[0];
                if (u.bits.s) {
                    if ((R[u.bits.rd] & 0x80) != 0) {
                        R[u.bits.rd] |= EXT_SIGN_8;
                    }
                }
            }
            if (u.bits.rd == Reg_pc) {
                icpu_->setBranch(R[u.bits.rd]);
            }
        } else {
            if (u.bits.s && !u.bits.h) {
                R[u.bits.rd] = trans_.rpayload.b32[0];
                R[u.bits.rd + 1] = trans_.rpayload.b32[1];
            }
        }
        return INSTR_LEN[icpu_->getInstrMode()];
    }

 protected:
    virtual uint64_t get_increment(HWordSignedDataTransferType u) {
        uint64_t incr;
        if (u.bits.reg_imm) {
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

/** Double Word Data Transfer */
class DWordSignedDataTransferInstruction : public ArmInstruction {
 public:
    DWordSignedDataTransferInstruction(CpuCortex_Functional *icpu,
        const char *name, const char *bits) :
        ArmInstruction (icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        // LDRD imm      ????000?_?1?0????_????????_1101????
        // LDRD literal  ????0001_?1001111_????????_1101????
        // LDRD reg      ????000?_?0?0????_????0000_1101????
        // STRD imm      ????000?_?1?0????_????????_1111????
        // STRD reg      ????000?_?0?0????_????0000_1111????
        DWordDataTransferType u;
        u.value = payload->buf32[0];
        uint64_t off = R[u.bits.rn];
        if (u.bits.rn == Reg_pc) {
            off += PREFETCH_OFFSET[icpu_->getInstrMode()];
        }

        if (u.bits.P) {
            off += get_increment(u);
        }

        trans_.addr = off;
        trans_.xsize = 8;
        trans_.wstrb = 0;
        if (!u.bits.ld_st) {
            trans_.action = MemAction_Read;
            trans_.rpayload.b64[0] = 0;
        } else {
            trans_.action = MemAction_Write;
            trans_.wstrb = (1 << trans_.xsize) - 1;
            trans_.wpayload.b64[0] = R[u.bits.rt] | (R[u.bits.rt + 1] << 32);
        }
        icpu_->dma_memop(&trans_);

        if (u.bits.W) {
            if (!u.bits.P) {
                off += get_increment(u);
            }
            R[u.bits.rn] = off;
        }
        if (!u.bits.ld_st) {
            R[u.bits.rt] = trans_.rpayload.b32[0];
            R[u.bits.rt + 1] = trans_.rpayload.b32[1];
            if (u.bits.rt == Reg_pc) {
                icpu_->setBranch(R[u.bits.rt]);
            }
        }
        return 4;
    }
 protected:
    virtual uint64_t get_increment(DWordDataTransferType u) {
        uint64_t incr;
        if (u.bits.reg_imm) {
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

/** Bits Field Operation */
class BitsFieldInstruction : public ArmInstruction {
 public:
    BitsFieldInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction (icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        // BFC      ????0111_110?????_????????_?0011111
        // BFI      ????0111_110?????_????????_?001????
        BitsFieldType u;
        u.value = payload->buf32[0];

        if (u.bits.msb < u.bits.lsb) {
            RISCV_error("%s", "BFI unpredicateble");
        }

        uint64_t mskd = (1ull << (u.bits.msb + 1)) - 1;
        mskd = (mskd >> u.bits.lsb) << u.bits.lsb;

        R[u.bits.rd] &= ~mskd;
        if (u.bits.rn != 0xF) {
            uint64_t mskn = (1ull << (u.bits.msb - u.bits.lsb + 1)) - 1;
            R[u.bits.rd] |= (R[u.bits.rn] & mskn) << u.bits.lsb;
        }
        return 4;
    }
};


/** Divide common */
class DivideInstruction : public ArmInstruction {
 public:
    DivideInstruction(CpuCortex_Functional *icpu, const char *name,
        const char *bits) : ArmInstruction(icpu, name, bits) {}

    virtual int exec_checked(Reg64Type *payload) {
        DivType u;
        u.value = payload->buf32[0];
        uint64_t res;

        if (u.bits.S) {     //UDIV
            res = R[u.bits.rn] / R[u.bits.rm];
        } else {            //SDIV
            res = (int32_t)R[u.bits.rn] / (int32_t)R[u.bits.rm];
        }

        R[u.bits.rd] = static_cast<uint32_t>(res);
        return 4;
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
        return INSTR_LEN[icpu_->getInstrMode()];
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
class AND : public ArmDataProcessingInstruction {
 public:
    AND(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "AND", "????00?0000?????????????????????") {}
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
class EOR : public ArmDataProcessingInstruction {
 public:
    EOR(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "EOR", "????00?0001?????????????????????") {}
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
class SUB : public ArmSubInstruction {
 public:
    SUB(CpuCortex_Functional *icpu) : ArmSubInstruction(
        icpu, "SUB", "????00?0010?????????????????????") {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Subtruct right.
 */
class RSB : public ArmSubInstruction {
 public:
    RSB(CpuCortex_Functional *icpu) : ArmSubInstruction(
        icpu, "RSB", "????00?0011?????????????????????") {}
 protected:
    virtual bool is_inverted() { return true; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Addition.
 */
class ADD : public ArmAddInstruction {
 public:
    ADD(CpuCortex_Functional *icpu) : ArmAddInstruction(
        icpu, "ADD", "????00?0100?????????????????????") {}
 protected:
    virtual EOperationResult op_result() { return OP_Write; }
    virtual bool with_carry() { return false; }
};

/** 
 * @brief Addition with carry bit.
 */
class ADC : public ArmAddInstruction {
 public:
    ADC(CpuCortex_Functional *icpu) : ArmAddInstruction(
        icpu, "ADC", "????00?0101?????????????????????") {}
 protected:
    virtual EOperationResult op_result() { return OP_Write; }
    virtual bool with_carry() { return true; }
};

/** 
 * @brief Subtruct with carry bit: Op1 - Op2 + C - 1 !!!!!!.
 */
class SBC : public ArmSubInstruction {
 public:
    SBC(CpuCortex_Functional *icpu) : ArmSubInstruction(
        icpu, "SBC", "????00?0110?????????????????????") {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return true; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/** 
 * @brief Subtruct right with carry bit: Op2 - Op1 + C - 1 !!!!.
 */
class RSC : public ArmSubInstruction {
 public:
    RSC(CpuCortex_Functional *icpu) : ArmSubInstruction(
        icpu, "RSC", "????00?0111?????????????????????") {}
 protected:
    virtual bool is_inverted() { return true; }
    virtual bool with_carry() { return true; }
    virtual EOperationResult op_result() { return OP_Write; }
};

/**
 * @brief Set condition codes on Op1 AND Op2.
 * S-flag must be set otherwise it can be MOVW instruction
 */
class TST : public ArmDataProcessingInstruction {
 public:
    TST(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "TST", "????00?10001????????????????????") {}
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
class TEQ : public ArmDataProcessingInstruction {
 public:
    TEQ(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "TEQ", "????00?1001?????????????????????") {}
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
class CMP : public ArmSubInstruction {
 public:
    CMP(CpuCortex_Functional *icpu) : ArmSubInstruction(
        icpu, "CMP", "????00?1010?????????????????????") {}
 protected:
    virtual bool is_inverted() { return false; }
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Drop; }
};

/**
 * @brief Set condition codes on Op1 + Op2.
 */
class CMN : public ArmAddInstruction {
 public:
    CMN(CpuCortex_Functional *icpu) : ArmAddInstruction(
        icpu, "CMN", "????00?1011?????????????????????") {}
 protected:
    virtual bool with_carry() { return false; }
    virtual EOperationResult op_result() { return OP_Drop; }
};

/**
 * @brief OR
 */
class ORR : public ArmDataProcessingInstruction {
 public:
    ORR(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "ORR", "????00?1100?????????????????????") {}
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
class MOV : public ArmDataProcessingInstruction {
 public:
    MOV(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "MOV", "????00?1101?????????????????????") {}
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

/** Rd:=Rn/Rm. */
class UDIV : public DivideInstruction {
public:
    UDIV(CpuCortex_Functional *icpu) : DivideInstruction(icpu,
        "UDIV", "????01110011????1111????0001????") {}
};

class SDIV : public DivideInstruction {
public:
    SDIV(CpuCortex_Functional *icpu) : DivideInstruction(icpu,
        "SDIV", "????01110001????1111????0001????") {}
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
class BIC : public ArmDataProcessingInstruction {
 public:
    BIC(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "BIC", "????00?1110?????????????????????") {}
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
class MVN : public ArmDataProcessingInstruction {
 public:
    MVN(CpuCortex_Functional *icpu) : ArmDataProcessingInstruction(
        icpu, "MVN", "????00?1111?????????????????????") {}
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
        int mode = static_cast<int>(icpu_->getInstrMode());
        off = static_cast<uint32_t>(
            R[Reg_pc] + (off << PC_SHIFT[mode]) + PREFETCH_OFFSET[mode]);
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        return INSTR_LEN[mode];
    }
};

/** Branch with Link */
class BL : public ArmInstruction {
 public:
    BL(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "BL", "????1011????????????????????????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchType u;
        int mode = static_cast<int>(icpu_->getInstrMode());
        u.value = payload->buf32[0];
        R[Reg_lr] = (R[Reg_pc] + 4) | LR_LSB[mode]; // Thumb BL = 4 bytes
        uint32_t off = u.bits.offset;               // next instruction addr
        if ((u.value >> 23) & 0x1) {
            off |= 0xFF000000;
        }
        off = static_cast<uint32_t>(
            R[Reg_pc] + (off << PC_SHIFT[mode]) + PREFETCH_OFFSET[mode]);
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        icpu_->pushStackTrace();
        return 4;
    }
};

/** Branch with Link and Exchange*/
class BLX : public ArmInstruction {
 public:
    BLX(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "BLX", "????000100101111111111110011????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchExchangeIndirectType u;
        int mode = static_cast<int>(icpu_->getInstrMode());
        int modenxt;
        u.value = payload->buf32[0];
        // next instruction addr
        R[Reg_lr] = (R[Reg_pc] + INSTR_LEN[mode]) | LR_LSB[mode];
        // ARMv5T and above:
        uint64_t off = R[u.bits.rm];
        if (off & 0x1) {
            modenxt = THUMB_mode;
        } else {
            modenxt = ARM_mode;
        }
        off &= PC_MASK[modenxt];
        R[Reg_pc] = off;                             // ARMv5T
        icpu_->setBranch(off);
        icpu_->pushStackTrace();
        icpu_->setInstrMode(static_cast<EInstructionModes>(modenxt));
        return INSTR_LEN[mode];
    }
};

/** Branch and Exchange */
class BX : public ArmInstruction {
 public:
    BX(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "BX", "????000100101111111111110001????") {}

    virtual int exec_checked(Reg64Type *payload) {
        BranchType u;
        int modenxt;
        int mode = static_cast<int>(icpu_->getInstrMode());
        u.value = payload->buf32[0];
        uint32_t off = static_cast<uint32_t>(R[u.bits.offset & 0xf]);
        if (off & 0x1) {
            modenxt = THUMB_mode;
        } else {
            modenxt = ARM_mode;
        }
        off &= PC_MASK[modenxt];
        R[Reg_pc] = off;
        icpu_->setBranch(off);
        if ((u.bits.offset & 0xf) == Reg_lr) {
            icpu_->popStackTrace();
        }
        icpu_->setInstrMode(static_cast<EInstructionModes>(modenxt));
        return INSTR_LEN[mode];
    }
};

/** Load word pre-adding immediate offset to base register */
class LDR : public SingleDataTransferInstruction {
 public:
    LDR(CpuCortex_Functional *icpu) : SingleDataTransferInstruction(
        icpu, "LDR", "????01?????1????????????????????") {}
};

class LDRB : public SingleDataTransferInstruction {
 public:
    LDRB(CpuCortex_Functional *icpu) :
        SingleDataTransferInstruction(icpu, "LDRB",
            "????01???1?1????????????????????") {}
};

/** Load Signed Byte */
class LDRSB : public HWordSignedDataTransferInstruction {
 public:
    LDRSB(CpuCortex_Functional *icpu) : HWordSignedDataTransferInstruction(
        icpu, "LDRSB", "????000????1????????????1101????") {}
};

/** Load Unsigned Half-Word */
class LDRH : public HWordSignedDataTransferInstruction {
 public:
    LDRH(CpuCortex_Functional *icpu) : HWordSignedDataTransferInstruction(
        icpu, "LDRH", "????000????1????????????1011????") {}
};

/** Load Signed Half-word */
class LDRSH : public HWordSignedDataTransferInstruction {
 public:
    LDRSH(CpuCortex_Functional *icpu) : HWordSignedDataTransferInstruction(
        icpu, "LDRSH", "????000????1????????????1111????") {}
};

/** Load Double-Word */
class LDRD : public DWordSignedDataTransferInstruction {
 public:
    LDRD(CpuCortex_Functional *icpu) : DWordSignedDataTransferInstruction(
        icpu, "LDRD", "????000????0????????????1101????") {}
};


/** Load Block data */
class LDM : public BlockDataTransferInstruction {
 public:
    LDM(CpuCortex_Functional *icpu) : BlockDataTransferInstruction(
        icpu, "LDM", "????100????1????????????????????") {}
};

/** Store word */
class STR : public SingleDataTransferInstruction {
 public:
    STR(CpuCortex_Functional *icpu) : SingleDataTransferInstruction(
        icpu, "STR", "????01???0?0????????????????????") {}
};

/** Store Byte */
class STRB : public SingleDataTransferInstruction {
 public:
    STRB(CpuCortex_Functional *icpu) : SingleDataTransferInstruction(
        icpu, "STRB", "????01???1?0????????????????????") {}
};

/** Store Half-Word */
class STRH : public HWordSignedDataTransferInstruction {
 public:
    STRH(CpuCortex_Functional *icpu) : HWordSignedDataTransferInstruction(
        icpu, "STRH", "????000????0????????????1?11????") {}
};

/** Store Double-Word */
class STRD : public DWordSignedDataTransferInstruction {
 public:
    STRD(CpuCortex_Functional *icpu) : DWordSignedDataTransferInstruction(
        icpu, "STRD", "????000????0????????????1111????") {}
};

/** Store Block data */
class STM : public BlockDataTransferInstruction {
 public:
    STM(CpuCortex_Functional *icpu) : BlockDataTransferInstruction(
        icpu, "STM", "????1001???0????????????????????") {}
};

/** Move from coprocessor to ARM7TDMI-S register (L=1) */
class MRC : public ArmInstruction {
 public:
    MRC(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRC", "????1110???1???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        //CoprocessorTransferType u;
        //u.value = payload->buf32[0];
        return 4;
    }
};

/** Move from ARM7TDMI-S register to coprocessor (L=0) */
class MCR : public ArmInstruction {
 public:
    MCR(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRC", "????1110???0???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        //CoprocessorTransferType u;
        //u.value = payload->buf32[0];
        return 4;
    }
};

/** 
 * @brief Transfer register to PSR flags.
 */
class MSR : public ArmInstruction {
 public:
    MSR(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MSR", "????00?10?10????1111????????????") {}

    virtual int exec_checked(Reg64Type *payload) {
        //PsrTransferType u;
        //u.value = payload->buf32[0];
        return 4;
    }
};

class MRS : public ArmInstruction {
 public:
    MRS(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "MRS", "????00010?001111????000000000000") {}

    virtual int exec_checked(Reg64Type *payload) {
        //PsrTransferType u;
        //u.value = payload->buf32[0];
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
        return INSTR_LEN[icpu_->getInstrMode()];
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
        return INSTR_LEN[icpu_->getInstrMode()];
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
 * ARM does not define a specific breakpoint instruction. It can be different
 * in different OSes. On ARM Linux it's usually an UND opcode
 * (e.g. FE DE FF E7) in ARM mode and BKPT (BE BE) in Thumb.
 */
class SWI : public ArmInstruction {
 public:
    SWI(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "SWI", "????1110???1???????????????1????") {}

    virtual int exec_checked(Reg64Type *payload) {
        icpu_->raiseSoftwareIrq();
        icpu_->doNotCache(icpu_->getPC());
        return INSTR_LEN[icpu_->getInstrMode()];
    }
};

/**
 * @brief SWP (Single Data Swap)
 *
 */
class SWP : public ArmInstruction {
 public:
    SWP(CpuCortex_Functional *icpu) :
        ArmInstruction(icpu, "SWP", "????00010?00????????00001001????") {}

    virtual int exec_checked(Reg64Type *payload) {
        SingleDataTransferType u;
        u.value = payload->buf32[0];
        uint32_t opsz[2] = {4, 1};

        trans_.addr = R[u.reg_bits.rn];
        trans_.wstrb = 0;
        trans_.xsize = opsz[u.reg_bits.B];
        trans_.action = MemAction_Read;
        trans_.rpayload.b64[0] = 0;
        
        icpu_->dma_memop(&trans_);
        R[u.reg_bits.rd] = trans_.rpayload.b64[0];

        trans_.action = MemAction_Write;
        trans_.wstrb = (1 << trans_.xsize) - 1;
        trans_.wpayload.b64[0] = R[u.reg_bits.rm];
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

class BFC : public BitsFieldInstruction {
 public:
    BFC(CpuCortex_Functional *icpu) :
        BitsFieldInstruction(icpu, "BFC", "????0111110??????????????0011111") {}
};

class BFI : public BitsFieldInstruction {
 public:
    BFI(CpuCortex_Functional *icpu) :
        BitsFieldInstruction(icpu, "BFI", "????0111110??????????????001????") {}
};

/** 4.6.3 ADD (immediate) */
class ADD_I_T1 : public T1Instruction {
 public:
    ADD_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_I_T2 : public T1Instruction {
 public:
    ADD_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;
        bool setflags = !icpu_->InITBlock();
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t Rn = static_cast<uint32_t>(R[dn]);

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_I_T3 : public T1Instruction {
 public:
    ADD_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.4 ADD (register) */
class ADD_R_T1 : public T1Instruction {
 public:
    ADD_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;

        result = AddWithCarry(Rn, Rm, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class ADD_R_T2 : public T1Instruction {
 public:
    ADD_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t DN = (ti >> 7) & 0x1;
        uint32_t dn = (DN << 3) | (ti & 0x7);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;

        result = AddWithCarry(Rn, Rm, 0, &overflow, &carry);
        icpu_->setReg(dn, result);
        if (dn == Reg_pc) {
            ALUWritePC(result);
        }
        return 2;
    }
};

class ADD_R_T3 : public T1Instruction {
 public:
    ADD_R_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t type = (ti1 >> 4) & 0x3;
        uint32_t shift_n;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t shifted;

        SRType shift_t = DecodeImmShift(type, (imm3 << 2) | imm2, &shift_n);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        shifted = Shift_C(Rm, shift_t, shift_n, icpu_->getC(), &carry);
        result = AddWithCarry(Rn, shifted, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        if (d == Reg_pc) {
            ALUWritePC(result);     // setflags always FALSE
        } else {
            if (setflags) {
                // Mask 0x7 no need to check on SP or PC
                icpu_->setN((result >> 31) & 1);
                icpu_->setZ(result == 0 ? 1: 0);
                icpu_->setC(carry);
                icpu_->setV(overflow);
            }
        }
        return 4;
    }
};

/** 4.6.5 ADD (SP plus immediate) */
class ADDSP_I_T1 : public T1Instruction {
 public:
    ADDSP_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADD") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t SP = static_cast<uint32_t>(R[Reg_sp]);

        result = AddWithCarry(SP, imm32, 0, &overflow, &carry);
        icpu_->setReg(d, result);
        return 2;
    }
};

/** 4.6.7 ADR */
class ADR_T1 : public T1Instruction {
 public:
    ADR_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ADR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = (ti >> 8) & 0x7;
        uint32_t imm32 = (ti & 0xFF) << 2;
        uint32_t pc = static_cast<uint32_t>(icpu_->getPC()) + 4;

        pc &= ~0x3;             // Word-aligned PC
        icpu_->setReg(d, pc + imm32);
        return 2;
    }
};

/** 4.6.8 AND (immediate) */
class AND_I_T1 : public T1Instruction {
 public:
    AND_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "AND.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.9 AND (register) */
class AND_R_T1 : public T1Instruction {
 public:
    AND_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ANDS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t result = Rn & Rm;

        icpu_->setReg(dn, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            // C the same because no shoft
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.10 ASR (immediate) */
class ASR_I_T1 : public T1Instruction {
 public:
    ASR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ASR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t result;
        uint32_t d = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t shift_n;
        uint32_t carry;
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        DecodeImmShift(2, imm5, &shift_n);
        result = Shift_C(Rm, SRType_ASR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.12 B */
class B_T1 : public T1Instruction {
 public:
    B_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t cond = (ti >> 8) & 0xF;
        uint32_t imm8 = ti & 0xFF;
        uint32_t imm32 = imm8 << 1;
        if (ti & 0x80) {
            imm32 |= (~0ul) << 9;
        }

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (check_cond(icpu_, cond)) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

class B_T2 : public T1Instruction {
 public:
    B_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t imm11 = ti & 0x7FF;
        uint32_t imm32 = imm11 << 1;
        if (ti & 0x400) {
            imm32 |= (~0ul) << 12;
        }

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
        BranchWritePC(npc);
        return 2;
    }
};

class B_T3 : public T1Instruction {
 public:
    B_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "B.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t cond = (ti >> 6) & 0xF;
        uint32_t imm6 = ti & 0x3F;
        uint32_t imm11 = ti1 & 0x7FF;
        uint32_t S = (ti >> 10) & 1;
        uint32_t J1 = (ti1 >> 13) & 1;
        uint32_t J2 = (ti1 >> 11) & 1;
        uint32_t imm32 = (J2 << 19) | (J1 << 18) | (imm6 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 20;
        }

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (check_cond(icpu_, cond)) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 4;
    }
};

/** 4.6.15 BIC (immediate) */
class BIC_I_T1 : public T1Instruction {
 public:
    BIC_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BIC.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti0  & 0xF;
        bool setflags = (ti0 >> 4) & 1;
        uint32_t i = (ti0 >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & ~imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.18 BL, BLX (immediate) */
class BL_I_T1 : public T1Instruction {
 public:
    BL_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BL_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t instr0 = payload->buf16[0];
        uint32_t instr1 = payload->buf16[1];
        uint32_t S = (instr0 >> 10) & 1;
        uint32_t I1 = (((instr1 >> 13) & 0x1) ^ S) ^ 1;
        uint32_t I2 = (((instr1 >> 11) & 0x1) ^ S) ^ 1;
        uint32_t imm11 = instr1 & 0x7FF;
        uint32_t imm10 = instr0 & 0x3FF;
        uint32_t imm32 = 
        imm32 = (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
        if (S) {
            imm32 |= (~0ul) << 24;
        }

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t pc = static_cast<uint32_t>(icpu_->getPC()) + 4;
        uint32_t npc = pc + imm32;
        icpu_->setInstrMode(THUMB_mode);
        icpu_->setReg(Reg_lr, pc | 0x1);
        BranchWritePC(npc);
        return 4;
    }
};

/** 4.6.19 BLX (register) */
class BLX_R_T1 : public T1Instruction {
 public:
    BLX_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BLX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint64_t next_instr_addr = icpu_->getPC() + 4 - 2;

        if (m == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        icpu_->setReg(Reg_lr, next_instr_addr | 0x1);
        BXWritePC(Rm);
        return 2;
    }
};

/** 4.6.20 BX */
class BX_T1 : public T1Instruction {
 public:
    BX_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "BX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr0 = payload->buf16[0];
        uint32_t m = (instr0 >> 3) & 0xF;

        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t Rm = static_cast<uint32_t>(R[m]);
        BXWritePC(Rm);
        return 2;
    }
};

/** 4.6.22 CBNZ. Compare and Branch on Non-Zero */
class CBNZ_T1 : public T1Instruction {
 public:
    CBNZ_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CBNZ") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint32_t imm32 = (i << 6) | (imm5 << 1);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (R[n] != 0) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

/** 4.6.23 CBZ. Compare and Branch on Zero */
class CBZ_T1 : public T1Instruction {
 public:
    CBZ_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CBZ") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = ti & 0x7;
        uint32_t imm5 = (ti >> 3) & 0x1F;
        uint32_t i = (ti >> 9) & 0x1;
        uint32_t imm32 = (i << 6) | (imm5 << 1);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (R[n] == 0) {
            uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4 + imm32;
            BranchWritePC(npc);
        }
        return 2;
    }
};

/** 4.6.29 CMP (immediate) */
class CMP_I_T1 : public T1Instruction {
 public:
    CMP_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = (ti >> 8) & 0x7;
        uint32_t imm32 = ti & 0xFF;

        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 2;
    }
};

class CMP_I_T2 : public T1Instruction {
 public:
    CMP_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = ti & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 4;
    }
};

/** 4.6.30 CMP (register) */
class CMP_R_T1 : public T1Instruction {
 public:
    CMP_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CMP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t carry;
        uint32_t overflow;
        uint32_t result;
        uint32_t n = ti  & 0x7;
        uint32_t m = (ti >> 3) & 0x7;

        uint32_t shifted = static_cast<uint32_t>(R[m]); // no shifting
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        icpu_->setV(overflow);
        return 2;
    }
};

/** 4.6.31 CPS (Change Processor State) */
class CPS_T1 : public T1Instruction {
 public:
    CPS_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "CPS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t disable = (ti >> 4) & 1;
        uint32_t A = (ti >> 2) & 1;
        uint32_t I = (ti >> 1) & 1;
        uint32_t F = (ti >> 0) & 1;

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (!disable) {
            if (A) {
                icpu_->setA(0);
            }
            if (I) {
                icpu_->setI(0);
            }
            if (F) {
                icpu_->setF(0);
            }
        } else {
            if (A) {
                icpu_->setA(1);
            }
            if (I) {
                icpu_->setI(1);
            }
            if (F) {
                icpu_->setF(1);
            }
        }
        return 2;
    }
};

/** 4.6.39 IT (IT Block) */
class IT_T1 : public T1Instruction {
 public:
    IT_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "IT") {}

    virtual int exec(Reg64Type *payload) {
        uint32_t ti = payload->buf16[0];
        uint32_t firstcond = (ti >> 4) & 0xF;
        uint32_t mask = ti & 0xF;
        uint32_t BitCount = 0;
        for (int i = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                BitCount++;
            }
        }

        if (firstcond == 0xF) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (firstcond == 0xE && BitCount != 1) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        icpu_->StartITBlock(firstcond, mask);
        return 2;
    }
};

/** 4.6.43 LDR (immediate) */
class LDR_I_T1 : public T1Instruction {
 public:
    LDR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRI") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t t = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint64_t imm32 = ((instr >> 6) & 0x1F) << 2;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC();
            }
        } else {
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }

        return 2;
    }
};

class LDR_I_T2 : public T1Instruction {
 public:
    LDR_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRI") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = (t >> 8) & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[Reg_sp]);
        uint64_t imm32 = (ti & 0xFF) << 2;

        trans_.addr = Rn + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        icpu_->setReg(t, trans_.rpayload.b32[0]);
        return 2;
    }
};

class LDR_I_T3 : public T1Instruction {
 public:
    LDR_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = Rn + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC();
            }
        } else {
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

class LDR_I_T4 : public T1Instruction {
 public:
    LDR_I_T4(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xf;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFF;
        bool index = (ti1 >> 10) & 1;
        bool add = (ti1 >> 9) & 1;
        bool wback = (ti1 >> 8) & 1;
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (wback && n == t) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t offset_addr = add ? Rn + imm32 : Rn - imm32;
        trans_.addr = index ? offset_addr : Rn;
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }

        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        if (t == Reg_pc) {
            if (trans_.addr & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC();
            }
        } else {
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

/** 4.6.44 LDR (literal) */
class LDR_L_T1 : public T1Instruction {
 public:
    LDR_L_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = (ti >> 8) & 0x7;
        uint64_t imm8 = (ti & 0xFF) << 2;
        uint64_t base = icpu_->getPC() + 4;
        base &= ~0x3;  // Align(base, 4)
        uint64_t address = base + imm8;

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        if (t == Reg_pc) {
            if (address & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC();
            }
        } else {
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }

        return 2;
    }
};

/** 4.6.45 LDR (register) */
class LDR_R_T1 : public T1Instruction {
 public:
    LDR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t t = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint32_t m = (instr >> 6) & 0x7;
        uint32_t address = static_cast<uint32_t>(R[n])
                         + static_cast<uint32_t>(R[m]);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b32[0]);
        return 2;
    }
};

class LDR_R_T2 : public T1Instruction {
 public:
    LDR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int shift_n = (ti1 >> 4) & 0x3;
        uint32_t c_out;

        if (BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (t == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);

        if (t == Reg_pc) {
            if (address & 0x3) {
                RISCV_error("%s", "UNPREDICTABLE");
            } else {
                LoadWritePC();
            }
        } else {
            icpu_->setReg(t, trans_.rpayload.b32[0]);
        }
        return 4;
    }
};

/** 4.6.46 LDRB (immediate) */
class LDRB_I_T1 : public T1Instruction {
 public:
    LDRB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t imm32 = (ti >> 6) & 0x1F;

        trans_.addr = static_cast<uint32_t>(R[n]) + imm32;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 2;
    }
};

/** 4.6.48 LDRB (register) */
class LDRB_R_T1 : public T1Instruction {
 public:
    LDRB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        int shift_n = 0;
        uint32_t c_out;

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 2;
    }
};

class LDRB_R_T2 : public T1Instruction {
 public:
    LDRB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int shift_n = static_cast<int>((ti1 >> 4) & 0x3);
        uint32_t c_out;

        if (t == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n])
                         + LSL_C(static_cast<uint32_t>(R[m]), shift_n, &c_out);

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        icpu_->setReg(t, trans_.rpayload.b8[0]);
        return 4;
    }
};

/** 4.6.59 LDRSB (immediate) */
class LDRSB_I_T1 : public T1Instruction {
 public:
    LDRSB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LDRSB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t imm32 = ti1 & 0xFFF;

        if (t == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        uint32_t address = static_cast<uint32_t>(R[n]) + imm32;

        trans_.addr = address;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        icpu_->dma_memop(&trans_);
        int32_t result = static_cast<int8_t>(trans_.rpayload.b8[0]);
        icpu_->setReg(t, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.68 LSL (immediate) */
class LSL_I_T1 : public T1Instruction {
 public:
    LSL_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSLS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t carry;
        uint32_t shift_n;
        uint32_t result;

        DecodeImmShift(0, imm5, &shift_n);
        result = Shift_C(Rm, SRType_LSL, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** LSL (register) */
class LSL_R_T2 : public T1Instruction {
 public:
    LSL_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSL.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = Shift_C(Rn, SRType_LSL, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.70 LSR (immediate) */
class LSR_I_T1 : public T1Instruction {
 public:
    LSR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSRS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm5 = (ti >> 6) & 0x1F;
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t carry;
        uint32_t shift_n;
        uint32_t result;

        DecodeImmShift(1, imm5, &shift_n);
        result = Shift_C(Rm, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

/** 4.6.71 LSR (register) */
class LSR_R_T1 : public T1Instruction {
 public:
    LSR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSRS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t dn = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        uint32_t carry;
        uint32_t result;

        result = Shift_C(Rn, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(dn, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

class LSR_R_T2 : public T1Instruction {
 public:
    LSR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "LSR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t shift_n = static_cast<uint32_t>(R[m] & 0xFF);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t carry;
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = Shift_C(Rn, SRType_LSR, shift_n, icpu_->getC(), &carry);
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.76 MOV (immediate) */
class MOV_I_T1 : public T1Instruction {
 public:
    MOV_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOVS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t d = (instr >> 8) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = instr  & 0xFF;
        uint32_t carry = icpu_->getC();

        icpu_->setReg(d, imm32);

        if (setflags) {
            icpu_->setN((imm32 >> 31) & 1);
            icpu_->setZ(imm32 == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 2;
    }
};

class MOV_I_T2 : public T1Instruction {
 public:
    MOV_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

class MOV_I_T3 : public T1Instruction {
 public:
    MOV_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm4 = ti & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm32 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;

        if (BadReg(d)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = imm32;
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.77 MOV (register) */
class MOV_R_T1 : public T1Instruction {
 public:
    MOV_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t D = (ti >> 7) & 1;
        uint32_t d = (D << 3) | (ti & 0x7);
        uint32_t m = (ti >> 3) & 0xF;
        uint32_t result = static_cast<uint32_t>(R[m]);

        if (d == Reg_pc && icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        icpu_->setReg(d, result);
        if (d == Reg_pc) {
            ALUWritePC(result);   // ALUWritePC
        }
        return 2;
    }
};

class MOV_R_T2 : public T1Instruction {
 public:
    MOV_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MOV.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t result = static_cast<uint32_t>(R[m]);

        if (icpu_->InITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        icpu_->setReg(d, result);           // d < 8 always
        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(0);     // No data in specification
        return 2;
    }
};

/** 4.6.84 MUL */
class MUL_T1 : public T1Instruction {
 public:
    MUL_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MUL") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t dm = ti & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[dm]);
        uint32_t result;

        result = Rn * Rm;
        icpu_->setReg(dm, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
        }
        return 2;
    }
};

class MUL_T2 : public T1Instruction {
 public:
    MUL_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "MUL.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn * Rm;
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.88 NOP */
class NOP_T1 : public T1Instruction {
 public:
    NOP_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "NOP") {}

    virtual int exec(Reg64Type *payload) {
        return 2;
    }
};

/** 4.6.91 ORR (immediate) */
class ORR_I_T1 : public T1Instruction {
 public:
    ORR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "ORR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti  & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(d) || n == Reg_sp) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn | imm32;
        icpu_->setReg(d, result);

        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            // V unchanged
        }
        return 4;
    }
};

/** 4.6.98 POP */
class POP_T1 : public T1Instruction {
 public:
    POP_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "POP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t address = static_cast<uint32_t>(R[Reg_sp]);

        trans_.action = MemAction_Read;
        trans_.xsize = 4;
        trans_.wstrb = 0;

        for (int i = 0; i < 8; i++) {
            if (instr & (1ul << i)) {
                trans_.addr = address;
                icpu_->dma_memop(&trans_);
                icpu_->setReg(i, trans_.rpayload.b32[0]);
                address += 4;
            }
        }
        if (instr & 0x100) {
            trans_.addr = address;
            icpu_->dma_memop(&trans_);

            LoadWritePC();
            address += 4;
        }

        icpu_->setReg(Reg_sp, address);
        return 2;
    }
};

/** 4.6.99 PUSH */
class PUSH_T1 : public T1Instruction {
 public:
    PUSH_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "PUSH_T1") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint64_t address = R[Reg_sp];

        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        if (instr & 0x100) {
            address -= 4;
            trans_.addr = address;
            trans_.wpayload.b64[0] = R[Reg_lr];
            icpu_->dma_memop(&trans_);
        }

        for (int i = 7; i >= 0; i--) {
            if (instr & (1ul << i)) {
                address -= 4;
                trans_.addr = address;
                trans_.wpayload.b64[0] = R[i];
                icpu_->dma_memop(&trans_);
            }
        }

        icpu_->setReg(Reg_sp, address);
        return 2;
    }
};

/** 4.6.126 SDIV */
class SDIV_T1 : public T1Instruction {
 public:
    SDIV_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SDIV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        int32_t Rn = static_cast<int32_t>(R[n]);
        int32_t Rm = static_cast<int32_t>(R[m]);
        int32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (Rm == 0) {
            if (icpu_->getDZ()) {
                //icpu_->raiseSignal(ZeroDivide);
            }
            result = 0;
        } else {
            result = Rn / Rm;
        }
        icpu_->setReg(d, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.160 STMDB/STMFD */
class STMDB_T1 : public T1Instruction {
 public:
    STMDB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STMDB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t n = ti & 0xF;
        uint32_t wback = (ti >> 5) & 1;
        uint64_t address = R[n];
        uint32_t BitCount = 0;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        for (int i = 14; i >= 0; i--) {
            if (ti1 & (1ul << i)) {
                address -= 4;
                trans_.addr = address;
                trans_.wpayload.b64[0] = R[i];
                icpu_->dma_memop(&trans_);
                BitCount++;
            }
        }
        if (BitCount < 2) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (wback) {
            if (n == Reg_sp) {
                // T2_PUSH equivalent
                icpu_->setReg(n, static_cast<uint32_t>(R[n] - 4*BitCount));
            } else {
                icpu_->setReg(n, static_cast<uint32_t>(R[n] + 4*BitCount));
            }
        }
        return 4;
    }
};

/** 4.6.162 STR (immediate) */
class STR_I_T1 : public T1Instruction {
 public:
    STR_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRI") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = ((ti >> 6) & 0x1F) << 2;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

/** 4.6.163 STR (register) */
class STR_R_T1 : public T1Instruction {
 public:
    STR_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint32_t m = (ti >> 6) & 0x7;
        uint32_t address = static_cast<uint32_t>(R[n])
                         + static_cast<uint32_t>(R[m]);

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STR_R_T2 : public T1Instruction {
 public:
    STR_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STR.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t c_out;
        uint32_t address = Rn + LSL_C(Rm, shift_n, &c_out);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (t == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 4;
        trans_.wstrb = 0xF;
        trans_.wpayload.b32[0] = static_cast<uint32_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.164 STRB (immediate) */
class STRB_I_T1 : public T1Instruction {
 public:
    STRB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = (ti >> 6) & 0x1F;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

class STRB_I_T2 : public T1Instruction {
 public:
    STRB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint64_t imm32 = ti1 & 0xFFF;

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

class STRB_I_T3 : public T1Instruction {
 public:
    STRB_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint64_t imm32 = ti1 & 0xFF;
        uint32_t index = (ti1 >> 10) & 1;
        uint32_t add = (ti1 >> 9) & 1;
        uint32_t wback = (ti1 >> 8) & 1;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t offset_addr;
        uint32_t address;

        if (n == Reg_pc || (index == 0 && wback == 0)) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (BadReg(t) || (wback && n == t)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        if (add) {
            offset_addr = Rn + imm32;
        } else {
            offset_addr = Rn - imm32;
        }
        if (index) {
            address = offset_addr;
        } else {
            address = Rn;
        }
        if (wback) {
            icpu_->setReg(n, offset_addr);
        }
        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.165 */
class STRB_R_T2 : public T1Instruction {
 public:
    STRB_R_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t t = (ti1 >> 12) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t shift_n = (ti1 >> 4) & 0x3;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t c_out;
        uint32_t address = Rn + LSL_C(Rm, shift_n, &c_out);

        if (n == Reg_pc) {
            RISCV_error("%s", "UNDEFINED");
        }
        if (t == Reg_pc || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        trans_.addr = address;
        trans_.action = MemAction_Write;
        trans_.xsize = 1;
        trans_.wstrb = 0x1;
        trans_.wpayload.b32[0] = static_cast<uint8_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 4;
    }
};

/** 4.6.172 STRH (immediate) */
class STRH_I_T1 : public T1Instruction {
 public:
    STRH_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "STRH") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t t = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        uint64_t imm32 = ((ti >> 6) & 0x1F) << 1;

        trans_.addr = R[n] + imm32;
        trans_.action = MemAction_Write;
        trans_.xsize = 2;
        trans_.wstrb = 0x3;
        trans_.wpayload.b32[0] = static_cast<uint16_t>(R[t]);
        icpu_->dma_memop(&trans_);
        return 2;
    }
};

/** 4.6.176 SUB (immediate) */
class SUB_I_T1 : public T1Instruction {
 public:
    SUB_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t d = ti & 0x7;
        uint32_t n = (ti >> 3) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = (ti >> 6) & 0x7;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class SUB_I_T2 : public T1Instruction {
 public:
    SUB_I_T2(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBS") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t dn = (ti >> 8) & 0x7;
        bool setflags = !icpu_->InITBlock();
        uint32_t imm32 = ti & 0xFF;
        uint32_t Rn = static_cast<uint32_t>(R[dn]);
        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);

        icpu_->setReg(dn, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

class SUB_I_T3 : public T1Instruction {
 public:
    SUB_I_T3(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUB.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        bool setflags = (ti >> 4) & 1;
        uint32_t i = (ti >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);
        uint32_t Rn = static_cast<uint32_t>(R[n]);

        if (BadReg(d) || n == Reg_pc) {
            RISCV_error("%s", "UNPREDICTABLE");
        }

        result = AddWithCarry(Rn, ~imm32, 1, &overflow, &carry);
        icpu_->setReg(d, result);
        if (setflags) {
            // Mask 0x7 no need to check on SP or PC
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 4;
    }
};

/** 4.6.177 SUB (register) */
class SUB_R_T1 : public T1Instruction {
 public:
    SUB_R_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBR") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t instr = payload->buf16[0];
        uint32_t d = instr & 0x7;
        uint32_t n = (instr >> 3) & 0x7;
        uint32_t m = (instr >> 6) & 0x7;
        bool setflags = !icpu_->InITBlock();

        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        
        uint32_t shifted = Rm;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result = AddWithCarry(Rn, ~shifted, 1, &overflow, &carry);

        icpu_->setReg(d, result);
        if (setflags) {
            icpu_->setN((result >> 31) & 1);
            icpu_->setZ(result == 0 ? 1: 0);
            icpu_->setC(carry);
            icpu_->setV(overflow);
        }
        return 2;
    }
};

/** 4.6.178 SUB (SP minus immediate) */
class SUB_SP_T1 : public T1Instruction {
 public:
    SUB_SP_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "SUBSP") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t sp = static_cast<uint32_t>(R[Reg_sp]);
        uint32_t instr = payload->buf16[0];
        uint32_t imm32 = (instr & 0x7F) << 2;
        uint32_t overflow;
        uint32_t carry;
        uint32_t result;

        result = AddWithCarry(sp, ~imm32, 1, &overflow, &carry);
        icpu_->setReg(Reg_sp, result);
        // Not modify flags
        return 2;
    }
};

/** 4.6.188 TBB: Table Branch Cycle */
class TBB_T1 : public T1Instruction {
 public:
    TBB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "TBB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf32[0];
        uint32_t n = ti & 0xF;
        uint32_t m = (ti >> 16) & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        if (n == Reg_sp || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (icpu_->InITBlock() && !icpu_->LastInITBlock()) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (n == Reg_pc) {
            Rn += 4;
        }

        trans_.addr = Rn + Rm;
        trans_.action = MemAction_Read;
        trans_.xsize = 1;
        trans_.wstrb = 0;
        
        icpu_->dma_memop(&trans_);
        uint32_t halfwords = trans_.rpayload.b8[0];
        uint32_t npc = static_cast<uint32_t>(icpu_->getPC()) + 4
                     + (halfwords << 1);
        BranchWritePC(npc);
        return 4;
    }
};

/** 4.6.192 TST (immediate) */
class TST_I_T1 : public T1Instruction {
 public:
    TST_I_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "TST.W") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t carry;
        uint32_t result;
        uint32_t n = ti0  & 0xF;
        uint32_t i = (ti0 >> 10) & 1;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm8 = ti1 & 0xFF;
        uint32_t imm12 = (i << 11) | (imm3 << 8) | imm8;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t imm32 = ThumbExpandImmWithC(imm12, &carry);

        if (BadReg(n)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = Rn & imm32;

        icpu_->setN((result >> 31) & 1);
        icpu_->setZ(result == 0 ? 1: 0);
        icpu_->setC(carry);
        // V unchanged
        return 4;
    }
};

/** 4.6.197 UBFX */
class UBFX_T1 : public T1Instruction {
 public:
    UBFX_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UBFX") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti0 = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t result;
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti0 & 0xF;
        uint32_t imm3 = (ti1 >> 12) & 0x7;
        uint32_t imm2 = (ti1 >> 6) & 0x3;
        uint32_t lsbit = (imm3 << 2) | imm2;
        uint32_t widthm1 = ti1 & 0x1F;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t msbit = lsbit + widthm1;
        uint64_t mask = (1ull << (widthm1 + 1)) - 1;

        if (BadReg(d) || BadReg(n) || msbit > 31) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = (Rn >> lsbit) & static_cast<uint32_t>(mask);
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.198 UDIV */
class UDIV_T1 : public T1Instruction {
 public:
    UDIV_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UDIV") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t d = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint32_t result;

        if (BadReg(d) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (Rm == 0) {
            if (icpu_->getDZ()) {
                //icpu_->raiseSignal(ZeroDivide);
            }
            result = 0;
        } else {
            result = Rn / Rm;
        }
        icpu_->setReg(d, result);
        return 4;
    }
};

/** 4.6.207 UMULL */
class UMULL_T1 : public T1Instruction {
 public:
    UMULL_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UMULL") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 4;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t ti1 = payload->buf16[1];
        uint32_t dLo = (ti1 >> 12) & 0xF;
        uint32_t dHi = (ti1 >> 8) & 0xF;
        uint32_t n = ti & 0xF;
        uint32_t m = ti1 & 0xF;
        uint32_t Rn = static_cast<uint32_t>(R[n]);
        uint32_t Rm = static_cast<uint32_t>(R[m]);
        uint64_t result;

        if (BadReg(dLo) || BadReg(dHi) || BadReg(n) || BadReg(m)) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        if (dHi == dLo) {
            RISCV_error("%s", "UNPREDICTABLE");
        }
        result = static_cast<uint64_t>(Rn) * static_cast<uint64_t>(Rm);
        icpu_->setReg(dHi, static_cast<uint32_t>(result >> 32));
        icpu_->setReg(dLo, static_cast<uint32_t>(result));
        return 4;
    }
};

/** 4.6.224 UXTB */
class UXTB_T1 : public T1Instruction {
 public:
    UXTB_T1(CpuCortex_Functional *icpu) : T1Instruction(icpu, "UXTB") {}

    virtual int exec(Reg64Type *payload) {
        if (!ConditionPassed()) {
            return 2;
        }
        uint32_t ti = payload->buf16[0];
        uint32_t d = ti & 0x7;
        uint32_t m = (ti >> 3) & 0x7;
        uint32_t Rm = static_cast<uint32_t>(R[m]);

        // rotation = 0 no need to call ROR_C
        icpu_->setReg(d, Rm & 0xFF);
        return 2;
    }
};

void CpuCortex_Functional::addArm7tmdiIsa() {
    isaTableArmV7_[ARMV7_BX] = new BX(this);
    isaTableArmV7_[ARMV7_B] = new B(this);
    isaTableArmV7_[ARMV7_BL] = new BL(this);
    isaTableArmV7_[ARMV7_BLX] = new BLX(this);
    isaTableArmV7_[ARMV7_AND] = new AND(this);
    isaTableArmV7_[ARMV7_EOR] = new EOR(this);
    isaTableArmV7_[ARMV7_SUB] = new SUB(this);
    isaTableArmV7_[ARMV7_RSB] = new RSB(this);
    isaTableArmV7_[ARMV7_ADD] = new ADD(this);
    isaTableArmV7_[ARMV7_ADC] = new ADC(this);
    isaTableArmV7_[ARMV7_SBC] = new SBC(this);
    isaTableArmV7_[ARMV7_RSC] = new RSC(this);
    isaTableArmV7_[ARMV7_TST] = new TST(this);
    isaTableArmV7_[ARMV7_TEQ] = new TEQ(this);
    isaTableArmV7_[ARMV7_CMP] = new CMP(this);
    isaTableArmV7_[ARMV7_CMN] = new CMN(this);
    isaTableArmV7_[ARMV7_ORR] = new ORR(this);
    isaTableArmV7_[ARMV7_MOV] = new MOV(this);
    isaTableArmV7_[ARMV7_BIC] = new BIC(this);
    isaTableArmV7_[ARMV7_MVN] = new MVN(this);
    isaTableArmV7_[ARMV7_MRS] = new MRS(this);
    isaTableArmV7_[ARMV7_MSR] = new MSR(this);
    isaTableArmV7_[ARMV7_MUL] = new MUL(this);
    isaTableArmV7_[ARMV7_MLA] = new MLA(this);
    isaTableArmV7_[ARMV7_UMULL] = new UMULL(this);
    isaTableArmV7_[ARMV7_UMLAL] = new UMLAL(this);
    isaTableArmV7_[ARMV7_SMULL] = new SMULL(this);
    isaTableArmV7_[ARMV7_SMLAL] = new SMLAL(this);
    isaTableArmV7_[ARMV7_LDR] = new LDR(this);
    isaTableArmV7_[ARMV7_LDRB] = new LDRB(this);
    isaTableArmV7_[ARMV7_STR] = new STR(this);
    isaTableArmV7_[ARMV7_STRB] = new STRB(this);
    isaTableArmV7_[ARMV7_SWP] = new SWP(this);
    isaTableArmV7_[ARMV7_LDRH] = new LDRH(this);
    isaTableArmV7_[ARMV7_LDRSB] = new LDRSB(this);
    isaTableArmV7_[ARMV7_LDRSH] = new LDRSH(this);
    isaTableArmV7_[ARMV7_STRH] = new STRH(this);
    isaTableArmV7_[ARMV7_LDM] = new LDM(this);
    isaTableArmV7_[ARMV7_STM] = new STM(this);
    isaTableArmV7_[ARMV7_SWI] = new SWI(this);
    isaTableArmV7_[ARMV7_MRC] = new MRC(this);
    isaTableArmV7_[ARMV7_MCR] = new MCR(this);
    isaTableArmV7_[ARMV7_LDRD] = new LDRD(this);
    isaTableArmV7_[ARMV7_STRD] = new STRD(this);
    isaTableArmV7_[ARMV7_UXTB] = new UXTB(this);
    isaTableArmV7_[ARMV7_UXTAB] = new UXTAB(this);
    isaTableArmV7_[ARMV7_UXTB16] = new UXTB16(this);
    isaTableArmV7_[ARMV7_UXTAB16] = new UXTAB16(this);
    isaTableArmV7_[ARMV7_UXTH] = new UXTH(this);
    isaTableArmV7_[ARMV7_UXTAH] = new UXTAH(this);
    isaTableArmV7_[ARMV7_NOP] = new NOP(this);
    isaTableArmV7_[ARMV7_MOVT] = new MOVT(this);
    isaTableArmV7_[ARMV7_MOVW] = new MOVW(this);
    isaTableArmV7_[ARMV7_UDIV] = new UDIV(this);
    isaTableArmV7_[ARMV7_SDIV] = new SDIV(this);
    isaTableArmV7_[ARMV7_BFC] = new BFC(this);
    isaTableArmV7_[ARMV7_BFI] = new BFI(this);
    isaTableArmV7_[T1_ADD_I] = new ADD_I_T1(this);
    isaTableArmV7_[T2_ADD_I] = new ADD_I_T2(this);
    isaTableArmV7_[T3_ADD_I] = new ADD_I_T3(this);
    isaTableArmV7_[T1_ADD_R] = new ADD_R_T1(this);
    isaTableArmV7_[T2_ADD_R] = new ADD_R_T2(this);
    isaTableArmV7_[T3_ADD_R] = new ADD_R_T3(this);
    isaTableArmV7_[T1_ADDSP_I] = new ADDSP_I_T1(this);
    isaTableArmV7_[T1_ADR] = new ADR_T1(this);
    isaTableArmV7_[T1_AND_I] = new AND_I_T1(this);
    isaTableArmV7_[T1_AND_R] = new AND_R_T1(this);
    isaTableArmV7_[T1_ASR_I] = new ASR_I_T1(this);
    isaTableArmV7_[T1_B] = new B_T1(this);
    isaTableArmV7_[T2_B] = new B_T2(this);
    isaTableArmV7_[T3_B] = new B_T3(this);
    isaTableArmV7_[T1_BIC_I] = new BIC_I_T1(this);
    isaTableArmV7_[T1_BL_I] = new BL_I_T1(this);
    isaTableArmV7_[T1_BLX_R] = new BLX_R_T1(this);
    isaTableArmV7_[T1_BX] = new BX_T1(this);
    isaTableArmV7_[T1_CBNZ] = new CBNZ_T1(this);
    isaTableArmV7_[T1_CBZ] = new CBZ_T1(this);
    isaTableArmV7_[T1_CMP_I] = new CMP_I_T1(this);
    isaTableArmV7_[T2_CMP_I] = new CMP_I_T2(this);
    isaTableArmV7_[T1_CMP_R] = new CMP_R_T1(this);
    isaTableArmV7_[T1_CPS] = new CPS_T1(this);
    isaTableArmV7_[T1_IT] = new IT_T1(this);
    isaTableArmV7_[T1_LDR_I] = new LDR_I_T1(this);
    isaTableArmV7_[T2_LDR_I] = new LDR_I_T2(this);
    isaTableArmV7_[T3_LDR_I] = new LDR_I_T3(this);
    isaTableArmV7_[T4_LDR_I] = new LDR_I_T4(this);
    isaTableArmV7_[T1_LDR_L] = new LDR_L_T1(this);
    isaTableArmV7_[T1_LDR_R] = new LDR_R_T1(this);
    isaTableArmV7_[T2_LDR_R] = new LDR_R_T2(this);
    isaTableArmV7_[T1_LDRB_I] = new LDRB_I_T1(this);
    isaTableArmV7_[T1_LDRB_R] = new LDRB_R_T1(this);
    isaTableArmV7_[T2_LDRB_R] = new LDRB_R_T2(this);
    isaTableArmV7_[T1_LDRSB_I] = new LDRSB_I_T1(this);
    isaTableArmV7_[T1_LSL_I] = new LSL_I_T1(this);
    isaTableArmV7_[T2_LSL_R] = new LSL_R_T2(this);
    isaTableArmV7_[T1_LSR_I] = new LSR_I_T1(this);
    isaTableArmV7_[T1_LSR_R] = new LSR_R_T1(this);
    isaTableArmV7_[T2_LSR_R] = new LSR_R_T2(this);
    isaTableArmV7_[T1_MOV_I] = new MOV_I_T1(this);
    isaTableArmV7_[T2_MOV_I] = new MOV_I_T2(this);
    isaTableArmV7_[T3_MOV_I] = new MOV_I_T3(this);
    isaTableArmV7_[T1_MOV_R] = new MOV_R_T1(this);
    isaTableArmV7_[T2_MOV_R] = new MOV_R_T2(this);
    isaTableArmV7_[T1_MUL] = new MUL_T1(this);
    isaTableArmV7_[T2_MUL] = new MUL_T2(this);
    isaTableArmV7_[T1_NOP] = new NOP_T1(this);
    isaTableArmV7_[T1_POP] = new POP_T1(this);
    isaTableArmV7_[T1_PUSH] = new PUSH_T1(this);
    isaTableArmV7_[T1_ORR_I] = new ORR_I_T1(this);
    isaTableArmV7_[T1_SDIV] = new SDIV_T1(this);
    isaTableArmV7_[T1_STMDB] = new STMDB_T1(this);
    isaTableArmV7_[T1_STR_I] = new STR_I_T1(this);
    isaTableArmV7_[T1_STR_R] = new STR_R_T1(this);
    isaTableArmV7_[T2_STR_R] = new STR_R_T2(this);
    isaTableArmV7_[T1_STRB_I] = new STRB_I_T1(this);
    isaTableArmV7_[T2_STRB_I] = new STRB_I_T2(this);
    isaTableArmV7_[T3_STRB_I] = new STRB_I_T3(this);
    isaTableArmV7_[T2_STRB_R] = new STRB_R_T2(this);
    isaTableArmV7_[T1_STRH_I] = new STRH_I_T1(this);
    isaTableArmV7_[T1_SUB_I] = new SUB_I_T1(this);
    isaTableArmV7_[T2_SUB_I] = new SUB_I_T2(this);
    isaTableArmV7_[T3_SUB_I] = new SUB_I_T3(this);
    isaTableArmV7_[T1_SUB_R] = new SUB_R_T1(this);
    isaTableArmV7_[T1_SUB_SP] = new SUB_SP_T1(this);
    isaTableArmV7_[T1_TBB] = new TBB_T1(this);
    isaTableArmV7_[T1_TST_I] = new TST_I_T1(this);
    isaTableArmV7_[T1_UBFX] = new UBFX_T1(this);
    isaTableArmV7_[T1_UDIV] = new UDIV_T1(this);
    isaTableArmV7_[T1_UMULL] = new UMULL_T1(this);
    isaTableArmV7_[T1_UXTB] = new UXTB_T1(this);
}

}  // namespace debugger
