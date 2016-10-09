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
#include "coreservices/ithread.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icmdexec.h"
#include "MainWindow/DbgMainWindow.h"


namespace debugger {

class GuiPlugin : public IService,
                  public IThread,
                  public IHap,
                  public IGui {
public:
    GuiPlugin(const char *name);
    ~GuiPlugin();

    /** IService interface */
    virtual void initService(const AttributeType *args);
    virtual void postinitService();
    virtual void predeleteService();

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    /** IGui interface */
    virtual IFace *getSocInfo();
    virtual void registerMainWindow(void *iwindow);
    virtual void registerWidgetInterface(IFace *iface);
    virtual void unregisterWidgetInterface(IFace *iface);
    virtual void registerCommand(IGuiCmdHandler *src, AttributeType *cmd, bool silent);

protected:
    /** IThread interface */
    virtual void busyLoop();
    virtual void stop();
    virtual void breakSignal();

private:
    bool processCmdQueue();

private:
    /**
     * This UiThread and UiInitDone event allow us to register all widgets
     * interfaces before PostInit stage started and as results make them 
     * visible to all other plugins.
     */
    class UiThreadType : public IThread {
    public:
        UiThreadType(IGui *igui, event_def *init_done) {
            igui_ = igui;
            eventInitDone_ = init_done;
        }
    protected:
        /** IThread interface */
        virtual void busyLoop();
    private:
        IGui *igui_;
        event_def *eventInitDone_;
    } *ui_;

    static const int CMD_QUEUE_SIZE = 128;

    AttributeType guiConfig_;
    AttributeType socInfo_;
    AttributeType cmdExecutor_;

    ISocInfo *info_;
    ICmdExecutor *iexec_;

    event_def eventUiInitDone_;
    event_def eventCommandAvailable_;
    mutex_def mutexCommand_;
    DbgMainWindow *mainWindow_;
    struct CmdQueueItemType {
        AttributeType cmd;
        IGuiCmdHandler *src;
        bool silent;
    } cmdQueue_[CMD_QUEUE_SIZE];
    int cmdQueueWrPos_;
    int cmdQueueRdPos_;
    int cmdQueueCntTotal_;
};

DECLARE_CLASS(GuiPlugin)

}  // namespace debugger

#endif  // __DEBUGGER_GUI_PLUGIN_H__