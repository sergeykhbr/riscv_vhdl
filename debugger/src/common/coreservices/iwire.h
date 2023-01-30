/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>
#include <iservice.h>
#include "coreservices/icmdexec.h"

namespace debugger {

static const char *const IFACE_WIRE = "IWire";

class IWire : public IFace {
 public:
    IWire() : IFace(IFACE_WIRE) {}

    virtual void raiseLine() = 0;
    virtual void lowerLine() = 0;
    virtual void setLevel(bool level) = 0;
    virtual bool getLevel() = 0;
};

class GenericWireAttribute : public AttributeType,
                             public IWire {
 public:
    GenericWireAttribute(IService *parent, const char *name)
        : parent_(parent) {
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

class WireCmdType : public ICommand {
 public:
    WireCmdType(IService *parent, const char *name)
        : ICommand(parent, name) {
        iwire_ = static_cast<IWire *>(parent->getInterface(IFACE_WIRE));
        briefDescr_.make_string("Generic Wire instance management command.");
        detailedDescr_.make_string(
            "Read/Write value:\n"
            "    wire_objname <write_value>\n"
            "Response:\n"
            "    int i1:\n"
            "        i1  - signal value\n"
            "Usage:\n"
            "    pin0\n"
            "    pin1 1");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args) {
        if (!iwire_ || !(*args)[0u].is_equal(cmdParent_->getObjName())) {
            return CMD_INVALID;
        }
        return CMD_VALID;
    }

    virtual void exec(AttributeType *args, AttributeType *res) {
        bool vol = iwire_->getLevel();
        if (args->size() > 1) {
            vol = (*args)[1].to_bool();
            iwire_->setLevel(vol);
        }
        res->make_boolean(vol);
    }

 private:
    IWire *iwire_;
};

}  // namespace debugger
