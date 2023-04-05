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

 #pragma once

#include <api_core.h>
#include "coreservices/imemop.h"

namespace debugger {

static const char *const IFACE_SERVICE = "IService";

class IService : public IFace {
 public:
    explicit IService(const char *obj_name) : IFace(IFACE_SERVICE) {
        listInterfaces_ = AttributeType(Attr_List);
        listAttributes_ = AttributeType(Attr_List);
        listPorts_ = AttributeType(Attr_List);
        registerInterface(static_cast<IService *>(this));
        registerAttribute("Parent", &parent_);
        registerAttribute("LogLevel", &logLevel_);
        registerAttribute("ObjDescription", &obj_descr_);
        classown_.make_nil();
        parent_.make_string("");
        obj_name_.make_string(obj_name);
        obj_descr_.make_string("");
        logLevel_.make_int64(LOG_ERROR);
    
        if (strcmp(obj_name, "CoreService") != 0) {
            // Do not call this method for Core class because lists are not initialized
            RISCV_register_service(static_cast<IService *>(this));
        }
    }
    IService(IService *parent, const char *obj_name)
        : IService(obj_name) {
        if (parent) {
            parent_.make_string(parent->getObjName());
        }
    }
    virtual ~IService() {
        // @warning: NEED to unregister attribute from class destructor
        /*for (unsigned i = 0; i < listAttributes_.size(); i++) {
            IAttribute *iattr = static_cast<IAttribute *>(
                                    listAttributes_[i].to_iface());
            iattr->freeAttrName();
            iattr->freeAttrDescription();
        }*/
    }

    virtual void setClassOwner(IFace *icls) {
        classown_.make_iface(icls);
    }

    virtual const char *getParentName() {
        return parent_.to_string();
    }

    virtual void initService(const AttributeType *args) {
        if (!args || !args->is_list()) {
            return;
        }
        AttributeType *cur_attr;
        for (unsigned i = 0; i < args->size(); i++) {
            const AttributeType &item = (*args)[i];
            if (item.size() < 2 || !item[0u].is_string()) {
                continue;
            }
            cur_attr = static_cast<AttributeType *>(
                                getAttribute(item[0u].to_string()));
            if (cur_attr == NULL) {
                RISCV_error("Attribute '%s' not found", item[0u].to_string());
                continue;
            }
            (*cur_attr) = item[1];
            if (item.size() >= 3 && item[2].is_string()) {
                static_cast<IAttribute *>(cur_attr)->allocAttrDescription(
                    item[2].to_string());
            }
        }
    }

    virtual void postinitService() {}
    virtual void predeleteService() {}

    virtual void registerInterface(IFace *iface) {
        AttributeType item(iface);
        listInterfaces_.add_to_list(&item);
        if (strcmp(iface->getFaceName(), IFACE_MEMORY_OPERATION) == 0) {
            IMemoryOperation *imemop = static_cast<IMemoryOperation *>(iface);
            registerAttribute("MapList", &imemop->listMap_);
            registerAttribute("BaseAddress", &imemop->baseAddress_);
            registerAttribute("Length", &imemop->length_);
            registerAttribute("Priority", &imemop->priority_);
        }
    }
    virtual void registerPortInterface(const char *portname, IFace *iface) {
        AttributeType item;
        item.make_list(2);
        item[0u].make_string(portname);
        item[1].make_iface(iface);
        listPorts_.add_to_list(&item);
    }

    virtual void unregisterInterface(IFace *iface) {
        for (unsigned i = 0; i < listInterfaces_.size(); i++) {
            if (listInterfaces_[i].to_iface() == iface) {
                listInterfaces_.remove_from_list(i);
                break;
            }
        }
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

    virtual IFace *getPortInterface(const char *portname,
                                    const char *facename) {
        IFace *tmp;
        for (unsigned i = 0; i < listPorts_.size(); i++) {
            AttributeType &item = listPorts_[i];
            if (!item[0u].is_equal(portname)) {
                continue;
            }
            tmp = item[1].to_iface();
            if (strcmp(facename, tmp->getFaceName()) == 0) {
                return tmp;
            }
        }
        return NULL;
    }

    virtual const AttributeType *getPortList() { return &listPorts_; }

    virtual void registerAttribute(const char *name, IAttribute *iface) {
        AttributeType item(iface);
        iface->allocAttrName(name);
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

    virtual const char *getObjName() { return obj_name_.to_string(); }

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
    AttributeType classown_;
    AttributeType parent_;
    AttributeType namespaceName_;
    AttributeType listInterfaces_;
    AttributeType listPorts_;       // [['portname',iface],*]
    AttributeType listAttributes_;
    AttributeType logLevel_;
    AttributeType obj_name_;
    AttributeType obj_descr_;       // Describe service in JSON config
};

}  // namespace debugger
