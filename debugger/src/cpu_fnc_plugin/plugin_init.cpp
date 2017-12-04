/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plugin library initialization method.
 */

#include "cpu_riscv_func.h"
#include "srcproc/srcproc.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS_IDX(CpuRiver_Functional, 1);
    REGISTER_CLASS_IDX(RiscvSourceService, 2);
}

}  // namespace debugger
