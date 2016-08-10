#include "DbgMainWindow.h"
#include "UnclosableQMdiSubWindow.h"
#include "moc_DbgMainWindow.h"
#include "CpuWidgets/RegsViewWidget.h"
#include <QtWidgets/QtWidgets>


namespace debugger {

static const int REDRAW_TIMER_MSEC = 10;
#define IS_HALTED(dsu_control) ((dsu_control & 0x1ull) == 1)

DbgMainWindow::DbgMainWindow(IGui *igui, event_def *init_done) {
    igui_ = igui;
    initDone_ = init_done;
    igui_->registerMainWindow(this);

    setWindowTitle(tr("RISC-V platform debugger"));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    listConsoleListeners_.make_list(0);
    /** DSU control region 0x80080000 + 0x10000: 
     *              offset 0x0000 = Control register
     */
    cmdReadStatus_.from_config("['read',0x80090000,4]");

    createActions();
    createMenus();
    createStatusBar();
    addWidgets();
    
    setUnifiedTitleAndToolBarOnMac(true);

    riseSyncEvent_ = true;
    timerRedraw_ = new QTimer(this);
    timerRedraw_->setSingleShot(false);
    connect(timerRedraw_, SIGNAL(timeout()), this, SLOT(slotTimerRedraw()));
    timerRedraw_->start(REDRAW_TIMER_MSEC);

    //timerPollStatus_ = new QTimer(this);
    //timerPollStatus_->setSingleShot(false);
    //connect(timerPollStatus_, SIGNAL(timeout()), 
    //        this, SLOT(slotTimerPollingStatus()));

    QTimer *tmr3 = new QTimer(this);
    connect(tmr3, SIGNAL(timeout()), 
            this, SLOT(slotTimerPollingStatus()));
    tmr3->setSingleShot(false);
    tmr3->start(500);

}

DbgMainWindow::~DbgMainWindow() {
}

void DbgMainWindow::handleResponse(AttributeType *req, AttributeType *resp) {
    if (!req->is_list() || req->size() != 3) {
        return;
    }
    
    uint64_t addr = (*req)[1].to_uint64();
    uint64_t val = 0;
    switch (addr) {
    case 0x80090000ull:
        /** Bit[0] = Halt state when is High */
        val = resp->to_uint64();
        if (actionRun_->isChecked() && IS_HALTED(val)
            || !actionRun_->isChecked() && !IS_HALTED(val)) {
            emit signalTargetStateChanged(val == 0);
        }
        break;
    default:;
    }
}

void DbgMainWindow::setConfiguration(AttributeType cfg) {
    int ms = 0;
    config_ = cfg;

    for (unsigned i = 0; i < cfg.size(); i++) {
        const AttributeType &attr = cfg[i];
        if (strcmp(attr[0u].to_string(), "PollingMs") == 0) {
            ms = static_cast<int>(attr[1].to_uint64());
        }
    }

    emit signalConfigure(&config_);

    if (ms) {
        //timerPollStatus_->start(ms);
    }
}

void DbgMainWindow::getConfiguration(AttributeType &cfg) {
    cfg = config_;
}


void DbgMainWindow::closeEvent(QCloseEvent *e) {
    timerRedraw_->stop();
    //timerPollStatus_->stop();

    emit signalClosingMainForm();
    AttributeType exit_cmd;
    exit_cmd.from_config("['exit']");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &exit_cmd);
    e->accept();
}

void DbgMainWindow::createActions() {
    actionRegs_ = new QAction(QIcon(tr(":/images/toggle.png")),
                              tr("&Regs"), this);
    actionRegs_->setToolTip(tr("CPU Registers view"));
    actionRegs_->setShortcut(QKeySequence("Ctrl+r"));
    actionRegs_->setCheckable(true);
    actionRegs_->setChecked(false);

    actionRun_ = new QAction(QIcon(tr(":/images/toggle.png")),
                             tr("&Run"), this);
    actionRun_ ->setToolTip(tr("Start Execution"));
    actionRun_ ->setShortcut(QKeySequence("F5"));
    actionRun_ ->setCheckable(true);
    actionRun_ ->setChecked(false);
    connect(actionRun_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetRun()));
    connect(this, SIGNAL(signalTargetStateChanged(bool)),
            actionRun_ , SLOT(setChecked(bool)));

    actionHalt_ = new QAction(QIcon(tr(":/images/toggle.png")),
                              tr("&Halt"), this);
    actionHalt_ ->setToolTip(tr("Stop Execution"));
    actionHalt_ ->setShortcut(QKeySequence("Ctrl+b"));
    connect(actionHalt_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetHalt()));

    actionStep_ = new QAction(QIcon(tr(":/images/toggle.png")),
                              tr("&Step Into"), this);
    actionStep_ ->setToolTip(tr("Instruction Step"));
    actionStep_ ->setShortcut(QKeySequence("F11"));
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
    toolbarCpu->addAction(actionRegs_);
}

