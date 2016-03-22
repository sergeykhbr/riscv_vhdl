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
#include "api_utils.h"

namespace debugger {

static const char *const IFACE_SERVICE = "IService";

class IService : public IFace {
public:
    IService(const char *obj_name) 
        : IFace(IFACE_SERVICE) {
        listInterfaces_ = AttributeType(Attr_List);
        listAttributes_ = AttributeType(Attr_List);
        registerInterface(static_cast<IService *>(this));
        registerAttribute("LogLevel", &logLevel_);
        obj_name_ = obj_name;
        logLevel_.make_int64(LOG_ERROR);
    }
    virtual ~IService() {}

    virtual void initService(const AttributeType *args) {
        if (!args || !args->is_list()) {
            return;
        }
        AttributeType *cur_attr;
        for (unsigned i = 0; i < args->size(); i++) {
            const AttributeType &item = (*args)[i];
            if (item.size() != 2 || !item[0u].is_string()) {
                continue;
            }
            cur_attr = static_cast<AttributeType *>(
                                getAttribute(item[0u].to_string()));
            if (cur_attr == NULL) {
                RISCV_error("Attribute '%s' not found", item[0u].to_string());
                continue;
            }
            (*cur_attr) = item[1];
        }
    }

    virtual void postinitService() {}
    virtual void predeleteService() {}

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

    virtual IAttribute *getAttribute(const char *name) {
        IAttribute *tmp;
        for (unsigned i = 0; i < listAttributes_.size(); i++) {
            tmp = static_cast<IAttribute *>(listAttributes_[i].to_iface());
            if (strcmp(name, tmp->getAttrName()) == 0) {
                return tmp;
            }
        }
        return NULL;
    }

    virtual const char *getObjName() { return obj_name_; }

    virtual AttributeType getConfiguration() {
        AttributeType ret(Attr_Dict);
        ret["Name"] = AttributeType(getObjName());
        ret["Attr"] = AttributeType(Attr_List);

        IAttribute *tmp = NULL;
        for (unsigned i = 0; i < listAttributes_.size(); i++) {
            tmp = static_cast<IAttribute *>(listAttributes_[i].to_iface());
            AttributeType item;
            item.make_list(2);
            item[0u].make_string(tmp->getAttrName());
            item[1] = *static_cast<AttributeType *>(tmp);
            ret["Attr"].add_to_list(&item);
        }
        return ret;
    }

protected:
    AttributeType listInterfaces_;
    AttributeType listAttributes_;
    AttributeType logLevel_;
    const char *obj_name_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SERVICE_H__
