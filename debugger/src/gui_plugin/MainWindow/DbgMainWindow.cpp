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

#include "DbgMainWindow.h"
#include "ControlWidget/PnpWidget.h"
#include "CpuWidgets/RegsViewWidget.h"
#include "CpuWidgets/AsmViewWidget.h"
#include "CpuWidgets/MemViewWidget.h"
#include "CpuWidgets/SymbolBrowserWidget.h"
#include "CpuWidgets/StackTraceWidget.h"
#include "CpuWidgets/CodeCoverage/CodeCoverageWidget.h"
#include "ControlWidget/ConsoleWidget.h"
#include "PeriphWidgets/UartWidget.h"
#include "PeriphWidgets/GpioWidget.h"
#include "PeriphWidgets/DemoM4Widget.h"
#include "GnssWidgets/MapWidget.h"
#include "GnssWidgets/PlotWidget.h"
#include <QtWidgets/QtWidgets>
#include <QtCore/QDate>
#include <QtGui/QScreen>

#ifdef WIN32
//#define GITHUB_SCREENSHOT_SIZE
#endif

namespace debugger {

DbgMainWindow::DbgMainWindow(IGui *igui) : QMainWindow() {
    igui_ = igui;
    requestedCmd_ = 0;
    simSecPrev_ = 0;
    realMSecPrev_ = QDateTime::currentMSecsSinceEpoch();

    setWindowIcon(QIcon(":/images/river_96x96.png"));
    setWindowTitle(tr("Universal platform simulator"));
#ifdef GITHUB_SCREENSHOT_SIZE
    resize(QSize(872, 600));
#else
    QScreen* screen = QGuiApplication::primaryScreen();
    resize(screen->availableGeometry().size() * 0.7);
#endif

    listConsoleListeners_.make_list(0);
    /** Console commands used by main window: */
    cmdStatus_.make_string("status");
    cmdRun_.make_string("c");
    cmdHalt_.make_string("halt");
    cmdStep_.make_string("c 1");
    cmdSteps_.make_string("reg insret");
 
    createActions();
    createMenus();
    createStatusBar();
    createMdiWindow();

    /** QT documeneted behaviour:
     *
     * If you add a child widget to an already visible widget 
     * you must explicitly show the child to make it visible.
     */
    addWidgets();
    
    setUnifiedTitleAndToolBarOnMac(true);

    /** 
     * To use the following type in SIGNAL -> SLOT definitions 
     * we have to register them using qRegisterMetaType template
     */
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<uint32_t>("uint32_t");

    tmrGlobal_ = new QTimer(this);
    connect(tmrGlobal_, SIGNAL(timeout()), this, SLOT(slotUpdateByTimer()));
    tmrGlobal_->setSingleShot(false);
    const AttributeType &cfg = *igui->getpConfig();
    stepToSecHz_ = cfg["StepToSecHz"].to_float();
    int t1 = cfg["PollingMs"].to_int();
    if (t1 < 10) {
        t1 = 10;
    }
    tmrGlobal_->setInterval(t1);
    tmrGlobal_->start();

    connect(this, SIGNAL(signalSimulationTime(double)),
                  SLOT(slotSimulationTime(double)));
}

DbgMainWindow::~DbgMainWindow() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void DbgMainWindow::closeEvent(QCloseEvent *ev) {
    tmrGlobal_->stop();
    delete tmrGlobal_;
    ev->accept();
    emit signalAboutToClose();
}

#ifndef QT_NO_CONTEXTMENU
void DbgMainWindow::contextMenuEvent(QContextMenuEvent *ev_) {
    QMenu menu(this);
    menu.addAction(actionRegs_);
    menu.exec(ev_->globalPos());
}
#endif

void DbgMainWindow::handleResponse(const char *cmd) {
    if (strcmp(cmd, cmdStatus_.to_string()) == 0) {
        requestedCmd_ &= ~0x1;
        if (respStatus_.is_nil()) {
            return;
        }
        uint64_t halted = respStatus_.to_uint64() & (1 << 0);       // hart select = 0
        if ((actionRun_->isChecked() && halted)
            || (!actionRun_->isChecked() && !halted)) {
            emit signalTargetStateChanged(halted == 0);
        }
    } else if (strcmp(cmd, cmdSteps_.to_string()) == 0) {
        if (respSteps_.is_dict()) {
            double tsec = static_cast<double>(
                respSteps_["insret"].to_uint64()) / stepToSecHz_;
            emit signalSimulationTime(tsec);
        } else {
            bool err = true;
        }
    }
}

void DbgMainWindow::createActions() {
    actionRegs_ = new QAction(QIcon(tr(":/images/cpu_96x96.png")),
                              tr("&Regs"), this);
    actionRegs_->setToolTip(tr("CPU Registers view"));
    actionRegs_->setShortcut(QKeySequence(tr("Ctrl+r")));
    actionRegs_->setCheckable(true);
    actionRegs_->setChecked(false);
    connect(actionRegs_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerRegs(bool)));

