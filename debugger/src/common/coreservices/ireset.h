/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Generic reset interface.
 */

#ifndef __DEBUGGER_PLUGIN_IRESET_H__
#define __DEBUGGER_PLUGIN_IRESET_H__

#include "iface.h"
#include "attribute.h"
#include <inttypes.h>

namespace debugger {

static const char *IFACE_RESET_LISTENER = "IResetListener";

class IResetListener : public IFace {
public:
    IResetListener() : IFace(IFACE_RESET_LISTENER) {}

    virtual void reset(bool active) =0;
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

    virtual void powerOnPressed() =0;
    virtual void powerOnReleased() =0;

protected:
    AttributeType resetListeners_;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IRESET_H__
