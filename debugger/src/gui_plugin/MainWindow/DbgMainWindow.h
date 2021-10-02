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

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "igui.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include "MdiAreaWidget.h"

namespace debugger {

class DbgMainWindow : public QMainWindow,
                      public IGuiCmdHandler {
    Q_OBJECT

 public:
    DbgMainWindow(IGui *igui);
    virtual ~DbgMainWindow();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 signals:
    void signalUpdateByTimer();
    void signalTargetStateChanged(bool);
    void signalRedrawDisasm();
    void signalAboutToClose();
    void signalSimulationTime(double t);

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
    void slotActionTriggerGnssPlot(bool val);
    void slotActionTriggerCodeCoverage(bool val);
    void slotActionTriggerSymbolBrowser();
    void slotActionTriggerDemoM4(bool val);
    void slotOpenDisasm(uint64_t addr, uint64_t sz);
    void slotOpenMemory(uint64_t addr, uint64_t sz);
    void slotBreakpointsChanged();
    void slotSimulationTime(double t);

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
    QAction *actionGnssPlot_;
    QMdiSubWindow *viewGnssPlot_;
    QAction *actionCodeCoverage_;
    QMdiSubWindow *viewCodeCoverage_;
    QAction *actionDemoM4_;
    QMdiSubWindow *viewDemoM4_;
    QTimer *tmrGlobal_;
    MdiAreaWidget *mdiArea_;
    
    AttributeType config_;
    AttributeType listConsoleListeners_;
    AttributeType cmdStatus_;
    AttributeType respStatus_;
    AttributeType cmdRun_;
    AttributeType respRun_;
    AttributeType cmdHalt_;
    AttributeType respHalt_;
    AttributeType cmdStep_;
    AttributeType respStep_;
    AttributeType cmdSteps_;
    AttributeType respSteps_;

    IGui *igui_;
    int requestedCmd_;
    double stepToSecHz_;
    double simSecPrev_;
    uint64_t realMSecPrev_;
};

}  // namespace debugger
