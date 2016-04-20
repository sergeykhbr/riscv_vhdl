/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simulator of the FPGA board with Ethernet UDP/EDCL interface.
 */

#include "api_core.h"
#include "boardsim.h"

namespace debugger {

BoardSim::BoardSim(const char *name)  : IService(name) {
    registerInterface(static_cast<IBoardSim *>(this));
}

void BoardSim::postinitService() {
}

}  // namespace debugger

