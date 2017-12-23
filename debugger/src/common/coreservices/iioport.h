/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      IO-port interface declaration.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__
#define __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__

#include <iface.h>

namespace debugger {

static const char *IFACE_IOPORT = "IIOPort";

class IIOPort : public IFace {
 public:
    IIOPort() : IFace(IFACE_IOPORT) {}

    virtual void registerPortListener(IFace *listener) = 0;
    virtual void unregisterPortListener(IFace *listener) = 0;
};

static const char *IFACE_IOPORT_LISTENER = "IIOPortListener";

class IIOPortListener : public IFace {
 public:
    IIOPortListener() : IFace(IFACE_IOPORT_LISTENER) {}

    virtual void readData(uint8_t *val, uint8_t mask) = 0;
    virtual void writeData(uint8_t val, uint8_t mask) = 0;
    virtual void latch() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__
