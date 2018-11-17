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

#ifndef __DEBUGGER_COMMON_CODESERVICES_ICMDEXEC_H__
#define __DEBUGGER_COMMON_CODESERVICES_ICMDEXEC_H__

#include <iface.h>
#include <attribute.h>
#include "icommand.h"

namespace debugger {

static const char *IFACE_CMD_EXECUTOR = "ICmdExecutor";

class ICmdExecutor : public IFace {
 public:
    ICmdExecutor() : IFace(IFACE_CMD_EXECUTOR) {}

    /** Register command with ICommand interface */
    virtual void registerCommand(ICommand *icmd) = 0;
    virtual void unregisterCommand(ICommand *icmd) = 0;

    /** Execute string as a command */
    virtual void exec(const char *line, AttributeType *res, bool silent) = 0;

    /** Get list of supported comands starting with substring 'substr' */
    virtual void commands(const char *substr, AttributeType *res) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CODESERVICES_ICMDEXEC_H__
