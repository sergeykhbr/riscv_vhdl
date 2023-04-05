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

#ifndef __DEBUGGER_CLASS_H__
#define __DEBUGGER_CLASS_H__

#include <iface.h>
#include <iservice.h>
#include <api_core.h>

namespace debugger {

static const char *const IFACE_CLASS = "IClass";

class IClass : public IFace {
 public:
    explicit IClass(const char *class_name)
        : IFace(IFACE_CLASS) {
        class_name_.make_string(class_name);
        RISCV_register_class(static_cast<IClass *>(this));
    }
    virtual ~IClass() {
    }

    virtual IService *createService(const char *obj_name) = 0;

    virtual const char *getClassName() { return class_name_.to_string(); }

    virtual AttributeType getConfiguration() {
        AttributeType ret(Attr_Dict);
        ret["Class"] = AttributeType(getClassName());
        ret["Instances"] = AttributeType(Attr_List);

        /*IService *tmp = NULL;
        for (unsigned i = 0; i < listInstances_.size(); i++) {
            tmp = static_cast<IService *>(listInstances_[i].to_iface());
            AttributeType val = tmp->getConfiguration();
            ret["Instances"].add_to_list(&val);
        }*/
        return ret;
    }

 protected:
    AttributeType class_name_;
};

/**
 * @brief Unified macros of plugin class declaration.
 * @see simple_plugin.cpp example
 */
#define DECLARE_CLASS(name) \
class name ## Class : public IClass { \
 public: \
    name ## Class() : IClass(# name "Class") {} \
    virtual IService *createService(const char *obj_name) {  \
        name *serv = new name(obj_name); \
        serv->setClassOwner(this); \
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
