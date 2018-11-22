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

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICOMMAND_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICOMMAND_H__

#include <iface.h>
#include <attribute.h>
#include "coreservices/itap.h"

namespace debugger {

static const char *IFACE_COMMAND = "ICommand";

static const int CMD_VALID      = 0;
static const int CMD_WRONG_ARGS = 1;
static const int CMD_INVALID    = 2;

class ICommand : public IFace {
 public:
    ICommand(const char *name, ITap *tap)
        : IFace(IFACE_COMMAND) {
        cmdName_.make_string(name);
        tap_ = tap;
    }
    virtual ~ICommand() {}

    virtual const char *cmdName() { return cmdName_.to_string(); }
    virtual const char *briefDescr() { return briefDescr_.to_string(); }
    virtual const char *detailedDescr() { return detailedDescr_.to_string(); }

    virtual int isValid(AttributeType *args) = 0;
    virtual void exec(AttributeType *args, AttributeType *res) = 0;

    virtual void generateError(AttributeType *res, const char *descr) {
        res->make_list(3);
        (*res)[0u].make_string("ERROR");
        (*res)[1].make_string(cmdName_.to_string());
        (*res)[2].make_string(descr);
    }

 protected:
    AttributeType cmdName_;
    AttributeType briefDescr_;
    AttributeType detailedDescr_;
    ITap *tap_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICOMMAND_H__
