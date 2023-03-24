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
 * @brief      RISC-V extension-A (Atomic Instructions).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {


/**
 * @brief Atmoic Memory Operations (AMO).
 *
 * The atomic memory operation (AMO) instructions perform read-modify-write
 * operations for multiprocessor synchronization and are encoded with an R-type
 * instruction format. These AMO instructions atomically load a data value from
 * the address in rs1, place the value into register rd, apply a binary operator
 * to the loaded value and the original value in rs2, then store the result back
 * to the address in rs1.
 *
 * Release consistency semantic (flags aquire/release) is OPTIONAL
 */ 
class RiscvAmoGeneric : public RiscvInstruction {
public:
    RiscvAmoGeneric(CpuRiver_Functional *icpu, const char *name,
                    const char *bits, int rvbytes) :
        RiscvInstruction(icpu, name, bits), rvbytes_(rvbytes) {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_R_type u;
        u.value = payload->buf32[0];
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1];
        trans.xsize = rvbytes_;
        if (trans.addr & (rvbytes_ - 1)) {
            trans.rpayload.b64[0] = 0;
            // AMO always should generate Store exceptions (spike)
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                // AMO always should generate Store exceptions (spike)
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            } else {
                uint64_t t;
                trans.action = MemAction_Write;
                trans.wstrb = (1 << trans.xsize) - 1;
                if (rvbytes_ == 4) {
                    t = trans.rpayload.b32[0];
                    if (t & 0x80000000ull) {
                        t |= EXT_SIGN_32;
                    }
                } else {
                    t = trans.rpayload.b64[0];
                }
                trans.wpayload.b64[0] = amo_op(R[u.bits.rs2], t);
                if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                    icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
                }
                icpu_->setReg(u.bits.rd, t);
            }
        }
        return 4;
    }
 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) = 0;

 protected:
    int rvbytes_;
};

// 32 bits
class AMOADD_W : public RiscvAmoGeneric {
 public:
    AMOADD_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOADD_W", "00000????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a + b;
    }
};

class AMOXOR_W : public RiscvAmoGeneric {
public:
    AMOXOR_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOXOR_W", "00100????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a ^ b;
    }
};

class AMOOR_W : public RiscvAmoGeneric {
public:
    AMOOR_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOOR_W", "01000????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a | b;
    }
};

class AMOAND_W : public RiscvAmoGeneric {
public:
    AMOAND_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOAND_W", "01100????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a & b;
    }
};

class AMOMIN_W : public RiscvAmoGeneric {
public:
    AMOMIN_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMIN_W", "10000????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        int64_t sa = static_cast<int64_t>(a);
        int64_t sb = static_cast<int64_t>(b);
        return sa < sb ? a: b;
    }
};

class AMOMAX_W : public RiscvAmoGeneric {
public:
    AMOMAX_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMAX_W", "10100????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        int64_t sa = static_cast<int64_t>(a);
        int64_t sb = static_cast<int64_t>(b);
        return sa > sb ? a: b;
    }
};

class AMOMINU_W : public RiscvAmoGeneric {
public:
    AMOMINU_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMINU_W", "11000????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a < b ? a: b;
    }
};

class AMOMAXU_W : public RiscvAmoGeneric {
public:
    AMOMAXU_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMAXU_W", "11100????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a > b ? a: b;
    }
};

class AMOSWAP_W : public RiscvAmoGeneric {
public:
    AMOSWAP_W(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOSWAP_W", "00001????????????010?????0101111", 4) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a;
    }
};


// 64-bits
class AMOADD_D : public RiscvAmoGeneric {
 public:
    AMOADD_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOADD_D", "00000????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a + b;
    }
};

class AMOXOR_D : public RiscvAmoGeneric {
public:
    AMOXOR_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOXOR_D", "00100????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a ^ b;
    }
};

class AMOOR_D : public RiscvAmoGeneric {
public:
    AMOOR_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOOR_D", "01000????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a | b;
    }
};

