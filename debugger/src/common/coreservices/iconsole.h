/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Console interface declaration.
 */

#ifndef __DEBUGGER_ICONSOLE_H__
#define __DEBUGGER_ICONSOLE_H__

#include "iface.h"

namespace debugger {

static const char *const IFACE_CONSOLE = "IConsole";

class IConsole : public IFace {
public:
    IConsole() : IFace(IFACE_CONSOLE) {}

    virtual void writeBuffer(const char *buf) =0;
    virtual void writeCommand(const char *cmd) =0;
    virtual int registerKeyListener(IFace *iface) =0;
    virtual void setCmdString(const char *buf) =0;
    virtual void enableLogFile(const char *filename) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ICONSOLE_H__
