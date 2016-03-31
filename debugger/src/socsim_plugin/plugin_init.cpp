/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plugin library initialization method.
 */

#include "boardsim.h"
#include "memsim.h"
#include "gpio.h"
#include "uart.h"
#include "pnp.h"
#include "irqctrl.h"
#include "gnss_stub.h"
#include "dsu.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS_IDX(BoardSim, 1);
    REGISTER_CLASS_IDX(MemorySim, 2);
    REGISTER_CLASS_IDX(GPIO, 3);
    REGISTER_CLASS_IDX(UART, 4);
    REGISTER_CLASS_IDX(PNP, 5);
    REGISTER_CLASS_IDX(IrqController, 6);
    REGISTER_CLASS_IDX(GNSSStub, 7);
    REGISTER_CLASS_IDX(DSU, 8);
}

}  // namespace debugger
