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

static const char *IFACE_RESET_LISTENER = "IResetListener";

class IResetListener : public IFace {
 public:
    IResetListener() : IFace(IFACE_RESET_LISTENER) {}

    virtual void reset(bool active) = 0;
};

static const char *const IFACE_RESET = "IReset";

class IReset : public IFace {
 public:
    IReset() : IFace(IFACE_RESET) {
        resetListeners_.make_list(0);
    }

    virtual void registerResetListener(IFace *listener) {
        AttributeType item;
        item.make_iface(listener);
        resetListeners_.add_to_list(&item);
    }

    virtual void unregisterResetListener(IFace *listener) {
        for (unsigned i = 0; i < resetListeners_.size(); i++) {
            if (listener == resetListeners_[i].to_iface()) {
                resetListeners_.remove_from_list(i);
                return;
            }
        }
    }

    void reset(bool active) {
        IResetListener *l;
        for (unsigned i = 0; i < resetListeners_.size(); i++) {
            l = static_cast<IResetListener *>(resetListeners_[i].to_iface());
            l->reset(active);
        }
    }

    virtual void powerOnPressed() = 0;
    virtual void powerOnReleased() = 0;

 protected:
    AttributeType resetListeners_;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IRESET_H__
