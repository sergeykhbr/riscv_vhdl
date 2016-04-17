/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction types parser.
 */

#include "riscv-isa.h"
#include "api_utils.h"

namespace debugger {

unsigned addSupportedInstruction(IsaProcessor *instr, AttributeType *out) {
    AttributeType tmp(instr);
    out[instr->hash()].add_to_list(&tmp);
    return 0;
}

}  // namespace debugger
