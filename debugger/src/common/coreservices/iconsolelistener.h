/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Listener of the console requests interface.
 */

#ifndef __DEBUGGER_ICONSOLELISTENER_H__
#define __DEBUGGER_ICONSOLELISTENER_H__

#include "iface.h"

namespace debugger {

static const char *IFACE_CONSOLE_LISTENER = "IConsoleListener";

class IConsoleListener : public IFace {
public:
    IConsoleListener() : IFace(IFACE_CONSOLE_LISTENER) {}

    virtual void udpateCommand(const char *line) =0;
    virtual void autocompleteCommand(const char *line) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ICONSOLELISTENER_H__