    actionCpuAsm_ = new QAction(QIcon(tr(":/images/asm_96x96.png")),
                              tr("&Memory"), this);
    actionCpuAsm_->setToolTip(tr("Disassembler view"));
    actionCpuAsm_->setShortcut(QKeySequence(tr("Ctrl+d")));
    actionCpuAsm_->setCheckable(true);
    connect(actionCpuAsm_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerCpuAsmView(bool)));

    actionStackTrace_ = new QAction(QIcon(tr(":/images/stack_96x96.png")),
                              tr("&Stack"), this);
    actionStackTrace_->setToolTip(tr("Stack trace"));
    actionStackTrace_->setShortcut(QKeySequence(tr("Ctrl+t")));
    actionStackTrace_->setCheckable(true);
    connect(actionStackTrace_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerStackTraceView(bool)));

    actionSymbolBrowser_ = new QAction(QIcon(tr(":/images/info_96x96.png")),
                              tr("&Symbols"), this);
    actionSymbolBrowser_->setToolTip(tr("Symbol Browser"));
    actionSymbolBrowser_->setShortcut(QKeySequence(tr("Ctrl+s")));
    actionSymbolBrowser_->setCheckable(false);
    connect(actionSymbolBrowser_, SIGNAL(triggered()),
            this, SLOT(slotActionTriggerSymbolBrowser()));

    actionMem_ = new QAction(QIcon(tr(":/images/mem_96x96.png")),
                              tr("&Memory"), this);
    actionMem_->setToolTip(tr("Memory view"));
    actionMem_->setShortcut(QKeySequence(tr("Ctrl+m")));
    actionMem_->setCheckable(true);
    actionMem_->setChecked(false);
    connect(actionMem_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerMemView(bool)));

    actionPnp_ = new QAction(QIcon(tr(":/images/board_96x96.png")),
                              tr("&Pnp"), this);
    actionPnp_->setToolTip(tr("Plug'n'play information view"));
    actionPnp_->setShortcut(QKeySequence(tr("Ctrl+p")));
    actionPnp_->setCheckable(true);
    actionPnp_->setChecked(false);
    connect(actionPnp_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerPnp(bool)));

    actionGpio_ = new QAction(QIcon(tr(":/images/gpio_96x96.png")),
                              tr("&GPIO"), this);
    actionGpio_->setToolTip(tr("GPIO control view"));
    actionGpio_->setCheckable(true);
    actionGpio_->setChecked(false);
    connect(actionGpio_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerGpio(bool)));

    actionSerial_ = new QAction(QIcon(tr(":/images/serial_96x96.png")),
                              tr("&Serial port"), this);
    actionSerial_->setToolTip(tr("Serial port console view"));
    actionSerial_->setCheckable(true);
    actionSerial_->setChecked(false);
    connect(actionSerial_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerUart0(bool)));

    actionDemoM4_ = new QAction(QIcon(tr(":/images/serial_96x96.png")),
                              tr("&ARM M4 display"), this);
    actionDemoM4_->setToolTip(tr("STM32L4xx ARM Demo"));
    actionDemoM4_->setCheckable(true);
    actionDemoM4_->setChecked(false);
    connect(actionDemoM4_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerDemoM4(bool)));

    actionGnssMap_ = new QAction(QIcon(tr(":/images/opmap_96x96.png")),
                              tr("&OpenStreetMap"), this);
    actionGnssMap_->setToolTip(tr("Open Street map with GNSS tracks view"));
    actionGnssMap_->setCheckable(true);
    actionGnssMap_->setChecked(false);
    connect(actionGnssMap_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerGnssMap(bool)));

    actionGnssPlot_ = new QAction(QIcon(tr(":/images/plot_96x96.png")),
                              tr("HW Statistic"), this);
    actionGnssPlot_->setToolTip(tr("Open HW statistic"));
    actionGnssPlot_->setCheckable(true);
    actionGnssPlot_->setChecked(false);
    connect(actionGnssPlot_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerGnssPlot(bool)));

    actionCodeCoverage_ = new QAction(QIcon(tr(":/images/coverage_96x96.png")),
                              tr("&Code Coverage"), this);
    actionCodeCoverage_->setToolTip(tr("Code Coverage Info"));
    actionCodeCoverage_->setCheckable(true);
    actionCodeCoverage_->setChecked(false);
    connect(actionCodeCoverage_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerCodeCoverage(bool)));

    actionRun_ = new QAction(QIcon(tr(":/images/start_96x96.png")),
                             tr("&Run"), this);
    actionRun_->setToolTip(tr("Start Execution (F5)"));
    actionRun_->setShortcut(QKeySequence(tr("F5")));
    actionRun_->setCheckable(true);
    actionRun_->setChecked(false);
    connect(actionRun_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetRun()));
    connect(this, SIGNAL(signalTargetStateChanged(bool)),
            actionRun_, SLOT(setChecked(bool)));

    actionHalt_ = new QAction(QIcon(tr(":/images/pause_96x96.png")),
                              tr("&Halt"), this);
    actionHalt_->setToolTip(tr("Stop Execution (Ctrl+Alt+F5)"));
    actionHalt_->setShortcut(QKeySequence(tr("Ctrl+Alt+F5")));
    connect(actionHalt_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetHalt()));

    actionStep_ = new QAction(QIcon(tr(":/images/stepinto_96x96.png")),
                              tr("&Step Into"), this);
    actionStep_->setToolTip(tr("Instruction Step (F11)"));
    actionStep_->setShortcut(QKeySequence(tr("F11")));
    connect(actionStep_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetStepInto()));


    actionQuit_ = new QAction(tr("&Quit"), this);
    actionQuit_->setShortcuts(QKeySequence::Quit);
    actionQuit_->setStatusTip(tr("Quit the application"));
    connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));

    actionAbout_ = new QAction(tr("&About"), this);
    actionAbout_->setStatusTip(tr("Show the application's About box"));
    connect(actionAbout_, SIGNAL(triggered()), this, SLOT(slotActionAbout()));

    QToolBar *toolbarRunControl = addToolBar(tr("toolbarRunControl"));
    toolbarRunControl->addAction(actionRun_);
    toolbarRunControl->addAction(actionHalt_);
    toolbarRunControl->addAction(actionStep_);

    QToolBar *toolbarCpu = addToolBar(tr("toolbarCpu"));
    toolbarCpu->addAction(actionCpuAsm_);
    toolbarCpu->addAction(actionRegs_);
    toolbarCpu->addAction(actionStackTrace_);
    toolbarCpu->addAction(actionMem_);
    QToolBar *toolbarPeriph = addToolBar(tr("toolbarPeriph"));
    toolbarPeriph->addAction(actionPnp_);
}

