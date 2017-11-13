/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GUI for the RISC-V debugger.
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