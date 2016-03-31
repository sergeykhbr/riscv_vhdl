/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base Class interface declaration.
 */

#ifndef __DEBUGGER_CLASS_H__
#define __DEBUGGER_CLASS_H__

#include "iface.h"
#include "iservice.h"
#include "api_core.h"

namespace debugger {

static const char *const IFACE_CLASS = "IClass";

class IClass : public IFace {
public:
    IClass(const char *class_name) 
        : IFace(IFACE_CLASS), class_name_(class_name) {
        RISCV_register_class(static_cast<IClass *>(this));
        listInstances_ = AttributeType(Attr_List);
    }
    virtual ~IClass() {
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            delete static_cast<IService *>(listInstances_[i].to_iface());
        }
    }

    virtual IService *createService(const char *obj_name) =0;

    virtual void postinitServices() {
        IService *tmp = NULL;
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            tmp = static_cast<IService *>(listInstances_[i].to_iface());
            tmp->postinitService();
        }
    }

    virtual void predeleteServices(IService *inst) {
        IService *tmp = NULL;
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            tmp = static_cast<IService *>(listInstances_[i].to_iface());
            if (inst == tmp) {
                tmp->predeleteService();
            }
        }
    }


    virtual const char *getClassName() { return class_name_; }

    virtual IService *getInstance(const char *name) {
        IService *ret = NULL;
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            ret = static_cast<IService *>(listInstances_[i].to_iface());
            if (strcmp(name, ret->getObjName()) == 0) {
                return ret;
            }
        }
        return NULL;
    }

    virtual AttributeType getConfiguration() {
        AttributeType ret(Attr_Dict);
        ret["Class"] = AttributeType(getClassName());
        ret["Instances"] = AttributeType(Attr_List);

        IService *tmp = NULL;
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            tmp = static_cast<IService *>(listInstances_[i].to_iface());
            AttributeType val = tmp->getConfiguration();
            ret["Instances"].add_to_list(&val);
        }
        return ret;
    }

    virtual const AttributeType *getInstanceList() { return &listInstances_; }

protected:
    const char *class_name_;
    AttributeType listInstances_;
};

/**
 * @brief Unified macros of plugin class declaration.
 * @see simple_plugin.cpp example
 */
#define DECLARE_CLASS(name) \
class name ## Class : public IClass { \
public: \
    name ## Class () : IClass(# name "Class") {} \
    virtual IService *createService(const char *obj_name) {  \
        name *serv = new name(obj_name); \
        AttributeType item(static_cast<IService *>(serv)); \
        listInstances_.add_to_list(&item); \
        return serv; \
    } \
};

/**
 * @brief Unified macros of plugin class registration in kernel library.
 * @see simple_plugin.cpp example
 */
#define REGISTER_CLASS(name) static name ## Class local_class;
#define REGISTER_CLASS_IDX(name, idx) static name ## Class local_class_ ## idx;

}  // namespace debugger

#endif  // __DEBUGGER_CLASS_H__