void DbgMainWindow::createMenus() {
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(actionQuit_);
    menu->addSeparator();
    
    menu = menuBar()->addMenu(tr("&Views"));
    menu->addAction(actionSerial_);
    menu->addAction(actionGpio_);
    menu->addAction(actionPnp_);
    menu->addSeparator();
    menu->addAction(actionRegs_);
    menu->addAction(actionSymbolBrowser_);
    menu->addAction(actionCodeCoverage_);
    menu->addAction(actionGnssPlot_);

    menu = menuBar()->addMenu(tr("&GNSS"));
    menu->addAction(actionGnssMap_);
    
    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(actionAbout_);
}


void DbgMainWindow::createStatusBar() {
    slotSimulationTime(0);
}

void DbgMainWindow::slotSimulationTime(double t) {
    QString simTime;
    uint64_t cur_sec = QDateTime::currentMSecsSinceEpoch();

    double slowdown = 0;
    double dt_sim = t - simSecPrev_;
    if (t != simSecPrev_ && dt_sim) {
        double dt_real = static_cast<double>(cur_sec - realMSecPrev_) / 1000.0;
        slowdown = dt_real / dt_sim;
    }
    realMSecPrev_ = cur_sec;
    simSecPrev_ = t;

    simTime = QString::asprintf("Simulation Time: %.1f seconds. Slowdown: %.1f",
                    t, slowdown);
    statusBar()->showMessage(simTime);
    requestedCmd_ &= ~0x2;
}

