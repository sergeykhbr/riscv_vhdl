#include "DbgMainWindow.h"
#include "UnclosableQMdiSubWindow.h"
#include "UnclosableWidget.h"
#include "moc_DbgMainWindow.h"
#include "ControlWidget/PnpWidget.h"
#include "CpuWidgets/RegsViewWidget.h"
#include <QtWidgets/QtWidgets>


namespace debugger {

DbgMainWindow::DbgMainWindow(IGui *igui, event_def *init_done) {
    igui_ = igui;
    initDone_ = init_done;
    igui_->registerMainWindow(this);

    setWindowTitle(tr("RISC-V platform debugger"));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    listConsoleListeners_.make_list(0);
    /** Console commands used by main window: */
    cmdIsRunning_.make_string("isrunning");
    cmdRun_.make_string("c");
    cmdHalt_.make_string("halt");
    cmdStep_.make_string("c 1");
    cmdExit_.make_string("exit");
 
    createActions();
    createMenus();
    createStatusBar();
    addWidgets();
    
    setUnifiedTitleAndToolBarOnMac(true);

    /** 
     * To use the following type in SIGNAL -> SLOT definitions 
     * we have to register them using qRegisterMetaType template
     */
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<uint32_t>("uint32_t");

    connect(this, SIGNAL(signalConfigDone()), this, SLOT(slotConfigDone()));
    connect(this, SIGNAL(signalExitForm()), this, SLOT(slotExitForm()));
    RISCV_event_set(initDone_);
}

DbgMainWindow::~DbgMainWindow() {
}

void DbgMainWindow::handleResponse(AttributeType *req, AttributeType *resp) {
    if (req->is_equal(cmdIsRunning_.to_string())) {
        bool isrun = resp->to_bool();
        if ((actionRun_->isChecked() && !isrun)
            || (!actionRun_->isChecked() && isrun)) {
            emit signalTargetStateChanged(isrun);
        }
    }
}

void DbgMainWindow::postInit(AttributeType cfg) {
    config_ = cfg;
    emit signalPostInit(&config_);
}

void DbgMainWindow::configDone() {
    emit signalConfigDone();
}

void DbgMainWindow::getConfiguration(AttributeType &cfg) {
    cfg = config_;
}

void DbgMainWindow::closeForm() {
    emit signalExitForm();
}

void DbgMainWindow::closeEvent(QCloseEvent *e) {
    emit signalClosingMainForm();

    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdExit_, true);
    e->accept();
}