void DbgMainWindow::createMenus() {
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(actionQuit_);
    menu->addSeparator();
    
    menu = menuBar()->addMenu(tr("&Views"));
    menu->addAction(actionRegs_);
    
    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(actionAbout_);
}


void DbgMainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void DbgMainWindow::addWidgets() {
    AttributeType cfgMdi(Attr_Dict);
    cfgMdi["Tabbed"].make_boolean(false);
    MdiAreaWidget *mdiArea = new MdiAreaWidget(cfgMdi, this);
    setCentralWidget(mdiArea);


    QWidget *pnew;
    UnclosableQMdiSubWindow *subw;

    /** Docked Widgets: */
    QDockWidget *dock = new QDockWidget(tr("Debugger console"), this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    

    ConsoleWidget *consoleWidget = new ConsoleWidget(igui_, this);
    dock->setWidget(consoleWidget);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            consoleWidget, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
            consoleWidget, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            consoleWidget, SLOT(slotClosingMainForm()));


    /** MDI Widgets: */
    subw = new UnclosableQMdiSubWindow(this);
    subw->setWidget(pnew = new UartWidget(igui_, this));
    subw->setMinimumWidth(size().width() / 2);
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            pnew, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
            pnew, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            pnew, SLOT(slotClosingMainForm()));

    subw = new UnclosableQMdiSubWindow(this);
    subw->setWidget(pnew = new GpioWidget(igui_, this));
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            pnew, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
              pnew, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            pnew, SLOT(slotClosingMainForm()));

    subw = new UnclosableQMdiSubWindow(this);
    subw->setWidget(pnew = new RegsViewWidget(igui_, this));
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigure(AttributeType *)),
        pnew, SLOT(slotConfigure(AttributeType *)));
    connect(actionRegs_, SIGNAL(triggered(bool)),
            subw, SLOT(slotVisible(bool)));
    connect(subw, SIGNAL(signalVisible(bool)), 
            actionRegs_, SLOT(setChecked(bool)));
    connect(this, SIGNAL(signalTargetStateChanged(bool)),
            pnew, SLOT(slotTargetStateChanged(bool)));

   
    subw->setVisible(false);
}

void DbgMainWindow::slotTimerRedraw() {
    if (riseSyncEvent_) {
        RISCV_event_set(initDone_);
        riseSyncEvent_ = false;
    }
    emit signalRedrawByTimer();
}

void DbgMainWindow::slotTimerPollingStatus() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdReadStatus_);
}

void DbgMainWindow::slotActionAbout() {
    char date[128];
    memcpy(date, __DATE__, sizeof(date));
    QString build;
    build.sprintf("Version: 1.0\nBuild:     %s\n", date);
    build += tr("Author: Sergey Khabarov\n");
    build += tr("git:    http://github.com/sergeykhbr/riscv_vhdl\n");
    build += tr("e-mail: sergeykhbr@gmail.com\n\n")
       + tr("\tRISC-V debugger GUI plugin is the open source application\n")
       + tr("that upgrades the base RISC-V debugger funcitonality and can\n")
       + tr("provide more friendly interaction interface with SoC running\n")
       + tr("on FPGA or on simulator:\n\n");
    build += tr("www.gnss-sensor.com\n");

    QMessageBox::about(this, tr("About GUI plugin"),
        build
        /*tr("\tThe <b>GTerm</b> application allows to interact with the "
               "GNSS receiver via a serial port or ethernet.\n"
               "\tSet of plugins allow to view and analize binary data stream in "
               "convenient user-friendly ways.")*/
               );
}

void DbgMainWindow::slotActionTargetRun() {
    /** DSU control region 0x80080000 + 0x10000: 
     *              offset 0x0000 = Control register
     *  Bits[1:0]
     *      00 = Go
     *      01 = Halt
     *      02 = Stepping enable
     */
    AttributeType cmdRun;
    cmdRun.from_config("['write',0x80090000,4,0]");

    actionRun_->setChecked(true);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdRun);
}

void DbgMainWindow::slotActionTargetHalt() {
    /** DSU control region 0x80080000 + 0x10000: 
     *              offset 0x0000 = Control register
     *  Bits[1:0]
     *      00 = Go
     *      01 = Halt
     *      02 = Stepping enable
     */
    AttributeType cmdHalt;
    cmdHalt.from_config("['write',0x80090000,4,0x1]");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdHalt);
}

void DbgMainWindow::slotActionTargetStepInto() {
    /** DSU control region 0x80080000 + 0x10000: 
     *              offset 0x0008 = StepNumber register
     *              offset 0x0000 = Control register
     *  Bits[1:0]
     *      00 = Go
     *      01 = Halt
     *      02 = Stepping enable
     */
    AttributeType cmdStep, cmdStepValue;
    cmdStepValue.from_config("['write',0x80090008,8,1]");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdStepValue);
    cmdStep.from_config("['write',0x80090000,4,2]");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdStep);
}

}  // namespace debugger

