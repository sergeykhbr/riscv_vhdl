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

#ifndef __DEBUGGER_PLUGIN_IRESET_H__
#define __DEBUGGER_PLUGIN_IRESET_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *IFACE_POWER = "IPower";

enum EPowerAction {
    POWER_ON,
    POWER_OFF
};

class IPower : public IFace {
 public:
    IPower() : IFace(IFACE_POWER) {}

    virtual void power(EPowerAction onoff) = 0;
};

static const char *IFACE_RESET_LISTENER = "IResetListener";

class IResetListener : public IFace {
 public:
    IResetListener() : IFace(IFACE_RESET_LISTENER) {}

    virtual void reset(IFace *isource) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IRESET_H__
