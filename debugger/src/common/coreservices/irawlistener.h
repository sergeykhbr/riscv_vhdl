/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Raw Data listener interface declaration.
 */

#ifndef __DEBUGGER_IRAW_LISTENER_H__
#define __DEBUGGER_IRAW_LISTENER_H__

#include "iface.h"

namespace debugger {

static const char *IFACE_RAW_LISTENER = "IRawListener";

class IRawListener : public IFace {
public:
    IRawListener() : IFace(IFACE_RAW_LISTENER) {}

    virtual void updateData(const char *buf, int buflen) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IRAW_LISTENER_H__