void DbgMainWindow::createMdiWindow() {
    AttributeType cfgMdi(Attr_Dict);
    cfgMdi["Tabbed"].make_boolean(false);
    mdiArea_ = new MdiAreaWidget(cfgMdi, this);
    setCentralWidget(mdiArea_);

    /** Docked Widgets: */
    QDockWidget *dock = new QDockWidget(tr("Debugger console"), this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    
    ConsoleWidget *consoleWidget = new ConsoleWidget(igui_, this);
    dock->setWidget(consoleWidget);

#ifdef GITHUB_SCREENSHOT_SIZE
    QScreen* screen = QGuiApplication::primaryScreen();
    double desired_h = screen->availableGeometry().size().height() * 0.07;
    consoleWidget->setFixedHeight(desired_h);
#endif
}

void DbgMainWindow::addWidgets() {
    const AttributeType &openList = (*igui_->getpConfig())["OpenViews"];

    for (unsigned i = 0; i < openList.size(); i++) {
        AttributeType &viewName = const_cast<AttributeType &>(openList[i]);
        if (viewName.is_equal("UartQMdiSubWindow")) {
            slotActionTriggerUart0(true);
        } else if (viewName.is_equal("AsmQMdiSubWindow")) {
            slotActionTriggerCpuAsmView(true);
        } else if (viewName.is_equal("RegsQMdiSubWindow")) {
            slotActionTriggerRegs(true);
        } else if (viewName.is_equal("MapQMdiSubWindow")) {
            slotActionTriggerGnssMap(true);
        } else if (viewName.is_equal("DemoM4Window")) {
            slotActionTriggerDemoM4(true);
        } else if (viewName.is_equal("CodeCoverageQMdiSubWindow")) {
            slotActionTriggerCodeCoverage(true);
        }
    }
}

void DbgMainWindow::slotActionTriggerUart0(bool val) {
    if (val) {
        viewUart0_ = 
            new UartQMdiSubWindow(igui_, mdiArea_, this, actionSerial_);
    } else {
        viewUart0_->close();
    }
}

void DbgMainWindow::slotActionTriggerCpuAsmView(bool val) {
    if (val) {
        viewCpuAsm_ = 
            new AsmQMdiSubWindow(igui_, mdiArea_, this, actionCpuAsm_);
    } else {
        viewCpuAsm_->close();
    }
}

void DbgMainWindow::slotActionTriggerRegs(bool val) {
    if (val) {
        viewRegs_ = new RegsQMdiSubWindow(igui_, mdiArea_, this,
                                        actionRegs_);
    } else {
        viewRegs_->close();
    }
}

void DbgMainWindow::slotActionTriggerStackTraceView(bool val) {
    if (val) {
        viewStackTrace_ = 
            new StackTraceQMdiSubWindow(igui_, mdiArea_, this, actionStackTrace_);
    } else {
        viewStackTrace_->close();
    }
}

void DbgMainWindow::slotActionTriggerMemView(bool val) {
    if (val) {
        viewMem_ = new MemQMdiSubWindow(igui_, mdiArea_, this,
                                        0xfffff000, 20, actionMem_);
    } else {
        viewMem_->close();
    }
}

void DbgMainWindow::slotActionTriggerGpio(bool val) {
    if (val) {
        viewGpio_ = new GpioQMdiSubWindow(igui_, mdiArea_, this,
                                        actionGpio_);
    } else {
        viewGpio_->close();
    }
}

void DbgMainWindow::slotActionTriggerPnp(bool val) {
    if (val) {
        viewPnp_ = new PnpQMdiSubWindow(igui_, mdiArea_, this,
                                        actionPnp_);
    } else {
        viewPnp_->close();
    }
}

void DbgMainWindow::slotActionTriggerGnssMap(bool val) {
    if (val) {
        viewGnssMap_ = 
            new MapQMdiSubWindow(igui_, mdiArea_, this, actionGnssMap_);
    } else {
        viewGnssMap_->close();
    }
}

void DbgMainWindow::slotActionTriggerGnssPlot(bool val) {
    if (val) {
        viewGnssPlot_ = 
            new PlotQMdiSubWindow(igui_, mdiArea_, this, actionGnssPlot_);
    } else {
        viewGnssPlot_->close();
    }
}

void DbgMainWindow::slotActionTriggerCodeCoverage(bool val) {
    if (val) {
        viewCodeCoverage_ =
            new CodeCoverageQMdiSubWindow(igui_, mdiArea_, this,
                                          actionCodeCoverage_);
    } else {
        viewCodeCoverage_->close();
    }
}

void DbgMainWindow::slotActionTriggerSymbolBrowser() {
    new SymbolBrowserQMdiSubWindow(igui_, mdiArea_, this);
}

void DbgMainWindow::slotActionTriggerDemoM4(bool val) {
    if (val) {
        viewDemoM4_ = 
            new DemoM4QMdiSubWindow(igui_, mdiArea_, this, actionDemoM4_);
    } else {
        viewDemoM4_->close();
    }
}

void DbgMainWindow::slotOpenDisasm(uint64_t addr, uint64_t sz) {
    new AsmQMdiSubWindow(igui_, mdiArea_, this, NULL, addr);
}

void DbgMainWindow::slotOpenMemory(uint64_t addr, uint64_t sz) {
    new MemQMdiSubWindow(igui_, mdiArea_, this, addr, sz);
}

void DbgMainWindow::slotUpdateByTimer() {
    if (!requestedCmd_) {
        requestedCmd_ = 0x3;
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               cmdStatus_.to_string(), &respStatus_, true);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               cmdSteps_.to_string(), &respSteps_, true);
    }
    emit signalUpdateByTimer();
}

