/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core Service interface declaration.
 */

#ifndef __DEBUGGER_SERVICE_H__
#define __DEBUGGER_SERVICE_H__

#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *const IFACE_SERVICE = "IService";

class IService : public IFace {
public:
    IService(const char *obj_name) 
        : IFace(IFACE_SERVICE) {
        listInterfaces_ = AttributeType(Attr_List);
        listAttributes_ = AttributeType(Attr_List);
        registerInterface(static_cast<IService *>(this));
        obj_name_ = obj_name;
        config_.make_nil();
    }

    virtual void initService(const AttributeType *args) {
        if (args) {
            config_.clone(args);
        }
    }

    virtual void postinitService() {}
    virtual void deleteService() {}

    virtual void registerInterface(IFace *iface) {
        AttributeType item(iface);
        listInterfaces_.add_to_list(&item);
    }

    virtual IFace *getInterface(const char *name) {
        IFace *tmp;
        for (unsigned i = 0; i < listInterfaces_.size(); i++) {
            tmp = listInterfaces_[i].to_iface();
            if (strcmp(name, tmp->getFaceName()) == 0) {
                return tmp;
            }
        }
        return NULL;
    }

    virtual void registerAttribute(const char *name, IAttribute *iface) {
        AttributeType item(iface);
        iface->setAttrName(name);
        listAttributes_.add_to_list(&item);
    }

    virtual const char *getObjName() { return obj_name_; }

    virtual AttributeType getConfiguration() {
        AttributeType ret(Attr_Dict);
        ret["Name"] = AttributeType(getObjName());
        ret["attr"] = AttributeType(Attr_List);

        IAttribute *tmp = NULL;
        for (unsigned i = 0; i < listAttributes_.size(); i++) {
            tmp = static_cast<IAttribute *>(listAttributes_[i].to_iface());
            AttributeType val(tmp->getAttrName());
            ret["attr"].add_to_list(&val);
        }
        return ret;
    }

protected:
    AttributeType config_;
    AttributeType listInterfaces_;
    AttributeType listAttributes_;
    const char *obj_name_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SERVICE_H__
