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
                  public IGui {
public:
    GuiPlugin(const char *name);
    ~GuiPlugin();

    /** IService interface */
    virtual void initService(const AttributeType *args);
    virtual void postinitService();
    virtual void predeleteService();

    /** IGui interface */
    virtual IFace *getSocInfo();
    virtual void registerCommand(IGuiCmdHandler *src, AttributeType *cmd, bool silent);
    virtual void waitQueueEmpty();

    /** IThread interface */
    virtual void stop();
protected:
    virtual void busyLoop();

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
            mainWindow_ = 0;
        }
        DbgMainWindow *mainWindow() { return mainWindow_; }
    protected:
        /** IThread interface */
        virtual void busyLoop();
    private:
        IGui *igui_;
        DbgMainWindow *mainWindow_;
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
    event_def eventCmdQueueEmpty_;
    mutex_def mutexCommand_;
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