class AMOAND_D : public RiscvAmoGeneric {
public:
    AMOAND_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOAND_D", "01100????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a & b;
    }
};

class AMOMIN_D : public RiscvAmoGeneric {
public:
    AMOMIN_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMIN_D", "10000????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        int64_t sa = static_cast<int64_t>(a);
        int64_t sb = static_cast<int64_t>(b);
        return sa < sb ? a: b;
    }
};

class AMOMAX_D : public RiscvAmoGeneric {
public:
    AMOMAX_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMAX_D", "10100????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        int64_t sa = static_cast<int64_t>(a);
        int64_t sb = static_cast<int64_t>(b);
        return sa > sb ? a: b;
    }
};

class AMOMINU_D : public RiscvAmoGeneric {
public:
    AMOMINU_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMINU_D", "11000????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a < b ? a: b;
    }
};

class AMOMAXU_D : public RiscvAmoGeneric {
public:
    AMOMAXU_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOMAXU_D", "11100????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a > b ? a: b;
    }
};

class AMOSWAP_D : public RiscvAmoGeneric {
public:
    AMOSWAP_D(CpuRiver_Functional *icpu) :
        RiscvAmoGeneric(icpu, "AMOSWAP_D", "00001????????????011?????0101111", 8) {}

 protected:
    virtual uint64_t amo_op(uint64_t a, uint64_t b) {
        return a;
    }
};

/** @brief Load-Reserved/Store-Conditional Instruction
 *
 * Complex atomic memory operations on a single memory word are performed
 * with the load-reserved (LR) and store-conditional (SC) instructions. LR
 * loads a word from the address in rs1, places the sign-extended value in
 * rd, and registers a reservation on the memory address. SC writes a word in
 * rs2 to the address in rs1, provided a valid reservation still exists on
 * that address. SC writes zero to rd on success or a nonzero code on failure
 */
class LR_W : public RiscvInstruction {
public:
    LR_W(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LR_W", "00010??00000?????010?????0101111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_R_type u;
        u.value = payload->buf32[0];
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1];
        trans.xsize = 4;
        if (trans.addr & (trans.xsize - 1)) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            } else {
                uint64_t t;
                t = trans.rpayload.b32[0];
                if (t & 0x80000000ull) {
                    t |= EXT_SIGN_32;
                }
                icpu_->mmuAddrReserve(trans.addr);
                icpu_->setReg(u.bits.rd, t);
            }
        }
        return 4;
    }
};

class LR_D : public RiscvInstruction {
public:
    LR_D(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "LR_D", "00010??00000?????011?????0101111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_R_type u;
        u.value = payload->buf32[0];
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1];
        trans.xsize = 8;
        if (trans.addr & (trans.xsize - 1)) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            } else {
                icpu_->mmuAddrReserve(trans.addr);
                icpu_->setReg(u.bits.rd, trans.rpayload.b32[0]);
            }
        }
        return 4;
    }
};

class SC_W : public RiscvInstruction {
public:
    SC_W(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SC_W", "00011????????????010?????0101111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_R_type u;
        u.value = payload->buf32[0];
        bool error = 1;
        if (icpu_->mmuAddrRelease(R[u.bits.rs1])) {
            trans.action = MemAction_Write;
            trans.addr = R[u.bits.rs1];
            trans.xsize = 4;
            trans.wstrb = (1 << trans.xsize) - 1;
            trans.wpayload.b64[0] = R[u.bits.rs2];
            if (trans.addr & (trans.xsize - 1)) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
            } else {
                if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                    icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
                } else {
                    error = 0;
                }
            }
        }
        icpu_->setReg(u.bits.rd, error);    // success
        return 4;
    }
};

