/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Signal listener interface declaration.
 */

#ifndef __DEBUGGER_ISIGNAL_LISTENER_H__
#define __DEBUGGER_ISIGNAL_LISTENER_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *IFACE_SIGNAL_LISTENER = "ISignalListener";

class ISignalListener : public IFace {
public:
    ISignalListener() : IFace(IFACE_SIGNAL_LISTENER) {}

    /**
    * @brief Levels of the specified pins were changed.
    * @param[in] start Index of the start pin to change.
    * @param[in] width Total number of pins to change.
    * @param[in] value New value that will be written to pins.
    */
    virtual void updateSignal(int start, int width, uint64_t value) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISIGNAL_LISTENER_H__
