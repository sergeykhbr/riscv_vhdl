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
    IAttribute() : IFace(IFACE_ATTRIBUTE), attr_name_ (NULL), attr_descr_(NULL) {}

    virtual void allocAttrName(const char *name) =0;
    virtual void freeAttrName() =0;
    virtual const char *getAttrName() { return attr_name_; }

    virtual void allocAttrDescription(const char *descr) =0;
    virtual void freeAttrDescription() =0;
    virtual const char *getAttrDescription() { return attr_descr_; }

    virtual void postinitAttribute() {}

protected:
    char *attr_name_;
    char *attr_descr_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IATTRIBUTE_H__