class SC_D : public RiscvInstruction {
public:
    SC_D(CpuRiver_Functional *icpu) :
        RiscvInstruction(icpu, "SC_D", "00011????????????011?????0101111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_R_type u;
        u.value = payload->buf32[0];
        bool error = 1;
        if (icpu_->mmuAddrRelease(R[u.bits.rs1])) {
            trans.action = MemAction_Write;
            trans.addr = R[u.bits.rs1];
            trans.xsize = 8;
            trans.wstrb = (1 << trans.xsize) - 1;
            trans.wpayload.b64[0] = R[u.bits.rs2];
            if (trans.addr & (trans.xsize - 1)) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
            } else {
                if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                    icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
                } else {
                    error = 0;
                }
            }
        }
        icpu_->setReg(u.bits.rd, error);    // success
        return 4;
    }
};


void CpuRiver_Functional::addIsaExtensionA() {
    // TODO
    /*
    addInstr("AMOADD_W",           "00000????????????010?????0101111", NULL, out);
    addInstr("AMOXOR_W",           "00100????????????010?????0101111", NULL, out);
    addInstr("AMOOR_W",            "01000????????????010?????0101111", NULL, out);
    addInstr("AMOAND_W",           "01100????????????010?????0101111", NULL, out);
    addInstr("AMOMIN_W",           "10000????????????010?????0101111", NULL, out);
    addInstr("AMOMAX_W",           "10100????????????010?????0101111", NULL, out);
    addInstr("AMOMINU_W",          "11000????????????010?????0101111", NULL, out);
    addInstr("AMOMAXU_W",          "11100????????????010?????0101111", NULL, out);
    addInstr("AMOSWAP_W",          "00001????????????010?????0101111", NULL, out);
    addInstr("LR_W",               "00010??00000?????010?????0101111", NULL, out);
    addInstr("SC_W",               "00011????????????010?????0101111", NULL, out);
    addInstr("AMOADD_D",           "00000????????????011?????0101111", NULL, out);
    addInstr("AMOXOR_D",           "00100????????????011?????0101111", NULL, out);
    addInstr("AMOOR_D",            "01000????????????011?????0101111", NULL, out);
    addInstr("AMOAND_D",           "01100????????????011?????0101111", NULL, out);
    addInstr("AMOMIN_D",           "10000????????????011?????0101111", NULL, out);
    addInstr("AMOMAX_D",           "10100????????????011?????0101111", NULL, out);
    addInstr("AMOMINU_D",          "11000????????????011?????0101111", NULL, out);
    addInstr("AMOMAXU_D",          "11100????????????011?????0101111", NULL, out);
    addInstr("AMOSWAP_D",          "00001????????????011?????0101111", NULL, out);
    addInstr("LR_D",               "00010??00000?????011?????0101111", NULL, out);
    addInstr("SC_D",               "00011????????????011?????0101111", NULL, out);
    */
    addSupportedInstruction(new AMOADD_W(this));
    addSupportedInstruction(new AMOXOR_W(this));
    addSupportedInstruction(new AMOOR_W(this));
    addSupportedInstruction(new AMOAND_W(this));
    addSupportedInstruction(new AMOMIN_W(this));
    addSupportedInstruction(new AMOMAX_W(this));
    addSupportedInstruction(new AMOMINU_W(this));
    addSupportedInstruction(new AMOMAXU_W(this));
    addSupportedInstruction(new AMOSWAP_W(this));
    addSupportedInstruction(new LR_W(this));
    addSupportedInstruction(new SC_W(this));

    addSupportedInstruction(new AMOADD_D(this));
    addSupportedInstruction(new AMOXOR_D(this));
    addSupportedInstruction(new AMOOR_D(this));
    addSupportedInstruction(new AMOAND_D(this));
    addSupportedInstruction(new AMOMIN_D(this));
    addSupportedInstruction(new AMOMAX_D(this));
    addSupportedInstruction(new AMOMINU_D(this));
    addSupportedInstruction(new AMOMAXU_D(this));
    addSupportedInstruction(new AMOSWAP_D(this));
    addSupportedInstruction(new LR_D(this));
    addSupportedInstruction(new SC_D(this));

    portCSR_.write(CSR_misa, portCSR_.read(CSR_misa).val | (1LL << ('A' - 'A')));
}

}  // namespace debugger
