/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debugger Main Window form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "igui.h"

#include <QtWidgets/QMainWindow>
#include "MdiAreaWidget.h"
#include "ControlWidget/ConsoleWidget.h"
#include "PeriphWidgets/UartWidget.h"
#include "PeriphWidgets/GpioWidget.h"


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

namespace debugger {

class DbgMainWindow : public QMainWindow,
                      public IGuiCmdHandler {
    Q_OBJECT

public:
    DbgMainWindow(IGui *igui, event_def *init_done);
    virtual ~DbgMainWindow();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

    /** Global methods */
    void setConfiguration(AttributeType cfg);
    void getConfiguration(AttributeType &cfg);

signals:
    void signalConfigure(AttributeType *cfg);
    void signalRedrawByTimer();
    void signalClosingMainForm();
    void signalTargetStateChanged(bool);

private slots:
    void slotTimerRedraw();
    void slotTimerPollingStatus();
    void slotActionAbout();
    void slotActionTargetRun();
    void slotActionTargetHalt();
    void slotActionTargetStepInto();

protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void addWidgets();

private:
    QTimer *timerRedraw_;
    QTimer *timerPollStatus_;

    QAction *actionAbout_;
    QAction *actionQuit_;
    QAction *actionRun_;
    QAction *actionHalt_;
    QAction *actionStep_;
    QAction *actionRegs_;
    QAction *actionGpio_;
    QAction *actionPnp_;
    QAction *actionSerial_;
    
    AttributeType config_;
    AttributeType listConsoleListeners_;
    AttributeType cmdReadStatus_;

    IGui *igui_;
    event_def *initDone_;
    bool riseSyncEvent_;
};

}  // namespace debugger
