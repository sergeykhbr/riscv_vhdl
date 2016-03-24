/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Keyboard events listener interface declaration.
 */

#ifndef __DEBUGGER_IKEYLISTENER_H__
#define __DEBUGGER_IKEYLISTENER_H__

#include "iface.h"

namespace debugger {

static const char *IFACE_KEY_LISTENER = "IKeyListener";

class IKeyListener : public IFace {
public:
    IKeyListener() : IFace(IFACE_KEY_LISTENER) {}

    virtual int keyDown(int value) =0;
    virtual int keyUp(int value) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IKEYLISTENER_H__
