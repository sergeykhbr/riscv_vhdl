#include "DbgMainWindow.h"
#include "UnclosableQMdiSubWindow.h"
#include "UnclosableWidget.h"
#include "moc_DbgMainWindow.h"
#include "ControlWidget/PnpWidget.h"
#include "CpuWidgets/RegsViewWidget.h"
#include "CpuWidgets/MemViewWidget.h"
#include <QtWidgets/QtWidgets>


namespace debugger {

DbgMainWindow::DbgMainWindow(IGui *igui, event_def *init_done) {
    igui_ = igui;
    initDone_ = init_done;

    setWindowTitle(tr("RISC-V platform debugger"));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    listConsoleListeners_.make_list(0);
    /** Console commands used by main window: */
    cmdIsRunning_.make_string("isrunning");
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
     *
     * @todo Fix exception with PNP when initially opened 
     */
    addWidgets();
    
    setUnifiedTitleAndToolBarOnMac(true);

    /** 
     * To use the following type in SIGNAL -> SLOT definitions 
     * we have to register them using qRegisterMetaType template
     */
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<uint32_t>("uint32_t");

    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            this, SLOT(slotPostInit(AttributeType *)));
    connect(this, SIGNAL(signalExit()), this, SLOT(slotExit()));

    tmrGlobal_ = new QTimer(this);
    connect(tmrGlobal_, SIGNAL(timeout()), this, SLOT(slotConfigDone()));
    tmrGlobal_->setSingleShot(true);
    tmrGlobal_->setInterval(1);
    tmrGlobal_->start();
}

DbgMainWindow::~DbgMainWindow() {
}

void DbgMainWindow::closeEvent(QCloseEvent *ev) {
    igui_->waitQueueEmpty();
    tmrGlobal_->stop();
    ev->accept();
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

void DbgMainWindow::postInit(AttributeType *cfg) {
    emit signalPostInit(cfg);
}

void DbgMainWindow::getConfiguration(AttributeType &cfg) {
    cfg = config_;
}

void DbgMainWindow::callExit() {
    emit signalExit();
}

void DbgMainWindow::slotExit() {
    close();
}

void DbgMainWindow::createActions() {
    actionRegs_ = new QAction(QIcon(tr(":/images/cpu_96x96.png")),
                              tr("&Regs"), this);
    actionRegs_->setToolTip(tr("CPU Registers view"));
    actionRegs_->setShortcut(QKeySequence("Ctrl+r"));
    actionRegs_->setCheckable(true);
    actionRegs_->setChecked(false);

    actionMem_ = new QAction(QIcon(tr(":/images/mem_96x96.png")),
                              tr("&Memory"), this);
    actionMem_->setToolTip(tr("Memory view"));
    actionMem_->setShortcut(QKeySequence("Ctrl+m"));
    actionMem_->setCheckable(true);
    actionMem_->setChecked(false);

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
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            consoleWidget, SLOT(slotPostInit(AttributeType *)));
}

void DbgMainWindow::addWidgets() {
    UnclosableWidget *pnew;
    UnclosableQMdiSubWindow *subw;

    /** MDI Widgets: */
    actionSerial_->setChecked(true);
    subw = new UnclosableQMdiSubWindow(this);
    pnew = new UartWidget(igui_, this);
    subw->setUnclosableWidget("uart0", pnew, actionSerial_);
    subw->setMinimumWidth(size().width() / 2);
    mdiArea_->addSubWindow(subw);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            pnew, SLOT(slotPostInit(AttributeType *)));

    actionGpio_->setChecked(false);
    subw = new UnclosableQMdiSubWindow(this);
    pnew = new GpioWidget(igui_, this);
    subw->setUnclosableWidget("gpio0", pnew, actionGpio_);
    mdiArea_->addSubWindow(subw);
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            pnew, SLOT(slotPostInit(AttributeType *)));
    connect(this, SIGNAL(signalUpdateByTimer()),
            pnew, SLOT(slotUpdateByTimer()));

    actionRegs_->setChecked(false);
    subw = new UnclosableQMdiSubWindow(this);
    pnew = new RegsViewWidget(igui_, this);
    subw->setUnclosableWidget("Registers", pnew, actionRegs_);
    mdiArea_->addSubWindow(subw);
    connect(this, SIGNAL(signalUpdateByTimer()),
            pnew, SLOT(slotUpdateByTimer()));

    actionMem_->setChecked(false);
    subw = new UnclosableQMdiSubWindow(this);
    pnew = new MemViewWidget(igui_, this);
    subw->setUnclosableWidget("Memory", pnew, actionMem_);
    mdiArea_->addSubWindow(subw);
    connect(this, SIGNAL(signalUpdateByTimer()),
            pnew, SLOT(slotUpdateByTimer()));

    actionPnp_->setChecked(false);
    subw = new UnclosableQMdiSubWindow(this, true);
    pnew = new PnpWidget(igui_, this);
    subw->setUnclosableWidget("Plug'n'Play info", pnew, actionPnp_);
    mdiArea_->addSubWindow(subw);
}


void DbgMainWindow::slotPostInit(AttributeType *cfg) {
    config_ = *cfg;
    // Enable polling timer:
    connect(tmrGlobal_, SIGNAL(timeout()), this, SLOT(slotUpdateByTimer()));
    int ms = static_cast<int>(config_["PollingMs"].to_uint64());
    tmrGlobal_->setInterval(ms);
    tmrGlobal_->setSingleShot(false);
    tmrGlobal_->start(ms);
}

void DbgMainWindow::slotConfigDone() {
    RISCV_event_set(initDone_);
    disconnect(tmrGlobal_, SIGNAL(timeout()), this, SLOT(slotConfigDone()));
    tmrGlobal_->stop();
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

}  // namespace debugger

