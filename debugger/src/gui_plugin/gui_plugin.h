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

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "igui.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/icmdexec.h"
#include "MainWindow/DbgMainWindow.h"
#include "qt_wrapper.h"

namespace debugger {

class GuiPlugin : public IService,
                  public IThread,
                  public IHap,
                  public IGui {
public:
    GuiPlugin(const char *name);
    ~GuiPlugin();

    /** IService interface */
    virtual void postinitService();

    /** IHap */
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

    /** IGui interface */
    virtual IService *getParentService();
    virtual const AttributeType *getpConfig();
    virtual void registerCommand(IGuiCmdHandler *iface,
                                 const char *cmd, AttributeType *resp,
                                 bool silent);
    virtual void removeFromQueue(IFace *iface);
    virtual void externalCommand(AttributeType *req);
    virtual void *getQGui() { return ui_; }

    /** IThread interface */
    virtual void stop();
protected:
    virtual void busyLoop();

private:
    bool processCmdQueue();

private:
    static const int CMD_QUEUE_SIZE = 256;

    AttributeType guiConfig_;
    AttributeType cmdexec_;

    ICmdExecutor *iexec_;
    QtWrapper *ui_;

    event_def config_done_;

    char cmdbuf_[1024*1024];
    char *pcmdwr_;
    struct CmdType {
        bool silent;
        const char *req;
        IGuiCmdHandler *iface;
        AttributeType *resp;
    } cmds_[CMD_QUEUE_SIZE];
    uint8_t cmdwrcnt_;
    uint8_t cmdrdcnt_;
};

DECLARE_CLASS(GuiPlugin)

}  // namespace debugger
