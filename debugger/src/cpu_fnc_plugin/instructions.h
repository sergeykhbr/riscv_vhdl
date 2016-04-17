/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction object declaration.
 */

#ifndef __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__
#define __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__

#include <inttypes.h>
#include "iface.h"
#include "attribute.h"
#include "iinstr.h"

namespace debugger {

class IsaProcessor : public IInstruction {
public:
    IsaProcessor(const char *name, const char *bits) {
        name_ = name;
        mask_ = 0;
        opcode_ = 0;
        for (int i = 0; i < 32; i++) {
            switch (bits[i]) {
            case '0':
                break;
            case '1':
                opcode_ |= (1 << (31 - i));
                break;
            case '?':
                mask_ |= (1 << (31 - i));
                break;
            default:;
            }
        }
        mask_ ^= ~0;
    }

    // IInstruction interface:
    virtual const char *name() { return name_; }

    virtual bool parse(uint32_t *payload) {
        return ((payload[0] & mask_) == opcode_);
    }

    virtual void exec(uint32_t *payload, CpuContextType *regs) =0;

    virtual uint32_t hash() {
        return (opcode_ >> 2) & 0x1F;
    }

protected:
    const char *name_;
    uint32_t mask_;
    uint32_t opcode_;
};

unsigned addSupportedInstruction(IsaProcessor *instr, AttributeType *out);

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_INSTRUCTIONS_H__
