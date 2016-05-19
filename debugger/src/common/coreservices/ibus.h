/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System with Bus Direct Memory Access interface.
 */

#ifndef __DEBUGGER_IBUS_PLUGIN_H__
#define __DEBUGGER_IBUS_PLUGIN_H__

#include "iface.h"
#include <inttypes.h>
#include "imemop.h"

namespace debugger {

static const char *const IFACE_BUS = "IBus";

class IBus : public IFace {
public:
    IBus() : IFace(IFACE_BUS) {}

    virtual void map(IMemoryOperation *imemop) =0;

    virtual int read(uint64_t addr, uint8_t *payload, int sz) =0;

    virtual int write(uint64_t addr, uint8_t *payload, int sz) =0;

    virtual void addBreakpoint(uint64_t addr) =0;

    virtual void removeBreakpoint(uint64_t addr) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IBUS_PLUGIN_H__
