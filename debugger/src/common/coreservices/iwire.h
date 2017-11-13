/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Single wire interface.
 */

#ifndef __DEBUGGER_PLUGIN_IWIRE_H__
#define __DEBUGGER_PLUGIN_IWIRE_H__

#include "iface.h"
#include <inttypes.h>
#include "attribute.h"
#include "iservice.h"

namespace debugger {

static const char *const IFACE_WIRE = "IWire";

class IWire : public IFace {
public:
    IWire() : IFace(IFACE_WIRE) {}

    virtual void raiseLine() =0;
    virtual void lowerLine() =0;
    virtual void setLevel(bool level) =0;
    virtual bool getLevel() =0;
};

class GenericWireAttribute : public AttributeType,
                             public IWire {
public:
    GenericWireAttribute(IService *parent, const char *name) : parent_(parent) {
        parent->registerAttribute(name, static_cast<IAttribute *>(this));
        parent->registerPortInterface(name, static_cast<IWire *>(this));
        wireName_.make_string(name);
        make_boolean(false);
    }

    /** IWire */
    virtual void raiseLine() { make_boolean(true); }
    virtual void lowerLine() { make_boolean(false); }
    virtual void setLevel(bool level) { make_boolean(level); }
    virtual bool getLevel() { return to_bool(); }

protected:
    // Debug output compatibility
    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }

protected:
    IService *parent_;
    AttributeType wireName_;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IWIRE_H__
