/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base ISA implementation (extension I, user level).
 */

#include "api_utils.h"
#include "riscv-isa.h"
#include "instructions.h"

namespace debugger {

void generateException(uint64_t code, CpuContextType *data);

/** 
 * @brief Addition. Overflows are ignored
 */
class ADD : public IsaProcessor {
public:
    ADD() : IsaProcessor("ADD", "0000000??????????000?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
                data->regs[u.bits.rs1] + data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
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
class ADDI : public IsaProcessor {
public:
    ADDI() : IsaProcessor("ADDI", "?????????????????000?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] + imm;
        data->npc = data->pc + 4;

        if (u.bits.rs1 == 0) {
            RISCV_sprintf(data->disasm, sizeof(data->disasm), 
                "li %s,%d", IREGS_NAMES[u.bits.rd], static_cast<uint32_t>(imm));
        } else {
            RISCV_sprintf(data->disasm, sizeof(data->disasm), 
                "addi %s,%s,%d", IREGS_NAMES[u.bits.rd], 
                                 IREGS_NAMES[u.bits.rs1], 
                                 static_cast<uint32_t>(imm));
        }
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
class ADDIW : public IsaProcessor {
public:
    ADDIW() : IsaProcessor("ADDIW", "?????????????????000?????0011011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        data->regs[u.bits.rd] = (data->regs[u.bits.rs1] + imm) & 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;

        if (u.bits.imm == 0) {
            //RISCV_sprintf(data->disasm, sizeof(data->disasm), "sext.w %s,%d",);
        } else {
            //RISCV_sprintf(data->disasm, sizeof(data->disasm), "addiw %s,%s,%d",);
        }
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
class ADDW : public IsaProcessor {
public:
    ADDW() : IsaProcessor("ADDW", "0000000??????????000?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
    
        data->regs[u.bits.rd] = 
            (data->regs[u.bits.rs1] + data->regs[u.bits.rs2]) & 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief AND bitwise logical operation.
 */
class AND : public IsaProcessor {
public:
    AND() : IsaProcessor("AND", "0000000??????????111?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
                data->regs[u.bits.rs1] & data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief ANDI logical operation with immediate.
 *
 * ANDI, ORI, XORI are logical operations that perform bitwise AND, OR, 
 * and XOR on register rs1 and the sign-extended 12-bit immediate and place 
 * the result in rd.
 */
class ANDI : public IsaProcessor {
public:
    ANDI() : IsaProcessor("ANDI", "?????????????????111?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }

        data->regs[u.bits.rd] = data->regs[u.bits.rs1] & imm;
        data->npc = data->pc + 4;
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
class AUIPC : public IsaProcessor {
public:
    AUIPC() : IsaProcessor("AUIPC", "?????????????????????????0010111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_U_type u;
        u.value = payload[0];

        uint64_t off = u.bits.imm31_12 << 12;
        if (off & (1LL << 31)) {
            off |= EXT_SIGN_32;
        }
        data->regs[u.bits.rd] = data->pc + off;
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The BEQ (Branch if registers are equal)
 */
class BEQ : public IsaProcessor {
public:
    BEQ() : IsaProcessor("BEQ", "?????????????????000?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (data->regs[u.bits.rs1] == data->regs[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
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
class BGE : public IsaProcessor {
public:
    BGE() : IsaProcessor("BGE", "?????????????????101?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (static_cast<int64_t>(data->regs[u.bits.rs1]) >= 
            static_cast<int64_t>(data->regs[u.bits.rs2])) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
    }
};


/**
 * @brief The BGEU (Branch if greater than or equal using unsigned comparision)
 */
class BGEU : public IsaProcessor {
public:
    BGEU() : IsaProcessor("BGEU", "?????????????????111?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (data->regs[u.bits.rs1] >= data->regs[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
    }
};

/**
 * @brief The BLT (Branch if less than using signed comparision)
 */
class BLT : public IsaProcessor {
public:
    BLT() : IsaProcessor("BLT", "?????????????????100?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (static_cast<int64_t>(data->regs[u.bits.rs1]) < 
            static_cast<int64_t>(data->regs[u.bits.rs2])) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
    }
};

/**
 * @brief The BLTU (Branch if less than using unsigned comparision)
 */
class BLTU : public IsaProcessor {
public:
    BLTU() : IsaProcessor("BLTU", "?????????????????110?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (data->regs[u.bits.rs1] < data->regs[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
    }
};

/**
 * @brief The BNE (Branch if registers are unequal)
 */
class BNE : public IsaProcessor {
public:
    BNE() : IsaProcessor("BNE", "?????????????????001?????1100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_SB_type u;
        u.value = payload[0];
    
        if (data->regs[u.bits.rs1] != data->regs[u.bits.rs2]) {
            uint64_t imm = (u.bits.imm12 << 12) | (u.bits.imm11 << 11)
                    | (u.bits.imm10_5 << 5) | (u.bits.imm4_1 << 1);
            if (u.bits.imm12) {
                imm |= EXT_SIGN_12;
            }
            data->npc = data->pc + imm;
        } else {
            data->npc = data->pc + 4;
        }
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
class JAL : public IsaProcessor {
public:
    JAL() : IsaProcessor("JAL", "?????????????????????????1101111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_UJ_type u;
        u.value = payload[0];
        uint64_t off = 0;
        if (u.bits.imm20) {
            off = 0xfffffffffff00000LL;
        }
        off |= (u.bits.imm19_12 << 12);
        off |= (u.bits.imm11 << 11);
        off |= (u.bits.imm10_1 << 1);
        if (u.bits.rd != 0) {
            data->regs[u.bits.rd] = data->pc + 4;
        }
        data->npc = data->pc + off;
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
class JALR : public IsaProcessor {
public:
    JALR() : IsaProcessor("JALR", "?????????????????000?????1100111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (u.bits.imm & 0x800) {
            off |= 0xfffffffffffff000LL;
        }
        off += data->regs[u.bits.rs1];
        off &= ~0x1LL;
        if (u.bits.rd != 0) {
            data->regs[u.bits.rd] = data->pc + 4;
        }
        data->npc = off;
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
class LD : public IsaProcessor {
public:
    LD() : IsaProcessor("LD", "?????????????????011?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 8);
        data->regs[u.bits.rd] = dcache;
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] => %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 32-bits with sign extending.
 */
class LW : public IsaProcessor {
public:
    LW() : IsaProcessor("LW", "?????????????????010?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 4);
        data->regs[u.bits.rd] = dcache;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                    "[%08x] => %016" RV_PRI64 "x\n",
                    static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 32-bits with zero extending.
 */
class LWU : public IsaProcessor {
public:
    LWU() : IsaProcessor("LWU", "?????????????????110?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 4);
        data->regs[u.bits.rd] = dcache;
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                    "[%08x] => %016" RV_PRI64 "x\n",
                    static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 16-bits with sign extending.
 */
class LH : public IsaProcessor {
public:
    LH() : IsaProcessor("LH", "?????????????????001?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 2);
        data->regs[u.bits.rd] = dcache & 0xFFFF;
        if (data->regs[u.bits.rd] & (1LL << 15)) {
            data->regs[u.bits.rd] |= EXT_SIGN_16;
        }
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] => %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 16-bits with zero extending.
 */
class LHU : public IsaProcessor {
public:
    LHU() : IsaProcessor("LHU", "?????????????????101?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 2);
        data->regs[u.bits.rd] = dcache & 0xFFFF;
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] => %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 8-bits with sign extending.
 */
class LB : public IsaProcessor {
public:
    LB() : IsaProcessor("LB", "?????????????????000?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 1);
        data->regs[u.bits.rd] = dcache & 0xFF;
        if (data->regs[u.bits.rd] & (1LL << 7)) {
            data->regs[u.bits.rd] |= EXT_SIGN_8;
        }
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] => %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * Load 8-bits with zero extending.
 */
class LBU : public IsaProcessor {
public:
    LBU() : IsaProcessor("LBU", "?????????????????100?????0000011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t dcache = 0;
        ISA_I_type u;
        u.value = payload[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        data->ibus->read(addr, reinterpret_cast<uint8_t *>(&dcache), 1);
        data->regs[u.bits.rd] = dcache & 0xFF;
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                    "[%08x] => %016" RV_PRI64 "x\n",
                    static_cast<int>(addr), dcache);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * @brief LUI (load upper immediate).
 *
 * It is used to build 32-bit constants and uses the U-type format. LUI places
 * the U-immediate value in the top 20 bits of the destination register rd,
 * filling in the lowest 12 bits with zeros.
 */
class LUI : public IsaProcessor {
public:
    LUI() : IsaProcessor("LUI", "?????????????????????????0110111") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_U_type u;
        u.value = payload[0];
        uint64_t tmp = u.bits.imm31_12 << 12;
        if (tmp & 0x80000000) {
            tmp |= EXT_SIGN_32;
        }
        data->regs[u.bits.rd] = tmp;
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief OR bitwise operation
 */
class OR : public IsaProcessor {
public:
    OR() : IsaProcessor("OR", "0000000??????????110?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
                data->regs[u.bits.rs1] | data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief OR on register rs1 and the sign-extended 12-bit immediate.
 */
class ORI : public IsaProcessor {
public:
    ORI() : IsaProcessor("ORI", "?????????????????110?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] | imm;
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SLLI is a logical left shift (zeros are shifted into the lower bits)
 */
class SLLI : public IsaProcessor {
public:
    SLLI() : IsaProcessor("SLLI", "000000???????????001?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] << u.bits.imm;
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The SLT (signed comparision)
 *
 * It places the value 1 in register rd if rs1 < rs2, 0 otherwise 
 */
class SLT : public IsaProcessor {
public:
    SLT() : IsaProcessor("SLT", "0000000??????????010?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        if (static_cast<int64_t>(data->regs[u.bits.rs1]) <
                static_cast<int64_t>(data->regs[u.bits.rs2])) {
            data->regs[u.bits.rd] = 1;
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The SLTI (set less than immediate)
 *
 * It places the value 1 in register rd if register rs1 is less than the
 * sign-extended immediate when both are treated as signed numbers, else 0 
 * is written to rd. 
 */
class SLTI : public IsaProcessor {
public:
    SLTI() : IsaProcessor("SLTI", "?????????????????010?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        if (static_cast<int64_t>(data->regs[u.bits.rs1]) <
                static_cast<int64_t>(imm)) {
            data->regs[u.bits.rd] = 1;
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief The SLTU (unsigned comparision)
 *
 * SLTU perform unsigned compares, writing 1 to rd if rs1 < rs2, 0 otherwise.
 * @note SLTU rd, x0, rs2 sets rd to 1 if rs2 is not equal to zero, otherwise 
 * sets rd to zero (assembler pseudo-op SNEZ rd, rs).
 */
class SLTU : public IsaProcessor {
public:
    SLTU() : IsaProcessor("SLTU", "0000000??????????011?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        if (data->regs[u.bits.rs1] < data->regs[u.bits.rs2]) {
            data->regs[u.bits.rd] = 1;
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
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
class SLTIU : public IsaProcessor {
public:
    SLTIU() : IsaProcessor("SLTIU", "?????????????????011?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        if (data->regs[u.bits.rs1] < imm) {
            data->regs[u.bits.rd] = 1;
        } else {
            data->regs[u.bits.rd] = 0;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SLL logical shift left
 */
class SLL : public IsaProcessor {
public:
    SLL() : IsaProcessor("SLL", "0000000??????????001?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            data->regs[u.bits.rs1] << (data->regs[u.bits.rs2] & 0x3F);
        data->npc = data->pc + 4;
    }
};


/**
 * @brief SLLW is a left shifts by register defined value (RV64I).
 */
class SLLW : public IsaProcessor {
public:
    SLLW() : IsaProcessor("SLLW", "0000000??????????001?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] << data->regs[u.bits.rs2];
        data->regs[u.bits.rd] &= 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SLLIW is a shifts by a constant (RV64I).
 *
 * SLLIW, SRLIW, and SRAIW are RV64I-only instructions that operate on 32-bit 
 * values and produce signed 32-bit results.
 * @exception Illegal_Instruction if imm[5] not equal to 0.
 */
class SLLIW : public IsaProcessor {
public:
    SLLIW() : IsaProcessor("SLLIW", "0000000??????????001?????0011011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] << u.bits.imm;
        data->regs[u.bits.rd] &= 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        if (u.bits.imm >> 5) {
            generateException(EXCEPTION_InstrIllegal, data);
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRA arithmetic shift right
 */
class SRA : public IsaProcessor {
public:
    SRA() : IsaProcessor("SRA", "0100000??????????101?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = static_cast<int64_t>(data->regs[u.bits.rs1])
                                >> (data->regs[u.bits.rs2] & 0x3F);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRAW 32-bits arithmetic shift right (RV64I)
 */
class SRAW : public IsaProcessor {
public:
    SRAW() : IsaProcessor("SRAW", "0100000??????????101?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        int32_t t1 = static_cast<int32_t>(data->regs[u.bits.rs1]);
        data->regs[u.bits.rd] = 
            static_cast<int64_t>(t1 >> (data->regs[u.bits.rs2] & 0x1F));
        data->npc = data->pc + 4;
    }
};


/**
 * @brief SRAI is an arithmetic right shift.
 *
 * The original sign bit is copied into the vacanted upper bits.
 */
class SRAI : public IsaProcessor {
public:
    SRAI() : IsaProcessor("SRAI", "010000???????????101?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            static_cast<int64_t>(data->regs[u.bits.rs1]) >> u.bits.imm;
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRAIW arithmetic right shift (RV64I)
 */
class SRAIW : public IsaProcessor {
public:
    SRAIW() : IsaProcessor("SRAIW", "0100000??????????101?????0011011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        int32_t t1 = static_cast<int32_t>(data->regs[u.bits.rs1]);
        data->regs[u.bits.rd] = static_cast<int64_t>(t1 >> u.bits.imm);
        if (u.bits.imm >> 5) {
            generateException(EXCEPTION_InstrIllegal, data);
        }
        data->npc = data->pc + 4;
    }
};


/**
 * @brief SRL logical shift right
 */
class SRL : public IsaProcessor {
public:
    SRL() : IsaProcessor("SRL", "0000000??????????101?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            data->regs[u.bits.rs1] >> (data->regs[u.bits.rs2] & 0x3F);
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRLI is a logical right shift (zeros are shifted into the upper bits)
 */
class SRLI : public IsaProcessor {
public:
    SRLI() : IsaProcessor("SRLI", "000000???????????101?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] >> u.bits.imm;
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRLIW logical right shift (RV64I)
 */
class SRLIW : public IsaProcessor {
public:
    SRLIW() : IsaProcessor("SRLIW", "0000000??????????101?????0011011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            static_cast<uint32_t>(data->regs[u.bits.rs1]) >> u.bits.imm;
        if (u.bits.imm >> 5) {
            generateException(EXCEPTION_InstrIllegal, data);
        }
        data->npc = data->pc + 4;
    }
};

/**
 * @brief SRLW is a right shifts by register defined value (RV64I).
 */
class SRLW : public IsaProcessor {
public:
    SRLW() : IsaProcessor("SRLW", "0000000??????????101?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
            data->regs[u.bits.rs1] >> data->regs[u.bits.rs2];
        data->regs[u.bits.rd] &= 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
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
class SD : public IsaProcessor {
public:
    SD() : IsaProcessor("SD", "?????????????????011?????0100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t wdata;
        ISA_S_type u;
        u.value = payload[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        wdata = data->regs[u.bits.rs2];
        data->ibus->write(addr, reinterpret_cast<uint8_t *>(&wdata), 8);
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] <= %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), wdata);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * @brief Store rs2[31:0] to memory.
 */
class SW : public IsaProcessor {
public:
    SW() : IsaProcessor("SW", "?????????????????010?????0100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t wdata = 0;
        ISA_S_type u;
        u.value = payload[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        wdata = data->regs[u.bits.rs2];
        data->ibus->write(addr, reinterpret_cast<uint8_t *>(&wdata), 4);
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] <= %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), wdata & 0xFFFFFFFF);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * @brief Store rs2[15:0] to memory.
 */
class SH : public IsaProcessor {
public:
    SH() : IsaProcessor("SH", "?????????????????001?????0100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t wdata;
        ISA_S_type u;
        u.value = payload[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        wdata = data->regs[u.bits.rs2] & 0xFFFF;
        data->ibus->write(addr, reinterpret_cast<uint8_t *>(&wdata), 2);
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] <= %016" RV_PRI64 "x\n",
                        static_cast<int>(addr), wdata & 0xFFFF);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/**
 * @brief Store rs2[7:0] to memory.
 */
class SB : public IsaProcessor {
public:
    SB() : IsaProcessor("SB", "?????????????????000?????0100011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        uint64_t wdata;
        ISA_S_type u;
        u.value = payload[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        uint64_t addr = data->regs[u.bits.rs1] + off;
        wdata = data->regs[u.bits.rs2] & 0xFF;
        data->ibus->write(addr, reinterpret_cast<uint8_t *>(&wdata), 1);
        data->npc = data->pc + 4;
        if (data->mem_trace_file) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                        "[%08x] <= %016I64x\n", (int)addr, (uint8_t)wdata);
            (*data->mem_trace_file) << tstr;
            data->mem_trace_file->flush();
        }
    }
};

/** 
 * @brief Subtruction. Overflows are ignored
 */
class SUB : public IsaProcessor {
public:
    SUB() : IsaProcessor("SUB", "0100000??????????000?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] - data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
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
class SUBW : public IsaProcessor {
public:
    SUBW() : IsaProcessor("SUBW", "0100000??????????000?????0111011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        
        data->regs[u.bits.rd] = 
            (data->regs[u.bits.rs1] - data->regs[u.bits.rs2]) & 0xFFFFFFFFLL;
        if (data->regs[u.bits.rd] & (1LL << 31)) {
            data->regs[u.bits.rd] |= EXT_SIGN_32;
        }
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief XOR bitwise operation
 */
class XOR : public IsaProcessor {
public:
    XOR() : IsaProcessor("XOR", "0000000??????????100?????0110011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_R_type u;
        u.value = payload[0];
        data->regs[u.bits.rd] = 
                data->regs[u.bits.rs1] ^ data->regs[u.bits.rs2];
        data->npc = data->pc + 4;
    }
};

/** 
 * @brief XOR on register rs1 and the sign-extended 12-bit immediate.
 *
 * XORI rd, rs1, -1 performs a bitwise logical inversion of register rs1 
 * (assembler pseudo-instruction NOT rd, rs).
 */
class XORI : public IsaProcessor {
public:
    XORI() : IsaProcessor("XORI", "?????????????????100?????0010011") {}

    virtual void exec(uint32_t *payload, CpuContextType *data) {
        ISA_I_type u;
        u.value = payload[0];
    
        uint64_t imm = u.bits.imm;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        data->regs[u.bits.rd] = data->regs[u.bits.rs1] ^ imm;
        data->npc = data->pc + 4;
    }
};

void addIsaUserRV64I(CpuContextType *data, AttributeType *out) {
    addSupportedInstruction(new ADD, out);
    addSupportedInstruction(new ADDI, out);
    addSupportedInstruction(new ADDIW, out);
    addSupportedInstruction(new ADDW, out);
    addSupportedInstruction(new AND, out);
    addSupportedInstruction(new ANDI, out);
    addSupportedInstruction(new AUIPC, out);
    addSupportedInstruction(new BEQ, out);
    addSupportedInstruction(new BGE, out);
    addSupportedInstruction(new BGEU, out);
    addSupportedInstruction(new BLT, out);
    addSupportedInstruction(new BLTU, out);
    addSupportedInstruction(new BNE, out);
    addSupportedInstruction(new JAL, out);
    addSupportedInstruction(new JALR, out);
    addSupportedInstruction(new LD, out);
    addSupportedInstruction(new LW, out);
    addSupportedInstruction(new LWU, out);
    addSupportedInstruction(new LH, out);
    addSupportedInstruction(new LHU, out);
    addSupportedInstruction(new LB, out);
    addSupportedInstruction(new LBU, out);
    addSupportedInstruction(new LUI, out);
    addSupportedInstruction(new OR, out);
    addSupportedInstruction(new ORI, out);
    addSupportedInstruction(new SLL, out);
    addSupportedInstruction(new SLLI, out);
    addSupportedInstruction(new SLLIW, out);
    addSupportedInstruction(new SLLW, out);
    addSupportedInstruction(new SLT, out);
    addSupportedInstruction(new SLTI, out);
    addSupportedInstruction(new SLTU, out);
    addSupportedInstruction(new SLTIU, out);
    addSupportedInstruction(new SRA, out);
    addSupportedInstruction(new SRAI, out);
    addSupportedInstruction(new SRAIW, out);
    addSupportedInstruction(new SRAW, out);
    addSupportedInstruction(new SRL, out);
    addSupportedInstruction(new SRLI, out);
    addSupportedInstruction(new SRLIW, out);
    addSupportedInstruction(new SRLW, out);
    addSupportedInstruction(new SUB, out);
    addSupportedInstruction(new SUBW, out);
    addSupportedInstruction(new SD, out);
    addSupportedInstruction(new SW, out);
    addSupportedInstruction(new SH, out);
    addSupportedInstruction(new SB, out);
    addSupportedInstruction(new XOR, out);
    addSupportedInstruction(new XORI, out);

    /** Base[XLEN-1:XLEN-2]
     *      1 = 32
     *      2 = 64
     *      3 = 128
     */
    data->csr[CSR_misa] = 0x8000000000000000LL;
    data->csr[CSR_misa] |= (1LL << ('I' - 'A'));
}

/**
 * When a trap is taken, the stack is pushed to the left and PRV is set to the 
 * privilege mode of the activated trap handler with
 * IE=0.
 *
 * By default, all traps at any privilege level are handled in machine mode, 
 * though a machine-mode  * handler can quickly redirect traps back to the 
 * appropriate level using mrts and mrth instructions (Section 3.2.2). 
 * To increase performance, implementations can provide individual read/write 
 * bits within mtdeleg to indicate that certain traps should be processed 
 * directly by a lower privilege level.
 * 
 * The machine trap delegation register (mtdeleg) is an XLEN-bit read/write 
 * register that must be implemented, but which can contain a read-only value 
 * of zero, indicating that hardware will always direct all traps to machine 
 * mode.
 */
void generateException(uint64_t code, CpuContextType *data) {
    csr_mcause_type cause;
    cause.value     = 0;
    cause.bits.irq  = 0;
    cause.bits.code = code;
    data->csr[CSR_mcause] = cause.value;
    data->exception |= 1LL << code;
}

}  // namespace debugger
