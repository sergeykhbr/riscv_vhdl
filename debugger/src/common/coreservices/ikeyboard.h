/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Keyboard interface declaration.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__
#define __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__

#include <iface.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_KEYBOARD = "IKeyboard";

class IKeyboard : public IFace {
 public:
    IKeyboard() : IFace(IFACE_KEYBOARD) {}

    virtual void keyPress(const char *keyname) = 0;
    virtual void keyRelease(const char *keyname) = 0;
    virtual int getRow() { return 0; }
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__
