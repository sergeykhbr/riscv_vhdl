/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plugin library initialization method.
 */

#include "boardsim.h"

namespace debugger {

extern "C" void plugin_init(void) {
    static BoardSimClass local_class;
}

}  // namespace debugger
