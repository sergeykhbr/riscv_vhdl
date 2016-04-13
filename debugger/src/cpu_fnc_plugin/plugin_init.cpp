/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plugin library initialization method.
 */

#include "cpu_riscv_func.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS(CpuRiscV_Functional);
}

}  // namespace debugger
