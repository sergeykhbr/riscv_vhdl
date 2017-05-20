#include "DbgMainWindow.h"
#include "moc_DbgMainWindow.h"
#include "ControlWidget/PnpWidget.h"
#include "CpuWidgets/RegsViewWidget.h"
#include "CpuWidgets/AsmViewWidget.h"
#include "CpuWidgets/MemViewWidget.h"
#include "CpuWidgets/SymbolBrowserWidget.h"
#include "CpuWidgets/StackTraceWidget.h"
#include "ControlWidget/ConsoleWidget.h"
#include "PeriphWidgets/UartWidget.h"
#include "PeriphWidgets/GpioWidget.h"
#include "GnssWidgets/MapWidget.h"
#include <QtWidgets/QtWidgets>

namespace debugger {

DbgMainWindow::DbgMainWindow(IGui *igui) : QMainWindow() {
    igui_ = igui;
    //initDone_ = init_done;
    statusRequested_ = false;
    ebreak_ = 0;

    setWindowTitle(tr("RISC-V platform debugger"));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    listConsoleListeners_.make_list(0);
    /** Console commands used by main window: */
    cmdStatus_.make_string("status");
    cmdRun_.make_string("c");
    cmdHalt_.make_string("halt");
    cmdStep_.make_string("c 1");
 
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

    ISocInfo *isoc = static_cast<ISocInfo *>(igui_->getSocInfo());
    if (isoc) {
        DsuMapType *dsu = isoc->getpDsu();
        ebreak_ = new EBreakHandler(igui_);

        ebreak_->setBrAddressFetch(
            reinterpret_cast<uint64_t>(&dsu->udbg.v.br_address_fetch));
        ebreak_->setHwRemoveBreakpoint(
            reinterpret_cast<uint64_t>(&dsu->udbg.v.remove_breakpoint));
    }

    /** 
     * To use the following type in SIGNAL -> SLOT definitions 
     * we have to register them using qRegisterMetaType template
     */
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<uint32_t>("uint32_t");
}

DbgMainWindow::~DbgMainWindow() {
    if (ebreak_) {
        delete ebreak_;
    }
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void DbgMainWindow::closeEvent(QCloseEvent *ev) {
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

void DbgMainWindow::handleResponse(AttributeType *req, AttributeType *resp) {
    if (req->is_equal(cmdStatus_.to_string())) {
        statusRequested_ = false;
        if (resp->is_nil()) {
            return;
        }
        DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
        ctrl.val = resp->to_uint64();
        if ((actionRun_->isChecked() && ctrl.bits.halt)
            || (!actionRun_->isChecked() && !ctrl.bits.halt)) {
            emit signalTargetStateChanged(ctrl.bits.halt == 0);
        }
        if (ctrl.bits.breakpoint && ebreak_) {
            ebreak_->skip();
        }
#if 0
        static const char *xSTATES[] = {"Idle", "WaitGrant", "WaitResp", "WaitAccept"};
        static const char *CSTATES[] = {"Idle", "IMem", "DMem"};
        RISCV_printf(0, 0, "istate=%s dstate=%s; cstate=%s",
          xSTATES[ctrl.bits.istate],
          xSTATES[ctrl.bits.dstate],
          CSTATES[ctrl.bits.cstate]);
#endif
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

    actionGnssMap_ = new QAction(QIcon(tr(":/images/opmap_96x96.png")),
                              tr("&OpenStreetMap"), this);
    actionGnssMap_->setToolTip(tr("Open Street map with GNSS tracks view"));
    actionGnssMap_->setCheckable(true);
    actionGnssMap_->setChecked(false);
    connect(actionGnssMap_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionTriggerGnssMap(bool)));

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

    menu = menuBar()->addMenu(tr("&GNSS"));
    menu->addAction(actionGnssMap_);
    
    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(actionAbout_);
}


void DbgMainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
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
}

void DbgMainWindow::addWidgets() {
    slotActionTriggerUart0(true);
    slotActionTriggerCpuAsmView(true);

    slotActionTriggerGnssMap(true);
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


void DbgMainWindow::slotActionTriggerSymbolBrowser() {
    new SymbolBrowserQMdiSubWindow(igui_, mdiArea_, this);
}

void DbgMainWindow::slotOpenDisasm(uint64_t addr, uint64_t sz) {
    new AsmQMdiSubWindow(igui_, mdiArea_, this, NULL, addr);
}

void DbgMainWindow::slotOpenMemory(uint64_t addr, uint64_t sz) {
    new MemQMdiSubWindow(igui_, mdiArea_, this, addr, sz);
}

//void DbgMainWindow::slotPostInit(AttributeType *cfg) {
//    config_ = *cfg;
    // Enable polling timer:
    //connect(tmrGlobal_, SIGNAL(timeout()), this, SLOT(slotUpdateByTimer()));
    //int ms = static_cast<int>(config_["PollingMs"].to_uint64());
    //tmrGlobal_->setInterval(ms);
    //tmrGlobal_->setSingleShot(false);
    //tmrGlobal_->start(ms);


    // Debug:
    //slotActionTriggerGnssMap(true);
//}

void DbgMainWindow::slotUpdateByTimer() {
    if (!statusRequested_) {
        statusRequested_ = true;
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               &cmdStatus_, true);
    }
    emit signalUpdateByTimer();
}

void DbgMainWindow::slotActionAbout() {
    char date[128];
    memcpy(date, __DATE__, sizeof(date));
    QString build;
    build.sprintf("Version: 1.0\nBuild:     %s\n", date);
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
                           &cmdRun_, true);
}

void DbgMainWindow::slotActionTargetHalt() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdHalt_, true);
}

void DbgMainWindow::slotActionTargetStepInto() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdStep_, true);
}

/** Redraw all opened disassembler views */
void DbgMainWindow::slotBreakpointsChanged() {
    emit signalRedrawDisasm();
}

}  // namespace debugger

