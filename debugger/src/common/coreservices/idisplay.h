/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
