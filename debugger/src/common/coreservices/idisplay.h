/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Dismplay Simulation interface.
 */

#ifndef __DEBUGGER_PLUGIN_IDISPLAY_H__
#define __DEBUGGER_PLUGIN_IDISPLAY_H__

#include <iface.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_DISPLAY = "IDisplay";

class IDisplay : public IFace {
 public:
    IDisplay() : IFace(IFACE_DISPLAY) {}

    virtual void initFrame() = 0;
    virtual void setFramePixel(int x, int y, uint32_t rgb) = 0;
    virtual void updateFrame() = 0;
};

static const char *const IFACE_LED_CONTROLLER = "ILedController";

class ILedController : public IFace {
 public:
    ILedController() : IFace(IFACE_LED_CONTROLLER) {}

    virtual void getResolution(int *width, int *height) = 0;
    virtual void registerDisplay(IDisplay *led) = 0;
    virtual void unregisterDisplay(IDisplay *led) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IDISPLAY_H__
