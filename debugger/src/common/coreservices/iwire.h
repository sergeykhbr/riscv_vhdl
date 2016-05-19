/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Single wire interface.
 */

#ifndef __DEBUGGER_PLUGIN_IWIRE_H__
#define __DEBUGGER_PLUGIN_IWIRE_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_WIRE = "IWire";

class IWire : public IFace {
public:
    IWire() : IFace(IFACE_WIRE) {}

    virtual void raiseLine(int idx) =0;
    virtual void lowerLine() =0;
    virtual void setLevel(bool level) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IWIRE_H__