void DbgMainWindow::createActions() {
    actionRegs_ = new QAction(QIcon(tr(":/images/cpu_96x96.png")),
                              tr("&Regs"), this);
    actionRegs_->setToolTip(tr("CPU Registers view"));
    actionRegs_->setShortcut(QKeySequence("Ctrl+r"));
    actionRegs_->setCheckable(true);
    actionRegs_->setChecked(false);

    actionPnp_ = new QAction(QIcon(tr(":/images/board_96x96.png")),
                              tr("&Pnp"), this);
    actionPnp_->setToolTip(tr("Plug'n'play information view"));
    actionPnp_->setShortcut(QKeySequence("Ctrl+p"));
    actionPnp_->setCheckable(true);
    actionPnp_->setChecked(false);

    actionGpio_ = new QAction(QIcon(tr(":/images/gpio_96x96.png")),
                              tr("&GPIO"), this);
    actionGpio_->setToolTip(tr("GPIO control view"));
    actionGpio_->setCheckable(true);
    actionGpio_->setChecked(false);


    actionSerial_ = new QAction(QIcon(tr(":/images/serial_96x96.png")),
                              tr("&Serial port"), this);
    actionSerial_->setToolTip(tr("Serial port console view"));
    actionSerial_->setCheckable(true);
    actionSerial_->setChecked(true);

    actionRun_ = new QAction(QIcon(tr(":/images/start_96x96.png")),
                             tr("&Run"), this);
    actionRun_ ->setToolTip(tr("Start Execution"));
    actionRun_ ->setShortcut(QKeySequence("F5"));
    actionRun_ ->setCheckable(true);
    actionRun_ ->setChecked(false);
    connect(actionRun_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetRun()));
    connect(this, SIGNAL(signalTargetStateChanged(bool)),
            actionRun_ , SLOT(setChecked(bool)));

    actionHalt_ = new QAction(QIcon(tr(":/images/pause_96x96.png")),
                              tr("&Halt"), this);
    actionHalt_ ->setToolTip(tr("Stop Execution"));
    actionHalt_ ->setShortcut(QKeySequence("Ctrl+b"));
    connect(actionHalt_ , SIGNAL(triggered()),
            this, SLOT(slotActionTargetHalt()));

    actionStep_ = new QAction(QIcon(tr(":/images/stepinto_96x96.png")),
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
    toolbarCpu->addAction(actionPnp_);
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
    UnclosableWidget *pnew_unclose;
    UnclosableQMdiSubWindow *subw;

    /** Docked Widgets: */
    QDockWidget *dock = new QDockWidget(tr("Debugger console"), this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    

    ConsoleWidget *consoleWidget = new ConsoleWidget(igui_, this);
    dock->setWidget(consoleWidget);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            consoleWidget, SLOT(slotPostInit(AttributeType *)));
    connect(this, SIGNAL(signalClosingMainForm()), 
            consoleWidget, SLOT(slotClosingMainForm()));


    /** MDI Widgets: */
    subw = new UnclosableQMdiSubWindow(this);
    subw->setWidget(pnew = new UartWidget(igui_, this));
    
    subw->setMinimumWidth(size().width() / 2);
    subw->setWindowIcon(actionSerial_->icon());
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            pnew, SLOT(slotPostInit(AttributeType *)));
    connect(actionSerial_, SIGNAL(triggered(bool)),
            subw, SLOT(slotVisible(bool)));
    connect(this, SIGNAL(signalClosingMainForm()), 
            pnew, SLOT(slotClosingMainForm()));
    actionSerial_->setChecked(true);
    subw->setVisible(actionSerial_->isChecked());


    subw = new UnclosableQMdiSubWindow(this);
    subw->setWidget(pnew = new GpioWidget(igui_, this));
    subw->setWindowIcon(actionGpio_->icon());
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            pnew, SLOT(slotPostInit(AttributeType *)));
    connect(this, SIGNAL(signalUpdateByTimer()),
            pnew, SLOT(slotUpdateByTimer()));
    connect(actionGpio_, SIGNAL(triggered(bool)),
            subw, SLOT(slotVisible(bool)));
    actionGpio_->setChecked(true);
    subw->setVisible(actionGpio_->isChecked());

    subw = new UnclosableQMdiSubWindow(this);
    subw->setUnclosableWidget(pnew_unclose = new RegsViewWidget(igui_, this));
    subw->setWindowIcon(actionRegs_->icon());
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
        pnew_unclose, SLOT(slotPostInit(AttributeType *)));
    connect(this, SIGNAL(signalUpdateByTimer()),
        pnew_unclose, SLOT(slotUpdateByTimer()));
    connect(actionRegs_, SIGNAL(triggered(bool)),
            subw, SLOT(slotVisible(bool)));
    connect(subw, SIGNAL(signalVisible(bool)), 
            actionRegs_, SLOT(setChecked(bool)));
    connect(this, SIGNAL(signalTargetStateChanged(bool)),
            pnew_unclose, SLOT(slotTargetStateChanged(bool)));
    actionRegs_->setChecked(false);
    subw->setVisible(actionRegs_->isChecked());

    subw = new UnclosableQMdiSubWindow(this);
    subw->setUnclosableWidget(pnew_unclose = new PnpWidget(igui_, this));
    subw->setWindowIcon(actionPnp_->icon());
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigDone()),
        pnew_unclose, SLOT(slotConfigDone()));
    connect(actionPnp_, SIGNAL(triggered(bool)),
            subw, SLOT(slotVisible(bool)));
    connect(subw, SIGNAL(signalVisible(bool)), 
            actionPnp_, SLOT(setChecked(bool)));
    actionPnp_->setChecked(false);
    subw->setVisible(actionPnp_->isChecked());
}

void DbgMainWindow::slotConfigDone() {
    // Enable polling timer:
    QTimer *timerPoll = new QTimer(this);
    connect(timerPoll, SIGNAL(timeout()), this, SLOT(slotUpdateByTimer()));

    int ms = static_cast<int>(config_["PollingMs"].to_uint64());
    timerPoll->setInterval(ms);
    timerPoll->setSingleShot(false);
    timerPoll->start();
}

void DbgMainWindow::slotUpdateByTimer() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           &cmdIsRunning_, true);
    emit signalUpdateByTimer();
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

void DbgMainWindow::slotExitForm() {
    close();
}

}  // namespace debugger

