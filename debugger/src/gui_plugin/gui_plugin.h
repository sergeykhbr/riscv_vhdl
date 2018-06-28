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

#ifndef __DEBUGGER_GUI_PLUGIN_H__
#define __DEBUGGER_GUI_PLUGIN_H__

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "igui.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/isocinfo.h"
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
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    /** IGui interface */
    virtual IService *getParentService();
    virtual IFace *getSocInfo();
    virtual const AttributeType *getpConfig();
    virtual void registerCommand(IGuiCmdHandler *src, AttributeType *cmd,
                                 bool silent);
    virtual void removeFromQueue(IFace *iface);

    /** IThread interface */
    virtual void stop();
protected:
    virtual void busyLoop();

private:
    bool processCmdQueue();

private:
    static const int CMD_QUEUE_SIZE = 128;

    AttributeType guiConfig_;
    AttributeType socInfo_;
    AttributeType cmdExecutor_;

    ISocInfo *info_;
    ICmdExecutor *iexec_;
    QtWrapper *ui_;

    GuiAsyncTQueueType queue_;

    event_def eventCommandAvailable_;
    event_def config_done_;
};

DECLARE_CLASS(GuiPlugin)

}  // namespace debugger

#endif  // __DEBUGGER_GUI_PLUGIN_H__