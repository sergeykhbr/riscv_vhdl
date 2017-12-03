/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction types parser.
 */

#include "api_utils.h"
#include "riscv-isa.h"
#include "instructions.h"
#include "cpu_riscv_func.h"

namespace debugger {

RiscvInstruction::RiscvInstruction(CpuRiver_Functional *icpu, const char *name,
                                    const char *bits) {
    icpu_ = icpu;
    R = icpu->getpRegs();
    name_.make_string(name);
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

}  // namespace debugger
