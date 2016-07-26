/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Signal Interface declaration.
 *
 *             This interface corresponds to a IO-pin or a group of pins.
 */

#ifndef __DEBUGGER_ISIGNAL_H__
#define __DEBUGGER_ISIGNAL_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *IFACE_SIGNAL = "ISignal";

class ISignal : public IFace {
public:
    ISignal() : IFace(IFACE_SIGNAL) {}

    /**
    * @brief Assign new value from an external module.
    * @param[in] start Index of the start pin to change.
    * @param[in] width Total number of pins to change.
    * @param[in] value New value that will be written to pins.
    */
    virtual void setLevel(int start, int width, uint64_t value) =0;

    /**
    * @brief Register listener of signal changing.
    */
    virtual void registerSignalListener(IFace *listener) =0;

    /**
    * @brief Unregister listener.
    */
    virtual void unregisterSignalListener(IFace *listener) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISIGNAL_H__
