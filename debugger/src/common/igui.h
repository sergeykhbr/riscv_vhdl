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

#ifndef __DEBUGGER_IGUI_H__
#define __DEBUGGER_IGUI_H__

#include "iface.h"
#include "attribute.h"
#include "iservice.h"

namespace debugger {

static const char *const IFACE_GUI_PLUGIN = "IGui";
static const char *const IFACE_GUI_CMD_HANDLER = "IGuiCmdHandler";

class IGuiCmdHandler : public IFace {
 public:
    IGuiCmdHandler() : IFace(IFACE_GUI_CMD_HANDLER) {}

    virtual void handleResponse(const char *cmd) = 0;
};

class IGui : public IFace {
 public:
    IGui() : IFace(IFACE_GUI_PLUGIN) {}

    virtual IService *getParentService() = 0;

    virtual const AttributeType *getpConfig() = 0;

    virtual void registerCommand(IGuiCmdHandler *iface,
                                const char *cmd, AttributeType *resp,
                                bool silent) = 0;
    virtual void removeFromQueue(IFace *iface) = 0;

    // External events:
    virtual void externalCommand(AttributeType *req) = 0;
    virtual void *getQGui() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IGUI_H__
