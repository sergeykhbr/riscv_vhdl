/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debugger Main Window form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "igui.h"
#include "coreservices/isocinfo.h"
#include "ebreakhandler.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include "MdiAreaWidget.h"

namespace debugger {

class DbgMainWindow : public QMainWindow,
                      public IGuiCmdHandler {
    Q_OBJECT

public:
    DbgMainWindow(IGui *igui);
    virtual ~DbgMainWindow();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalUpdateByTimer();
    void signalTargetStateChanged(bool);
    void signalRedrawDisasm();
    void signalAboutToClose();

protected:
    virtual void closeEvent(QCloseEvent *ev_);
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *ev_) override;
#endif // QT_NO_CONTEXTMENU

private slots:
    void slotUpdateByTimer();
    void slotActionAbout();
    void slotActionTargetRun();
    void slotActionTargetHalt();
    void slotActionTargetStepInto();
    void slotActionTriggerUart0(bool val);
    void slotActionTriggerRegs(bool val);
    void slotActionTriggerCpuAsmView(bool val);
    void slotActionTriggerStackTraceView(bool val);
    void slotActionTriggerMemView(bool val);
    void slotActionTriggerGpio(bool val);
    void slotActionTriggerPnp(bool val);
    void slotActionTriggerGnssMap(bool val);
    void slotActionTriggerSymbolBrowser();
    void slotOpenDisasm(uint64_t addr, uint64_t sz);
    void slotOpenMemory(uint64_t addr, uint64_t sz);
    void slotBreakpointsChanged();

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void createMdiWindow();
    void addWidgets();

private:
    QAction *actionAbout_;
    QAction *actionQuit_;
    QAction *actionRun_;
    QAction *actionHalt_;
    QAction *actionStep_;
    QAction *actionSymbolBrowser_;
    QAction *actionRegs_;
    QMdiSubWindow *viewRegs_;
    QAction *actionCpuAsm_;
    QMdiSubWindow *viewCpuAsm_;
    QAction *actionStackTrace_;
    QMdiSubWindow *viewStackTrace_;
    QAction *actionMem_;
    QMdiSubWindow *viewMem_;
    QAction *actionGpio_;
    QMdiSubWindow *viewGpio_;
    QAction *actionPnp_;
    QMdiSubWindow *viewPnp_;
    QAction *actionSerial_;
    QMdiSubWindow *viewUart0_;
    QAction *actionGnssMap_;
    QMdiSubWindow *viewGnssMap_;
    QTimer *tmrGlobal_;
    MdiAreaWidget *mdiArea_;
    
    AttributeType config_;
    AttributeType listConsoleListeners_;
    AttributeType cmdStatus_;
    AttributeType cmdRun_;
    AttributeType cmdHalt_;
    AttributeType cmdStep_;

    IGui *igui_;
    //event_def *initDone_;
    bool statusRequested_;
    EBreakHandler *ebreak_;
};

}  // namespace debugger
