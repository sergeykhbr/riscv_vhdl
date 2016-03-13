/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core Attribute interface declaration.
 */

#ifndef __DEBUGGER_IATTRIBUTE_H__
#define __DEBUGGER_IATTRIBUTE_H__

#include "iface.h"

namespace debugger {

static const char *const IFACE_ATTRIBUTE = "IAttribute";

class IAttribute : public IFace {
public:
    IAttribute() : IFace(IFACE_ATTRIBUTE), attr_name_ (NULL) {}

    virtual void setAttrName(const char *name) { attr_name_ = name; }

    virtual const char *getAttrName() { return attr_name_; }

protected:
    const char *attr_name_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IATTRIBUTE_H__