void DbgMainWindow::slotActionAbout() {
    QString build;
    build = QString::asprintf("Version: 1.0\nBuild:     %s\n", __DATE__);
    build += tr("Author: Sergey Khabarov\n");
    build += tr("git:    http://github.com/sergeykhbr/riscv_vhdl\n");
    build += tr("e-mail: sergeykhbr@gmail.com\n\n\n"
       "RISC-V debugger GUI plugin is the open source extension of\n"
       "the base RISC-V debugger functionality providing friendly interface\n"
       "with running SoC target or with the Simulated platform.\n"
       "\n"
       "This extension doesn't use any specific for GUI scripts or commands\n"
       "and all shown information maybe achievied using debugger's console\n"
       "commands. See 'help' command.\n\n"
    );
    build += tr("www.gnss-sensor.com\n");

    QMessageBox::about(this, tr("About GUI plugin"), build);
}

void DbgMainWindow::slotActionTargetRun() {
    actionRun_->setChecked(true);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           cmdRun_.to_string(), &respRun_, true);
}

void DbgMainWindow::slotActionTargetHalt() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           cmdHalt_.to_string(), &respHalt_, true);
}

void DbgMainWindow::slotActionTargetStepInto() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           cmdStep_.to_string(), &respStep_, true);
}

/** Redraw all opened disassembler views */
void DbgMainWindow::slotBreakpointsChanged() {
    emit signalRedrawDisasm();
}

}  // namespace debugger

