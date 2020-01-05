/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __SRC_COMMON_GENERIC_RISCV_DISASM_H__
#define __SRC_COMMON_GENERIC_RISCV_DISASM_H__

#include <inttypes.h>

namespace debugger {

int riscv_disassembler(uint32_t instr, char *str, size_t strsz);

}  // namespace debugger

#endif  // __SRC_COMMON_GENERIC_RISCV_DISASM_H__
