/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction object declaration.
 */

#ifndef __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__
#define __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__

#include <inttypes.h>
#include "generic/cpu_generic.h"

namespace debugger {

class CpuRiver_Functional;

class RiscvInstruction : public GenericInstruction {
public:
    RiscvInstruction(CpuRiver_Functional *icpu, const char *name,
                    const char *bits);

    // IInstruction interface:
    virtual const char *name() { return name_.to_string(); }

    virtual bool parse(uint32_t *payload) {
        return ((payload[0] & mask_) == opcode_);
    }

    virtual uint32_t hash() {
        return (opcode_ >> 2) & 0x1F;
    }

    uint16_t hash16() {
        uint16_t t1 = static_cast<uint16_t>(opcode_) & 0x3;
        return 0x20 | ((static_cast<uint16_t>(opcode_) >> 13) << 2) | t1;
    }

protected:
    AttributeType name_;
    CpuRiver_Functional *icpu_;
    uint32_t mask_;
    uint32_t opcode_;
    uint64_t *R;
};

class RiscvInstruction16 : public RiscvInstruction {
public:
    RiscvInstruction16(CpuRiver_Functional *icpu, const char *name,
                    const char *bits) : RiscvInstruction(icpu, name, bits) {}

    // IInstruction interface:
    virtual uint32_t hash() {
        uint16_t t1 = static_cast<uint16_t>(opcode_) & 0x3;
        return 0x20 | ((static_cast<uint16_t>(opcode_) >> 13) << 2) | t1;
    }
};


}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__
