/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      General interface of the hardware access.
 */

#ifndef __DEBUGGER_ITAP_H__
#define __DEBUGGER_ITAP_H__

#include "iface.h"
#include "attribute.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_TAP = "ITap";

static const char *const ITap_brief = 
"Test Access Point (TAP) software interface.";

static const char *const ITap_detail = 
"This interface is used for the direct access to the Hardware. "
"Typically, it is doing via JTAG or other transport interface.";

class ITap : public IFace {
public:
    ITap() : IFace(IFACE_TAP) {}

    virtual const char *getBrief() { return ITap_brief; }

    virtual const char *getDetail() { return ITap_detail; }

    virtual int read(uint64_t addr, int bytes, uint8_t *obuf) =0;
    virtual int write(uint64_t addr, int bytes, uint8_t *ibuf) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ITAP_H__
